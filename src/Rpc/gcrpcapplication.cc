#include"Rpc/gcrpcapplication.h"
#include <iostream>
#include <unistd.h>  //-i parser: getopt function
#include <string>

using namespace GcRpc;
GcRpcApplication::GcRpcApplication(/* args */){
    
}

GcRpcApplication::~GcRpcApplication(){

}

void GcRpcApplication::ShowArgsHelp(){
    std::cout << "format: command -i <configfile>" << std::endl;
}

GcRpcConfig GcRpcApplication::m_config;
    //lazy Singleton
GcRpcApplication & GcRpcApplication::getInstance(){
    static GcRpcApplication instance;
    return instance;
}

void GcRpcApplication::Init(int argc, char** argv){
    if(argc < 2){
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    //parser argc and argv
    while((c = getopt(argc, argv, "i:")) != -1){  //means there's one parameter after -i
        switch (c)
        {
        case 'i':
            config_file = optarg;  // a static or global member in <unistd.h> I think.
            break;
        case '?':  //when getopt don't find 'i', c will become '?'
            std::cout << "wrong arg" << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);    
        default:
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        }
    }

    //load config_file: rpcserver_ip , rpcserver_port..., using fstream
    m_config.LoadConfigFile(config_file.c_str());
}
