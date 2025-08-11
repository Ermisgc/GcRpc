#include "Rpc/service_register.h"

namespace GcRpc{

    LeaseManager::LeaseManager(etcd::SyncClient & etcd, int ttl): _etcd(etcd){
        _lease_keeper = _etcd.leasekeepalive(ttl);
        _lease_id = _lease_keeper->Lease();
    }

    LeaseManager::~LeaseManager() noexcept{
        cancelLease();
        _etcd.leaserevoke(_lease_id);
    }

    bool LeaseManager::put(const std::string & key, const std::string & value){
        auto put_resp = _etcd.put(key, value, _lease_id);

        if(!put_resp.is_ok()){
            return false;
        } 
        return true;
    }

    void LeaseManager::healthCheck(){
        //检查etcd是否健康，如果租约过期了，要及时续租
        try{
            _lease_keeper->Check();
        } catch(const std::exception & e) {
            std::cerr << e.what() << std::endl;
            throw("Lease is not alive");  //TODO:此处应执行应急操作
        }
    }

    void LeaseManager::cancelLease(){
        _lease_keeper->Cancel();
    }

    ServiceRegister::ServiceRegister(const std::string & ip_and_port):_etcd(ip_and_port){
        auto response = _etcd.get("health_check_key");
        if(response.is_ok()){
            throw("ETCD SERVICE NOT PROVIDED");
        }
    }

    ServiceRegister::~ServiceRegister() noexcept{
        //std::unique_ptr和etcd都满足自动析构
    }

    bool ServiceRegister::registerService(const std::string & key, const std::string & value){
        if(!_lease_manager) {
            return false;
        } else {
            return _lease_manager->put(key, value);
        }
    }

    void ServiceRegister::startRegister(){
        if(_lease_manager) _lease_manager.reset();
        _lease_manager = std::make_unique<LeaseManager>(_etcd, 20); //TODO:租约ttl的大小应该可以通过配置文件调控
    }

    void ServiceRegister::stopRegister(){
        if(!_lease_manager) return;
        _lease_manager->cancelLease();
        _lease_manager.reset();
    }

    void ServiceRegister::healthCheck(){
        if(!_lease_manager) return;
        _lease_manager->healthCheck();
    }
}