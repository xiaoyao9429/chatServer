#ifndef GROUP_H
#define GROUP_H
#include <vector>
#include "groupUser.hpp"
#include <string>
using namespace std;

class Group{
    public:
        Group(int id=0,string name="",string desc=""):groupid(id),groupname(name),groupdesc(desc){}
        void setId(int id){this->groupid=id;}
        int getId() const {return groupid;}

        void setName(const string& name){this->groupname=name;}
        string getName() const {return groupname;}

        void setDesc(const string& desc){this->groupdesc=desc;}
        string getDesc() const {return groupdesc;}

        void push_UserVec(const GroupUser& user){userVec.push_back(user);}
        vector<GroupUser> getUserVec() const {return userVec;}

    private:
        int groupid;//群id
        string groupname;//群名称
        string groupdesc;//群描述
        vector<GroupUser> userVec;//群成员列表
};

#endif