#ifndef CHAT_SERVICE_H
#define CHAT_SERVICE_H

#include <muduo/net/TcpConnection.h>
#include <functional>
#include <unordered_map>
#include <mutex>
#include "json.hpp"
#include "userModel.h"
#include "offlineMSGmodel.h"
#include "friendModel.h"
#include "groupModel.h"
#include "redis.h"
using json=nlohmann::json;
using namespace muduo;
using namespace muduo::net;
using namespace std;

using MsgHandler=function<void(const TcpConnectionPtr& conn,json& js,Timestamp time)>;


class ChatService{
    public:
        static ChatService* instance();
        //处理登录业务
        void login(const TcpConnectionPtr& con,json& js,Timestamp time);
        //处理退出登录业务
        void loginOut(const TcpConnectionPtr& con,json& js,Timestamp time);
        //处理注册业务
        void reg(const TcpConnectionPtr& con,json& js,Timestamp time);
        //处理未知业务
        void unknow(const TcpConnectionPtr& con,json& js,Timestamp time);
        //一对一聊天业务
        void SendOneChat(const TcpConnectionPtr& con,json& js,Timestamp time);
        //添加好友业务
        void addFriend(const TcpConnectionPtr& con,json& js,Timestamp time);
        //创建群组业务
        void createGroup(const TcpConnectionPtr& con,json& js,Timestamp time);
        //加入群组业务
        void addGroup(const TcpConnectionPtr& con,json& js,Timestamp time);
        //群聊天业务
        void groupChat(const TcpConnectionPtr& con,json& js,Timestamp time);
        //获取消息对应的处理器
        MsgHandler getHandler(int msgid);
        //处理客户端异常退出
        void clientCloseExpt(const TcpConnectionPtr& con);
        //服务器异常，业务重置方法
        void reset();

        //从redis消息队列中获取订阅的消息
        void handleRedisSubscribeMessage(int channel, string message);

    private:
        ChatService();
        ChatService(const ChatService&)=delete;
        ChatService& operator=(const ChatService&)=delete;

        mutex m_connMapMutex;
        unordered_map<int,TcpConnectionPtr> m_userConnMap;//存储用户的连接
        UserModel m_userModel;//用户数据操作类
        OffLineMsgModel m_offLineMsgModel;//离线消息操作类
        FriendModel m_friendModel;//好友操作类
        unordered_map<int,MsgHandler> m_msgHandlerMap;//消息id和其对应的业务处理方法
        GroupModel m_groupModel;//群组操作类

        Redis m_redis;//redis操作对象


};

#endif