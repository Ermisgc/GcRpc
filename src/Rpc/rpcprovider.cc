#include "Rpc/rpcprovider.h"
#include "google/protobuf/descriptor.h"
#include "Rpc/generic_rpc.pb.h"
using namespace GcRpc;

RpcProvider::RpcProvider(){
    
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
}

void RpcProvider::run(){

}

void RpcProvider::onConnection(){


}
void RpcProvider::onMessage(){
    

}

void RpcProvider::parseProtoMessage(const std::string & recv_str){
    int header_size = stoi(recv_str.substr(0, 4));
    RpcHeader header;
    if(!header.ParseFromString(recv_str.substr(4, header_size))){
        throw "Rpcheader cannot parse from received string";
    }

    //Parse The Header
    const std::string & service_name = header.service_name();
    if(serviceTable.find(service_name) == serviceTable.end()) throw "Service " + service_name + " is not existed";
    const std::string & method_name = header.method_name();
    const uint32_t args_size = header.args_size();

    //Call the Service
    MethodTable & method_table = serviceTable[service_name];
    const ::google::protobuf::MethodDescriptor * desc = method_table.methodTable[method_name];
    ::google::protobuf::Message* request_message = method_table.service->GetRequestPrototype(desc).New();

    if(!request_message->ParseFromString(recv_str.substr(4 + header_size, args_size))){
        throw "RequestMessage cannot parse from received string";
    }
    ::google::protobuf::Message * response_message = method_table.service->GetResponsePrototype(desc).New();
    //TODO: finish the CallMethod here.
    method_table.service->CallMethod(method_table.methodTable[method_name], nullptr, request_message, response_message, nullptr);
}