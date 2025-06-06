#include "Rpc/channel.h"

namespace GcRpc{
    Channel::Channel(int fd):_fd(fd), event_status(USER_EVENT_ACCEPT), len(sizeof(sockaddr_in)){
        _buf = new Buffer();
    }

    Channel::~Channel(){
        //在这里的暂时先析构buf，否则会内存泄漏，后续有Channel对象池时就不用考虑这么问题了，而是归还buf
        delete _buf;
    }
}