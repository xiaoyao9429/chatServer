#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>
#include "user.hpp"
#include "public.h"
using namespace std;
class FriendModel{

    public:
        //添加好友
        bool addFriend(int userid,int friendid);

        //删除好友
        void deleteFriend(int userid,int friendid);

        //返回用户的好友列表(返回好友状态信息)
        vector<User> query(int userid);

};

#endif