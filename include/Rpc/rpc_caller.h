/**
 * @brief
 */
#pragma once
#include <google/protobuf/service.h>
#include "Rpc/net_base.h"
#include <future>
#include <etcd/Client.hpp>
#include <vector>
#include "Rpc/channel.h"
#include "liburing.h"

namespace GcRpc{  
    using Request = ::google::protobuf::Message;
    using Response = ::google::protobuf::Message;
    class ConnectionPool;
    class ServiceSeeker;

    class ServiceCaller{
    private:
        std::unique_ptr<ServiceSeeker> _service_seeker;
        std::unique_ptr<ConnectionPool> _connection_pool;
        void uring_looping(); 
    

    public:
        std::future<Response> asyncCall(const std::string & method_name, const Request * request);
        
    };

    //先实现单服务调用，然后再实现多服务调用
    class RpcCaller: public ::google::protobuf::RpcChannel{
        int sockfd; 

        std::shared_ptr<etcd::Client> _etcd_client;
        // std::unordered_map<GcRpc::>       ; 
        //这里要设计两个线程，主线程用于同步提交，直接调用对应的channel进行读写
        //额外的线程用于异步执行和提交，内部维护一个io_uring
        struct io_uring ring;
        struct io_uring_params params;
        std::thread uring_thread;  //用于异步处理uring指令

    public:
        RpcCaller();
        ~RpcCaller();
        
        template <typename STUB>
        bool call(const std::string & method_name, const ::google::protobuf::Message * request, ::google::protobuf::Message * response){
            using T = STUB;
            STUB s(this);
            s.CallMethod(s.GetDescriptor()->FindMethodByName(method_name), nullptr, request, response, nullptr);
            return true;
        }

        // std::future<Response> asyncCall(const std::string & method_name, const Request * request);

        virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
            ::google::protobuf::RpcController* controller, const ::google::protobuf::Message* request,
            ::google::protobuf::Message* response, ::google::protobuf::Closure* done);
        
    private:
        //io_uring线程的循环，用于支持异步调用
        // void uring_looping();  
    };

}