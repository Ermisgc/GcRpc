#include "Rpc/channel.h"

namespace GcRpc{
    RpcChannel::RpcChannel(int fd,EventHandler && callback):_fd(fd), event_status(USER_EVENT_ACCEPT), \
     _callback(callback) {
        _buf = new Buffer();
    }

    RpcChannel::~RpcChannel(){
        //在这里的暂时先析构buf，否则会内存泄漏，后续有Channel对象池时就不用考虑这么问题了，而是归还buf
        delete _buf;
    }
}