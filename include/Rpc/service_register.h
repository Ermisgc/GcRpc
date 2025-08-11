#ifndef SERVICE_REGISTER_H
#define SERVICE_REGISTER_H
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>

namespace GcRpc{
    //遵循RAII管理租约，构造时创建租约，提供函数停止租约/开始，设置租约内容，析构时析构租约
    class LeaseManager{
        std::shared_ptr<etcd::KeepAlive> _lease_keeper;
        int64_t _lease_id;
        etcd::SyncClient & _etcd; //etcd客户端的指针
    public:
        LeaseManager(etcd::SyncClient & etcd, int ttl);
        ~LeaseManager() noexcept;

        //在某个本租约下put某个key
        bool put(const std::string & key, const std::string & value);

        void healthCheck();

        void cancelLease();
    };

    //遵循RAII的etcd服务注册者
    class ServiceRegister{
        std::unique_ptr<LeaseManager> _lease_manager;
        etcd::SyncClient _etcd;
    public:
        ServiceRegister(const std::string & ip_and_port = "http://127.0.0.1:2379");
        ~ServiceRegister() noexcept;

        bool registerService(const std::string & key, const std::string & value);

        void startRegister();

        void stopRegister();

        void healthCheck();
    };

}


#endif