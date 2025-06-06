#pragma once
#include <mutex>
#include "gcrpcconfig.h"

namespace GcRpc{
    class GcRpcApplication
    {
    private:
        GcRpcApplication(/* args */);
        ~GcRpcApplication();

        void ShowArgsHelp();

        static GcRpcApplication *instance;
        static std::mutex locker; 
        static GcRpcConfig m_config;

    public:
        static GcRpcApplication& getInstance();
        void Init(int argc, char** argv);
        
        static inline const std::string Load(const std::string & key){
            return std::move(m_config.Load(key));
        }
    };
;}
