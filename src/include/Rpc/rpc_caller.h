/**
 * @brief
 */
#pragma once
#include <google/protobuf/service.h>

namespace GcRpc{
    class RpcCaller: public ::google::protobuf::RpcChannel{
        //RpcCaller() = delete;
    public:
        RpcCaller();
        
        template <typename STUB>
        bool call(const std::string & method_name, const ::google::protobuf::Message * request, ::google::protobuf::Message * response){
            using T = STUB;
            STUB s(this);
            s.CallMethod(s.GetDescriptor()->FindMethodByName(method_name), nullptr, request, response, nullptr);
            return true;
        }

        virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
            ::google::protobuf::RpcController* controller, const ::google::protobuf::Message* request,
            ::google::protobuf::Message* response, ::google::protobuf::Closure* done);
    private:
        int sockfd;
    };

}