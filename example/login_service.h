#include "user.pb.h"
#include <google/protobuf/service.h>

namespace GcRpc{
    class LoginService: public ::gctemp::UserServiceRpc{
    public:
        LoginService() = default;
        ~LoginService() = default;
        void Login(::google::protobuf::RpcController* controller,
            const ::gctemp::LoginRequest* request,
            ::gctemp::LoginResponse* response,
            ::google::protobuf::Closure* done);
        
    private:
        bool Login(std::string name, std::string psw);
    };
}