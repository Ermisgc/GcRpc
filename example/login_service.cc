#include "login_service.h"
#include <string>
namespace GcRpc{
    void LoginService::Login(::google::protobuf::RpcController* controller, \
        const ::gctemp::LoginRequest* request, \
        ::gctemp::LoginResponse* response, \
        ::google::protobuf::Closure* done){
            std::string name = request->name();
            std::string password = request->pwd();
            //std::cout << "method called" << std::endl;
            bool res_code = Login(name, password);
            gctemp::ResultCode * rc = response->mutable_result();
            rc->set_errcode(res_code ? 1 : 0);
            rc->set_errmsg(res_code ? "success" : "fail");
            response->set_success(res_code);
            if(done) done->Run();
        }
    
    // TODO:GetFriendLists not implement yet
    void LoginService::GetFriendLists(::google::protobuf::RpcController* controller,
        const ::gctemp::GetFriendListRequest* request,
        ::gctemp::GetFriendListResponse* response,
        ::google::protobuf::Closure* done){
            if(done) done->Run();

        }

    bool LoginService::Login(std::string name, std::string psw){
        std::cout << name << ": " << psw << std::endl;
        return true;
    }
}