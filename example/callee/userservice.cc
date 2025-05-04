#include "Rpc/gcrpcapplication.h"
#include "Rpc/rpcprovider.h"
#include "login_service.h"
#include <iostream>

int main(int argc, char** argv){
    //initial a application
    // provide -i config.conf
    GcRpc::GcRpcApplication::getInstance().Init(argc, argv);

    //notify a service
    GcRpc::RpcProvider provider;
    provider.Notify(new GcRpc::LoginService());

    provider.run();

    std::cout << "run done" << std::endl;
    return 0;
}