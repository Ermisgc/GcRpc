#ifndef RPC_PROVIDER_H
#define RPC_PROVIDER_H
#include "google/protobuf/service.h"
#include "hipe/dynamic_threadpool.h"
#include <functional>
#include "arpa/inet.h"
#include "net_base.h"
#include <unordered_map>
#include <vector>
#include <atomic>
#include "liburing.h"
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include "connection_pool.h"
#include "service_register.h"

namespace GcRpc{
    class Channel;
    class LogMsg;
    class IProtocalDetector;
    using LockFreeChannelQueue = moodycamel::ConcurrentQueue<Channel *>;
    using LockFreeLogQueue = moodycamel::ConcurrentQueue<LogMsg>;
    using Service = ::google::protobuf::Service;

    class RpcProvider
    {
        friend class TcpConnection;  //设定TcpConnection为友元，让它可以访问私有方法
        friend class EtcdClient;     //TODO:有没有什么优雅的方法让基类和它的派生类都可以访问类的私有成员对象
        friend class ServiceRegister;

        struct MethodTable{
            Service * service;
            std::unordered_map<std::string, const ::google::protobuf::MethodDescriptor *> methodTable;
        };

        //工作线程的线程池对象
        gchipe::DynamicThreadPool * threadPond;

        //本地的ip地址
        sockaddr_in * addr;
        
        //本地的sockfd
        int _fd;

        //io_uring相关结构体
        struct io_uring ring;
        struct io_uring_params params;

        //连接管理器
        std::unique_ptr<ConnectionManager> _connection_manager;

        //服务表
        std::unordered_map<std::string, MethodTable> serviceTable;

        //当前连接的状态信息
        int _active_conn = 0;
        int _max_conn = 0;
        static std::atomic<int> _status;
        RpcNodeInfo _node_info_rpc;

        //etcd相关
        ServiceRegister _service_register; 

        void onMessage(RequestInformation && ri);

        static void sigint_handler(int sig);  //Ctrl + C退出事件

        struct io_uring_sqe * getOneSqe();

        void asyncHandleRequest(RequestInformation && ri);

        template<typename _Callable>
        void asyncHandle(_Callable && fun){
            threadPond->execute(std::forward<_Callable>(fun));
        }

        std::string getNodeInfo();

    public:
        RpcProvider();
        
        ~RpcProvider();

        void Notify(::google::protobuf::Service *);

        void loop();        
    };
};

#endif