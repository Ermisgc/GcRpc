#include "hipe/dynamic_threadpool.h"
#include <iostream>
namespace gchipe{
    DynamicThreadPool * DynamicThreadPool::instance = nullptr;

    DynamicThreadPool * DynamicThreadPool::getInstance(unsigned int th_num){
        static std::mutex mtx;
        if(!instance){
            mtx.lock();
            if(!instance){
                instance = new DynamicThreadPool(th_num);
            }
            mtx.unlock();
        }
        return instance;
    }
    //the initial number of thread;
    DynamicThreadPool::DynamicThreadPool(unsigned int th_num){
        for(int i = 0;i < th_num; ++i){
            threads.emplace_back(std::thread(&DynamicThreadPool::worker, this, i));
        }
    }

    DynamicThreadPool::~DynamicThreadPool(){
        close();
    }

    void DynamicThreadPool::waitforAllTasks(){
        waiting.store(true);
        {
            std::unique_lock<std::mutex> ulck(shared_lock);
            task_done_cv.wait(ulck, [this]()->bool{return tasks.empty();});
        }
        //waitforCurrentTasks();
        waiting.store(false);
        //waitforCurrentTasks();
    }

    void DynamicThreadPool::waitforCurrentTasks(){
        for(auto & thread: threads){
            thread.join();
        }
    }

    void DynamicThreadPool::setTaskOverflow(std::function<void(void)> & _overflow_fun){
        this->overflow_fun = _overflow_fun;
    }

    void DynamicThreadPool::worker(int index){
        while(!stop){
            std::unique_lock<std::mutex> locker(shared_lock);
            //awake if tasks are not empty or we try to stop this threadpool
            wait_cv.wait(locker, [this]()->bool{ return !tasks.empty() || stop;});

            //awake when user try to stop
            if(stop) break;

            HipeTask task = std::move(tasks.front());
            tasks.pop();
            gchipe::invoke(task);
            if(waiting){
                task_done_cv.notify_one();
            }
        }
    }

    void DynamicThreadPool::setMaxTaskNum(uint32_t num){
        this->max_task_num = num;
    }

    void DynamicThreadPool::close(){
        this->stop.store(true);
        wait_cv.notify_all();
        waitforCurrentTasks();
        this->stop.store(false);
    }
}