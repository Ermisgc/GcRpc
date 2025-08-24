#include "Rpc/connection_pool.h"
#include "Rpc/channel.h"
#include "Rpc/rpcprovider.h"

namespace GcRpc{
    TcpConnection::TcpConnection(int socket, EventLoop * event_loop):_sockfd(socket), _event_loop(event_loop) {
        //一开始TcpConnection是空的，现在要配置这个TcpConnection，这个TcpConnection应该只有空Channel
        _channel = std::make_unique<ChannelV2>();
        _active = false;
    }

    TcpConnection::~TcpConnection(){

    }

    bool TcpConnection::connect(const InetAddr && addr){
        //客户端调用connect是一个同步调用
        auto ret = ::connect(_sockfd, addr.addr(), sizeof(sockaddr_in));
        if(ret == 0) {
            //成功调用
            _active = true;
            _channel->sock_addr = addr.m_addr; 
            return true;
        } 
        return false;
    }

    void TcpConnection::do_io_uring(int last_res, int last_event_type){
        //io_uring状态机，根据上一步的状态确定下一步做什么
        switch (last_event_type)
        {
        case USER_EVENT_CLOSE:
            callback_closed(last_res);
            break;

        case USER_EVENT_ACCEPT:
            callback_accepted(last_res);
            break;
        
        case USER_EVENT_READ:
            callback_read(last_res);         
            break;
            
        case USER_EVENT_HANDLE:
            break;
        
        case USER_EVENT_WRITE_THEN_CLOSE:
            callback_written_then_close(last_res);
            break;
        case USER_EVENT_WRITE:
            callback_written(last_res);
            break;

        default:  
            std::cerr << "Invalid event status" << std::endl;          
            break;
        }
    }

    bool TcpConnection::disconnect(){
        return ::close(_clientfd) == 0;
    }

    bool TcpConnection::disconnect_async(){
        if(!_event_loop) return false;
        auto sqe = _event_loop->getOneSqe();
        sqe->user_data = getIOUringUserdata(this, USER_EVENT_CLOSE);
        io_uring_prep_close(sqe, _clientfd);
        return true;        
    }

    bool TcpConnection::is_connected() const{
        return _active;
    }

    bool TcpConnection::accept(){
        auto sqe = _event_loop->getOneSqe();
        sqe->user_data = getIOUringUserdata(this, USER_EVENT_ACCEPT);
        io_uring_prep_accept(sqe, _sockfd, (sockaddr *)&_channel->sock_addr, &_channel->len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        return true;
    }

    bool TcpConnection::send(const std::string & to_be_send){
        int ret = _channel->write_buffer->append(to_be_send);
        if(ret != to_be_send.size()){
            std::cerr << "Buffer doesn't have enough space to append" << std::endl;
        }
        _to_send_bytes.fetch_add(ret);

        auto sqe = _event_loop->getOneSqe();
        sqe->user_data = getIOUringUserdata(this, USER_EVENT_WRITE);
        io_uring_prep_writev(sqe, _clientfd, _channel->write_buffer->readvBegin(), 2, 0);

        return true;
    }

    void TcpConnection::setFd(int clientfd){
        _clientfd = clientfd;
    }

    void TcpConnection::sendThenClose(const std::string & to_be_send){
        int ret = _channel->write_buffer->append(to_be_send);
        if(ret != to_be_send.size()){
            std::cerr << "Buffer doesn't have enough space to append" << std::endl;
        }

        auto sqe = _event_loop->getOneSqe();
        sqe->user_data = getIOUringUserdata(this, USER_EVENT_WRITE_THEN_CLOSE);
        io_uring_prep_writev(sqe, _clientfd, _channel->write_buffer->readvBegin(), 2, 0);
        // ssize_t res = writev(_clientfd, _channel->buffer->readvBegin(), 2);
        // callback_written_then_close(res);
        _channel->write_buffer->hasRead(ret);
    }

    void TcpConnection::read(){
        //CAS操作解决并发冲突
        while(true){
            uint8_t connection_status = _connect_status.load();
            if(connection_status & CONNECTION_STATUS_READABLE == 0){  //如果是不可读的，直接return
                return; 
            }

            if(_connect_status.compare_exchange_weak(connection_status, connection_status)){  //竞态成功了
                io_uring_sqe * sqe = _event_loop->getOneSqe();
                sqe->user_data = getIOUringUserdata(this, USER_EVENT_READ);
                io_uring_prep_readv(sqe, _clientfd, _channel->read_buffer->writevBegin(), 2, 0);
                break;
            }
        }
    }

    std::string TcpConnection::retrieveAllAsString(){
        return _channel->read_buffer->retrieveAllBufferAsString();
        // io_uring_prep_epoll_ctl
    }


    //使用Buffer时，记录写入的字节数
    void TcpConnection::recordWrittenBytes(size_t written_bytes){
        _channel->read_buffer->hasWritten(written_bytes);
    }

    //使用Buffer时，记录读取的字节数
    void TcpConnection::recordReadBytes(size_t read_bytes){

    }

    void TcpConnection::callback_accepted(int res){
        if(res < 0){
            perror("accept");
            return;
        }
        setFd(res); 
        //将连接的状态改为可读可写，然后开始read
        _connect_status.store( CONNECTION_STATUS_READABLE | CONNECTION_STATUS_WRITABLE );
        _event_loop->_connection_manager->acceptDone();
        read();
    }

    void TcpConnection::callback_read(int res){
        if(res < 0){
            perror("read");
            return;
        } else if (res == 0){  //对端关闭了连接
            disconnect_async();
            return;
        }

        //_buffer需要手动记录写入的字节
        _channel->read_buffer->hasWritten(res);

        //TODO:大请求，有时候用RingBuffer就处理不了，需要直接根据后面的size为ri的body预分配空间
        uint16_t readableBytes = _channel->read_buffer->readableBytes();
        while(readableBytes >= _target_bytes){
            if(!_header_process){  //如果还没有解析头，那就先解析头
                auto ret = parserProtocalHeader(_channel->read_buffer->read(_target_bytes), _ri);
                if(ret.has_value()) {
                    readableBytes -= _target_bytes;
                    _target_bytes = ret.value();
                    _header_process = 1;
                } else {
                    throw("Bad Request For Protocal Header");
                }

            } else if(_header_process == 1) {  //解析Rpc头
                auto ret = parserProtocalRpcHeader(_channel->read_buffer->read(_target_bytes), _ri);
                if(ret.has_value()) {
                    readableBytes -= _target_bytes;
                    _target_bytes = ret.value();
                    _header_process = 2;
                } else {
                    throw("Bad Request For Rpc Header");
                }

            } else {  //解析body
                parserProtocalBody(_channel->read_buffer->read(_target_bytes), _ri);
                readableBytes -= _target_bytes;
                _target_bytes = PROTOCAL_HEADER_LEN;
                _header_process = 0;
                _ri.conn = this;
                //body解析完了之后，交给EventLoop进行异步处理
                _event_loop->asyncHandleRequest(std::move(_ri));
            }
        }

        read();  //然后继续进行读过程
    }

    void TcpConnection::callback_written(int res){
        if(res < 0){
            std::cout << strerror(-res) << std::endl;
            disconnect_async();
            return;
        } 

        // cout << "written: " << res << endl;
        _channel->write_buffer->hasRead(res);
        _to_send_bytes.fetch_sub(res);
        //没发送完，继续发送
        if(_to_send_bytes > 0){
            auto sqe = _event_loop->getOneSqe();
            sqe->user_data = getIOUringUserdata(this, USER_EVENT_WRITE);
            io_uring_prep_writev(sqe, _clientfd, _channel->write_buffer->readvBegin(), 2, 0);
        } 
    }

    void TcpConnection::callback_closed(int res){
        // cout << "fd: " << _clientfd << " Close: " << res << std::endl;
        // if(res < 0) cout << "Close Error" << strerror(-res) << std::endl;
        clearBuffer();
        _event_loop->_connection_manager->sendBack(this);
    }

    void TcpConnection::callback_written_then_close(int res){
        this->disconnect_async();
    }

    void TcpConnection::clearBuffer(){
        _channel->read_buffer->clear();
        _channel->write_buffer->clear();
    }

    ConnectionManager::ConnectionManager(int sockfd, EventLoop * loop,size_t init_conn, size_t max_conn):_sockfd(sockfd), _upper_limit_count(max_conn), \
        _idle_pool(init_conn), _loop(loop){
        expandConnections(init_conn);
    }

    ConnectionManager::~ConnectionManager(){
        //TODO:连接池的回收管理
        while(_allocated_count){
            TcpConnection * tc;
            _idle_pool.try_dequeue(tc);
            _accepting_count--;
            delete tc;
        }
    }

    //添加size个TcpConnection进行accept操作
    void ConnectionManager::acceptConnection(int size){
        //如果当前已经达到了可申请的资源上限，那么就没法加了
        int add_size = min(size, _upper_limit_count - _outer_count - _accepting_count);
        if(_idle_pool.size_approx() < add_size){
            expandConnections(2 * add_size);
        }

        for(int i = 0;i < add_size; ++i){
            TcpConnection * tc;
            if(_idle_pool.try_dequeue(tc)){
                tc->accept();
            } else break;
        }
    }

    void ConnectionManager::acceptForCount(int size){
        int add_size = min(size - _accepting_count, _upper_limit_count - _outer_count - _accepting_count);
        if(_idle_pool.size_approx() < add_size){
            expandConnections(2 * add_size);
        }

        for(int i = 0;i < add_size; ++i){
            TcpConnection * tc;
            if(_idle_pool.try_dequeue(tc)){
                _accepting_count.fetch_add(1);
                tc->accept();
            } else break;
        }
    }

    //申请扩容
    void ConnectionManager::expandConnections(int size){
        //TODO:怎么做到一次性分配内存
        int allocate_size = min(size, _upper_limit_count - _outer_count - _accepting_count);
        for(int i = 0;i < allocate_size; ++i){
            _allocated_count.fetch_add(1);
            _idle_pool.enqueue(new TcpConnection(_sockfd, _loop));
        }
    }

    //只允许TcpConnection自己调用，归还TcpConnection;
    void ConnectionManager::sendBack(TcpConnection * tc){
        _idle_pool.enqueue(tc);
        _outer_count.fetch_sub(1);
        _total_count.fetch_add(1);
    }

    inline void ConnectionManager::acceptDone(){
        _accepting_count.fetch_sub(1);
        _outer_count.fetch_add(1);
    }

    int ConnectionManager::getActiveCoon(){
        return _outer_count.load();
    }
}