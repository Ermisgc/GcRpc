#include "Rpc/service_seeker.h"
#include <functional>

namespace GcRpc{
    ServiceSeeker::ServiceSeeker(std::shared_ptr<LoadBalancer> balancer): \
        _etcd(GcRpcApplication::Load("etcd_addr")), balancer_(balancer), \
        _watcher_thread(std::bind(&ServiceSeeker::thread_working, this)) {        
        _watcher = std::make_unique<etcd::Watcher>(
            _etcd,
            "/services/",  //监听所有services前缀
            [this](etcd::Response resp){
                auto events = resp.events();
                for(const auto & event : events){
                    auto kv_ = event.kv();
                    std::string key = kv_.key();
                    //然后从key当中提取出来service_name, key应该是:/services/serviceA/node1
                    int pos1 = key.find('/', 2);  //pos1是从第二个字符开始往后找
                    int pos2 = key.rfind('/');
                    std::string service_name = key.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::string node_name = key.substr(pos2 + 1, key.size() - pos2 - 1);

                    std::lock_guard<std::mutex> map_locker(_watch_mtx); 
                    if(auto itr = _endpoint_map.find(service_name); itr == _endpoint_map.end()){
                        //service_name并不在关心的服务里，直接返回
                        continue;
                    } else {
                        auto & meta = itr->second;

                        //这里在写入时应该是独写，要加锁
                        switch ( event.event_type()) {
                            case etcd::Event::EventType::PUT : {
                                {   //这里被设定为唯一会写的地方，本线程其它的地方调用这个时不用加锁
                                    std::lock_guard<std::mutex> locker(meta->pond_mtx);
                                    if(auto it = meta->nodes_pond.find(node_name); it != meta->nodes_pond.end()){  //已经存在，那么就只做修改，否则的话，要新增   
                                        ServiceEndpoint node = it->second;
                                        parserNodeFromEtcd(kv_, node);
                                    } else {   //要监听的key并不存在
                                        ServiceEndpoint new_node;
                                        parserNodeFromEtcd(kv_, new_node);
                                        meta->nodes_pond[node_name] = std::move(new_node);
                                    }
                                }
                                meta->node_list.insert(node_name, meta->nodes_pond[node_name].load_score);
                                break;
                            }

                            case etcd::Event::EventType::DELETE_: {  
                                {
                                    std::lock_guard<std::mutex> locker(meta->pond_mtx);
                                    if(auto it = meta->nodes_pond.find(node_name); it != meta->nodes_pond.end()){
                                        meta->nodes_pond.erase(node_name);
                                    }
                                }
                                meta->node_list.remove(node_name);
                                break;
                            }

                            default:
                                break;
                        }
                    }

                }
            },
            true
        );
    }

    bool ServiceSeeker::watch(const std::string & service_name){
        //增加某个service到watch列表
        std::lock_guard<std::mutex> map_locker(_watch_mtx);
        if(_endpoint_map.find(service_name) != _endpoint_map.end()) return false;  //已经有了，直接返回吧
        
        auto snm = std::make_unique<ServiceNodeMeta>();
        _endpoint_map[service_name] = std::move(snm);
        auto & meta = _endpoint_map[service_name];  //这里是引用        

        //直接采用同步调用，因此这里直接get，这里需要去阻塞，获得服务的初始列表
        std::string service_prefix = "/services/" + service_name + "/";
        auto resp = _etcd.ls(service_prefix).get();
        
        for(auto & kv : resp.values()){
            std::string key = kv.key();
            int pos2 = key.rfind('/');
            // std::string service_name = key.substr(pos1 + 1, pos2 - pos1 - 1);
            std::string node_name = key.substr(pos2 + 1, key.size() - pos2 - 1);

            ServiceEndpoint new_node;
            parserNodeFromEtcd(kv, new_node);
            {
                std::unique_lock<std::mutex> locker(meta->pond_mtx);
                meta->nodes_pond[node_name] = std::move(new_node);
            }

            meta->node_list.insert(node_name, new_node.load_score);
        }
        return true;
    }

    //将service剔除出Watcher队列
    void ServiceSeeker::unwatch(const std::string & service_name){
        std::lock_guard<std::mutex> map_locker(_watch_mtx);
        _endpoint_map.erase(service_name);
    }

    ServiceSeeker::~ServiceSeeker(){
        _destorying.store(true);
        // stopWatch();  //先停止再通知，否则容易导致线程退出时还在notify
        _watch_cv.notify_all();
        while(!_watcher->Cancelled()) _watcher->Cancel();
        
        if(_watcher_thread.joinable()) _watcher_thread.join();  //析构掉线程
    }

    void ServiceSeeker::thread_working() noexcept{
        while(!_destorying){
            {
                std::unique_lock<std::mutex> locker(_watch_mtx);
                _watch_cv.wait(locker, [this]()->bool{ return _watching || _destorying; });
                if(_destorying) break;
            }

            _watcher->Wait();
        }
    }

    inline std::optional<InetAddr> ServiceSeeker::getServiceNode(const std::string & service_name) {
        std::lock_guard<std::mutex> locker(_watch_mtx);
        // auto itr = _endpoint_map.end();
        auto itr = _endpoint_map.find(service_name);
        if(itr == _endpoint_map.end()) return std::nullopt;  //并没有找到对应的服务，直接返回

        auto node_name = balancer_->select(itr->second->node_list);
        if(!node_name.has_value()) return std::nullopt;
        {
            std::unique_lock<std::mutex> locker(itr->second->pond_mtx);
            if(auto it = itr->second->nodes_pond.find(node_name.value()); it != itr->second->nodes_pond.end()){
                return *it->second.addr;
            } else {
                return std::nullopt;
            }
        }
    }
}