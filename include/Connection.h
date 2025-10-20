#include<mysql/mysql.h>
#include<string>
#include<chrono>
using namespace std;
//实现MySQL数据库操作
class Connection{
public:
    Connection();
    ~Connection();

    //连接数据库
    bool connect(string& ip,unsigned int port,string& user,string& password,string& dbname);
    //更新操作
    bool update(const string& sql);
    //查询操作
    MYSQL_RES* query(const string& sql);

    //刷新连接的起始空闲时间点
    void refreshAliveTime();
    // 返回连接空闲的时长（单位：秒）
    long long getAliveTime();
private:
    MYSQL *_conn;//表示和mysql服务器的连接
    chrono::steady_clock::time_point _alivetime;//记录进入空闲状态后的起始时间点
};