#ifndef PB_ENCODING_H
#define PB_ENCODING_H
#include <string>
#include <stdint.h>

/**
 * @brief Encodes a varint into a string buffer.
 */
inline std::string pb_encode_varint(int32_t value){
    std::string ret;
    while (value > 0x7f){ 
        ret += static_cast<char>((value & 0x7f) | 0x80);
        value >>= 7;
    }
    ret += value;
    return ret;
}

inline int32_t pb_decode_varint(std::string &buffer){
    int32_t ret = 0;
    for(int i = 0;i < buffer.size();i++){
        ret <<= 7;
        ret |= buffer[i] & 0x7f;
    }
    return ret;
}



#endif