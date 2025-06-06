#include "Rpc/gcrpcconfig.h"
using namespace GcRpc;
#include <fstream>
#include <iostream>

//parser .conf file
void GcRpcConfig::LoadConfigFile(const char * config_file){
    std::ifstream ifs;
    ifs.open(config_file, std::ios_base::in);
    if(!ifs.is_open()){
        //not open
        std::cout << "The file can not be opened" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << ">>> Loading configuration ..." << std::endl;
    std::string line;
    while(std::getline(ifs, line)){
        //delete the space before and after each configure
        size_t pos = line.find_first_of('#');
        if(pos != std::string::npos) line = line.substr(0, pos);

        size_t pos3 = line.find('=');
        if(pos3 == std::string::npos) continue; //we don't find a "="
        size_t pos1 = line.find_first_not_of(' ');
        size_t pos2 = line.find_last_not_of(' ');

        std::string key = line.substr(pos1, pos3 - pos1);
        std::string value = line.substr(pos3 + 1, pos2 - pos3);

        pos2 = key.find_last_not_of(' ');
        key = key.substr(0, pos2 + 1);

        pos1 = value.find_first_not_of(' ');
        value = value.substr(pos1, value.length() - pos1);
        m_configMap[key] = value;
        std::cout << ">>> " << key <<  ":"  << value << std::endl;
    }
    std::cout << ">>> Load done !!!" << std::endl;
    ifs.close();
}

std::string GcRpcConfig::Load(const std::string & key){
    if(m_configMap.find(key) != m_configMap.end()){
        return m_configMap[key];
    }
    else return "";
}