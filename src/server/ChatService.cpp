#include "ChatService.h"
#include "public.h"
#include <muduo/base/Logging.h>
#include <string>
#include <iostream>

ChatService* ChatService::instance(){
        static ChatService service;
        return &service;
}

ChatService::ChatService(){
    //登录
    m_msgHandlerMap.insert({LOGIN_MSG,bind(&ChatService::login,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    //注册
    m_msgHandlerMap.insert({REG_MSG,bind(&ChatService::reg,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    //未知
    m_msgHandlerMap.insert({UNKNOW_MSG,bind(&ChatService::unknow,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    //一对一聊天
    m_msgHandlerMap.insert({SEND_ONE_CHAT_MSG,bind(&ChatService::SendOneChat,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    //添加好友
    m_msgHandlerMap.insert({ADD_FRIEND_MSG,bind(&ChatService::addFriend,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    //创建群聊
    m_msgHandlerMap.insert({CREATE_GROUP_MSG,bind(&ChatService::createGroup,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    //加入群组
    m_msgHandlerMap.insert({ADD_GROUP_MSG,bind(&ChatService::addGroup,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    //群聊天
    m_msgHandlerMap.insert({GROUP_CHAT_MSG,bind(&ChatService::groupChat,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    //退出登录
    m_msgHandlerMap.insert({LOGINOUT_MSG,bind(&ChatService::loginOut,this,placeholders::_1,placeholders::_2,placeholders::_3)});

    if(m_redis.connect()){
        //设置上报消息的回调函数
        m_redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,placeholders::_1,placeholders::_2));
    }
}

void ChatService::handleRedisSubscribeMessage(int channel, string message){
    //收到订阅的消息后，查找用户是否在线
    lock_guard<mutex> lock(m_connMapMutex);
    auto it=m_userConnMap.find(channel);
    if(it!=m_userConnMap.end()){//用户在线
        //给用户发送消息
        it->second->send(message);
        
    }
    else{
        //用户不在线，存储离线消息
        m_offLineMsgModel.insert(channel,message);
    }
}

void ChatService::login(const TcpConnectionPtr& con,json& js,Timestamp time){

    int id=js["id"].get<int>();
    string pwd=js["password"].get<string>();
    User user=m_userModel.query(id);
    if(user.getPassword()==pwd&&user.getId()==id){
        if(user.getState()=="online"){
            //用户已处于登录状态
            json response;
            response["info"]="用户已处于登录状态";
            response["msgid"]=LOGIN_MSG_ACK;
            response["errno"]=2;
            con->send(response.dump());
            return;
            
        }
        //登录成功
        user.setState("online");

        //用户登录成功后，服务器向redis订阅该用户的ID(channel)
        m_redis.subscribe(id);

        m_userModel.updateState(user);//更新用户状态(在线)
        
        
        {   
            //存储用户连接
            lock_guard<mutex> lock(m_connMapMutex);
            m_userConnMap.insert({user.getId(),con});
        }
        
        

        json response;
        response["msgid"]=LOGIN_MSG_ACK;
        response["errno"]=0;
        response["id"]=user.getId();
        response["name"]=user.getName();
        response["state"]=user.getState();
        response["info"]="登录成功";

        //获取当前用户好友列表信息
        vector<User> vecu=m_friendModel.query(id);
        if(!vecu.empty()){
            vector<string> temps;
            for(auto f:vecu){
                json temp;
                temp["id"]=f.getId();
                temp["name"]=f.getName();
                temp["state"]=f.getState();
                temps.push_back(temp.dump());
            }
            response["friends"]=temps;
        }

        //获取当前用户群组列表信息
        vector<Group> vecg=m_groupModel.queryGroups(id);
        
        if(!vecg.empty()){
            vector<string> temps;
            for(auto g:vecg){
                json groupjson;
                groupjson["groupid"]=g.getId();
                groupjson["groupname"]=g.getName();
                groupjson["groupdesc"]=g.getDesc();
                //存储群组成员信息
                vector<string> usertemps;
                for(auto u:g.getUserVec()){
                    json userjson;
                    userjson["id"]=u.getId();
                    userjson["name"]=u.getName();
                    userjson["state"]=u.getState();
                    userjson["role"]=u.getRole();
                    usertemps.push_back(userjson.dump());
                }
                groupjson["users"]=usertemps;
                temps.push_back(groupjson.dump());
            }
            response["groups"]=temps;
        }

        //查询该用户是否有离线消息
        vector<string> vec=m_offLineMsgModel.query(id);
        if(!vec.empty()){
            response["offlinemsg"]=vec;
            m_offLineMsgModel.remove(id);//删除该用户的离线消息

          }
        con->send(response.dump());
    }

    else{
        json response;
        response["msgid"]=LOGIN_MSG_ACK;
        response["errno"]=1;
        response["info"]="登录失败,账号或密码错误";
        con->send(response.dump());
        //登录失败
    }

    return ;
  
}

void ChatService::createGroup(const TcpConnectionPtr& con,json& js,Timestamp time){
    string groupname=js["groupname"].get<string>();
    string groupdesc=js["groupdesc"].get<string>();
    Group group(-1,groupname,groupdesc);
    if(m_groupModel.createGroup(group)){
        //存储创建者信息
        int userid=js["fromID"].get<int>();
        m_groupModel.addGroup(userid,group.getId(),"creator");

        json response;
        response["msgid"]=CREATE_GROUP_MSG_ACK;
        response["errno"]=0;
        response["groupid"]=group.getId();
        response["info"]="创建群组成功";
        con->send(response.dump());
        return;
    }


    else{
         json response;
        response["msgid"]=CREATE_GROUP_MSG_ACK;
        response["errno"]=1;
        response["info"]="创建群组失败";
        con->send(response.dump());
        return;
    }
}


void ChatService::addGroup(const TcpConnectionPtr& con,json& js,Timestamp time){
    int userid=js["fromID"].get<int>();
    int groupid=js["groupID"].get<int>();
    m_groupModel.addGroup(userid,groupid,"normal");

    json response;
    response["msgid"]=ADD_GROUP_MSG_ACK;
    response["errno"]=0;
    response["info"]="加入群组成功";
    con->send(response.dump());

    return;
}

void ChatService::groupChat(const TcpConnectionPtr& con,json& js,Timestamp time){
    int userid=js["fromID"].get<int>();//发送群聊消息的用户ID
    int groupid=js["groupID"].get<int>();
    vector<int> useridVec=m_groupModel.queryGroupUsers(userid,groupid);
    lock_guard<mutex> lock(m_connMapMutex);
    for(int id:useridVec){
        auto it=m_userConnMap.find(id);
        if(it!=m_userConnMap.end()){//在线
            it->second->send(js.dump());
        }
        else {
            //不在线，查询是否在其他服务器上在线
            User user=m_userModel.query(id);
            if(user.getState()=="online"){
                m_redis.publish(id,js.dump());
            }
        
            else{
            //不在线，存储离线消息
            m_offLineMsgModel.insert(id,js.dump());
            }
      }
    }


        //  json response;
        //  response["msgid"]=GROUP_CHAT_MSG_ACK;
        //  response["errno"]=0;
        //  response["info"]="群聊消息发送成功";  
        //先不回复ACK，等之后在看看
    return;
}


void ChatService::reg(const TcpConnectionPtr& con,json& js,Timestamp time){
   

    string name=js["name"].get<string>();
    string password=js["password"].get<string>();
    User user;
    user.setName(name);
    user.setPassword(password);
    bool state= m_userModel.insert(user);  //保存用户数据
    if(state){

        json response;
        response["msgid"]=REG_MSG_ACK;
        response["id"]=user.getId();//获取自动生成的id
        response["errno"]=0;
        response["info"]="注册成功";
        con->send(response.dump());
        
    }else{
         json response;
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=1;
        response["info"]="注册失败";
        con->send(response.dump());
    }


    
}

void ChatService::unknow(const TcpConnectionPtr& con,json& js,Timestamp time){
    LOG_ERROR<<"msgid:"<<js["msgid"].get<int>()<<" 没有对应的处理函数";
    json response;
    response["errno"]=1;
    response["info"]="未知消息";
    con->send(response.dump());
}

MsgHandler ChatService::getHandler(int msgid){

    //记录错误日志，当msgid没有对应的处理函数时，给客户端发送一个json格式的错误提示信息
    unordered_map<int,MsgHandler>::iterator it=m_msgHandlerMap.find(msgid);
    if(it==m_msgHandlerMap.end()){
        //msgid没有对应的处理函数时，返回一个默认的处理函数(unkonw)
        return m_msgHandlerMap[UNKNOW_MSG];
    }
    return m_msgHandlerMap[msgid];
}


void ChatService::clientCloseExpt(const TcpConnectionPtr& con){

    User user;
    {
        lock_guard<mutex> lock(m_connMapMutex);
        for(auto it=m_userConnMap.begin();it!=m_userConnMap.end();it++){
            if(it->second==con){
                user.setId(it->first);
                user.setState("offline"); //更新用户状态为离线
                m_userModel.updateState(user);

                m_userConnMap.erase(it);//删除用户的连接信息
             
                break;
            }
        }
    }

       m_redis.unsubscribe(user.getId());//用户退出登录后，服务器取消订阅该用户的ID(channel)
    std::cout<<"用户异常退出"<<endl;//当一个用户也没有登录时也会打印这句话，这是有问题的，先放着
    return ;
}

void ChatService::SendOneChat(const TcpConnectionPtr& con,json& js,Timestamp time){//

     int toid=js["toID"].get<int>();

     {
        lock_guard<mutex> lock(m_connMapMutex);
        unordered_map<int,TcpConnectionPtr>::iterator it=m_userConnMap.find(toid);
        if(it!=m_userConnMap.end()){//如果用户在线(用户登录在本服务器上)
            //转发消息给对应用户
            it->second->send(js.dump());
            return;
        }
     }

        //查询toid用户是否在线(用户登录在其他服务器上)
        User user=m_userModel.query(toid);
        if(user.getState()=="online"){
            m_redis.publish(toid,js.dump());
            return;
        }


     //用户不在线，存储离线消息;
     m_offLineMsgModel.insert(toid,js.dump());
}

void ChatService::reset(){
    m_userModel.resetState();
}


void ChatService::addFriend(const TcpConnectionPtr& con,json& js,Timestamp time){

    int userid=js["fromID"].get<int>();//当前用户ID
    int friendid=js["friendID"].get<int>();

     //存储好友信息
    //此处可以添加一些逻辑，比如判断好友是否存在，是否已经是好友等
    //为了简化代码，这里不做这些判断
    if(m_friendModel.addFriend(userid,friendid)){
    json response;
         response["msgid"]=ADD_FRIEND_MSG_ACK;
         response["errno"]=0;
         response["info"]="添加好友成功";
         con->send(response.dump());
        
    }

    else{
         json response;
         response["msgid"]=ADD_FRIEND_MSG_ACK;
         response["errno"]=1;
         response["info"]="添加好友失败";
         con->send(response.dump());
    }
    return;
}

void ChatService::loginOut(const TcpConnectionPtr& con,json& js,Timestamp time){
 
    //判断用户是否在线
    int userid=js["id"].get<int>();
    {
        lock_guard<mutex> lock(m_connMapMutex);
        auto it=m_userConnMap.find(userid);
        if(it!=m_userConnMap.end()){//在线
            char sql[1024]={0};
            sprintf(sql,"update User set state='offline' where id=%d",userid);
            Mysql mysql;
            if(mysql.connect()){
                mysql.update(sql);
            }
            //删除用户的连接信息
            m_userConnMap.erase(it);
            m_redis.unsubscribe(userid);//用户退出登录后，服务器取消订阅该用户的ID(channel)

            js["msgid"]=LOGINOUT_MSG_ACK;
            js["msg"]="用户退出登录";
            js["error"]=0;
            con->send(js.dump());
        }

        else{
            //用户不在线，无法退出登录
            js["msgid"]=LOGINOUT_MSG_ACK;
            js["msg"]="用户不在线，无法退出登录";
            js["error"]=1;
            con->send(js.dump());
        }
    }

    return ;
}