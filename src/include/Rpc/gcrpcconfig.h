#pragma once
#include <unordered_map>
#include <string>

namespace GcRpc{
    class GcRpcConfig{
    public:

    //parser .conf file
    void LoadConfigFile(const char * config_file);

    std::string Load(std::string & key);

    private:
        std::unordered_map<std::string, std::string> m_configMap;
    };
};
