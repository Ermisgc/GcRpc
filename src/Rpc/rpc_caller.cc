#include "Rpc/rpc_caller.h"
#include "Rpc/gcrpcapplication.h"
#include "Rpc/generic_rpc.pb.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "Rpc/rpc_protocal.h"
#include <iostream>
#include <iomanip>
#include <charconv>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

namespace GcRpc{
RpcCaller::RpcCaller(){
    //Step1. network setup
    sockfd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in sock_addr;
    sock_addr.sin_addr.s_addr = inet_addr(GcRpcApplication::Load("client_ip").c_str());
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(stoi(GcRpcApplication::Load("port")));
    while(::connect(sockfd, (sockaddr*) &sock_addr, sizeof(sock_addr))){
        std::cout << ">>> Fail to connect " + GcRpcApplication::Load("client_ip") + ":" + GcRpcApplication::Load("port") << std::endl;
        std::cout << ">>> Retry to connect ..." << std::endl;
        // throw std::runtime_error("Fail to connect " + GcRpcApplication::Load("client_ip") + ":" + GcRpcApplication::Load("port"));
    }
}

RpcCaller::~RpcCaller(){
    ::close(sockfd);
}

void RpcCaller::CallMethod(const ::google::protobuf::MethodDescriptor* method,
    ::google::protobuf::RpcController* controller, const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response, ::google::protobuf::Closure* done){
    std::string serializedString_args = request->SerializeAsString();
    
    //根据协议发送请求
    RequestInformation info;
    info.method_name = method->name();
    info.body_data = serializedString_args;
    info.service_name = method->service()->name();
    info.message_type = MESSAGE_TYPE_REQUEST;
    info.flag = 1;  //发送完请求之后就关闭
    info.status = STATUS_SUCCESS;
    info.version = 0;
    info.request_id = 0x12345678;
    std::string send_str;
    generateStringFromProtocal(info, send_str);

    if(send(sockfd, send_str.c_str(), send_str.length(), 0) <= 0){
        std::cout << ">>> Fail to send:" << send_str << std::endl;
        return;
    }

    static int max_buffer_size = stoi(GcRpcApplication::Load("max_buffer_size"));
    char buf[max_buffer_size];
    // int target_size = 15;
    // while(target_size > 0){
    //     int ret = recv(sockfd, buf + (15 - target_size), max_buffer_size - (15 - target_size), 0);
    //     if(ret < 0){  //blocking
    //         std::cout << ">>> Fail to recv:" << send_str << std::endl;
    //         return;
    //     }
    //     target_size -= ret;
    // }
    if(recv(sockfd, buf, max_buffer_size, 0) < 0){  //blocking
        std::cout << ">>> Fail to recv:" << send_str << std::endl;
        return;
    }

    std::string recv_ed = buf;
    // std::cout << "[RECV] Bytes: " << recv_ed.size() << std::endl;
    // std::cout << "[RECV] Hex: ";
    // for (size_t i = 0; i < recv_ed.size(); ++i) {
    //     std::cout << std::hex << std::setfill('0') << std::setw(2) 
    //             << (static_cast<unsigned int>(recv_ed[i]) & 0xFF) << " ";
    // }
    
    // std::cout << std::dec << std::endl;
    try{
        response->ParseFromString(recv_ed);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    // response->ParseFromString(recv_ed);

}
}