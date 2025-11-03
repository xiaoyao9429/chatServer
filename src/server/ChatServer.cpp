#include "ChatServer.h"
#include <functional>
#include "json.hpp"
#include "ChatService.h"
#include <string>
#include <iostream>
#include <muduo/base/Logging.h>
using josn=nlohmann::json;
using namespace std;


ChatServer::ChatServer(EventLoop* loop,const InetAddress& listenAddr,const string& nameArg):m_server(loop,listenAddr,nameArg)
{
    this->m_loop=loop;
    this->m_server.setThreadNum(4);//设置底层subloop的数量
    //注册连接回调
    m_server.setConnectionCallback(bind(&ChatServer::onConnectionCallback,this,placeholders::_1));
    //注册消息回调
    m_server.setMessageCallback(bind(&ChatServer::onMessageCallback,this,placeholders::_1,placeholders::_2,placeholders::_3));
}

void ChatServer::start(){
    m_server.start();
}


void ChatServer::onConnectionCallback(const TcpConnectionPtr& conn){
    
    if(!conn->connected()){
        conn->shutdown();//关闭连接
        ChatService::instance()->clientCloseExpt(conn);//客户端异常退出
    }

    else{
       string client=conn->peerAddress().toIpPort();
       string server=conn->localAddress().toIpPort();
       conn->send(client+" -> "+server+" 已连接\n");
    }
}

void ChatServer::onMessageCallback(const TcpConnectionPtr& conn,
                       Buffer* buf,
                       Timestamp time){
    string msg=buf->retrieveAllAsString();             
    //数据的反序列化
    json js=json::parse(msg);
    cout<<js<<endl;
    //通过js["msgid"]获取消息id，然后调用对应的业务处理方法
    MsgHandler msgHandler=ChatService::instance()->getHandler(js["msgid"].get<int>());
    //回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn,js,time);
}
