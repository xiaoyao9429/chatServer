#ifndef DATABASE_H
#define DATABASE_H
#include <mysql/mysql.h>
#include <string>
using namespace std;


static string server="127.0.0.1";
static string user="root";
static string password="123456";
static string dbname="chat";


class Mysql{
    public:
        Mysql();
        ~Mysql();
        bool connect();
        bool update(string sql);
        MYSQL_RES* query(string sql);
        MYSQL* getConnection() const { return m_conn; }

    private:
        MYSQL* m_conn;
};

#endif