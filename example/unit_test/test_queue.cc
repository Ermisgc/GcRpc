#include "data_structure/lockfreequeue.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <chrono>
using namespace std;

atomic<int> sum;

void add(){
    for(int i = 0;i < 50000000; ++i){
        sum++;
    }
}

int sum2 = 0;
std::mutex mtx;
void add2(){
    for(int i = 0;i < 50000000; ++i){
        unique_lock<mutex> locker(mtx);
        sum2++;
    }
}


int main(){
    LockFreeQueue<int> q(512);
    auto start = std::chrono::system_clock::now();
    for(int i = 0;i < 10; ++i){
        sum.store(0);
        thread t1 = thread(add);
        thread t2 = thread(add);
        t1.join();
        t2.join();
        if(sum != 100000000){
            cout << "atomic 结果不对" << endl;
        }
    }
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "花费了" << double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒" << std::endl;
    

    start = std::chrono::system_clock::now();
    for(int i = 0;i < 10; ++i){
        sum2 = 0;
        thread t1 = thread(add2);
        thread t2 = thread(add2);
        t1.join();
        t2.join();
        if(sum != 100000000){
            cout << "mutex 结果不对" << endl;
        }
    }
    end = std::chrono::system_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "花费了" << double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒" << std::endl;
    
    return 0;
}