#include "data_structure/lockfreequeue.h"
#include "data_structure/concurrentqueue.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <chrono>
#include <cassert>
using namespace std;

void singleThreadTest() {
    LockFreeQueue<int> queue;
    int val = 0;
    assert(!queue.dequeue(&val)); // 队列为空
    queue.enqueue(1);
    queue.dequeue(&val);
    assert(val == 1); // 正确出队
    queue.enqueue(2);
    queue.enqueue(3);
    queue.dequeue(&val);
    assert(val == 2);
    queue.dequeue(&val);
    assert(val == 3);
    assert(!queue.dequeue(&val)); // 队列再次为空

    cout << "[+] SingleThreadTest Pass" << endl;
}

void multiThreadTest(int test_id) {
    LockFreeQueue<int> queue;
    std::atomic<int> sum(0);
    constexpr int numProducers = 4;
    constexpr int numConsumers = 4;
    constexpr int itemsPerProducer = 10000;

    // 生产者线程
    auto producer = [&](int start) {
        for (int i = 0; i < itemsPerProducer; ++i) {
            queue.enqueue(start + i + 1); //规避掉0这个变量
        }
    };

    // 消费者线程
    auto consumer = [&] {
            for (int i = 0; i < itemsPerProducer * numProducers / numConsumers; ++i) {
                int val = 0;
                while (!queue.dequeue(&val)) {} // 忙等直到取出元素
                sum += val;
            }
        };

    std::vector<std::thread> producers, consumers;
    for (int i = 0; i < numProducers; ++i) {
        producers.emplace_back(producer, 0);
    }
    
    for (int i = 0; i < numConsumers; ++i) {
        consumers.emplace_back(consumer);
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    int expected = numProducers * (itemsPerProducer + 1) * itemsPerProducer / 2;
    //std::cout << "expected sum: " << expected << std::endl;
    //std::cout << "actual sum: " << sum.load() << std::endl;
    assert(sum == expected);
    std::cout << "[+] MultiThreadTest Pass:" << test_id << endl;
}

void singleThreadTestForConcurrentqueue() {
    moodycamel::ConcurrentQueue<int> queue;
    int val = 0;
    assert(!queue.try_dequeue(val)); // 队列为空
    queue.enqueue(1);
    queue.try_dequeue(val);
    assert(val == 1); // 正确出队
    queue.enqueue(2);
    queue.enqueue(3);
    queue.try_dequeue(val);
    assert(val == 2);
    queue.try_dequeue(val);
    assert(val == 3);
    assert(!queue.try_dequeue(val)); // 队列再次为空

    cout << "[+] SingleThreadTest Pass" << endl;
}

void multiThreadTestForConcurrentqueue(int test_id) {
    moodycamel::ConcurrentQueue<int> queue;
    std::atomic<int> sum(0);
    constexpr int numProducers = 4;
    constexpr int numConsumers = 4;
    constexpr int itemsPerProducer = 10000;

    // 生产者线程
    auto producer = [&](int start) {
        for (int i = 0; i < itemsPerProducer; ++i) {
            queue.enqueue(start + i + 1); //规避掉0这个变量
        }
        };

    // 消费者线程
    auto consumer = [&] {
        for (int i = 0; i < itemsPerProducer * numProducers / numConsumers; ++i) {
            int val = 0;
            while (!queue.try_dequeue(val)) {} // 忙等直到取出元素
            sum += val;
        }
        };

    std::vector<std::thread> producers, consumers;
    for (int i = 0; i < numProducers; ++i) {
        producers.emplace_back(producer, 0);
    }

    for (int i = 0; i < numConsumers; ++i) {
        consumers.emplace_back(consumer);
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    int expected = numProducers * (itemsPerProducer + 1) * itemsPerProducer / 2;
    //std::cout << "expected sum: " << expected << std::endl;
    //std::cout << "actual sum: " << sum.load() << std::endl;
    assert(sum == expected);
    std::cout << "[+] ConcurrentQueue MultiThreadTest Pass:" << test_id << endl;
}

int main() {
    singleThreadTestForConcurrentqueue();
    singleThreadTest();
    for (int i = 0; i < 200; ++i) {
        multiThreadTestForConcurrentqueue(i + 1);
    }
    return 0;
}