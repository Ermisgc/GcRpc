/**
 * @struct HashHeap是哈希表+堆的一种数据结构，它其实是一种复合结构，内部维护了哈希表和堆
 * 它其实是模仿Redis的ZSet有序集合的一种简易实现，但这里并不需要做范围查询，而是高效地查询第一位即可
 * 
 */
#ifndef HASHHEAP_HPP
#define HASHHEAP_HPP
#include <unordered_map>
#include <map>
#include <mutex>
#include <optional>
#include <random>

namespace gcdst{
    template<typename Key, typename Score = int>
    class HashHeap{
    private:
        //multimap维护可重复键值的红黑树，作用是快速获得最大值
        std::multimap<Score, Key, std::greater<Score>> sorted_scores;

        //维护从key到score的索引
        std::unordered_map<Key, std::pair<Score, typename std::multimap<Score, Key>::iterator>> key_index;

        //key列表，用于随机访问
        std::vector<const Key *> random_list; 

        //维护key到random_list中位置的映射
        std::unordered_map<Key, size_t> key_to_random_list;

        //一把大锁，锁的都是O(logN)的区域，应该还好
        mutable std::mutex mtx;
        
        //这里设为mutable，为了让函数中传入的const HashHeap也能更改随机数生成器
        mutable std::mt19937 rng{std::random_device{}()};  

    public:
        //插入/更新
        void insert(const Key & key, const Score & score){
            std::unique_lock<std::mutex> locker(mtx);
            bool isExist = false;
            if(auto itr = key_index.find(key); itr != key_index.end()){
                sorted_scores.erase(itr->second.second);  //删除旧值
                isExist = true;
            }

            auto pos = sorted_scores.emplace(score, key);
            key_index[key] = {score, pos};
            if(!isExist) add_random_key(key);
        }

        std::optional<Key> maxOne() const noexcept {
            {
                std::unique_lock<std::mutex> locker(mtx);
                if(sorted_scores.empty()) return std::nullopt;
                return sorted_scores.begin()->second;
            }
        }

        std::optional<Key> randomOne() const noexcept {
            std::unique_lock<std::mutex> locker(mtx);
            if(sorted_scores.empty()) return std::nullopt;

            //均匀int分布
            std::uniform_int_distribution<size_t> dist(0, random_list.size() - 1);
            size_t index = dist(rng);  //仿函数获取index
            return *random_list[index];
        }

        std::optional<Key> getKey(size_t index) const noexcept{
            std::unique_lock<std::mutex> locker(mtx);
            if(sorted_scores.empty()) return std::nullopt;
            return *random_list[index % random_list.size()];
        }

        void remove(const Key & key){
            std::unique_lock<std::mutex> locker(mtx);
            if(auto itr = key_index.find(key); itr != key_index.end()){
                sorted_scores.erase(itr->second.second);
                key_index.erase(itr);
            }
            remove_random_key(key);
        }

        size_t size() const {
            std::unique_lock<std::mutex> locker(mtx);
            return key_index.size();
        }

        void clear() {
            std::unique_lock<std::mutex> locker(mtx);
            sorted_scores.clear();
            key_index.clear();
            random_list.clear();
            key_to_random_list.clear();
        }

    private:
        void add_random_key(const Key & key){
            random_list.push_back(&key);
            key_to_random_list[key] = random_list.size() - 1;
        }

        //用交换的方式从列表中移除key
        void remove_random_key(const Key & key){
            size_t pos_key = key_to_random_list[key];
            std::swap(random_list[pos_key], random_list.back());

            random_list.pop_back();
            key_to_random_list.erase(key);
            key_to_random_list[*random_list[pos_key]] = pos_key;
        }
    };

}

#endif