#include"offlineMSGmodel.h"
#include <vector>
#include "datebase.h"



bool OffLineMsgModel::insert(int userid,const string& msg){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into offLineMessage(userid,message) values(%d,'%s')",userid,msg.c_str());
    
    Mysql mysql;
    if(mysql.connect()){
       if(mysql.update(sql)) return true;
       
    }
     return false;  
}


bool OffLineMsgModel::remove(int userid){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"delete from offLineMessage where userid=%d",userid);

    Mysql mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            return true;
        }

    }
     return false;  
}


vector<string> OffLineMsgModel::query(int userid){
    //组织sql语句
    char sql[1024]={0};
    sprintf(sql,"select message from offLineMessage where userid=%d",userid);

    vector<string> vec;
    Mysql mysql;
    if(mysql.connect()){
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                vec.push_back(row[0]);
            }
            mysql_free_result(res);//必须手动释放
        }
    }

    return vec;
}