#ifndef GROUP_USER_H
#define GROUP_USER_H
#include "user.hpp"
#include <string>
using namespace std;
class GroupUser:public User//继承User类
{
    public:
       
        string getRole() const {return role;}
        void setRole(const string& role){this->role=role;}


    private:
       string role;//用户在群组中的角色(用户在群聊中附加的信息)
};


#endif