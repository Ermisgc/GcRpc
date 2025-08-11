#ifndef RPC_PROTOCAL_H
#define RPC_PROTOCAL_H
#include "gcrpcapplication.h"
#include "generic_rpc.pb.h"
#include <optional>
#define MagicNumber 0x48635270
#define MESSAGE_TYPE_REQUEST 0
#define MESSAGE_TYPE_RESPONSE 1
#define STATUS_SUCCESS 0x00
#define STATUS_FAILURE 0x01
#define STATUS_SERVER_REJECTED 0x02  //for backpressure
#define STATUS_SERVICE_NOT_EXISTED 0x04
#define STATUS_METHOD_NOT_EXISTED 0x08
#define STATUS_UNKNOWN_ERROR 0x10
namespace GcRpc{
    class TcpConnection;

    /**
     * @brief format of Protocal, the magic_number is 0x48635270
     * @protocol 
     * | magic_number (4)| version (1) | message_type (1) | status(1) | flag(1) | 
     * | request_id(8)  | proto_header_len(4)  of [Generic_Rpc_Protobuf_header] |;
     * for Generic_Rpc_Protobuf: | service_name | method_name | body_len (4) | [body] (body_len)|
     */
    struct RequestInformation{
        TcpConnection * conn;    

        // uint32_t magic_number;  //magic number is not need to store
        uint64_t request_id;  //generate for async messages.
        
        uint8_t version;  // till now it is 0.
        uint8_t message_type;  // 1byte, 0 is request，1 is response
        uint8_t  status;    // 1byte, 0 is success, 1 is failure
        uint8_t  flag;   // some flags can be set here，default 0, if wants to close after the request, 1
        // uint32_t body_len;  // length of protobuf-serialized string, not need to store
        
        std::string service_name;  //service_name
        std::string method_name;
        std::string body_data;
    };

    //TODO:响应也应该要设置一下这个类，因为响应也有

    //协议头的长度
    constexpr size_t PROTOCAL_HEADER_LEN = 20;

    //TODO:区分自定义Rpc协议和HTTP两种协议
    bool parser(const std::string & recv_str, RequestInformation & protocal_info); 

    //从接收到的字符串，识别请求信息
    bool parserProtocalFromString(const std::string & recv_str, RequestInformation & protocal_info);

    //解析头，返回的是后面的body部分的长度
    std::optional<uint16_t> parserProtocalHeader(const std::string & recv_str, RequestInformation & protocal_info);

    std::optional<uint16_t> parserProtocalRpcHeader(const std::string & recv_str, RequestInformation & protocal_info);

    //解析rpc调用的参数
    bool parserProtocalBody(const std::string & recv_str, RequestInformation & protocal_info);
    
    //通过请求消息生成字符串
    bool generateStringFromProtocal(const RequestInformation & protocal_info, std::string & send_str);
    
    //DEBUG时打印请求消息
    void printRequestInformation(const RequestInformation & protocal_info);

    uint64_t htonll(uint64_t);
    uint64_t ntohll(uint64_t);
}

#endif