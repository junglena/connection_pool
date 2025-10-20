#pragma once

#include<iostream>
#include<chrono>
#include<string>
#include<ctime>
using namespace std;
inline string getCurrentTime()
{
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);

    struct tm time_info;
    localtime_r(&in_time_t, &time_info);//将时间戳转换为本地时间的线程安全函数

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &time_info);
    return string(buffer);
}

#define LOG(str) \
    std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" \
              << " [" << getCurrentTime() << "] " << str << std::endl;