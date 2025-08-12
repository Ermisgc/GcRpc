#ifndef SERVICE_REGISTER_H
#define SERVICE_REGISTER_H
#include "Rpc/channel.h"
#include "Rpc/etcd_client.h"
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <linux/time_types.h>

#define ETCD_EVENT_TIMEOUT 0
#define ETCD_EVENT_CHECK_HEALTH 1
#define ETCD_EVENT_KEEPALIVE 2
#define ETCD_EVENT_REVOKE 3

namespace GcRpc{
    class EtcdClient;
    //遵循RAII管理租约，构造时创建租约，提供函数停止租约/开始，设置租约内容，析构时析构租约
    class LeaseManager{
        EtcdClient & _etcd; //etcd客户端的指针
        std::string _lease_id = "0";
        int _ttl;
    public:
        LeaseManager(EtcdClient & etcd, int ttl);
        ~LeaseManager() noexcept;

        //在某个本租约下put某个key
        void put(const std::string & key, const std::string & value);

        void healthCheck();

        void cancelLease();
    };

    //遵循RAII的etcd服务注册者
    class ServiceRegister : public UringChannel{
        std::unique_ptr<LeaseManager> _lease_manager;
        EtcdClient _etcd;
        int _ttl;
        struct __kernel_timespec _ts;

        EventLoop * _loop;
        std::unordered_set<std::string> _services_name;
        std::string _node_info_rpc;
        std::mutex _services_name_mutex;
        std::atomic<bool> _stoping = false;

    public:
        ServiceRegister( EventLoop * event_loop, int ttl, const std::string & ip_and_port = "http://127.0.0.1:2379");
        ~ServiceRegister() noexcept;

        void registerService(const std::string & key);

        void unregisterService(const std::string & key);

        void startRegister();

        void stopRegister();

        void updateNodeInfo();

        virtual void do_io_uring(int res, int event_type) override;

    protected:
        void callback_timeout(int res);

        void callback_healthCheck(int res);

    private:
        void set_io_uring_timer();
    };

}


#endif