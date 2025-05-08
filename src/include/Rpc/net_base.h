#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string>
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

        inline socklen_t size(){return sizeof(sockaddr_in);}

        inline uint32_t ip(){ return ntohl(m_addr.sin_addr.s_addr);}
        inline std::string ipString() {return std::to_string(m_addr.sin_addr.s_addr);}
        inline uint16_t port(){ return ntohs(m_addr.sin_port);}
        inline std::string portString(){return std::to_string(m_addr.sin_port);}
    };
}