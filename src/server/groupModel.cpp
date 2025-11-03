#include "groupModel.h"
#include "datebase.h"
bool GroupModel::createGroup(Group& group){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into AllGroup(groupname,groupdesc) values('%s','%s')",group.getName().c_str(),group.getDesc().c_str());
    
    Mysql mysql;
    if(mysql.connect()){
       if(mysql.update(sql)){
           group.setId(mysql_insert_id(mysql.getConnection()));//获取插入成功的用户数据生成的主键id
           return true;
       }
       
    }
     return false;  
}

//加入一个群组，名字起的有歧义
void GroupModel::addGroup(int userid,int groupid,string role){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into GroupUser(groupid,userid,groupRole) values(%d,%d,'%s')",groupid,userid,role.c_str());
    
    Mysql mysql;
    if(mysql.connect()){
       mysql.update(sql);
    }
     return ;  
}

vector<Group> GroupModel::queryGroups(int userid){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id=b.groupid where b.userid=%d",userid);

    vector<Group> vec;
    Mysql mysql;
    if(mysql.connect()){
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                vec.push_back(group);
            }
            mysql_free_result(res);//必须手动释放
        }
    }

    //查询每个群聊的用户信息
    for(auto& f:vec){
        //组织sql语句
        char sql[1024]={0};
        sprintf(sql,"select a.id,a.name,a.state,b.groupRole from User a inner join GroupUser b on b.userid=a.id where b.groupid=%d",f.getId());

        Mysql mysql;
        if(mysql.connect()){
            MYSQL_RES* res=mysql.query(sql);
            if(res!=nullptr){
                MYSQL_ROW row;
                while((row=mysql_fetch_row(res))!=nullptr){
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    f.push_UserVec(user);
                }
                mysql_free_result(res);//必须手动释放
            }
        }

    }




    return vec;
}

vector<int> GroupModel::queryGroupUsers(int userid,int groupid){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"select userid from GroupUser where groupid=%d and userid!=%d",groupid,userid);

    vector<int> vec;
    Mysql mysql;
    if(mysql.connect()){
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);//必须手动释放
        }
    }

    return vec;
}