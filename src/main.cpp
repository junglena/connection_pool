#include <iostream>
#include <thread>
#include <chrono>
#include "ConnectionPool.h"
using namespace std;
void worker(int begin, int end) 
{
    for (int i = begin; i < end; ++i) 
    {
        shared_ptr<Connection> sp = ConnectionPool::getInstance()->getConnection();
        if (sp) 
        {
            char sql[1024] = { 0 };
            sprintf(sql, "insert into `user` (name,age,sex) values('%s',%d,'%s')",
	        "Tom", 20, "male");
            sp->update(sql);
        }
    }
}
int main() {
    auto start = chrono::high_resolution_clock::now();

    // 创建并启动多个线程来模拟高并发场景
    thread t1(worker, 0, 500);
    thread t2(worker, 500, 1000);
    thread t3(worker, 1000, 1500);
    thread t4(worker, 1500, 2000);
    // 等待所有线程执行完毕
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    std::cout << "执行 2000 次数据库插入耗时: " << duration.count() << "ms" << std::endl;

    // 程序结束时，ConnectionPool的静态实例会被销毁，其析构函数会清理所有资源
    return 0;
}