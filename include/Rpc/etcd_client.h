#ifndef ETCD_CLIENT_H
#define ETCD_CLIENT_H
#include <unordered_map>
#include <functional>
#include <queue>
#include "thirdParty/curl/curl.h" // 使用 libcurl 处理 HTTP
#include "thirdParty/nlohmann/json.hpp"


//TODO:基于io_uring异步调用的etcd连接客户端
//需要用到json库处理json数据
//用curl库来实现调用
namespace GcRpc{
    
}

#endif