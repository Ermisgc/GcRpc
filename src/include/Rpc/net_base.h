#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string>
#include "rpc_node_info.pb.h"
// #include <etcd/Response.hpp>

namespace GcRpc{
    /**
     * @brief provides a cpp interface for sockaddr
     *
     * @example
     * InetAddr addr("127.0.0.1",8080);
     * bind(sockfd,addr.addr(),addr.size());
     * @param ip 32bit ip address, typed as uint32_t or char *
     * @param port 16bit port number, typed as uint16_t or char *
     */
    class InetAddr{
        sockaddr_in m_addr;  //4 bytes;
        //TODO:add ipv6 support
    public:
        InetAddr() = delete;
        
        explicit InetAddr(uint32_t _ip, uint16_t _port);
        InetAddr(const char * _ip, const char * _port);
        InetAddr(uint32_t _ip, const char * _port);
        InetAddr(const char * _ip, uint16_t _port);
        InetAddr(const InetAddr & );
        
        inline sockaddr* addr(){
            m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            return reinterpret_cast<sockaddr*>(&m_addr);
        }

        inline socklen_t size() const {return sizeof(sockaddr_in);}

        inline uint32_t ip() const { return ntohl(m_addr.sin_addr.s_addr);}
        inline std::string ipString() const  {return std::to_string(m_addr.sin_addr.s_addr);}
        inline uint16_t port() const { return ntohs(m_addr.sin_port);}
        inline std::string portString() const {return std::to_string(m_addr.sin_port);}
    };

    //服务节点
    struct ServiceEndpoint{
        // int port;
        int active_conns;
        int max_conns;
        //cpu_util 和 memory_util这两个利用率，说实话在我这里用处不是很大，因为服务端获取这个消息需要进行系统调用
        //从而且是cpu和memory两次系统调用，要额外发生4次用户态、内核态的切换，代价有点大，因此只考虑active_conns和max_conns
        float cpu_util;  
        float memory_util;  
        int load_score;   //应该是
        int circuit_time;
        std::string serviceName;
        InetAddr * addr;

        std::string fullAddress() const {
            return addr->ipString() + ":" + addr->portString();
        }
    };

    enum LoadBalancerType{
        LBT_ROBIN = 0,
        LBT_RANDOM = 1,
        LBT_SCORE = 2
    };

    //从etcd返回的节点value（经过protobuf编译）中, 
    //TODO:还没写完呢
    // ServiceEndpoint parserNodeFromEtcd(const etcd::Value & value){
    //     auto key = value.key();
    //     auto v = value.as_string();
    //     GcRpc::RpcNodeInfo node_info;
    //     node_info.ParseFromString(v);

    //     ServiceEndpoint ret;
    //     // ret.ip =
    //     return ret; 
    // }
}