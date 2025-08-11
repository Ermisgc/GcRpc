#include <string>
#include <stdint.h>
#include <iostream>
#include <assert.h>

/**
 * @brief Encodes a varint into a string buffer.
 */

inline std::string pb_encode_varint(int32_t value){
    uint32_t v = static_cast<uint32_t>(value);
    std::string ret;
    while (v > 0x7f){ 
        ret += static_cast<char>((v & 0x7f) | 0x80);
        v >>= 7;
    }
    ret += static_cast<char>(v);
    return ret;
}

inline int32_t pb_decode_varint(std::string &buffer){
    int32_t ret = 0;
    for(int i = buffer.size() - 1;i >= 0; i--){
        ret <<= 7;
        ret |= buffer[i] & 0x7f;
    }
    return ret;
}

inline uint32_t pb_encode_zigzag(int32_t num){
    return (static_cast<uint32_t>(num) << 1) | (static_cast<uint32_t>(num) >> 31); 
}

inline int32_t pb_decode_zigzag(uint32_t num){
    return static_cast<int32_t>((num) >> 1) | ((num << 31));
}

inline uint64_t pb_encode_zigzag(int64_t num){
    return (static_cast<uint64_t>(num) << 1) | (static_cast<uint64_t>(num) >> 63);
}

inline int64_t pb_decode_zigzag(uint64_t num){
    return static_cast<int64_t>((num) >> 1) | ((num << 63));
}
