#include "Rpc/channel.h"
#include <assert.h>

namespace GcRpc{
    unsigned long long getIOUringUserdata(UringChannel * tc, uint8_t event_type){
        assert((reinterpret_cast<uintptr_t>(tc) & 0xFFFF'0000'0000'0000) == 0);  //验证指针高位是否全是0
        return reinterpret_cast<uint64_t>(tc) | ((uint64_t)event_type << 48);
    }

    void executeIOUringCallback(unsigned long long user_data, int res){
        auto tc = reinterpret_cast<UringChannel *>(user_data & 0x0000'1111'1111'1111);
        uint8_t event_type = static_cast<uint8_t>(user_data >> 48);
        tc->do_io_uring(res, event_type);
    }
}