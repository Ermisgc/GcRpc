#include "Rpc/gcrpcapplication.h"
#include "Rpc/net_base.h"
#include <etcd/Client.hpp>
#include <etcd/Watcher.hpp>
#include <thread>
#include "Rpc/load_balancer.h"
#include <memory>

namespace GcRpc{
    class ServiceSeeker{
        std::string serviceName_;
        std::shared_ptr<LoadBalancer> balancer_;

        std::unordered_map<std::string, ServiceEndpoint> nodes_pond_;
        EndPointList nodes_;
        std::mutex mtx_;  //for protecting unordered_map

        //通过etcd的watch机制从etcd服务器拉取对应键值的列表
        std::shared_ptr<etcd::Client> etcd_;
        //这里得用指针，因为Watcher类禁止拷贝，后面就没办法reset了，
        //后续可以改为unique_ptr(但是因为和线程有析构顺序，因此选择手动控制)
        etcd::Watcher * watcher_;  
        std::atomic<bool> watching_;
        std::condition_variable watch_cv;
        std::thread watcher_thread_;
        std::mutex watch_mtx_;

        std::atomic<bool> destorying_; //destroying指示符用于析构本类时停止线程
    public:
        ServiceSeeker(std::shared_ptr<etcd::Client> etcd_client, const std::string & serviceName, std::shared_ptr<LoadBalancer> balancer);
        ~ServiceSeeker() noexcept;
        
        ServiceSeeker(const ServiceSeeker &) = delete;
        ServiceSeeker(const ServiceSeeker &&) = delete;
        
        void startWatch();

        void stopWatch();

        std::optional<InetAddr> getService();
        
        //这里选择用reset，可以让系统对服务进行复用
        void reset(std::shared_ptr<etcd::Client> etcd_client, const std::string & serviceName, std::shared_ptr<LoadBalancer> balancer);
    private:
        void thread_working() noexcept;

        etcd::Watcher * createWatcher();

        void initial_service_nodes();
    };
}