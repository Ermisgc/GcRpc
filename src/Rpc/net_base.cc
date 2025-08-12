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

    std::string getLocalIP(){
        static std::string ip_str;
        if(!ip_str.empty()){
            return ip_str;
        }

        int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
        if(sock < 0){
            std::string ip_str = "127.0.0.1";
            return ip_str;  
        }

        const char * google_dns = "8.8.8.8";
        uint16_t dns_port = 53;
        sockaddr_in serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_addr.s_addr = inet_addr(google_dns);
        serv.sin_port = htons(dns_port);
        
        connect(sock, (const sockaddr*)&serv, sizeof(serv));
        
        sockaddr_in name;
        socklen_t namelen = sizeof(name);
        getsockname(sock, (sockaddr*)&name, &namelen);
        
        char buffer[INET_ADDRSTRLEN];
        const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer));
        close(sock);
        
        ip_str = p ? std::string(buffer) : "127.0.0.1";
        // cout << ip_str << endl;
        return ip_str;
    }

    static constexpr char base64_charset_table[64] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    //TODO:这里应该加上无效字符检测，但是暂时没做
    unsigned char base64_reverse(char input){
        if(input == '+') return 62;
        if(input == '/') return 63;
        if(input >= 'a') return 26 + input - 'a';
        else if(input >= 'A') return input - 'A';
        else return 52 + input - '0';
    }

    std::string base64Encoding(const std::vector<uint8_t> & input){
        std::string encoded;
        int n = input.size();
        encoded.reserve( (4 * n + 2) / 3);  //首先预留一部分空间

        for(int i = 0;i < n; ++i){
            if(i % 3 != 2) continue;
            //else 的情况，可以进行encoding了，把input每三个一组变成base64
            encoded.push_back(base64_charset_table[(input[i - 2] & 0xfc) >> 2]);
            encoded.push_back(base64_charset_table[((input[i - 2] & 0x03) << 4) | ((input[i - 1] & 0xf0) >> 4)]);
            encoded.push_back(base64_charset_table[((input[i - 1] & 0x0f) << 2) | ((input[i] & 0xc0) >> 6)]);
            encoded.push_back(base64_charset_table[input[i] & 0x3f]);
        }

        //最后处理一下，可能会有剩余的字符
        if(n % 3 == 1){  //多了一个字符
            encoded.push_back(base64_charset_table[(input[n - 1] & 0xfc) >> 2]);
            encoded.push_back(base64_charset_table[(input[n - 1] & 0x03) << 4]);
            encoded.push_back('=');
            encoded.push_back('=');
        }else if(n % 3 == 2){  //多了两个字节
            encoded.push_back(base64_charset_table[(input[n - 2] & 0xfc) >> 2]);
            encoded.push_back(base64_charset_table[((input[n - 2] & 0x03) << 4) | ((input[n - 1] & 0xf0) >> 4)]);
            encoded.push_back(base64_charset_table[(input[n - 1] & 0x0f) << 2]);
            encoded.push_back('=');
        }

        return encoded;
    }

    std::string base64Encoding(const std::string & input){
        return base64Encoding(std::vector<uint8_t>(input.begin(), input.end()));
    }

    std::string base64Decoding(const std::string & input){
        std::string decoded;
        int n = input.length();
        decoded.reserve( (3 * n + 3) / 4);  //首先预留一部分空间

        int count = 0;
        //00111111,00111111,00111111,00111111 -> 11111111,11111111,11111111
        //11111111  -> 00111111, 00000011
        //每个字符都是前一个的后6位，前一个的前2位
        unsigned char char_group[4];
        for(char c : input){
            if(c == '=') break; //到了补充位就可以停止了
            if(c > 'z' || (c > 'Z' && c < 'a') || (c < 'A' && c > '9') || (c < '0' && c != '/' && c != '+')){
                std::cerr << "Unknown char when decoding base64." << std::endl;
            }
            //然后是每四个字符一组
            char_group[count++] = base64_reverse(c);
            if(count % 4 == 0){
                decoded.push_back(char_group[0] << 2 | char_group[1] >> 4);
                decoded.push_back(char_group[1] << 4 | char_group[2] >> 2);
                decoded.push_back(char_group[2] << 6 | char_group[3]);
                count = 0;
            }
        }

        //最后处理一下，可能会有剩余的字符
        if(count % 4 == 1){    //里面只剩一个字符了
            decoded.push_back(char_group[0] << 2);
        }else if(count % 4 == 2){
            decoded.push_back(char_group[0] << 2 | char_group[1] >> 4);
            decoded.push_back(char_group[1] << 4);
        } else if(count % 4 == 3){
            decoded.push_back(char_group[0] << 2 | char_group[1] >> 4);
            decoded.push_back(char_group[1] << 4 | char_group[2] >> 2);
            decoded.push_back(char_group[2] << 6);
        }

        if(decoded.back() == 0){
            decoded.pop_back();
        }

        return decoded;
    }    
}
