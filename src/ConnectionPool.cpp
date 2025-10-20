#include"ConnectionPool.h"
#include"public.h"
#include<fstream>
#include<iostream>
using namespace std;

ConnectionPool* ConnectionPool::getInstance()
{
    static ConnectionPool pool;
    return &pool;
}

bool ConnectionPool::loadConfigFile()
{
    ifstream file("/home/jungle/connection_pool_linux/config/mysql.ini");
    if(!file.is_open())
    {
        LOG("mysql.ini can not open!");
        return false;
    }
    string line;
    while(getline(file,line))
    {
        //忽略注释和空行
        if(line.empty()||line[0]=='#')
        {
            continue;
        }
        auto pos = line.find('=');
        string key = line.substr(0,pos);
        string value = line.substr(pos+1);
        if(key=="ip")
        {
            _ip = value;
        }
        else if(key=="port")
        {
            _port = stoi(value);
        }
        else if(key=="username")
        {
            _username=value;
        }
        else if(key=="password")
        {
            _password=value;
        }
        else if (key == "dbname")
        {
            _dbname = value;
        }
        else if (key == "initsize") 
        {
            _initSize = stoi(value);
        }
        else if (key == "maxsize") 
        {
            _maxSize = stoi(value);
        }
        else if (key == "maxidletime") 
        {
            _maxIdleTime = stoi(value);
        }
        else if (key == "connectionTimeout")
        {
            _connectionTimeout = stoi(value);
        }
    }
    return true;
}

ConnectionPool::ConnectionPool()
{
    if(!loadConfigFile())
    {
        return;
    }
    //创建初始数量的连接
    for(int i=0;i<_initSize;i++)
    {
        Connection* p = new Connection();
        p->connect(_ip,_port,_username,_password,_dbname);
        p->refreshAliveTime();
        _connectionCount++;
        _connectionQueue.push(p);
    }
    _producerThread = thread(&ConnectionPool::produceConnectionTask,this);
    _scannerThread = thread(&ConnectionPool::scannerConnectionTask,this);
}
ConnectionPool::~ConnectionPool() 
{
    _isExiting = true;
    _cv.notify_all(); // 唤醒所有等待的线程，让他们检查退出标志

    if (_producerThread.joinable()) 
    {
        _producerThread.join();
    }
    if (_scannerThread.joinable()) 
    {
        _scannerThread.join();
    }

    // 释放队列中所有的连接
    std::unique_lock<std::mutex> lock(_queueMutex);
    while (!_connectionQueue.empty()) 
    {
        Connection* conn = _connectionQueue.front();
        _connectionQueue.pop();
        delete conn;
    }
}

void ConnectionPool::produceConnectionTask()// 运行在独立的线程中，专门负责生产新连接
{
    while(!_isExiting)
    {
        unique_lock<mutex> lock(_queueMutex);
        _cv.wait(lock, [&](){
            return _connectionQueue.empty() || _isExiting;
        });
        if (_isExiting) 
        {
            break;
        }
        while(!_connectionQueue.empty())
        {
            // 如果队列不为空，生产者应该等待消费者消费
            _cv.wait(lock);
        }
        if(_connectionCount<_maxSize)
        {
            Connection* p = new Connection();
            if(p->connect(_ip,_port,_username,_password,_dbname))
            {
                p->refreshAliveTime();
                _connectionCount++;
                _connectionQueue.push(p);
                _cv.notify_all();//通知消费者有新连接可用
                LOG("新连接生产成功！");
            }
            else
            {
                LOG("新连接生产失败！");
                delete p;
            }
        }
    }
}
void ConnectionPool::scannerConnectionTask()// 扫描并回收空闲连接的线程函数
{
    while(!_isExiting)
    {

        // 扫描整个队列，移除空闲超时的连接
        unique_lock<mutex>lock(_queueMutex);
        _cv.wait_for(lock,chrono::seconds(_maxIdleTime),[&]{
            return _isExiting;
        });
        if(_isExiting)
        {
            break;
        }
        while(_connectionCount>_initSize&&!_connectionQueue.empty())
        {
            Connection* p = _connectionQueue.front();
            if(p->getAliveTime()>_maxIdleTime)//如果队头的连接超时
            {
                _connectionQueue.pop();
                _connectionCount--;
                delete p;
                LOG("一个空闲连接被回收");
            }
            else
            {
                // 如果队头的连接都没有超时，那么后面的连接（因为后入队）更不可能超时
                break;
            }
        }
    }
}

shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<std::mutex> lock(_queueMutex);
    while (_connectionQueue.empty()) 
    {
        if (_isExiting) return nullptr;
        // 等待，直到有可用连接或超时
        if (_cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)) == cv_status::timeout) 
        {
            if (_connectionQueue.empty()) {
                LOG("获取连接超时... 尝试重新获取");
                return nullptr;
            }
        }
    }
    // 自定义shared_ptr的删除器，用于将连接归还到池中
    Connection* _pcon =_connectionQueue.front();
    _connectionQueue.pop();
    std::shared_ptr<Connection> sp(_pcon, [&](Connection* pcon) 
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        pcon->refreshAliveTime();
        _connectionQueue.push(pcon);
        _cv.notify_one();
    });

    lock.unlock();
    
    return sp;
}