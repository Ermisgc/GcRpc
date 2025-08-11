#ifndef ETCD_CLIENT_H
#define ETCD_CLIENT_H
#include <unordered_map>
#include <functional>
#include <queue>
#include <string>
#include <future>
#include "thirdParty/curl/curl.h" // 使用 libcurl 处理 HTTP
#include "thirdParty/nlohmann/json.hpp"
#include "Rpc/channel.h"


//TODO:基于io_uring异步调用的etcd连接客户端
//需要用到json库处理json数据
//用curl库来实现调用
namespace GcRpc{
    using json = nlohmann::json;
    class EtcdResponse;

    class EtcdClient : public UringChannel{
        CURL * _curl;
        curl_slist * _headers;
        std::string _etcd_client_url;
        EventLoop * _loop;
        uint64_t _lease_id = 0;
        Buffer _response_data;

    public:
        EtcdClient(const std::string etcd_url, EventLoop * loop);
        ~EtcdClient() noexcept;

        //以下函数均需异步调用
        void leaseGrant(int ttl);

        void put(const std::string & key, const std::string & value, bool with_lease = false);

        void get(const std::string & key, bool prefix = false);

        void leastTimeToLive(int64_t lease_id);

        void leaseRevoke(int64_t lease_id);

        virtual void do_io_uring(int last_res, int last_event_type) override;

    private:

        //以下函数均需异步调用
        void async_leaseGrant(int ttl);

        void async_put(const std::string & key, const std::string & value, bool with_lease = false);

        void async_get(const std::string & key, bool prefix = false);

        void async_leastTimeToLive(int64_t lease_id);

        void async_leaseRevoke(int64_t lease_id);

        //curl请求
        void curlRequest(const std::string & path, const json & request); 

        void parserResponse(const std::string && response);

        //curl回调函数，接收返回来的数据，这里的s是我传入的指针
        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, Buffer *s);
    };
}

#endif