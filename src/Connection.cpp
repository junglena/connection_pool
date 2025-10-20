#include"Connection.h"
#include<iostream>
#include"public.h"

Connection::Connection()
{
    _conn=mysql_init(nullptr);//初始化数据库
    if(_conn==nullptr)
    {
        LOG("mysql init error!");
    }
}
Connection::~Connection()
{
    if(_conn!=nullptr)
    {
        mysql_close(_conn);
    }
}

//连接数据库
bool Connection::connect(string& ip,unsigned int port,string& user,string& password,string& dbname)
{
    MYSQL* p = mysql_real_connect(_conn,ip.c_str(),user.c_str(),password.c_str(),dbname.c_str(),port,nullptr,0);
    if(p==nullptr)
    {
        LOG("connect error: "+string(mysql_error(_conn)));
    }
    return p!=nullptr;
}
//更新操作
bool Connection::update(const string& sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG("更新失败:" + sql + " | 错误原因: " + std::string(mysql_error(_conn)));
        return false;
    }
    return true;
}
//查询操作
MYSQL_RES* Connection::query(const string& sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG("查询失败:" + sql + " | 错误原因: " + std::string(mysql_error(_conn)));
        return nullptr;
    }
    return mysql_store_result(_conn);
}

//刷新连接的起始空闲时间点
void Connection::refreshAliveTime()
{
    _alivetime = chrono::steady_clock::now();
}
// 返回连接空闲的时长（单位：秒）
long long Connection::getAliveTime()
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - _alivetime);
    return duration.count();
}
