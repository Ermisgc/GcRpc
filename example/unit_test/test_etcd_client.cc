// #include "Rpc/etcd_client.h"
// #include "Rpc/rpcprovider.h"

// using namespace GcRpc;
// using namespace std;

// int main(){
//     RpcProvider loop;
//     EtcdClient etcd_client("http://127.0.0.1:2379", &loop);
//     etcd_client.leaseGrant(10);
//     etcd_client.put("/test", "test");
//     etcd_client.get("/test");
//     loop.loop();
// }
