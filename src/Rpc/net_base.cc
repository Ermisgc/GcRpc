#include "Rpc/net_base.h"
#include <string>

namespace GcRpc{
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

    void parserNodeFromEtcd(const etcd::Value & value, ServiceEndpoint & ep){
        auto serialized_str = value.as_string();
        GcRpc::RpcNodeInfo node_info;
        try {
            node_info.ParseFromString(serialized_str);
            ep.active_conns = node_info.active_conn();
            ep.max_conns = node_info.max_conn();
            ep.load_score = (ep.max_conns - ep.active_conns) / (ep.max_conns + 1.0);
            ep.addr = new InetAddr(node_info.addr(), node_info.port());
        }
        catch (const std::exception & e){
            
        }
    }
}
