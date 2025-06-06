#include "login_service.h"
#include "Rpc/generic_rpc.pb.h"
#include <iostream>
using namespace std;
using namespace ::google::protobuf;


int main(){
    GcRpc::RpcHeader header;
    header.set_method_name("UserServiceRpc");
    header.set_service_name("Login");
    header.set_args_size(15);

    std::string srl = header.SerializeAsString();
    cout << srl << endl;

    GcRpc::RpcHeader header2;
    if(!header2.ParseFromString(srl)){
        cout << "parse fails" << endl;
    }
    cout << header2.method_name() << endl;
    cout << header2.service_name() << endl;
    cout << header2.args_size() << endl;
    return 0;
}