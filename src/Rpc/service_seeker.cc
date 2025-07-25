#include "Rpc/service_seeker.h"
#include <functional>

namespace GcRpc{
    ServiceSeeker::ServiceSeeker(std::shared_ptr<etcd::Client> client, const std::string & serviceName, std::shared_ptr<LoadBalancer> balancer): \
        etcd_(client), serviceName_(serviceName) , balancer_(balancer), \
        watcher_thread_(std::bind(&ServiceSeeker::thread_working, this)) {
        //初始化服务列表
        watcher_ = createWatcher();
        initial_service_nodes();
        startWatch();
    }

    ServiceSeeker::~ServiceSeeker(){
        destorying_.store(true);
        stopWatch();  //先停止再通知，否则容易导致线程退出时还在notify
        watch_cv.notify_all();

        if(watcher_thread_.joinable()) watcher_thread_.join();  //析构掉线程
        if(watcher_) delete watcher_;
    }

    void ServiceSeeker::startWatch(){
        std::lock_guard<std::mutex> lock(watch_mtx_);
        watching_.store(true);
        watch_cv.notify_one();
    }

    void ServiceSeeker::stopWatch(){
        watching_.store(false);
        {
            std::unique_lock<std::mutex> locker(watch_mtx_);
            if(!watcher_->Cancelled()) {
                watcher_->Cancel();  //停止时必须确保完全停止了再退出，否则会一直Cancel
            }  //这里要用锁来保护，以免仍处于函数调用阶段时watcher_被析构了
        }
    }

    void ServiceSeeker::thread_working() noexcept{
        while(!destorying_){
            {
                std::unique_lock<std::mutex> locker(watch_mtx_);
                watch_cv.wait(locker, [this]()->bool{ return watching_ || destorying_; });
                if(destorying_) break;
            }

            watcher_->Wait();
        }
    }

    inline std::optional<InetAddr> ServiceSeeker::getService(){
        auto node_name = balancer_->select(nodes_);
        if(!node_name.has_value()) return std::nullopt;
        {
            std::unique_lock<std::mutex> locker(mtx_);
            if(auto itr = nodes_pond_.find(node_name.value()); itr != nodes_pond_.end()){
                return *itr->second.addr;
            } else {
                return std::nullopt;
            }
        }
    }

    void ServiceSeeker::reset(std::shared_ptr<etcd::Client> etcd_client, const std::string & serviceName, std::shared_ptr<LoadBalancer> balancer){
        etcd_ = etcd_client;
        serviceName_ = serviceName;
        stopWatch();
        delete watcher_;
        balancer_ = balancer;
        watcher_ = createWatcher();
        {
            std::unique_lock<std::mutex> locker(mtx_);
            nodes_pond_.clear();
        }
        nodes_.clear();
        initial_service_nodes();
        startWatch();
    }

    etcd::Watcher * ServiceSeeker::createWatcher(){
        return new etcd::Watcher (
            *etcd_,
            "/services/" + serviceName_ + "/",
            [&](etcd::Response resp){
                auto events = resp.events();
                for(const auto & event : events){
                    auto kv_ = event.kv();
                    switch ( event.event_type())
                    {
                    case etcd::Event::EventType::PUT : {
                        {
                            std::unique_lock<std::mutex> locker(mtx_);
                            if(auto itr = nodes_pond_.find(kv_.key()); itr != nodes_pond_.end()){
                                ServiceEndpoint node = itr->second;
                                parserNodeFromEtcd(kv_, node);
                            } else {
                                ServiceEndpoint new_node;
                                parserNodeFromEtcd(kv_, new_node);
                                nodes_pond_[kv_.key()] = std::move(new_node);
                            }
                        }
                        nodes_.insert(kv_.key(), nodes_pond_[kv_.key()].load_score);
                        break;
                    }

                    case etcd::Event::EventType::DELETE_: {
                        std::unique_lock<std::mutex> locker(mtx_);
                        if(auto itr = nodes_pond_.find(kv_.key()); itr != nodes_pond_.end()){
                            nodes_.remove(kv_.key());
                            nodes_pond_.erase(kv_.key());
                        }
                        break;
                    }

                    default:
                        break;
                    }
                }
            },
            true
        );
    }

    void ServiceSeeker::initial_service_nodes(){
        std::string service_prefix = "/services/" + serviceName_ + "/";
        //直接采用同步调用，因此这里直接get，这里需要去阻塞，不然没办法正确提供服务
        auto resp = etcd_->ls(service_prefix).get();
        std::unique_lock<std::mutex> locker(mtx_);
        for(auto & kv : resp.values()){
            ServiceEndpoint new_node;
            parserNodeFromEtcd(kv, new_node);

            nodes_.insert(kv.key(), new_node.load_score);
            nodes_pond_[kv.key()] = std::move(new_node);
        }
    }
}