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


//基于io_uring调用的etcd连接客户端
//需要用到json库处理json数据
//用curl库来实现调用

namespace GcRpc{
    using json = nlohmann::json;

    class EtcdClient {
        friend class ServiceRegister;
        friend class LeaseManager;

        CURL * _curl;
        curl_slist * _headers = nullptr;
        std::string _etcd_client_url;
        EventLoop * _loop;
        // std::string _lease_id;
        Buffer _response_data;

    public:
        EtcdClient(const std::string etcd_url, EventLoop * loop = nullptr);
        ~EtcdClient() noexcept;

        //以下函数均需异步调用
        std::future<std::string> async_leaseGrant(int ttl);

        void async_put(const std::string & key, const std::string & value, std::string lease_id = "0");

        std::future<std::vector<std::pair<std::string, std::string>>> async_get(const std::string & key, bool prefix = false);

        std::future<int> async_leastTimeToLive(const std::string & lease_id);

        void async_leaseRevoke(const std::string & lease_id);

        //以下函数为同步调用
        std::string leaseGrant(int ttl);

        void put(const std::string & key, const std::string & value, std::string lease_id = "0");

        std::vector<std::pair<std::string, std::string>> get(const std::string & key, bool prefix = false);

        int leastTimeToLive(std::string lease_id);

        void leaseRevoke(std::string lease_id);
    private:
        //curl请求
        std::string curlRequest(const std::string & path, const json & request); 

        void parserResponseForGet(const std::string & response, std::vector<std::pair<std::string, std::string>> & );

        //curl回调函数，接收返回来的数据，这里的s是我传入的指针
        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, Buffer *s);

        void printResponse(const std::string & response);
    };
}

#endif