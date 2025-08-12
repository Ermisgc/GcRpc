#include "Rpc/etcd_client.h"
#include "Rpc/rpcprovider.h"
#include "Rpc/net_base.h"
#include <iostream>
#include <functional>

namespace GcRpc{
    EtcdClient::EtcdClient(const std::string etcd_url, EventLoop * loop):_etcd_client_url(etcd_url), _loop(loop){
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

    //以下函数均需异步调用
    std::future<std::string> EtcdClient::async_leaseGrant(int ttl){
        if(!_loop) return std::future<std::string>();
        std::promise<std::string> p;
        _loop->asyncHandle(std::bind(&EtcdClient::leaseGrant, this, ttl));
        return p.get_future();
    }

    void EtcdClient::async_put(const std::string & key, const std::string & value, std::string lease_id){
        if(!_loop) return;
        _loop->asyncHandle(std::bind(&EtcdClient::put, this, key, value, lease_id));
    }

    std::future<std::vector<std::pair<std::string, std::string>>> EtcdClient::async_get(const std::string & key, bool prefix){
        if(!_loop) return std::future<std::vector<std::pair<std::string, std::string>>>();
        std::promise<std::vector<std::pair<std::string, std::string>>> p;
        _loop->asyncHandle(std::bind(&EtcdClient::get, this, key, prefix));
        return p.get_future();
    }

    std::future<int> EtcdClient::async_leastTimeToLive(const std::string & lease_id){
        if(!_loop) return std::future<int>();
        std::promise<int> p;
        _loop->asyncHandle(std::bind(&EtcdClient::leastTimeToLive, this, lease_id));
        return p.get_future();
    }

    void EtcdClient::async_leaseRevoke(const std::string & lease_id){
        if(!_loop) return;
        _loop->asyncHandle(std::bind(&EtcdClient::leaseRevoke, this, lease_id));
    }

    std::string EtcdClient::leaseGrant(int ttl){
        json request = {
            {"TTL", ttl}
        };
        std::string response = curlRequest("/v3/lease/grant", request);
        try{
            // printResponse(response);
            json j = json::parse(response);
            if(j.contains("error") && !j["error"].get<std::string>().empty()){
                //有错误：
                std::cerr << "Etcd error: " << j["error"].get<std::string>() << std::endl; 
                return "";
            }

            if(j.contains("ID")){
                return j["ID"].get<std::string>();
            }
            return "";
        } catch (const std::exception & e){
            std::cerr << "Etcd parserResponse error: " << e.what() << std::endl;
            return "";
        }
    }

    void EtcdClient::put(const std::string & key, const std::string & value, std::string lease_id){
        json request = {
            {"key", base64Encoding(key)},
            {"value", base64Encoding(value)}
        };
        if(lease_id != "0"){
            request["lease"] = lease_id;
        }
        
        curlRequest("/v3/kv/put", request);
    }

    std::vector<std::pair<std::string, std::string>> EtcdClient::get(const std::string & key, bool prefix){
        json request = {
            {"key", base64Encoding(key)}
        };
        if(prefix){
            request["range_end"] = base64Encoding(key + "\xff");
        }
        std::string response = curlRequest("/v3/kv/range", request);
        std::vector<std::pair<std::string, std::string>> ret;
        parserResponseForGet(response, ret);
        return ret;
    }

    int EtcdClient::leastTimeToLive(std::string lease_id){
        json request = {
            {"ID", lease_id}
        };
        std::string response = curlRequest("/v3/lease/keepalive", request);
        try{
            json j = json::parse(response);
            if(j.contains("error") && !j["error"].get<std::string>().empty()){
                //有错误：
                std::cerr << "Etcd error: " << j["error"].get<std::string>() << std::endl; 
                return -1;
            }

            if(j.contains("TTL")){
                return j["TTL"].get<int>();
            }
            return -1;
        } catch (const std::exception & e){
            std::cerr << "Etcd parserResponseForGet error: " << e.what() << std::endl;
            return -1;
        }
    }

    void EtcdClient::leaseRevoke(std::string lease_id){
        json request = {
            {"ID", lease_id}
        };
        curlRequest("/v3/lease/revoke", request);
    }

    //同步操作
    std::string EtcdClient::curlRequest(const std::string & path, const json & request){
        std::string url = _etcd_client_url + path;
        std::string request_json = request.dump();

        // std::cout << request_json << std::endl;

        curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());   //url
        curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, request_json.c_str());   //-d 

        CURLcode res = curl_easy_perform(_curl);

        int http_code;
        if(res != CURLE_OK) {
            std::cerr << "cURL error: " << std::string(curl_easy_strerror(res));
        } else {
            curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &http_code);
            if(http_code == 200){  //成功响应
                return _response_data.retrieveAllBufferAsString();
            } else {
                std::cerr << "HTTP Response Error: " << http_code << std::endl;
            }
        }
        return "";
    }

    size_t EtcdClient::WriteCallback(void *contents, size_t size, size_t nmemb, Buffer *s){
        size_t newLength = size * nmemb;
        s->append((char*)contents, newLength);
        return newLength;
    }

    void EtcdClient::printResponse(const std::string & response){
        std::cout << response << std::endl;
    }

    void EtcdClient::parserResponseForGet(const std::string & response, std::vector<std::pair<std::string, std::string>> & ret){
        try{
            json j = json::parse(response);
            if(j.contains("error") && !j["error"].get<std::string>().empty()){
                //有错误：
                std::cerr << "Etcd error: " << j["error"].get<std::string>() << std::endl; 
                return;
            }

            if(j.contains("kvs")){
                for(auto & i : j["kvs"]){
                    ret.emplace_back(base64Decoding(i["key"].get<std::string>()), base64Decoding(i["value"].get<std::string>()));
                }
            }
        } catch (const std::exception & e){
            std::cerr << "Etcd parserResponseForGet error: " << e.what() << std::endl;
        }
    }
}