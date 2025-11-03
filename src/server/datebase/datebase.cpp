#include "datebase.h"
#include <muduo/base/Logging.h>
#include <string>
using namespace std;
Mysql::Mysql(){
    m_conn=mysql_init(nullptr);
}

Mysql::~Mysql(){
    if(m_conn!=nullptr){
        mysql_close(m_conn);
    }
}
bool Mysql::update(string sql){
    if(mysql_query(m_conn,sql.c_str())){
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"更新失败!";
        return false;
    }
    return true;
}


MYSQL_RES* Mysql::query(string sql){
    if(mysql_query(m_conn,sql.c_str())){
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"查询失败!";
        return nullptr;
    }
    return mysql_store_result(m_conn);
}

bool Mysql::connect(){
    MYSQL* p=mysql_real_connect(m_conn,server.c_str(),user.c_str(),password.c_str(),dbname.c_str(),3306,nullptr,0);
    if(p==nullptr){
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<"连接数据库失败!";
        return false;
    }
    //设置字符集
    mysql_set_character_set(m_conn,"UTF-8");
    return true;
}