#pragma once
#include "net_base.h"
#include <memory>
#include <chrono>
#include <functional>
#include "liburing.h"
#include "channel.h"
#include "hipe/dynamic_threadpool.h"
#include "rpc_protocal.h"
#include "data_structure/concurrentqueue.h"

namespace GcRpc{
    class ChannelV2;
    class TcpConnection;

    unsigned long long getIOUringUserdata(UringChannel * uc, uint8_t event_type);

    //根据io_uring的user data中的数据进行io_uring的自动调用
    void executeIOUringCallback(unsigned long long user_data, int res);

    //TODO:自定义事件回调机制设计完善
    #define CONNECTION_STATUS_READABLE 1  //可执行写指令
    #define CONNECTION_STATUS_WRITABLE 2  //可执行读指令
    #define CONNECTION_STATUS_CLOSING 4  //可执行关闭指令

    //TcpConnection理论上来讲只有两个线程在竞争
    //一个是事件循环线程，一个是worker线程，两者发生竞争的概率其实很小
    //因此采用乐观锁CAS操作来解决两者的并发冲突
    //TODO:怎么区分客户端和服务端的编译行为呢
    class TcpConnection :public UringChannel{
        friend class CallbackAccept;
        friend class CallbackRead;

        //来自EventLoop/RpcProvider的元数据
        int _sockfd;  //本机的socket fd
        int _clientfd;  //对应的用户端fd
        int _target_bytes = PROTOCAL_HEADER_LEN; //为了解决粘包问题,要达到预定的target,它由头长度和服务长度交替修改
        
        //数据头的解析进度，如果是0的话，说明应用层协议头还没有解析
        //如果是1的话，说明Rpc Header还没有解析
        //如果是2的话，说明头全部解析完了，现在需要去复制body
        uint8_t _header_process = 0;
        std::atomic<uint8_t> _connect_status{0};    //连接的状态，主要有是否可读，是否打算断连
        bool _active;

        RequestInformation _ri;  //临时性的_ri，每次执行异步操作时，会将它移动到工作线程

        EventLoop * _event_loop;
        std::unique_ptr<ChannelV2> _channel;
        std::unique_ptr<InetAddr> _ia; 
        std::chrono::steady_clock::time_point _last_active;
        

    protected:  //定义可继承重写的事件回调机制

        virtual void callback_accepted(int res);

        virtual void callback_read(int res);

        virtual void callback_written(int res);

        virtual void callback_closed(int res);

        virtual void callback_written_then_close(int res);

    public:
        //基于sockfd构建TcpConnection
        TcpConnection(int sockfd, EventLoop * event_loop = nullptr);

        ~TcpConnection() noexcept;

        void do_io_uring(int last_res, int last_event_type) override;
        
        //用于Client端调用，Client端调用connect
        bool connect(const InetAddr && addr);

        //Client端/Server端都可用的disconnect
        bool disconnect();

        //判断是否还连接，这里的连接状态是一种假连接状态，或者说，不能说是正确的连接状态，而是预估的
        bool is_connected() const;

        //清理Buffer
        void clearBuffer();

        //Server端可用，通过io_uring异步关闭连接
        bool disconnect_async();

        //使用io_uring异步建立连接
        bool accept();

        //使用io_uring异步发送数据
        bool send(const std::string & to_be_send);

        //使用io_uring异步读数据
        void read();

        //基于io_uring,发送完字符串后删除,TODO:不一定是全部数据
        void sendThenClose(const std::string & to_be_send);

        //从Buffer中倒出全部字符串
        std::string retrieveAllAsString();

        //设置本连接对应的客户端fd
        void setFd(int clientfd);

        //设置事件状态
        void setEventStatus(int status);

        //使用Buffer时，记录写入的字节数
        void recordWrittenBytes(size_t written_bytes);

        //使用Buffer时，记录读取的字节数
        void recordReadBytes(size_t read_bytes);
    };

    //连接池 //TODO:连接池LRU策略实现  //TODO:连接过期实现
    //GcRpc里，线程池是单线程
    //主要思路：每个连接会记录活跃时间，如果距离上一次活跃时间已经很长时间了，那么这个连接可以被收回了（connection_retract_time）
    //维护一个LRU链表，然后当发现连接数量不够的时候，从LRU链表尾部选择一个TcpConnection进行复用
    //如果这个Tcp Connection上一次活跃时间已经过去了connection_retract_time，那么进行回收，并且会启动某个工作线程，去执行回收任务
    //一口气回收connection_retract_counts个超时连接
    //先不用实现LRU策略吧，先实现池化
    class ConnectionManager{
        friend class TcpConnection;
        
        int _upper_limit_count;  //连接总数的上限，在外面的连接+在空闲队列里的连接
        int _sockfd;
        std::atomic<int> _outer_count;  //已经分配进行io_uring循环的连接数量
        std::atomic<int> _accepting_count;  //当前正在执行accept的连接数量

        EventLoop * _loop;

        moodycamel::ConcurrentQueue<TcpConnection* > _idle_pool;  //无锁空闲连接队列

    public:
        ConnectionManager(int sockfd, EventLoop * loop, size_t init_conn, size_t max_conn);
        ~ConnectionManager();

        //让size个TcpConnection进行accept操作
        void acceptConnection(int size);  

        //让当前进行accepting的连接数量保持在size
        void acceptForCount(int size);

    private:

        //申请扩容
        void expandConnections(int size);

        //只允许TcpConnection调用，归还TcpConnection;
        void sendBack(TcpConnection *);

        //只允许TcpConnection调用，告诉ConnectionManager我已经完成了accept
        void acceptDone();
    };  
}