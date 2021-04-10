#ifndef SERVER_H
#define SERVER_H
 
#include <string>
#include "Common.h"
using namespace std;

class Sever
{
public:
	//初始化端口结构体
	Sever();
	
	//socket和epoll
	void Init();
	
	//启动服务器
	void Start();
	
	//关闭socket，epoll
	void Close();
	
private:
	// 广播消息给所有客户端
    int SendBroadcastMessage(int clientfd);
	
	//服务器端serverAddr信息
	struct sockaddr_in serverAddr;
	
	//创建监听的socket
	int listener;
	
	//epoll_caret 的返回值
	int epfd;
	
	//客户端列表
	list<int> clients_list;
};

#endif //SERVER_H