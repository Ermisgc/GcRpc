#include "login_service.h"
#include "test.pb.h"
#include "Rpc/gcrpcapplication.h"
#include "Rpc/rpc_caller.h"
#include <google/protobuf/service.h>
using namespace ::google::protobuf;

int main(int argc, char ** argv){
    GcRpc::GcRpcApplication::getInstance().Init(argc, argv);
    GcRpc::RpcCaller caller;
    gctemp::LoginRequest request;
    request.set_name("zhangsan");
    request.set_pwd("123");

    gctemp::LoginResponse response;
    caller.call<::gctemp::UserServiceRpc_Stub>("Login", &request, &response);
    if(response.success()) std::cout << "注册成功" << std::endl;
    else std::cout << "注册失败" << std::endl;
    return 0;
}