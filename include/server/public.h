#ifndef PUBLIC_H
#define PUBLIC_H


enum EnMsgType
{
    LOGIN_MSG=1,//登录消息 1
    LOGIN_MSG_ACK,//登录响应消息 2
    REG_MSG,//注册消息 3
    REG_MSG_ACK,//注册响应消息 4
    SEND_ONE_CHAT_MSG,//一对一聊天消息 5
    ADD_FRIEND_MSG,//添加好友消息 6
    ADD_FRIEND_MSG_ACK,//添加好友响应消息 7
    CREATE_GROUP_MSG,//创建群组消息 8
    CREATE_GROUP_MSG_ACK,//创建群组响应消息 9
    ADD_GROUP_MSG,//加入群组消息 10
    ADD_GROUP_MSG_ACK,//加入群组响应消息 11
    GROUP_CHAT_MSG,//群聊天消息 12
    LOGINOUT_MSG,//退出登录消息 13
    LOGINOUT_MSG_ACK,//退出登录响应消息 14
    UNKNOW_MSG,//未知消息 15
    GROUP_CHAT_MSG_ACK//群聊天消息的响应消息 16

};

#endif