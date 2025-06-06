#include "Rpc/buffer.h"
#include <iostream>

namespace GcRpc{
    Buffer::Buffer():readIndex(0), writeIndex(0), buf(MAX_BUFFER_SIZE){
    }

    Buffer::Buffer(const Buffer & other){ //deepcopy
        buf = other.buf;
        readIndex = other.readIndex;
        writeIndex = other.writeIndex;
    }

    Buffer::Buffer(Buffer && other){
        buf = std::move(other.buf);
        readIndex = other.readIndex;
        writeIndex = other.writeIndex;
    }

    Buffer & Buffer::operator=(const Buffer & other){
        buf = other.buf;
        readIndex = other.readIndex;
        writeIndex = other.writeIndex;
        return *this;
    }
    Buffer & Buffer::operator=(Buffer && other){
        buf = std::move(other.buf); 
        readIndex = other.readIndex;
        writeIndex = other.writeIndex;
        return *this;
    }

    std::string Buffer::read(uint16_t read_size){ 
        int readable = readableBytes();
        if(readable > read_size) readable = read_size;
        std::string ans;
        if(readIndex + readable <= writeIndex || readIndex + readable < MAX_BUFFER_SIZE) 
            ans = std::string(buf.begin() + readIndex, buf.begin() + readIndex + readable);
        else ans = std::string(buf.begin() + readIndex, buf.end()) + \
                std::string(buf.begin(), buf.begin() + readable + readIndex - MAX_BUFFER_SIZE);
        
        hasRead(readable);
        return std::move(ans);
    }

    uint16_t Buffer::readableBytes(){ 
        return writeIndex >= readIndex ? writeIndex - readIndex: MAX_BUFFER_SIZE - readIndex + writeIndex;
    }

    uint16_t Buffer::writableBytes(){ 
        //return writeIndex >= readIndex ? MAX_BUFFER_SIZE - writeIndex + readIndex - 1: readIndex - writeIndex - 1;
        return MAX_BUFFER_SIZE - readableBytes() - 1;
    }

    char* Buffer::writeBegin(){
        return begin() + writeIndex;
    }

    const struct iovec * Buffer::readvBegin(){ 
        //there are two conditions:
        //1. writeIndex >= readIndex, only one area between readIndex and writeIndex - 1 can be read.
        //2. writeIndex < readIndex, two areas can be read: readIndex ~ MAX_BUFFER_SIZE - 1 and 0 ~ writeIndex.
        //corner condition should be concerned
        struct iovec * iov = new struct iovec[2];
        iov[0].iov_base = begin() + readIndex;
        iov[0].iov_len = (writeIndex >= readIndex ? writeIndex: MAX_BUFFER_SIZE) - readIndex;
        iov[1].iov_base = begin();
        iov[1].iov_len = writeIndex >= readIndex ? 0 : writeIndex;
        return iov;
    }

    const struct iovec * Buffer::writevBegin(){ 
        //there are two conditions:
        //1. writeIndex >= readIndex, the right side of writeIndex(writeIndex ~ MAX_BUFFER_SIZE - 1)
        //   and left side of readIndex(0 ~ readIndex - 1) can be written in.
        //2. writeIndex < readIndex, one area between writeIndex and readIndex - 1 can be written in.
        //corner condition is writeIndex == readIndex.
        struct iovec * iov = new struct iovec[2];
        iov[0].iov_base = begin() + writeIndex;
        iov[0].iov_len = (writeIndex >= readIndex? MAX_BUFFER_SIZE : (readIndex - 1)) - writeIndex;
        iov[1].iov_base = begin();
        iov[1].iov_len = writeIndex >= readIndex ? readIndex : 0;
        return iov;
    }

    void Buffer::clear(){
        writeIndex = 0;
        readIndex = 0;
    }

    char * Buffer::begin(){
        return &*buf.begin();
    }

    void Buffer::hasWritten(uint16_t size){
        writeIndex = (writeIndex + size) % MAX_BUFFER_SIZE; 
    }

    void Buffer::hasRead(uint16_t size){
        readIndex = (readIndex + size) % MAX_BUFFER_SIZE;
    }

    uint16_t Buffer::append(const std::string & str){
        int size = str.length();
        return append(str.c_str(), size);
    }

    uint16_t Buffer::append(const char * str, uint16_t size){
        int i = 0;
        for(;i < size && writeIndex != readIndex - 1; ++i){
            buf[writeIndex] = str[i];
            hasWritten(1);
        }
        if(i < size) throw "no more space for write";
        return i;
    }

    const std::string Buffer::retrieveAllBufferAsString(){
        int n = readableBytes();
        return std::move(read(readableBytes()));
    }
}