#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string>
#include "rpc_node_info.pb.h"
#include "data_structure/hashheap.hpp"
#include <etcd/Response.hpp>

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
        friend class TcpConnection;
        sockaddr_in m_addr;  //4 bytes;
        //TODO:add ipv6 support
    public:
        InetAddr() = delete;
        
        explicit InetAddr(uint32_t _ip, uint16_t _port);
        InetAddr(const char * _ip, const char * _port);
        InetAddr(uint32_t _ip, const char * _port);
        InetAddr(const char * _ip, uint16_t _port);
        InetAddr(const InetAddr & );
        
        inline sockaddr* addr() const {
            // m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            // return reinterpret_cast<sockaddr*>(&m_addr);
            return (sockaddr *)&m_addr;  //直接显式转换
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
        uint32_t active_conns;
        uint32_t max_conns;
        //cpu_util 和 memory_util这两个利用率，说实话在我这里用处不是很大，因为服务端获取这个消息需要进行系统调用
        //从而且是cpu和memory两次系统调用，要额外发生4次用户态、内核态的切换，代价有点大，因此只考虑active_conns和max_conns
        float cpu_util;  
        float memory_util;  
        float load_score;   //负载分数
        int circuit_time;
        // std::string serviceName;
        InetAddr * addr;

        std::string fullAddress() const {
            return addr->ipString() + ":" + addr->portString();
        }

        ~ServiceEndpoint() noexcept{
            if(addr) delete addr;
        }
    };

    enum LoadBalancerType{
        LBT_ROBIN = 0,
        LBT_RANDOM = 1,
        LBT_SCORE = 2
    };

    using EndPointList = gcdst::HashHeap<std::string, float>;

    void parserNodeFromEtcd(const etcd::Value & value, ServiceEndpoint & ep);

    std::string getLocalIP();

    unsigned char base64_reverse(char input);

    std::string base64Encoding(const std::vector<uint8_t> & input);

    std::string base64Encoding(const std::string & input);

    std::vector<uint8_t> base64Decoding(const std::string & input);
}