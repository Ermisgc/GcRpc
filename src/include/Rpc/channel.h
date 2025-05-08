/**
 * Channel provides interface of FDs and Events.
 * It should supports sockfd、eventfd、timerfd in Linux.
 * It should tell EventLoop what fd to add, what event happens and how to fetch the data corresponding.
 */

#pragma once
#include "buffer.h"
#include <functional>
#define USER_EVENT_ACCEPT 0
#define USER_EVENT_READ 1
#define USER_EVENT_WRITE 2
#define USER_EVENT_HANDLE 3
#define USER_EVENT_CLOSE 4

namespace GcRpc{
    class Buffer;
    class EventLoop;
    class BufferPool;
    class RpcChannel{
        using EventHandler = std::function<void(RpcChannel *)>;
    public:
        RpcChannel(int fd, EventHandler && callback);
        ~RpcChannel();

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

        inline void handle(){
            _callback(this);
        }

    private:
        int _fd;
        int event_status;
        Buffer * _buf;
        EventHandler _callback;
    };
}
