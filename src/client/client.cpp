#include <iostream>
#include <string>
#include "json.hpp"
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "group.hpp"
#include "user.hpp"
#include "public.h"
#include <chrono>
#include <thread>
#include <chrono>
#include <time.h>
#include <functional>
#include <unordered_map>
#include <atomic>
using json=nlohmann::json;
using namespace std;
User CurrentUser;//当前登录的用户
vector<User> CurrentUserFriendList;//好友列表
vector<Group> CurrentUserGroupList;//群组列表

atomic<bool> g_readThreadRunning{false};//线程退出标志
thread g_readThread;//用户接收服务器消息的线程(readTaskHandler)，全局唯一，用户退出登录时需要关闭

//--------以下函数为聊天界面中功能服务---------------
void showFriendList();//显示好友列表
void showGroupList();//显示群组列表
void sendMessage(int cfd ,const json& js);//向服务器发送JS字符串
void help(int cfd=-1,string msg="");//帮助菜单,显示所有支持的功能
void OneChat(int cfd,string msg);//1对1闲聊
void groupChat(int cfd,string msg);//发送群聊消息
void addFriend(int cfd,string msg);//添加好友
void createGroup(int cfd,string msg);//创建群组
void addGroup(int cfd,string msg);//加入群组
void logOut(int cfd,string msg);//退出当前用户登录
//------------------------------------------------
unordered_map<string,function<void(int,string)>> loginMenuMap={
   {"oneChat",OneChat},{"groupChat",groupChat},{"addFriend",addFriend},
    {"createGroup",createGroup},{"addGroup",addGroup},{"logOut",logOut},
    {"help",help}
};//登录后菜单选项映射表

void mainMenu();//主界面
void ShowLoginMenu();//聊天主界面(支持发送消息、查看好友列表、查看群组列表、退出登录、添加好友、创建群组、加入群组、发送群消息)

void showCurrentUserData();//显示当前用户信息
string getCurrentTime();//获取系统时间，聊天信息需要添加时间戳
void readTaskHandler(int clientfd);//子线程，负责接收服务器端发送的消息

const string serverIp="192.168.88.136";
const uint16_t serverPort=8000;
int cfd;

int main(int argc,char* argv[]){

    cfd=socket(AF_INET,SOCK_STREAM,0);
    if (cfd == -1)
    {
        cout<<"通信套接字创建失败"<<endl;
        exit(-1);
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(serverPort);
    serverAddr.sin_addr.s_addr=inet_addr(serverIp.c_str());

    int c=connect(cfd,(sockaddr*)&serverAddr,sizeof(serverAddr));
    if(c==-1){
        cout<<"连接服务器失败"<<endl;
        close(cfd);
        exit(-1);
    }
    
    char tempBuffer[1024]={0};
    int temp_r=recv(cfd,tempBuffer,1024,0);
    if(temp_r==-1){
        cout<<"接收服务器欢迎消息失败"<<endl;
        close(cfd);
        exit(-1);
    }
    else if(temp_r==0){
        cout<<"服务器断开连接";
        close(cfd);
        exit(-1);
    }
    else{
        cout<<tempBuffer<<endl;//连接服务器成功，打印返回消息
    }

    
     mainMenu();//进入主界面
    
    return 0;
}

void mainMenu(){
    while(1){
            cout<<"------------------欢迎进入聊天系统------------------"<<endl;
            cout<<"|                  1.登录账号                     |"<<endl;
            cout<<"|                  2.注册账号                     |"<<endl;
            cout<<"|                  3.退出系统                     |"<<endl;
            cout<<"--------------------------------------------------"<<endl;
            cout<<"请选择(1-3):";
            int choice=0;
              // 读取输入并检查是否成功
            if (!(cin >> choice)) { 
                // 输入失败（非整数），处理错误
                cin.clear(); // 清除错误标志位，恢复cin正常状态
                // 清空缓冲区中所有残留的无效数据（直到换行符）
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
                cout << "输入有误，请输入数字1-3！" << endl;
                return; // 直接返回，重新显示菜单
            }
        switch(choice){
            case 1:{
                    cout<<"请输入用户ID:"; int id=0;  cin>>id;   cin.get();  
                    cout<<"请输入用户密码:"; string pwd; cin>>pwd; cin.get();  
                    json js;
                    js["msgid"]=LOGIN_MSG;
                    js["id"]=id;
                    js["password"]=pwd;
                    string request=js.dump();
                    send(cfd,request.c_str(),strlen(request.c_str())+1,0);
                    char buffer[4096]={0};
                    int r=recv(cfd,buffer,sizeof(buffer),0);
                    if(r==-1){
                        cout<<"接收服务器响应失败"<<endl;
                            break;
                    }
                    else if(r==0){
                        cout<<"服务器断开连接,登录失败"<<endl;
                        close(cfd);
                        exit(-1);
                    }

                    else{
                        buffer[r]='\0';
                        json response=json::parse(buffer);
                        if(response["errno"].get<int>()==0){
                            //cout<<response<<endl; --->response中有很多数据
                            //登录成功
                            CurrentUser.setId(response["id"].get<int>());
                            CurrentUser.setName(response["name"].get<string>());
                            CurrentUser.setState(response["state"].get<string>());

                            //获取用户好友列表
                            if(response.contains("friends")){
                                vector<string> temps=response["friends"].get<vector<string>>();
                                for(auto &s:temps){
                                    json temp=json::parse(s);
                                    User f;
                                    f.setId(temp["id"].get<int>());
                                    f.setName(temp["name"].get<string>());
                                    f.setState(temp["state"].get<string>());
                                    CurrentUserFriendList.push_back(f);
                                }
                            }
                            
                            //获取用户群组列表信息
                            if(response.contains("groups")){
                                vector<string> temps=response["groups"].get<vector<string>>();
                                for(auto &s:temps){
                                    json temp=json::parse(s);
                                    Group g;
                                    g.setId(temp["groupid"].get<int>());
                                    g.setName(temp["groupname"].get<string>());
                                    g.setDesc(temp["groupdesc"].get<string>());
                                    CurrentUserGroupList.push_back(g);
                                }
                            }
                            
                            //显示当前用户信息
                            showCurrentUserData();

                            //显示当前离线消息，个人聊天消息或群聊消息
                            //todo


                            //启动接收线程
                            g_readThread=thread(readTaskHandler,cfd);

                            //登录成功后进入聊天主菜单
                            ShowLoginMenu();
                            
                        

                        }
                        else{
                            cout<<response<<endl;
                            cout<<"登录失败,错误信息:"<<response["info"].get<string>()<<endl;
                        }
                    }

                    
                break;
            }

            case 2:{
                    cout<<"请输入注册的昵称:";string newName; cin>>newName; cin.get();
                    cout<<"请输入注册的密码:";string pwd; cin>>pwd; cin.get();
                    cout<<"请再次输入密码:";string pwd2; cin>>pwd2; cin.get();
                    if(pwd!=pwd2){
                        cout<<"两次密码输入不一致，注册失败"<<endl;
                        break;
                    }
                    json js;
                    js["msgid"]=REG_MSG;
                    js["name"]=newName;
                    js["password"]=pwd;
                    string request=js.dump();
                    int s=send(cfd,request.c_str(),strlen(request.c_str())+1,0);
                    if(s==-1){
                        cout<<"发送注册信息失败"<<endl;
                        break;
                    }
                    else{
                        char buffer[4096]={0};
                        int r=recv(cfd,buffer,sizeof(buffer),0);
                        if(r==-1){
                            cout<<"接收服务器响应失败"<<endl;
                            break;
                        }
                        else if(r==0){
                            cout<<"服务器断开连接，注册失败"<<endl;
                            close(cfd);
                            exit(-1);
                        }
                        else{
                            buffer[r]='\0';
                            json response=json::parse(buffer);
                            if(response["errno"].get<int>()==0){
                                cout<<response<<endl;
                                cout<<"注册成功,请牢记您的用户ID:"<<response["id"].get<int>()<<endl;
                            }
                            else{
                                cout<<response<<endl;
                                cout<<"注册失败,错误信息:"<<response["info"].get<string>()<<endl;
                            }
                        }
                    }

                    break;
            }
            
            case 3:
                close(cfd);
                exit(0);
                break;
            default:
                cout<<"输入有误，请重新输入您的选择:"<<endl;
                break;

        }
    }
}

void showCurrentUserData(){
    cout<<"------------------当前用户信息------------------"<<endl;
    cout<<"|用户ID:"<<CurrentUser.getId()<<endl;
    cout<<"|用户名:"<<CurrentUser.getName()<<endl;
    cout<<"|用户状态:"<<CurrentUser.getState()<<endl;
    cout<<"------------------------------------------------"<<endl;
    cout<<endl;

   showFriendList();
   showGroupList();
}


void readTaskHandler(int clientfd){
   
    g_readThreadRunning = true;

    while(1){
        char buffer[4096]={0};
        int r=recv(clientfd,buffer,sizeof(buffer),0);
        json js;
        if(r==-1){
            cout<<"接收服务器消息失败"<<endl;
            break;
        }
        else if(r==0){
            cout<<"服务器断开连接,接收消息失败"<<endl;
            close(clientfd);
            exit(-1);
        }
        else{
            js=json::parse(buffer);
            buffer[r]='\0';
            cout<<"------------------"<<endl;
            cout<<"线程读取数据:"<<js<<endl;
            cout<<"------------------"<<endl;
            //根据msgid处理不同的消息
            //todo
            if(js["msgid"].get<int>()==SEND_ONE_CHAT_MSG){
                cout<<"在时间为:"<<getCurrentTime()<<"收到来自用户ID为"<<js["fromID"].get<int>()<<"的消息:"<<js["msg"].get<string>()<<endl;
            }
            else if(js["msgid"].get<int>()==GROUP_CHAT_MSG){
                cout<<"收到来自群组ID为"<<js["groupid"].get<int>()<<"中用户ID为"<<js["fromID"].get<int>()<<"的消息:"<<js["msg"].get<string>()<<endl;
            }

            else if(js["msgid"].get<int>()==ADD_FRIEND_MSG_ACK){
                cout<<js<<endl;   
            }

            if(g_readThreadRunning==false) break;//如果用户退出登录，标志被设置为false,跳出循环，结束线程
        }

    }
}

string getCurrentTime(){
    auto now=chrono::system_clock::now();
    time_t now_time_t=chrono::system_clock::to_time_t(now);
    char buf[40];
    tm mytm;
    localtime_r(&now_time_t,&mytm);
    strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&mytm);
    return string(buf);
}

void ShowLoginMenu(){
        help();
        while(1){
            cout<<"-------------欢迎来到聊天界面--------------"<<endl;
            cout<<"请输入您的功能指令:"<<endl;
            string buffer;  cin>>buffer; //cin>>的输入方式无法读取空格,是一个问题,待解决
            int pos=buffer.find(":");
            string command=buffer.substr(0,pos);

            if(command=="logOut"){
                loginMenuMap[command](cfd,buffer);
                
                break;
            }
            if(loginMenuMap.find(command)!=loginMenuMap.end()){

                loginMenuMap[command](cfd,buffer);
            }
            else{
                cout<<"无此功能指令，请重新输入!"<<endl;
            }

    }
     return ;
}

void help(int cfd ,string msg){
    cout<<"------------------帮助菜单------------------"<<endl;
    //help、oneChat、groupChat...为想要执行的功能
    cout<<"help  格式:help"<<endl;
    cout<<"oneChat 格式:oneChat:<toid>:<msg>"<<endl;
    cout<<"groupChat 格式:groupChat:<groupid>:<msg>"<<endl;
    cout<<"addFriend 格式:addFriend:<friendid>"<<endl;
    cout<<"createGroup 格式:createGroup:<groupname>:<groupdesc>"<<endl;
    cout<<"addGroup 格式:addGroup:<groupid>"<<endl;
    cout<<"showFriendList 格式:showFriendList"<<endl;
    cout<<"showGroupList 格式:showGroupList"<<endl;
    cout<<"logOut 格式:logOut"<<endl;
}

void showFriendList(){
    cout<<"------------------当前用户好友列表------------------"<<endl;
    if(CurrentUserFriendList.size()!=0){
          for(auto f:CurrentUserFriendList){
            cout<<"|好友ID:"<<f.getId()<<" "<<"好友昵称:"<<f.getName()<<" "<<"好友状态:"<<f.getState()<<endl;
        }
    }
    cout<<"----------------------------------------------------"<<endl;
    cout<<endl;

}

void showGroupList(){

     cout<<"------------------当前用户群组列表------------------"<<endl;
    if(CurrentUserGroupList.size()!=0){
        for(auto g:CurrentUserGroupList){
            cout<<"|群组ID:"<<g.getId()<<" "<<"群组名称:"<<g.getName()<<" "<<"群组描述:"<<g.getDesc()<<endl;
        }
    }
    cout<<"----------------------------------------------------"<<endl;
    cout<<endl;
    
}

void OneChat(int cfd,string msg){
    //1v1聊天json格式
    //向服务器发送:{"time":curTime,"fromName":curName,"fromID":curId,"toID":toid,"msg":sendmsg,"msgid":SEND_ONE_CHAT_MSG}
	int toid;
    string sendmsg;
    string s = msg;

    int st = 0;
    int ret= s.find(":", st);//第一个冒号的位置
    st = ret+1;
    int ret2 = s.find(":", st);//第二个冒号的位置


    toid = stoi(s.substr(ret+1, ret2-ret-1));
    sendmsg = s.substr(ret2+1, s.size()-(ret2+1));

    json js;
    js["msg"]=sendmsg;
    js["fromID"]=CurrentUser.getId();
    js["toID"]=toid;
    js["msgid"]=SEND_ONE_CHAT_MSG;
    js["fromName"]=CurrentUser.getName();
    js["time"]=getCurrentTime();
    sendMessage(cfd, js);//向服务器发送json字符串
    return ;
}

void sendMessage(int cfd, const json& js){
    string request=js.dump();
    int s=send(cfd,request.c_str(),strlen(request.c_str())+1,0);
    if(s==-1){
        cout<<"客户端发送请求消息失败"<<endl;
    }
}

void groupChat(int cfd,string msg){
    //发送群聊消息

    int groupid;
    string sendmsg;
    string s = msg;

    int st = 0;
    int ret= s.find(":", st);//第一个冒号的位置
    st = ret+1;
    int ret2 = s.find(":", st);//第二个冒号的位置

    groupid = stoi(s.substr(ret+1, ret2-ret-1));
    sendmsg = s.substr(ret2+1);

    json js;
    js["msgid"]=GROUP_CHAT_MSG;
    js["fromID"]=CurrentUser.getId();
    js["groupID"]=groupid;
    js["msg"]=sendmsg;
    sendMessage(cfd, js);
    return ;
}

void addFriend(int cfd,string msg){
    string s = msg;
    int friendid=stoi(s.substr(s.find(":")+1));//此处缺少对friend的判空(数据库用户表中无此用户)，其他某些业务逻辑也忘记写了
    json js;                                     //还缺少对msg格式的判断,比如addFriend:abc这种格式
    js["fromID"]=CurrentUser.getId();
    js["friendID"]=friendid;
    js["msgid"]=ADD_FRIEND_MSG;
    sendMessage(cfd, js);
    return ;
}

void createGroup(int cfd,string msg){
    
    //对msg格式的判断，之后再写

    //

    string s=msg;

    string groupname;
    string groupdesc;
    int st = 0;
    int ret= s.find(":", st);//第一个冒号的位置
    st = ret+1;
    int ret2 = s.find(":", st);//第二个冒号的位置
    groupname = s.substr(ret+1, ret2-ret-1);
    groupdesc = s.substr(ret2+1);


    json js;
    js["msgid"]=CREATE_GROUP_MSG;
    js["fromID"]=CurrentUser.getId();
    js["groupname"]=groupname;
    js["groupdesc"]=groupdesc;
    sendMessage(cfd, js);

    
    return ;
}

void addGroup(int cfd,string msg){
    //对msg格式的判断，之后再写
    string s = msg;
    int groupid;
    int st = 0;
    int ret= s.find(":", st);//第一个冒号的位置
    st = ret+1;
    int ret2 = s.find(":", st);//第二个冒号的位置
    groupid = stoi(s.substr(ret+1, ret2-ret-1));
    json js;
    js["msgid"]=ADD_GROUP_MSG;
    js["fromID"]=CurrentUser.getId();
    js["groupID"]=groupid;
    sendMessage(cfd, js);
    return ;
}

void logOut(int cfd,string msg){
    json js;
    js["msgid"]=LOGINOUT_MSG;
    js["id"]=CurrentUser.getId();
    sendMessage(cfd, js);
    g_readThreadRunning = false;
      // 等待线程结束（如果有的话）
    if (g_readThread.joinable()) {
        g_readThread.join();
    }
    //清空当前用户数据
    CurrentUser=User();
    CurrentUserFriendList.clear();
    CurrentUserGroupList.clear();
    return ;
}

