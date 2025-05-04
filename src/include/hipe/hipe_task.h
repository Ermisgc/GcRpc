#pragma once
#include <future>
#include <type_traits>
namespace gchipe
{
    //as a base class to run a function
    class HipeTask
    {
    private:
        struct BaseExcutable{
            virtual ~BaseExcutable(){}
            virtual void call() = 0;
        } *callable;

        template <class Funtype>
        struct Executable: public BaseExcutable{
            Funtype foo;
            //perfect forward
            Executable(Funtype && _clb):foo(std::forward<Funtype>(_clb)), BaseExcutable(){}  
            void call() override{
                this->foo();
            }
        };
        
    public:
        //delete the default constructor, because the task is created by a concrete function
        HipeTask() = delete;
        HipeTask(HipeTask &) = delete;

        template<class _Callable>
        HipeTask(_Callable && foo):callable(new Executable<_Callable>(std::forward<_Callable>(foo))){}

        HipeTask(HipeTask && other){
            this->callable = other.callable;
            other.callable = nullptr;
        }

        virtual ~HipeTask(){
            delete callable;
        }

        HipeTask& operator= (HipeTask &) = delete;
        
        virtual HipeTask& operator=(HipeTask && other){
            delete this->callable;
            this->callable = other.callable;
            other.callable = nullptr;
            return *this;
        }

        virtual void operator()(){
            //operator() function only means call this function, but parameters can not be determined by conpiler.
            callable->call();
        }
    };

    template<class _Callable, class... Args>
    void invoke(_Callable& callable, Args&&... arg){
        callable(std::forward<Args>(arg)...);
    }
} // namespace gchipe
