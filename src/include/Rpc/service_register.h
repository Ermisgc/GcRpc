#ifndef SERVICE_REGISTER_H
#define SERVICE_REGISTER_H
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>

namespace GcRpc{
    //遵循RAII管理租约，构造时创建租约，提供函数停止租约/开始，设置租约内容，析构时析构租约
    class LeaseManager{
        std::shared_ptr<etcd::KeepAlive> _lease_keeper;
        int64_t _lease_id;
        etcd::Client * _etcd; //etcd客户端的指针
    public:
        LeaseManager(etcd::Client * etcd, int ttl);
        ~LeaseManager();

        //在某个本租约下put某个key
        void put(const std::string & key, const std::string & value);

        void healthCheck();

        void cancelLease();
    };



}


#endif