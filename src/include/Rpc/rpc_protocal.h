#include "gcrpcapplication.h"
#include "generic_rpc.pb.h"
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
    /**
     * @brief format of Protocal, the magic_number is 0x48635270
     * @protocol 
     * | magic_number (4)| version (1) | message_type (1) | status(1) | flag(1) | 
     * | request_id(8)  | proto_header_len(4) | [Generic_Rpc_Protobuf data] (proto_header_len) |;
     * for Generic_Rpc_Protobuf: | service_name | method_name | body_len (4) | [body] (body_len)|
     */
    struct RequestInformation{
        uint64_t request_id;  //generate for async messages.
        // uint32_t magic_number;  //magic number is not need to store
        uint8_t version;  // till now it is 0.
        uint8_t message_type;  // 1byte, 0 is request，1 is response
        uint8_t  status;    // 1byte, 0 is success, 1 is failure
        uint8_t  flag;   // some flags can be set here
        // uint32_t body_len;  // length of protobuf-serialized string, not need to store
        
        std::string service_name;  //service_name
        std::string method_name;
        std::string body_data;
    };

    class ProtocalParser{
        enum ProtocalType{
            GcRpc = 0,
            Http = 1
        } types;

    public:
        ProtocalParser(const ProtocalType & type);
        ~ProtocalParser();

        bool parser(const std::string & recv_str);

        bool encoder();
    };

    bool parserProtocalFromString(const std::string & recv_str, RequestInformation & protocal_info);
    bool generateStringFromProtocal(const RequestInformation & protocal_info, std::string & send_str);
    void printRequestInformation(const RequestInformation & protocal_info);

    uint64_t htonll(uint64_t);
    uint64_t ntohll(uint64_t);
}