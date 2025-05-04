#pragma once
#include <unordered_map>
namespace gcdst{

    template<class KeyType = int, class ValueType>
    class LRUCache {
        typedef struct LRUCache_Node{
            KeyType key;
            ValueType value;
            LRUCache_Node * next;
            LRUCache_Node * last;
            LRUCache_Node(KeyType _key, ValueType val):key(_key), value(val){}
        } Node;

        int capacity;
        int current_size;
        Node * head;
        Node * tail;
        std::unordered_map<KeyType, Node *> hash;
    public:
        LRUCache(int capacity):capacity(capacity), current_size(0) {
            head = new Node();
            tail = new Node();
            head->next = tail;
            tail->last = head;
        }
        
        int get(KeyType key) {
            if(!hash.count(key)) return -1;
            else{
                hash[key]->last->next = hash[key]->next;
                hash[key]->next->last = hash[key]->last;
                moveToFront(key);
                return hash[key]->value;
            } 
        }
        
        void put(KeyType key, KeyType value) {
            if(hash.count(key)){
                hash[key]->value = value;
                hash[key]->last->next = hash[key]->next;
                hash[key]->next->last = hash[key]->last;
            } else {
                Node * temp = new Node(key, value);
                hash[key] = temp;          
                current_size += 1;
            }

            moveToFront(key);
            if(current_size > capacity) evictLast();
        }

    private:
        void moveToFront(KeyType key){
            hash[key]->next = head->next;
            head->next->last = hash[key];
            head->next = hash[key];
            hash[key]->last = head;  
        }

        void evictLast(){
            Node * temp = tail->last;
            temp->last->next = tail;
            tail->last = temp->last;
            hash.erase(temp->key);
            delete temp;
            current_size --;
        }
    };
}