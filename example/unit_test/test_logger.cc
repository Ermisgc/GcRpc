/**
 * [+] I have passed the test for spdlog
 */

#include "logger/logger.h"
#include "logger/log_base.h"

using namespace gcspdlog;
int main(){
    gcspdlog::Logger log("test");
    log.log(QuickMsgCRI("Hello World"));
    //std::cout << "Hello World?" << std::endl;
    return 0;
}