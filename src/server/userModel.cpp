#include "userModel.h"


bool UserModel::insert(User& user){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into User(name,password,state) values('%s','%s','%s')",user.getName().c_str(),user.getPassword().c_str(),user.getState().c_str());
    
    Mysql mysql;
    if(mysql.connect()){
       if(mysql.update(sql)){
           user.setId(mysql_insert_id(mysql.getConnection()));//获取插入成功的用户数据生成的主键id
           return true;
       }
       
    }
     return false;  
}


bool UserModel::deleteUser(int id){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"delete from User where id=%d",id);

    Mysql mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            return true;
        }

    }
     return false;  
}

void UserModel::resetState(){
    //把所有用户的状态设置为offline
    char sql[1024]={0};
    sprintf(sql,"update User set state='offline' where state='online'");

    Mysql mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }

    return ;
}

bool UserModel::updateState(User& user){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"update User set state='%s' where id=%d",user.getState().c_str(),user.getId());

    Mysql mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            return true;
        }

    }
     return false;  
}


User UserModel::query(int id){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"select * from User where id=%d",id);

    Mysql mysql;
    if(mysql.connect()){
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row=mysql_fetch_row(res);
            if(row!=nullptr){
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);//必须手动释放
                return user;
            }
            mysql_free_result(res);//必须手动释放
        }
    }

    return User(); //返回一个空用户对象
}