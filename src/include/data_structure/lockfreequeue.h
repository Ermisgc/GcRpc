#pragma once
#include <atomic>
#include <vector>
#ifdef _DEBUG
#include <iostream>
#endif

template<typename T>
class LockFreeQueue{
public:
    LockFreeQueue() = delete;
    LockFreeQueue(const LockFreeQueue &) = delete;
    LockFreeQueue(const LockFreeQueue &&) = delete;
    LockFreeQueue(int _capacity):capacity(_capacity), inner_queue(capacity), writeIndex(0), readIndex(0){
#ifdef _DEBUG
        std::cout << "create a queue with size: " << capacity << std::endl; 
#endif
    }

    void push(T && item){
        int index = writeIndex++ % capacity;
        inner_queue[index] = std::forward<T>(item);
    }

    T && pop() noexcept {
        int index = readIndex++;  //atomic operation
        if(index < writeIndex){
            return std::move(inner_queue[index % capacity]);
        } else{
            readIndex--;
            return T();
        }
    }


private:
    int capacity = 0;
    std::atomic<int> writeIndex;
    std::atomic<int> readIndex;
    std::vector<T> inner_queue;
};