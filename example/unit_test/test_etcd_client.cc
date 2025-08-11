#include "Rpc/etcd_client.h"
#include "Rpc/rpcprovider.h"

using namespace GcRpc;
using namespace std;

int main(){
    // RpcProvider loop;
    EtcdClient etcd_client("http://127.0.0.1:2379", nullptr);
    etcd_client.async_leaseGrant(10);
    etcd_client.async_put("/test", "test", true);
    etcd_client.async_get("/test");
    return 0;
}