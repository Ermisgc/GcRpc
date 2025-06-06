/**
 * Channel provides interface of FDs and Events.
 * It should supports sockfd、eventfd、timerfd in Linux.
 * It should tell EventLoop what fd to add, what event happens and how to fetch the data corresponding.
 */

#pragma once
#include "buffer.h"
#include "arpa/inet.h"
#include <functional>
#define USER_EVENT_ACCEPT 0
#define USER_EVENT_READ 1
#define USER_EVENT_WRITE 2
#define USER_EVENT_HANDLE 3
#define USER_EVENT_CLOSE 4

namespace GcRpc{
    class Buffer;
    class Channel{
        using EventHandler = std::function<void(void*)>;
    public:
        Channel(int fd);
        ~Channel();

        inline void setFileDescriptor(int fd){
            _fd = fd;
        }

        inline void setFd(int fd){
            _fd = fd;
        }

        inline int fd(){
            return _fd;
        }

        inline int eventStatus(){
            return event_status;
        }

        inline void setEventStatus(int es){
            event_status = es;
        }

        inline const struct iovec * getWriteBuffer(){
            return _buf->writevBegin();
        }

        inline const struct iovec * getReadBuffer(){
            return _buf->readvBegin();
        }

        inline const std::string retrieveAllBufferAsString(){
            return std::move(_buf->retrieveAllBufferAsString());
        }

        inline sockaddr * getSockAddr(){
            return reinterpret_cast<sockaddr *>(sock_addr);
        }

        inline socklen_t * getSockLen(){
            return &len;
        }

        inline void recordWriteBytes(size_t written_bytes){
            _buf->hasWritten(written_bytes);
        }

        inline void recordReadBytes(size_t read_bytes){
            _buf->hasRead(read_bytes);
        }

        inline void clearBuffer(){
            _buf->clear();
        }

        inline void append(const std::string & str){
            _buf->append(str);
        }
    private:
        int _fd;
        int event_status;
        Buffer * _buf;
        sockaddr_in * sock_addr;
        socklen_t len;
        //EventHandler _callback;
    };
}
