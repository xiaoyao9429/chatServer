#include "friendModel.h"
#include "datebase.h"
#include "json.hpp"
#include <vector>
using namespace std;
using json=nlohmann::json;
bool FriendModel::addFriend(int userid,int friendid){
   
   
  
    char sql[1024]={0};
    sprintf(sql,"insert into Friend(userid,friendid) values(%d,%d)",userid,friendid);
    
    Mysql mysql;
    if(mysql.connect()){
       if(mysql.update(sql)) return true;
       
    }
     return false;  
}


vector<User> FriendModel::query(int userid){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"select a.id,a.name,a.state from User a inner join Friend b on a.id=b.friendid where b.userid=%d union select a.id,a.name,a.state from User a inner join Friend b on a.id=b.userid where b.friendid=%d",userid,userid);

    vector<User> vec;
    Mysql mysql;
    if(mysql.connect()){
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);//必须手动释放
        }
    }

    return vec;
}


void FriendModel::deleteFriend(int userid,int friendid){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"delete from Friend where userid=%d and friendid=%d",userid,friendid);

    Mysql mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            return ;
        }

    }

    return ;
}