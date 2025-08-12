#include "Rpc/etcd_client.h"
#include "Rpc/rpcprovider.h"

using namespace GcRpc;
using namespace std;

int main(){
    // RpcProvider loop;
    EtcdClient etcd_client("http://127.0.0.1:2379", nullptr);
    std::string lease_id = etcd_client.leaseGrant(10);
    etcd_client.put("/test", "test", lease_id);
    auto resp = etcd_client.get("/test", true);
    for(auto & item : resp){
        std::cout << item.first << ": " << item.second << std::endl;
    }
    return 0;
}