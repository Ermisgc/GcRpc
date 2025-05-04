#pragma once
#include "google/protobuf/service.h"
namespace GcRpc{
    class RpcProvider
    {
    private:
        void onConnection();
        void onMessage();
    public:
        RpcProvider();
        ~RpcProvider();

        void Notify(::google::protobuf::Service *);

        void run();
    };
};