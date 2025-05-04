#include "test.pb.h"
#include <google/protobuf/service.h>
#include <iostream>
#include <string>

int main2(){
    // gctemp::LoginResponse rsp;
    // gctemp::ResultCode *rc = rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("Fail to login in");

    gctemp::GetFriendListResponse rsp;
    gctemp::ResultCode * rc = rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("Fail to login in");
    
    gctemp::User *user1 = rsp.add_friend_list();
    user1->set_name("gc");
    user1->set_age(20);
    user1->set_sex(gctemp::User::MAN);

    gctemp::User *user2 = rsp.add_friend_list();
    user1->set_name("gc");
    user1->set_age(19);
    user1->set_sex(gctemp::User::MAN);

    std::cout << rsp.friend_list_size() << std::endl;

    return 0;
}


int main1(){
    //package the data of LoginRequest
    gctemp::LoginRequest rq;
    rq.set_name("gc");
    rq.set_pwd("214061");

    //Serialize -> char *
    std::string send_str;
    if(rq.SerializeToString(&send_str)){
        std::cout << send_str.c_str() << std::endl;
    }

    //Parse
    gctemp::LoginRequest reqB;
    if(reqB.ParseFromString(send_str)){
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}

int main(){
    //package the data of LoginRequest
    gctemp::ResultCode rc;
    rc.set_errmsg("gc");
    rc.set_errcode(1);

    //Serialize -> char *
    std::string send_str;
    if(rc.SerializeToString(&send_str)){
        std::cout << std::hex << send_str.c_str() << std::endl;
    }

    //Parse
    gctemp::ResultCode rcB;
    if(rcB.ParseFromString(send_str)){
        std::cout << rcB.errmsg() << std::endl;
        std::cout << rcB.errcode() << std::endl;
    }

    return 0;
}