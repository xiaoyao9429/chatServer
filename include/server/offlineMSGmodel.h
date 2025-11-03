#ifndef OFFLINE_MSG_MODEL_H
#define OFFLINE_MSG_MODEL_H
#include <vector>
#include <string>
using namespace std;
class OffLineMsgModel{
    public:
        //存储用户的离线消息
        bool insert(int userid,const string& msg);
        //删除用户的离线消息
        bool remove(int userid);
        //查询用户的离线消息
        vector<string> query(int userid);
};

#endif