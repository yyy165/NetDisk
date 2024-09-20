#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
typedef unsigned int uint;

#define REGIST_OK "注册成功"
#define REGIST_FAILED "注册失败:用户名已存在"

#define LOGIN_OK "登录成功"
#define LOGIN_FAILED "登录失败:用户名或密码为空或用户已登录"
#define SEARCH_USR_NO "用户不存在"
#define SEARCH_USR_ONLINE "在线"
#define SEARCH_USR_OFFLINE "离线"
#define UNKNOWN_ERROR "未知错误"
#define EXISTED_FRIEND "好友已存在"
#define ADD_FRIEND_OFFLINE "用户离线"
#define ADD_FRIEND_NO_EXIST "用户不存在"
#define DEL_FRIEND_OK "删除成功"
#define DIR_NO_EXIST "文件夹不存在"
#define FILE_NAME_EXIST "文件名已存在"
#define CREATE_DIR_OK "创建成功"
#define DEL_DIR_OK "删除文件夹成功"
#define DEL_DIR_FAIL "删除文件夹失败 : 是常规文件"
#define RENAME_FILE_OK "重命名文件成功"
#define RENAME_FILE_FAIL "重命名文件失败"
#define ENTER_DIR_FAIL "进入文件夹失败 : 不可进入常规文件"

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST,   //注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND,   //注册回复

    ENUM_MSG_TYPE_LOGIN_REQUEST,    //登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,     //登录回复

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,   //在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,   //在线用户回复

    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,   //搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,   //搜索用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,   //添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,   //添加好友回复

    ENUM_MSG_TYPE_ADD_FRIEND_AGREE,   //同意添加好友
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,   //拒绝添加好友

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,   //刷新好友请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,  //刷新好友回复

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,   //删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,  //删除好友回复

    ENUM_MSG_TYPE_ADD_YOU,   //同意你的申请
    ENUM_MSG_TYPE_REJECT_YOU,  //拒绝你的申请

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,   //私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,  //私聊回复

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,   //群聊请求
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,  //群聊回复

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,   //创建文件夹请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,   //创建文件夹回复

    ENUM_MSG_TYPE_FLUSH_DIR_REQUEST,   //刷新文件夹请求
    ENUM_MSG_TYPE_FLUSH_DIR_RESPOND,   //刷新文件夹回复

    ENUM_MSG_TYPE_DELETE_DIR_REQUEST,   //删除文件夹请求
    ENUM_MSG_TYPE_DELETE_DIR_RESPOND,   //删除文件夹回复

    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,   //重命名文件请求
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,   //重命名文件回复

    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,   //进入文件夹请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,   //进入文件夹回复
    // ENUM_MSG_TYPE_REQUEST,
    // ENUM_MSG_TYPE_RESPOND,
    ENUM_MSG_TYPE_MAX = 0x00ffffff
};

struct FileInfo
{
    char caFileName[32];    //文件名
    int iFileType;          //文件类型
};

struct PDU
{
    uint uiPDULen;      //总的协议数据单元大小
    uint uiMsgType;     //消息类型
    char caData[64];
    uint uiMsgLen;      //实际消息长度
    int caMsg[];        //实际消息
};

PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
