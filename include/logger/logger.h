#pragma once
#include <string>
#include <list>
#include "sink.h"
#include "hipe/dynamic_threadpool.h"
namespace gcspdlog
{
    class Logger
    {
    protected:
        std::string m_name;
        std::list<Sink::ptr> m_sinks;
        Level m_level;
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger() = delete;   // we don't allowed a Logger of no name;
        Logger(const std::string & name, Level level = GCSPDLOG_LEVEL_OFF, Sink::ptr default_sink = std::make_shared<Sink>());
        void addSink(Sink::ptr);
        void removeSink(Sink::ptr);
        void removeAllSinks();
        virtual void log(LogMsg::ptr msg);  //it's overridable
        ~Logger();
    };

    template<class OuterThreadPool>
    class AsyncLogger: public Logger{
    private:
        OuterThreadPool & m_threadpool;

    public:
        AsyncLogger() = delete;
        AsyncLogger(const std::string & name, OuterThreadPool & tp, Level level = GCSPDLOG_LEVEL_OFF, Sink::ptr default_sink = std::make_shared<Sink>())\
            :m_threadpool{tp}, Logger(name, level, default_sink){};
        void log(LogMsg::ptr msg) override{
            if(msg->level < this->m_level) return;
            for(auto & sink: m_sinks){
                m_threadpool.execute(std::bind(&Sink::log, sink, msg));
            }
        }
    };
    
} // namespace gcspdlog
