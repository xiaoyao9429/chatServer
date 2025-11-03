#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer{
    public:
        ChatServer(EventLoop* loop,const InetAddress& listenAddr,const string& nameArg);
        void start();//启动服务

    private:
        void onConnectionCallback(const TcpConnectionPtr& conn);//连接回调
        void onMessageCallback(const TcpConnectionPtr& conn,//消息回调
                       Buffer* buf,
                       Timestamp time);

        TcpServer m_server;
        EventLoop* m_loop;
};



#endif