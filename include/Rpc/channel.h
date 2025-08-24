/**
 * Channel provides interface of FDs and Events.
 * It should supports sockfd、eventfd、timerfd in Linux.
 * It should tell EventLoop what fd to add, what event happens and how to fetch the data corresponding.
 */

#pragma once
#include "buffer.h"
#include "arpa/inet.h"
#include <functional>
#include <memory>

#define USER_EVENT_ACCEPT 0
#define USER_EVENT_READ 1
#define USER_EVENT_WRITE 2
#define USER_EVENT_HANDLE 3
#define USER_EVENT_CLOSE 4
#define USER_EVENT_WRITE_THEN_CLOSE 5
#define USER_EVENT_TIMEOUT 6

namespace GcRpc{
    class Buffer;
    class RpcProvider;
    using EventLoop = RpcProvider;

    struct ChannelV2{
        std::unique_ptr<Buffer> read_buffer = std::make_unique<Buffer>(); //异步读的缓冲 //TODO:Buffer重新设计，可能需要读写双传冲区
        std::unique_ptr<Buffer> write_buffer = std::make_unique<Buffer>(); //异步写的缓冲
        sockaddr_in sock_addr;
        socklen_t len {sizeof(sockaddr_in)};
    };

    //虚基类，或者说接口类，只提供一个do_io_uring方法
    class UringChannel{
    public: 
        UringChannel() = default;
        ~UringChannel() = default;

        virtual void do_io_uring(int last_res, int last_event_type) = 0;
    };

    unsigned long long getIOUringUserdata(UringChannel * uc, uint8_t event_type);

    //根据io_uring的user data中的数据进行io_uring的自动调用
    void executeIOUringCallback(unsigned long long user_data, int res);
}
