#include "Rpc/rpc_protocal.h"
#include <arpa/inet.h>
#include <endian.h>
#include <optional>

namespace GcRpc{
    bool parserProtocalFromString(const std::string & recv_str, RequestInformation & protocal_info){
        try{
            //parser the recv_str to protocal_info
            uint32_t magic_num;
            memcpy(&magic_num, recv_str.substr(0, 4).c_str(), 4);
#ifdef _Debug
            std::cout << "magic_num:" << magic_num << endl;
#endif
            magic_num = ntohl(magic_num);
            if(magic_num ^ MagicNumber){
                std::cout << "magic number is not correct !!!" << std::endl;
#ifdef _Debug
                std::cout << std::hex << MagicNumber << std::endl;
                std::cout << std::hex << magic_num << std::endl;
#endif
                return false;
            }

            protocal_info.version = (uint8_t)(recv_str[4]);
            protocal_info.message_type = (uint8_t)(recv_str[5]);
            protocal_info.status = (uint8_t)(recv_str[6]);
            protocal_info.flag = (uint8_t)(recv_str[7]);
            memcpy(&protocal_info.request_id, recv_str.substr(8, 8).c_str(), 8);
            protocal_info.request_id = ntohll(protocal_info.request_id);
            uint32_t proto_header_len;
            memcpy(&proto_header_len, recv_str.substr(16, 4).c_str(), 4);
            proto_header_len = ntohl(proto_header_len);
            
            std::string protobuf_str = recv_str.substr(20, proto_header_len);
            RpcHeader header;
            header.ParseFromString(protobuf_str);
            protocal_info.service_name = header.service_name();
            protocal_info.method_name = header.method_name();
            uint32_t body_size = header.args_size();

            protocal_info.body_data = recv_str.substr(20 + proto_header_len, body_size);
            return true;
        } catch(const std::out_of_range & e){
            std::cout << e.what() << std::endl;
            return false;
        }
    }

    std::optional<uint16_t> parserProtocalHeader(const std::string & recv_str, RequestInformation & protocal_info){
        assert(recv_str.size() == PROTOCAL_HEADER_LEN);
        try{
            //parser the recv_str to protocal_info
            uint32_t magic_num;
            memcpy(&magic_num, recv_str.substr(0, 4).c_str(), 4);

            magic_num = ntohl(magic_num);
            if(magic_num ^ MagicNumber){
                std::cout << "magic number is not correct !!!" << std::endl;
                return false;
            }

            protocal_info.version = (uint8_t)(recv_str[4]);
            protocal_info.message_type = (uint8_t)(recv_str[5]);
            protocal_info.status = (uint8_t)(recv_str[6]);
            protocal_info.flag = (uint8_t)(recv_str[7]);
            memcpy(&protocal_info.request_id, recv_str.substr(8, 8).c_str(), 8);
            protocal_info.request_id = ntohll(protocal_info.request_id);
            uint32_t proto_header_len;
            memcpy(&proto_header_len, recv_str.substr(16, 4).c_str(), 4);
            proto_header_len = ntohl(proto_header_len);
            
            assert(proto_header_len <= UINT16_MAX);
            return proto_header_len;
        } catch(const std::out_of_range & e){
            std::cout << e.what() << std::endl;
            return std::nullopt;
        }        
    }

    std::optional<uint16_t> parserProtocalRpcHeader(const std::string & protobuf_str, RequestInformation & protocal_info){
        try{
            RpcHeader header;
            header.ParseFromString(protobuf_str);
            protocal_info.service_name = header.service_name();
            protocal_info.method_name = header.method_name();
            return header.args_size();    //TODO:proto文件的大小最好能改到uint16_t，同步
        } catch(const std::exception & e){
            std::cout << e.what() << std::endl;
            return std::nullopt;
        }       
    }


    bool parserProtocalBody(const std::string & protobuf_str, RequestInformation & protocal_info){
        try{
            protocal_info.body_data = std::move(protobuf_str);
        } catch(const std::exception & e){
            std::cout << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool generateStringFromProtocal(const RequestInformation & protocal_info, std::string & send_str){
        RpcHeader header; 
        header.set_service_name(protocal_info.service_name);
        header.set_method_name(protocal_info.method_name);
        header.set_args_size(protocal_info.body_data.size());
        std::string proto_header = header.SerializeAsString();
        
        //reserve some space for send_str
        size_t total_size = 20 + proto_header.size() + protocal_info.body_data.size();
        send_str.resize(total_size);

        uint32_t magic_num = htonl(MagicNumber);
        memcpy(send_str.data(), &magic_num, 4);
        send_str[4] = protocal_info.version;
        send_str[5] = protocal_info.message_type;
        send_str[6] = protocal_info.status;
        send_str[7] = protocal_info.flag;

        uint64_t request_id = protocal_info.request_id;
        request_id = htonll(request_id);
        memcpy(send_str.data() + 8, &request_id, 8);

        uint32_t header_size = htonl(proto_header.size());
        memcpy(send_str.data() + 16, &header_size, sizeof(uint32_t));
        memcpy(send_str.data() + 20, proto_header.data(), proto_header.size());
        memcpy(send_str.data() + 20 + proto_header.size(), protocal_info.body_data.data(), protocal_info.body_data.size());
        return true;
    }

    void printRequestInformation(const RequestInformation & info){
        std::cout << ">>> <----Protocal Information---->" << std::endl;
        std::cout << ">>> version: " << (int)info.version << std::endl;
        std::cout << ">>> message_type: " << (int)info.message_type << std::endl;
        std::cout << ">>> status: " << (int)info.status << std::endl;
        std::cout << ">>> flag: " << (int)info.flag << std::endl;
        std::cout << ">>> request_id: " << info.request_id << std::endl;
        std::cout << ">>> service_name: " << info.service_name << std::endl;
        std::cout << ">>> method_name: " << info.method_name << std::endl;
        std::cout << ">>> body_data: " << info.body_data << std::endl;
    }

    uint64_t htonll(uint64_t value){
        //Check if host endian is the same as net endian
        static constexpr uint32_t test = 0x12345678;
        static const bool isNetEndian = (*reinterpret_cast<const uint8_t*> (&test) == 0x78); 
        if(isNetEndian){
            return (((value & 0xFF00000000000000ull) >> 56) |
                    ((value & 0x00FF000000000000ull) >> 40) |
                    ((value & 0x0000FF0000000000ull) >> 24) |
                    ((value & 0x000000FF00000000ull) >> 8) |
                    ((value & 0x00000000FF000000ull) << 8) |
                    ((value & 0x0000000000FF0000ull) << 24) |
                    ((value & 0x000000000000FF00ull) << 40) |
                    ((value & 0x00000000000000FFull) << 56));
        } else return value;
    }
    uint64_t ntohll(uint64_t value){
        return htonll(value);
    }
}