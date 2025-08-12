#include "Rpc/service_register.h"
#include "Rpc/rpcprovider.h"

namespace GcRpc{

    LeaseManager::LeaseManager(EtcdClient & etcd, int ttl): _etcd(etcd), _ttl(ttl){
        _lease_id = _etcd.leaseGrant(ttl);  //设置租约为20s
    }

    LeaseManager::~LeaseManager() noexcept{
        cancelLease();
    }

    void LeaseManager::put(const std::string & key, const std::string & value){
        _etcd.put(key, value, _lease_id);
    }

    void LeaseManager::healthCheck(){
        //检查etcd是否健康，如果租约过期了，续租，否则的话要手动keepalive
        _etcd.leastTimeToLive(_lease_id);
        if(_etcd.leastTimeToLive(_lease_id) < 0){
            //租约过期了
            _lease_id = _etcd.leaseGrant(_ttl);
        }
    }

    void LeaseManager::cancelLease(){
        _etcd.leaseRevoke(_lease_id);
    }

    ServiceRegister::ServiceRegister(EventLoop * event_loop, int ttl, const std::string & ip_and_port):_etcd(ip_and_port), _ttl(ttl){
        assert(event_loop);
        _loop = event_loop;

        _ts.tv_sec = ttl / 4;
        _ts.tv_nsec = 0;
    }

    ServiceRegister::~ServiceRegister() noexcept{
        //std::unique_ptr和etcd都满足自动析构
    }

    void ServiceRegister::registerService(const std::string & key){
        std::lock_guard<std::mutex> lock(_services_name_mutex);
        _services_name.insert(key);
    }

    void ServiceRegister::unregisterService(const std::string & key){
        std::lock_guard<std::mutex> lock(_services_name_mutex);
        _services_name.erase(key);
    }

    void ServiceRegister::startRegister(){
        if(_lease_manager) _lease_manager.reset();
        _stoping = false;
        _lease_manager = std::make_unique<LeaseManager>(_etcd, _ttl);  //TODO:租约时间应该是一sqe个参数
        set_io_uring_timer();
    }

    void ServiceRegister::stopRegister(){
        if(!_lease_manager) return;
        _stoping = true;
        _lease_manager->cancelLease();
    }

    void ServiceRegister::updateNodeInfo(){
        if(_stoping) return;
        _lease_manager->healthCheck();
        _node_info_rpc = _loop->getNodeInfo();

        {
            std::lock_guard<std::mutex> lock(_services_name_mutex);
            for(auto & it : _services_name){
                if(_stoping) return;

                std::string key = "/services/" + it + "/" + getLocalIP();
                _lease_manager->put(key, _node_info_rpc);
            }
        }

        callback_healthCheck(0);
    }

    void ServiceRegister::do_io_uring(int res, int event_type){
        //ServiceRegister在事件循环里面做三件事情
        //1. 向io_uring设置timeout事件，每隔ttl / 4秒向io_uring发送一次心跳，上传实际真实数据
        //2. timeout触发后，检查租约是否失效（工作线程）3. 如果租约失效了，更新租约，否则原租约续租
        //3. 有租约的情况下：异步实现put数据更新

        switch (event_type)
        {
        case ETCD_EVENT_TIMEOUT:
            //设置的timeout时间到了
            callback_timeout(res);
            break;
        
        default:
            break;
        }
    }

    void ServiceRegister::callback_timeout(int res){
        if(res == 0){
            _loop->asyncHandle(std::bind(&ServiceRegister::updateNodeInfo, this));
        } else {
            std::cerr << "timeout error" << std::endl;
            stopRegister();
        }
    }

    void ServiceRegister::callback_healthCheck(int res){
        if(res == 0 && !_stoping){
            set_io_uring_timer();
        } else {
            std::cerr << "health check error" << std::endl;
            stopRegister();
        }
    }

    void ServiceRegister::set_io_uring_timer(){
        auto sqe = _loop->getOneSqe();
        sqe->user_data = getIOUringUserdata(this, ETCD_EVENT_TIMEOUT);

        io_uring_prep_timeout(sqe, &_ts, 0, 0);
    }
}