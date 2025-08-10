#include "Rpc/rpcprovider.h"
#include "google/protobuf/descriptor.h"
#include "Rpc/generic_rpc.pb.h"
#include "Rpc/channel.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "Rpc/gcrpcapplication.h"
#include "Rpc/rpc_protocal.h"
#include <etcd/KeepAlive.hpp>
#include "Rpc/rpc_node_info.pb.h"
using namespace GcRpc;

#define p(n) std::cout << n << std::endl;
#define PROVIDER_UNSTART 0
#define PROVIDER_LOOPING 1
#define PROVIDER_WAIT_TO_STOP 2
#define PROVIDER_STOP 3

std::atomic<int> RpcProvider::_status = PROVIDER_UNSTART;

RpcProvider::RpcProvider(): ring(), params(),  _etcd(GcRpcApplication::Load("etcd_addr")){
    std::cout << ">>> " << "IO_uring is initializing ..." << std::endl;
    //检测etcd是否有效
    try {
        auto response = _etcd.get("health_check_key").get();
    }
    catch (const std::exception& e) {
        std::cerr << "etcd error: " << e.what() << std::endl;
        throw("ETCD SERVICE NOT PROVIDED");
    }
    
    //创建io_uring
    memset(&params, 0, sizeof(params));
    int ret = io_uring_queue_init_params(stoi(GcRpcApplication::Load("sq_entries")), &ring, &params);
    if(ret < 0){
        std::cerr << ">>> [-] Failed to initialize io_uring: " << strerror(-ret) << std::endl;
        throw std::runtime_error("io_uring initialization failed");
    }
#ifdef _Debug
    std::cout << ">>> " << "sq_entries:" << params.sq_entries << std::endl;
    std::cout << ">>> " << "cq_entries:" << params.cq_entries << std::endl;
    std::cout << ">>> " << "IO_uring initialization done !!!" << std::endl;
#endif
    _fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(_fd < 0){
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        throw std::runtime_error("Socket creation failed");
    }
    addr = new sockaddr_in;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_family = AF_INET;
    addr->sin_port = htons(stoi(GcRpcApplication::Load("port")));
    if(::bind(_fd, (sockaddr * )addr, sizeof(sockaddr_in)) < 0){
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        throw std::runtime_error("Socket bind failed");
    }

    //初始化线程池
    threadPond = gchipe::DynamicThreadPool::getInstance(stoi(GcRpcApplication::Load("worker_thread")));
    
    _max_conn = stoi(GcRpcApplication::Load("max_conn"));
    
    //TODO:连接管理器的初始连接数量需要由文件设定
    _connection_manager = std::make_unique<ConnectionManager>(_fd, this, params.sq_entries, _max_conn);


    signal(SIGINT, [](int sig){RpcProvider::sigint_handler(sig);});
}

RpcProvider::~RpcProvider() noexcept{
    unRegisterEtcdServices();
    auto sqe = getOneSqe();

    io_uring_prep_cancel(sqe, (void * )(uintptr_t) - 1, IORING_ASYNC_CANCEL_ALL);
}

void RpcProvider::Notify(::google::protobuf::Service * new_service){
    const google::protobuf::ServiceDescriptor * descriptor = new_service->GetDescriptor();
    std::string service_name = descriptor->name();  //Service is a pure virtual class
    struct MethodTable table;
    table.service = new_service;
    
    for(int i = 0;i < descriptor->method_count(); ++i){
        const google::protobuf::MethodDescriptor * method_descriptor = descriptor->method(i);
        table.methodTable[method_descriptor->name()] = method_descriptor;
    }

    serviceTable[service_name] = table;
}

void RpcProvider::loop(){
    //申请一个keepalive租约并且一直执行keepalive：
    //TODO:改用RAII来管理租约行为，类创建时获取租约，类析构时释放租约，并且提供租约自动续租类
    constexpr int ttl = 20;  //lease续租有效期20s
    _lease_keeper = _etcd.leasekeepalive(ttl).get();
    _lease_id = _lease_keeper->Lease();

    for(const auto & pr: serviceTable){
        const std::string etcd_node_key = "/services/" + pr.first + "/" + getLocalIP();
        RpcNodeInfo info;

        //addr->uint32_t
        info.set_addr(inet_addr(getLocalIP().c_str()));
        info.set_active_conn(this->_active_conn);
        info.set_port(1214);   //本服务的端口号
        info.set_max_conn(this->_max_conn);

        const std::string etcd_node_value = info.SerializeAsString();

        auto put_resp = _etcd.put(etcd_node_key, etcd_node_value, _lease_id);

        if(!put_resp.is_done()){
            std::cout << ">>> Service " << pr.first << " fails to registered to etcd. " << std::endl;
        }
    }

    _status.store(PROVIDER_LOOPING);

    //此处的listen一定要比accept先做
    ::listen(_fd, params.sq_entries);

    //创建若干个io_uring接收缓冲区
    io_uring_cqe * cqes[params.cq_entries];

    //开始事件循环
    while(_status == PROVIDER_LOOPING){
        //设置accept
        _connection_manager->acceptForCount(params.sq_entries);
        
        //批量化提交io_uring
        io_uring_submit(&ring);

        //获得返回值
        int finish_count = io_uring_peek_batch_cqe(&ring, cqes, params.cq_entries);
        for(int i = 0;i < finish_count; ++i){
            struct io_uring_cqe * cqe_now = cqes[i];
            executeIOUringCallback(cqe_now->user_data, cqe_now->res);
            if(cqe_now->res < 0){  //TODO:Error handling
                std::cout << ">>> [-] I/O Operation fails: " << strerror(-cqe_now->res) << std::endl;
                if(!cqe_now->user_data) std::cout << "nullptr of user_data" << std::endl;
                else {
                    Channel * channel = (Channel*)cqe_now->user_data;
                    channel->setEventStatus(USER_EVENT_CLOSE);
                }
            }
            
            io_uring_cqe_seen(&ring, cqe_now);
        }

        //检查租约是否还在
        healthCheck();
    }
}

void RpcProvider::onMessage(RequestInformation && ri){
    //Call the Service
    if(serviceTable.find(ri.service_name) == serviceTable.end()){
        std::cerr << std::dec << ">>> Service is not supported: " << ri.service_name << std::endl;
        return;
    }
    MethodTable & method_table = serviceTable[ri.service_name];
    if(method_table.methodTable.find(ri.method_name) == method_table.methodTable.end()){
        std::cerr << std::dec << ">>> Service: " << ri.service_name << " has no method named: " << ri.method_name << std::endl;
        return;
    }

    const ::google::protobuf::MethodDescriptor * desc = method_table.methodTable[ri.method_name];
    ::google::protobuf::Message* request_message = method_table.service->GetRequestPrototype(desc).New();

    ::google::protobuf::Message * response_message = method_table.service->GetResponsePrototype(desc).New();
    method_table.service->CallMethod(method_table.methodTable[ri.method_name], nullptr, request_message, response_message, nullptr);

    //然后现在要把这个response_message写出去：
    // ri.message_type = MESSAGE_TYPE_RESPONSE;
    std::string send_str = response_message->SerializeAsString();

    if(ri.flag == 0){
        ri.conn->send(std::move(send_str));
    } else if(ri.flag == 1){  //发送完毕后关闭
        ri.conn->sendThenClose(std::move(send_str));
    } else {
        std::cerr << "invalid flag" << std::endl;
    }
    
}

void RpcProvider::unRegisterEtcdServices(){
    //撤销租约
    auto resp = _etcd.leaserevoke(_lease_id);
}

void RpcProvider::healthCheck(){
    //TODO:healthCheck
}

void RpcProvider::sigint_handler(int sig){
	if(sig == SIGINT){
		// ctrl+c退出时执行的代码
		std::cout << "ctrl+c pressed!" << std::endl;
		_status.store(PROVIDER_WAIT_TO_STOP);
	}
}

struct io_uring_sqe * RpcProvider::getOneSqe(){
    return io_uring_get_sqe(&ring);
}

void RpcProvider::asyncHandleRequest(RequestInformation && ri){
    threadPond->execute(
        [this, ri = std::move(ri)]() mutable {  // 关键：移动捕获 ri
            this->onMessage(std::move(ri));      // 在 lambda 内显式移动
        }
    );
}

#undef p