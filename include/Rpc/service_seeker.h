#include "Rpc/gcrpcapplication.h"
#include "Rpc/net_base.h"
#include <etcd/Client.hpp>
#include <etcd/Watcher.hpp>
#include <thread>
#include "Rpc/load_balancer.h"
#include <memory>

namespace GcRpc{

    //TODO:ServiceSeeker的设计不适合RAII，etcd_client资源应该在内部完成
    class ServiceSeeker{
        struct ServiceNodeMeta{
            std::unordered_map<std::string, ServiceEndpoint> nodes_pond;
            std::mutex pond_mtx;
            EndPointList node_list;
        };

        std::unordered_map<std::string, std::unique_ptr<ServiceNodeMeta>> _endpoint_map;

        std::shared_ptr<LoadBalancer> balancer_;


        //通过etcd的watch机制从etcd服务器拉取对应键值的列表
        etcd::Client _etcd;
        std::unique_ptr<etcd::Watcher> _watcher;  //这里选择1 watcher 多 key的配置方式

        //这里得用指针，因为Watcher类禁止拷贝，后面就没办法reset了，
        //后续可以改为unique_ptr(但是因为和线程有析构顺序，因此选择手动控制)
        std::atomic<bool> _watching;
        std::condition_variable _watch_cv;
        std::thread _watcher_thread;
        std::mutex _watch_mtx;

        std::atomic<bool> _destorying; //destroying指示符用于析构本类时停止线程
    public:
        ServiceSeeker(std::shared_ptr<LoadBalancer> balancer);
        ~ServiceSeeker() noexcept;
        
        ServiceSeeker(const ServiceSeeker &) = delete;
        ServiceSeeker(ServiceSeeker &&) = delete;
        
        bool watch(const std::string & service_name);

        void unwatch(const std::string & service_name);

        std::optional<InetAddr> getServiceNode(const std::string & service_name);

    private:
        void thread_working() noexcept;
    };
}