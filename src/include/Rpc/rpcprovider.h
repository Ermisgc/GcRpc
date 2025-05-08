#pragma once
#include "google/protobuf/service.h"
#include "./data_structure/concurrentqueue.h"
#include <functional>
#include <unordered_map>
#include <vector>


namespace GcRpc{
    class EventLoop;
    class Channel;
    class LogMsg;
    using LockFreeChannelQueue = moodycamel::ConcurrentQueue<Channel>;
    using LockFreeLogQueue = moodycamel::ConcurrentQueue<LogMsg>;
    using Service = ::google::protobuf::Service;

    class RpcProvider
    {
    private:
        struct MethodTable{
            Service * service;
            std::unordered_map<std::string, const ::google::protobuf::MethodDescriptor *> methodTable;
        };

        std::vector<EventLoop *> loops;
        std::unordered_map<std::string, MethodTable> serviceTable;
        void onConnection();
        void onMessage();
        void parseProtoMessage(const std::string & proto_message);

    public:
        RpcProvider();
        ~RpcProvider();

        void Notify(::google::protobuf::Service *);

        void run();
    };
};