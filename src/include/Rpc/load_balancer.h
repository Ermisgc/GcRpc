#include "net_base.h"
#include "data_structure/hashheap.hpp"
#include <vector>
#include <string>
#include <optional>
#include <atomic>

namespace GcRpc{
    using EndPointList = gcdst::HashHeap<ServiceEndpoint *, int>;

    class LoadBalancer{
    public:
        virtual ~LoadBalancer() = default;
        virtual std::optional<ServiceEndpoint *> select(const EndPointList & endpoints) = 0;
    };

    class RandomRuleBalancer :public LoadBalancer{
    public:
        std::optional<ServiceEndpoint *> select(const EndPointList & endpoints) override;
    };

    class RoundRobinBalancer :public LoadBalancer {
        std::atomic<size_t> counter{0};
    public:
        std::optional<ServiceEndpoint *> select(const EndPointList & endpoints) override;        
    };

    class BestScoreBalancer :public LoadBalancer {
    public:
        std::optional<ServiceEndpoint *> select(const EndPointList & endpoints) override;  
    };

    //简单工厂模式，用于产出一个LoadBalancer的具体类
    class LoadBalancerFactory{
        using Creator = std::function<LoadBalancer *(void)>;
        static inline std::unordered_map<std::string, Creator> & registy() {  
            //这里设计为单例，那么只会构造出一个instance对象，该对象用来存储string到Creator的映射
            static std::unordered_map<std::string, Creator> instance;
            return instance;
        }
    public:
        static void registerType(const std::string & name, Creator creator){
            registy()[name] = creator;
        }

        static std::optional<std::string> checkType(const std::string & type){
            for(const auto & [name, _]: registy()){
                if(type.find(name) != 0 && name.find(type) != 0){
                    continue;
                }
                return name;
            }
            return std::nullopt;
        }

        static std::optional<LoadBalancer *> create(const std::string & type){
            //将type全部转为小写
            std::string temp;
            temp.reserve(type.size());
            for(char ch: type){
                if(ch < 'a'){
                    temp.push_back(ch + 'a' - 'A');
                }
            }

            if(auto _t_n = checkType(temp); _t_n.has_value()){
                return registy()[_t_n.value()]();
            } else return std::nullopt;            
        }   
    };

#define REGISTER_LOAD_BALANCER_INTERNAL(type, class_name) \
    class class_name##Register{ \
    public: \
        class_name##Register(){  \
            LoadBalancerFactory::registerType(  \
                type,\
                []() -> LoadBalancer *{ \
                    return new class_name();  \
                }\
            ); \
        } \
    }; \
    static class_name##Register class_name##_register; 

    REGISTER_LOAD_BALANCER_INTERNAL("random", RandomRuleBalancer);
    REGISTER_LOAD_BALANCER_INTERNAL("roundrobin", RoundRobinBalancer);
    REGISTER_LOAD_BALANCER_INTERNAL("bestscore", BestScoreBalancer);
}