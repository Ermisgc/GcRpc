#include "Rpc/event_loop.h"
#include "sys/epoll.h"
#include "unistd.h"

namespace GcRpc{
    EventLoop::EventLoop():epollfd(epoll_create(1)), _quit(false){

    }

    EventLoop::~EventLoop(){
        _quit.store(true);
        ::close(this->epollfd);
    }


    void EventLoop::loop(){
        while(!_quit){
            epoll_event events[1024];
            int event_count = epoll_wait(this->epollfd, events, 1024, 10);
        }    
    }

    void EventLoop::quit(){

    }


}
