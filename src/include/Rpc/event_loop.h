#pragma once
#include <atomic>

namespace GcRpc{
    class Channel;
    class Epoller;

    /**
     * @brief EventLoop is used as a worker thread, it loops with an epoll
     */
    class EventLoop{
    public:
        EventLoop();
        ~EventLoop();

        void loop();

        void quit();
    private:
        int epollfd;
        std::atomic<bool> _quit;

    };
}