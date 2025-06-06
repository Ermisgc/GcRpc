#pragma once
#include "google/protobuf/service.h"
#include "./data_structure/concurrentqueue.h"
#include "hipe/dynamic_threadpool.h"
#include <functional>
#include "arpa/inet.h"
#include <unordered_map>
#include <vector>
#include <atomic>
#include "liburing.h"

namespace GcRpc{
    class EventLoop;
    class Channel;
    class LogMsg;
    using LockFreeChannelQueue = moodycamel::ConcurrentQueue<Channel *>;
    using LockFreeLogQueue = moodycamel::ConcurrentQueue<LogMsg>;
    using Service = ::google::protobuf::Service;

    class RpcProvider
    {
    private:
        struct MethodTable{
            Service * service;
            std::unordered_map<std::string, const ::google::protobuf::MethodDescriptor *> methodTable;
        };

        //std::vector<EventLoop *> loops;
        LockFreeChannelQueue * feedback_queue;
        sockaddr_in * addr;
        gchipe::DynamicThreadPool * threadPond;
        int _fd;
        std::atomic<int> _status;
        struct io_uring ring;
        struct io_uring_params params;
        std::unordered_map<std::string, MethodTable> serviceTable;

        void onConnection();
        void onMessage(Channel *);
        void parseProtoMessage(const std::string & proto_message, Channel *);

    public:
        RpcProvider();
        ~RpcProvider();

        void Notify(::google::protobuf::Service *);

        void loop();
    };
};