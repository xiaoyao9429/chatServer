#include<iostream>
#include"ChatServer.h"
#include <signal.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include "ChatService.h"
using namespace std;

// 处理服务器Ctrl+C退出后，重置用户的状态信息
void resetHandler(int signum){
    ChatService::instance()->reset();
    exit(0);
}
int main(){

    cout<<"启动"<<endl;
    int port;
    cout<<"请输入服务器监听的端口:";
    cin>>port;
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr("0.0.0.0",port);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();


    


    return 0;
}