#include "redis.h"
#include <string>
#include <iostream>
#include <functional>
using namespace std;

Redis::Redis():_publish_context(nullptr),_subscribe_context(nullptr)
{

}

Redis::~Redis()
{
    if(_publish_context != nullptr)
    {
        redisFree(_publish_context);
    }
    if(_subscribe_context != nullptr)
    {
        redisFree(_subscribe_context);
    }
}

bool Redis::connect()
{
    //连接发布消息的上下文
    _publish_context = redisConnect("127.0.0.1", 6379);
    if(_publish_context == nullptr)
    {
        std::cout << "connect redis failed!" << std::endl;
        return false;
    }
    //连接订阅消息的上下文
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if(_subscribe_context == nullptr)
    {
        std::cout << "connect redis failed!" << std::endl;
        return false;
    }

    thread t(&Redis::observer_channel_message, this);
    t.detach();//分离线程

    return true;

}

//向redis指定通道channel发布消息
bool Redis::publish(int channel, std::string message)
{
    redisReply* reply = (redisReply*)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        std::cout << "publish message failed!" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

//向redis指定通道subscribe订阅消息
bool Redis::subscribe(int channel)
{
    //subscribe是一个阻塞的命令，这里只订阅通道消息，不接收消息
    //接收通道消息在observer_channel_message线程中进行
    if(REDIS_ERR==redisAppendCommand(_subscribe_context, "SUBSCRIBE %d", channel))
    {
        std::cout << "subscribe message failed!" << std::endl;
        return false;
    }


    int done=0;
    //确保命令发送完成
    while(!done)
    {
        if(REDIS_ERR==redisBufferWrite(_subscribe_context,&done))
        {
            std::cout << "subscribe message failed!" << std::endl;
            return false;
        }
    }

    return true;

}

void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    while (redisGetReply(_subscribe_context, (void**)&reply) == REDIS_OK)
    {
        cout<<"XXXXXXXXXXXXXXXXXXX"<<endl;
        //订阅收到消息
        if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            int channel = atoi(reply->element[1]->str);
            std::string message = reply->element[2]->str;

            cout<<message<<endl;
            //给service层上报消息
            if(notify_msg_handler)
            {
                
                notify_msg_handler(channel, message);
            }
        }
        freeReplyObject(reply);
    }

    std::cout << "observer_channel_message quit" << std::endl;
}


bool Redis::unsubscribe(int channel)
{
   if(REDIS_ERR==redisAppendCommand(_subscribe_context, "UNSUBSCRIBE %d", channel)){
        std::cout << "unsubscribe message failed!" << std::endl;
        return false;
   }

    int done=0;
    //确保命令发送完成
    while(!done)
    {
        if(REDIS_ERR==redisBufferWrite(_subscribe_context,&done))
        {
            std::cout << "unsubscribe message failed!" << std::endl;
            return false;
        }
    }

    return true;

}

void Redis::init_notify_handler(std::function<void(int, std::string)> fn)
{
    this->notify_msg_handler = fn;
}

