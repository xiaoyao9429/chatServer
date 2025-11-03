#ifndef USER_MODULE_H
#define USER_MODULE_H
#include "user.hpp"
#include "datebase.h"
#include <string>
using namespace std;
class UserModel{
    public:
        //增加用户
        bool insert(User& user);
        //删除用户
        bool deleteUser(int id);

        //更新用户状态
        bool updateState(User& user);

        //重置用户状态信息
        void resetState();

        //查询用户信息
        User query(int id);
    private:

};

#endif