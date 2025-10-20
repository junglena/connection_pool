#pragma once

#include<string>
#include<thread>
#include<mutex>
#include<iostream>
#include"Connection.h"
#include<queue>
#include<atomic>
#include<condition_variable>
using namespace std;

class ConnectionPool{
public:
    // 获取连接池单例实例
    static ConnectionPool* getInstance();
    // 从连接池中获取一个连接
    shared_ptr<Connection> getConnection();

    bool loadConfigFile();//加载配置文件
private:
    ConnectionPool();
    ~ConnectionPool();

    void produceConnectionTask();// 运行在独立的线程中，专门负责生产新连接
    void scannerConnectionTask(); // 扫描并回收空闲连接的线程函数

    string _ip;
    unsigned short _port;
    string _username;
    string _password;
    string _dbname;

    int _initSize;          // 初始连接数
    int _maxSize;           // 最大连接数
    int _maxIdleTime;       // 最大空闲时间（秒）
    int _connectionTimeout; // 连接超时时间（毫秒）

    queue<Connection*> _connectionQueue;//存储mysql连接的队列
    mutex _queueMutex;//维护连接队列的线程安全互斥锁
    atomic_int _connectionCount;//记录已创建的连接总数量
    condition_variable _cv;// 条件变量，用于生产者和消费者之间的通信

    bool _isExiting = false; // 标志连接池是否正在退出
    thread _producerThread;//生产者线程
    thread _scannerThread;//扫描器线程
};