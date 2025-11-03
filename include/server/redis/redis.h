#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <string>
#include <thread>
#include <functional>
class Redis{

    public:
        Redis();
        ~Redis();

        //连接redis服务器
        bool connect();

        //向redis指定通道channel发布消息
        bool publish(int channel, std::string message);

        //向redis指定通道subscribe订阅消息
        bool subscribe(int channel);
        
        //初始化订阅消息后的回调函数
        void init_notify_handler(std::function<void(int, std::string)> fn);
   

       //接收订阅通道的消息的线程函数
        void observer_channel_message();

        //向redis指定的通道取消订阅
        bool unsubscribe(int channel);
    private:
        redisContext* _publish_context;//发布消息的上下文
        redisContext* _subscribe_context;//订阅消息的上下文
        std::function<void(int, std::string)> notify_msg_handler;//订阅消息后的回调函数,给service层上报
       
};
#endif
