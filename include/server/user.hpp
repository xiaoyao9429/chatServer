#ifndef USER_H
#define USER_H

#include <string>
using namespace std;
class User{
    public:

        User(int id=0,string name="",string password="",string state="offline"):id(id),name(name),password(password),state(state){}
        void setId(int id){this->id=id;}
        int getId() const {return id;}

        void setName(const string& name){this->name=name;}
        string getName() const {return name;}

        void setPassword(const string& password){this->password=password;}
        string getPassword() const {return password;}

        void setState(const string& state){this->state=state;}
        string getState() const {return state;}

    private:
        int id;
        string name;
        string password;
        string state;
};

#endif