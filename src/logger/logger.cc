#include "logger/logger.h"
namespace gcspdlog{
Logger::Logger(const std::string & name, Level level, Sink::ptr default_sink)\
    :m_name(name), m_level(level){
        m_sinks.push_back(default_sink);
    }

void Logger::addSink(Sink::ptr sink){
    m_sinks.push_back(sink);
}

void Logger::removeSink(Sink::ptr sink){
    //how to remove a sink?
    for(auto m_sink = m_sinks.begin(); m_sink != m_sinks.end(); m_sink = std::next(m_sink)){
        if(*m_sink == sink) {
            m_sinks.erase(m_sink);
            break;
        }
    }
}

void Logger::removeAllSinks(){
    m_sinks.clear();
}

void Logger::log(LogMsg::ptr message){
    //if the level is small than the logger_level, the logger will not handle it.
    if(message->level > m_level) return;
    for(auto & sink:m_sinks){
        sink->log(message);
    }
}
Logger::~Logger(){

}
}