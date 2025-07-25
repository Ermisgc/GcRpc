#include "Rpc/load_balancer.h"
namespace GcRpc{

    std::optional<std::string> RandomRuleBalancer::select(const EndPointList & endpoints){
        return endpoints.randomOne();
    }

    std::optional<std::string> RoundRobinBalancer::select(const EndPointList & endpoints){
        //CAS操作
        while(1){
            size_t key = counter.load(); 
            if(counter.compare_exchange_weak(key, key + 1)){
                return endpoints.getKey(key);
            }
        }
    }

    std::optional<std::string> BestScoreBalancer::select(const EndPointList & endpoints){
        return endpoints.maxOne();
    }  
}