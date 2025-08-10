#pragma once
#include <atomic>
#include <vector>
#include <mutex>
#ifdef _DEBUG
#include <iostream>
#endif

namespace gcdst{
    //一个基于CAS操作和链表的MPSC无锁队列，这个队列目前而言
    template<typename T = int>
    class LockFreeQueue {
    private:
        struct Node {
            T data;
            std::atomic<Node *> next;
            std::atomic<uint8_t> ableToDelete;
            Node(T _data) : data(_data), next(nullptr), ableToDelete(false) {}
        };
        std::atomic<Node* > head;
        std::atomic<Node *> tail;
    public:
        LockFreeQueue() {
            Node* dummy = new Node({});
            head.store(dummy);
            dummy->ableToDelete.store(true);
            tail.store(dummy);
        }
        ~LockFreeQueue() {
            if (head) delete head;
        }

        void enqueue(T item) {
            Node* new_node = new Node(item);
            while (1) {
                Node* curr_tail = tail.load();
                if (tail.compare_exchange_weak(curr_tail, new_node)) {
                    curr_tail->next.store(new_node);
                    return;
                } //else 竞态失败，重新获得curr_tail
            }
        }

        bool dequeue(T* val) {
            while (true) {
                Node* curr_head = head.load();
                Node* curr_next = curr_head->next.load();
                
                if (curr_next != nullptr) {
                    if (head.compare_exchange_weak(curr_head, curr_next)) {
                        *val = curr_next->data;
                        curr_next->ableToDelete.store(true);
                        while (!curr_head->ableToDelete.load()) {};
                        delete curr_head;
                        return true;
                    }  //竞态失败了重新再进入true中再继续获取新的curr_next和curr_head.
                }
                else {
                    return false;
                }
            }
        }
    };


    template<typename T>
    class SimpleQueue{
    private:
        struct Node{
            T data;
            Node * next;
            Node(T _): data(_), next(nullptr){}
        };
        Node * head;
        Node * tail;
    public:
        SimpleQueue():head(new Node{}), tail(nullptr){}
        ~SimpleQueue(){ /*... */}

        void enqueue(T item){  //这里可以用std::move做到转移，但为简便没有这样做
            Node * new_node = new Node(item);
            if(tail) tail->next = new_node;
            else head->next = new_node;
            tail = new_node;
        }

        T dequeue(){
            Node * ret = head->next;
            head->next = ret->next;
            if(ret == tail) tail = nullptr;
            return ret.data;
        }
    };

    template<typename T>
    class LockedQueue{
    private:
        struct Node{
            T data;
            Node * next;
            Node(T _): data(_), next(nullptr){}
        };
        Node * head;
        Node * tail;
        std::mutex mtx;
    public:
        LockedQueue():head(new Node{}), tail(nullptr){}
        ~LockedQueue(){ /*... */}

        void enqueue(T item){  //这里可以用std::move做到转移，但为简便没有这样做
            Node * new_node = new Node(item);
            {
                std::unique_lock<std::mutex> locker(mtx);
                if(tail) tail->next = new_node;
                else head->next = new_node;
                tail = new_node;
            }
        }

        T dequeue(){
            Node * ret;
            {
                std::unique_lock<std::mutex> locker(mtx);
                ret = head->next;
                if(!ret) throw "empty queue";
                head->next = ret->next;
                if(ret == tail) tail = nullptr;
            }
            return ret.data;
        }
    };

}
