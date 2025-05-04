#pragma once
#include <atomic>
#include <list>
#include <queue>
#include <thread>
#include <functional>
#include <iostream>
#include <condition_variable>
#include "hipe_task.h"

namespace gchipe{
    class DynamicThreadPool{
    private:
        static DynamicThreadPool * instance;
        uint32_t max_task_num{1000};
        std::atomic<int> total_tasks{0};
        std::queue<HipeTask> tasks;
        std::condition_variable wait_cv;
        std::condition_variable task_done_cv;
        std::list<std::thread> threads;     
        std::mutex shared_lock;
        std::function<void(void)> overflow_fun;
        std::atomic<int> total_load{0}; 
        std::atomic<bool> stop{false};
        std::atomic<bool> waiting{false};

    public:
        DynamicThreadPool() = delete;
        
        //the initial number of thread;
        
        static DynamicThreadPool * getInstance(unsigned int th_num = 1);
        ~DynamicThreadPool();

        template<class _Callable>
        auto submit(_Callable &&task) -> std::future<std::invoke_result_t<decltype(task)>>{
            using RT = std::invoke_result_t<decltype(task)>;
            if(waiting.load()){
                std::cout << "The thread pool is waiting for the rest tasks, you can not add more tasks" << std::endl;
                return std::future<RT>();
            }
            std::packaged_task<RT()> pt(std::move(task));  //only rvalue is needed
            std::future<RT> ret = pt.get_future();
            {
                std::unique_lock<std::mutex> locker(shared_lock);
                if(tasks.size() >= max_task_num){
                    std::cout << "Task queue overflow" << std::endl;
                    return std::future<RT>();
                }         
                //there: task has been std::bind, so the task is a function<RT()>
                tasks.emplace(std::move(pt));
            }
            wait_cv.notify_one();
            return ret;
        }
        
        template<class _Callable>
        void execute(_Callable &&task){
            if(waiting.load()){
                std::cout << "The thread pool is waiting for the rest tasks, you can not add more tasks" << std::endl;
                return;
            }
            {
                std::unique_lock<std::mutex> locker(shared_lock);
                if(tasks.size() >= max_task_num){
                    std::cout << "Task queue overflow" << std::endl;
                    return;
                }
                tasks.emplace(HipeTask(std::forward<_Callable>(task)));
            }
            wait_cv.notify_one();
        }

        void waitforAllTasks();

        void waitforCurrentTasks();

        void close();

        void setTaskOverflow(std::function<void(void)> & _overflow_fun);

        void setMaxTaskNum(uint32_t num);

    private:
        //work function
        void worker(int index);

        DynamicThreadPool(unsigned int th_num = 1);
    };

}