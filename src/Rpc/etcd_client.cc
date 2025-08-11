#include "Rpc/etcd_client.h"
#include "Rpc/rpcprovider.h"
#include "Rpc/net_base.h"
#include <iostream>
#include <functional>

namespace GcRpc{
    EtcdClient::EtcdClient(const std::string etcd_url, EventLoop * loop):_etcd_client_url(etcd_url), _loop(loop){
        assert(loop); 
        _curl = curl_easy_init();
        if(!_curl){
            throw("curl can not be initiated");
        }

        _headers = curl_slist_append(_headers, "Content-Type: application/json");

        curl_easy_setopt(_curl, CURLOPT_POST, 1L);   //-X post
        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _headers);
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_response_data);
        curl_easy_setopt(_curl, CURLOPT_TIMEOUT, 5L);  //5s的超时时间

        //TODO:curl发送测试心跳
    }
        
    EtcdClient::~EtcdClient() noexcept{
        curl_slist_free_all(_headers);
        curl_easy_cleanup(_curl);
    }

    void EtcdClient::do_io_uring(int last_res, int last_event_type) {

    }

    //以下函数均需异步调用
    void EtcdClient::leaseGrant(int ttl){
        _loop->asyncHandle(std::bind(&EtcdClient::async_leaseGrant, this, ttl));
    }

    void EtcdClient::put(const std::string & key, const std::string & value, bool with_lease){
        _loop->asyncHandle(std::bind(&EtcdClient::async_put, this, key, value, with_lease));
    }

    void EtcdClient::get(const std::string & key, bool prefix){
        _loop->asyncHandle(std::bind(&EtcdClient::async_get, this, key, prefix));
    }

    void EtcdClient::leastTimeToLive(int64_t lease_id){
        _loop->asyncHandle(std::bind(&EtcdClient::async_leastTimeToLive, this, lease_id));
    }

    void EtcdClient::leaseRevoke(int64_t lease_id){
        _loop->asyncHandle(std::bind(&EtcdClient::async_leaseRevoke, this, lease_id));
    }

    //以下函数均需异步调用
    void EtcdClient::async_leaseGrant(int ttl){
        json request = {
            {"TTL", ttl}
        };
        curlRequest("/v3/lease/grant", request);
    }

    void EtcdClient::async_put(const std::string & key, const std::string & value, bool with_lease){
        json request = {
            {"key", base64Encoding(key)},
            {"value", base64Encoding(value)}
        };
        if(with_lease){
            request["lease"] = std::to_string(_lease_id);
        }
        curlRequest("/v3/kv/put", request);
    }

    void EtcdClient::async_get(const std::string & key, bool prefix){
        json request = {
            {"key", base64Encoding(key)}
        };
        if(prefix){
            request["range_end"] = base64Encoding(key + "\xff");
        }
        curlRequest("/v3/kv/get", request);
    }

    void EtcdClient::async_leastTimeToLive(int64_t lease_id){
        json request = {
            {"ID", std::to_string(lease_id)}
        };
        curlRequest("/v3/lease/keepalive", request);
    }

    void EtcdClient::async_leaseRevoke(int64_t lease_id){
        json request = {
            {"ID", std::to_string(lease_id)}
        };
        curlRequest("/v3/lease/revoke", request);
    }

    //同步操作
    void EtcdClient::curlRequest(const std::string & path, const json & request){
        std::string url = _etcd_client_url + path;
        std::string request_json = request.dump();

        curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());   //url
        curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, request_json.c_str());   //-d 

        CURLcode res = curl_easy_perform(_curl);

        int http_code;
        if(res != CURLE_OK) {
            std::cerr << "cURL error: " << std::string(curl_easy_strerror(res));
        } else {
            curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &http_code);
            if(http_code == 200){  //成功响应
                parserResponse(_response_data.retrieveAllBufferAsString());
            } else {
                std::cerr << "HTTP Response Error: " << http_code << std::endl;
            }
        }
    }

    void EtcdClient::parserResponse(const std::string && response){
        try{
            json j = json::parse(response);
            if(j.contains("error") && !j["error"].get<std::string>().empty()){
                //有错误：
                std::cerr << "Etcd error: " << j["error"].get<std::string>() << std::endl; 
                return;
            }

            if(j.contains("result")){
                _lease_id = j["result"]["ID"].get<int64_t>();
            }

        } catch (const std::exception & e){
            std::cerr << "Etcd parserResponse error: " << e.what() << std::endl;
        }
    }

    size_t EtcdClient::WriteCallback(void *contents, size_t size, size_t nmemb, Buffer *s){
        size_t newLength = size * nmemb;
        s->append((char*)contents, newLength);
        return newLength;
    }
}