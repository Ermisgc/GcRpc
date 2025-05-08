#include "Rpc/net_base.h"
#include <string>
using namespace GcRpc;

InetAddr::InetAddr(uint32_t _ip, uint16_t _port){
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = htonl(_ip);
    m_addr.sin_port = htons(_port);
}

InetAddr::InetAddr(const char * _ip, const char * _port){
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(_ip);
    m_addr.sin_port = htons(atoi(_port));
}
InetAddr::InetAddr(uint32_t _ip, const char * _port){
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = htonl(_ip);
    m_addr.sin_port = htons(atoi(_port));
}
InetAddr::InetAddr(const char * _ip, uint16_t _port){
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(_ip);
    m_addr.sin_port = htons(_port);
}

InetAddr::InetAddr(const InetAddr & other):m_addr(other.m_addr){}

