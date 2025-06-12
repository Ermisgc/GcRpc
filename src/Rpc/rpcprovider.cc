#include "Rpc/rpcprovider.h"
#include "google/protobuf/descriptor.h"
#include "Rpc/generic_rpc.pb.h"
#include "Rpc/channel.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "Rpc/gcrpcapplication.h"
#include "Rpc/rpc_protocal.h"
using namespace GcRpc;

#define p(n) std::cout << n << std::endl;
#define PROVIDER_UNSTART 0
#define PROVIDER_LOOPING 1
#define PROVIDER_WAIT_TO_STOP 2
#define PROVIDER_STOP 3

RpcProvider::RpcProvider(): ring(), params(), _status(PROVIDER_UNSTART){
    std::cout << ">>> " << "IO_uring is initializing ..." << std::endl;
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

    threadPond = gchipe::DynamicThreadPool::getInstance(stoi(GcRpcApplication::Load("worker_thread")));
    feedback_queue = new LockFreeChannelQueue();
}
RpcProvider::~RpcProvider(){
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
    for(int i = 0;i < params.sq_entries; ++i){
        io_uring_sqe * sqe = io_uring_get_sqe(&ring);
        Channel * one = new Channel(0);  
        socklen_t * len = one->getSockLen();
        sqe->user_data = reinterpret_cast<unsigned long long>(one);
        io_uring_prep_accept(sqe, _fd, one->getSockAddr(), len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    }
    _status.store(PROVIDER_LOOPING);

    ::listen(_fd, 100);
    io_uring_cqe * cqes[params.cq_entries];
    while(_status == PROVIDER_LOOPING){
        io_uring_submit(&ring);
        int finish_count = io_uring_peek_batch_cqe(&ring, cqes, params.cq_entries);
        //std::cout <<">>> finish_count:" << finish_count << std::endl;
        for(int i = 0;i < finish_count; ++i){
            struct io_uring_cqe * cqe_now = cqes[i];
            if(cqe_now->res < 0){  //TODO:Error handling
                std::cout << ">>> [-] I/O Operation fails: " << strerror(-cqe_now->res) << std::endl;
                if(!cqe_now->user_data) std::cout << "nullptr of user_data" << std::endl;
                else {
                    Channel * channel = (Channel*)cqe_now->user_data;
                    channel->setEventStatus(USER_EVENT_CLOSE);
                }
            }
            
            Channel * channel = reinterpret_cast<Channel*>(cqe_now->user_data);
            struct io_uring_sqe * another_sqe = io_uring_get_sqe(&ring);
            another_sqe->user_data = reinterpret_cast<unsigned long long>(channel);

            switch (channel->eventStatus())
            {
            case USER_EVENT_ACCEPT:
                channel->setFd(cqe_now->res);
                channel->setEventStatus(USER_EVENT_READ);
                io_uring_prep_readv(another_sqe, channel->fd(), channel->getWriteBuffer(), 2, 0);
                // p("accept");
                break;
            
            case USER_EVENT_READ:   // send 
                channel->setEventStatus(USER_EVENT_WRITE);
                //TODO:Ring Buffer have to move after write
                channel->recordWriteBytes(cqe_now->res);
                threadPond->execute(std::bind(&RpcProvider::onMessage, this, channel));
                // p("read");
                break;

            case USER_EVENT_WRITE:
                channel->setEventStatus(USER_EVENT_CLOSE);
                io_uring_prep_close(another_sqe, channel->fd());
                // p("write");
                break;

            case USER_EVENT_CLOSE:
                channel->setEventStatus(USER_EVENT_ACCEPT);
                channel->setFd(0);
                channel->clearBuffer();
                // p("close");
                io_uring_prep_accept(another_sqe, _fd, channel->getSockAddr(), channel->getSockLen(), SOCK_NONBLOCK | SOCK_CLOEXEC);
                break;

            default: 
                break;
            }
            io_uring_cqe_seen(&ring, cqe_now);
        }

        bool dequeueSuccess = true;
        while(dequeueSuccess){
            Channel * feedback_channel;
            dequeueSuccess = feedback_queue->try_dequeue(feedback_channel);
            if(dequeueSuccess){
                struct io_uring_sqe * another_sqe = io_uring_get_sqe(&ring);
                another_sqe->user_data = (unsigned long long)feedback_channel;
                io_uring_prep_writev(another_sqe, feedback_channel->fd(), feedback_channel->getReadBuffer(), 2, 0);
            }
        }
    }
}

void RpcProvider::onConnection(){
}

void RpcProvider::onMessage(Channel * channel){
    parseProtoMessage(channel->retrieveAllBufferAsString(), channel);
    channel->setEventStatus(USER_EVENT_WRITE);
    feedback_queue->enqueue(channel);
}

void RpcProvider::parseProtoMessage(const std::string & recv_str, Channel * channel){
    RequestInformation recv_info;
    if(!parserProtocalFromString(recv_str, recv_info)){
        std::cerr << std::dec << ">>> Cannot parse from received string: " << recv_str << std::endl;
        return;
    }

    //Call the Service
    if(serviceTable.find(recv_info.service_name) == serviceTable.end()){
        std::cerr << std::dec << ">>> Service is not supported: " << recv_info.service_name << std::endl;
        return;
    }
    MethodTable & method_table = serviceTable[recv_info.service_name];
    if(method_table.methodTable.find(recv_info.method_name) == method_table.methodTable.end()){
        std::cerr << std::dec << ">>> Service: " << recv_info.service_name << " has no method named: " << recv_info.method_name << std::endl;
        return;
    }
    const ::google::protobuf::MethodDescriptor * desc = method_table.methodTable[recv_info.method_name];
    ::google::protobuf::Message* request_message = method_table.service->GetRequestPrototype(desc).New();

    ::google::protobuf::Message * response_message = method_table.service->GetResponsePrototype(desc).New();
    //TODO: finish the CallMethod here.
    method_table.service->CallMethod(method_table.methodTable[recv_info.method_name], nullptr, request_message, response_message, nullptr);

    //然后现在要把这个response_message写出去：
    recv_info.message_type = MESSAGE_TYPE_RESPONSE;
    std::string send_str = response_message->SerializeAsString();
    channel->append(send_str);
}

#undef p