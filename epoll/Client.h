#ifndef CLIENT_H
#define CLIENT_H
/**
需要的接口：

1)连接服务端connect()

2)退出连接close()

3)启动客户端Start()
**/
#include <string>
#include "Common.h"
using namespace std;

class Client
{
	Client();
	
	void Connect();
	
	void Close();
	
	void Start();
	
private:
	
	// 当前连接服务器端创建的socket
	int sock;
	
	//当前进程ID
	int pid;
	
	//epoll_creat 的返回值
	int epfd;
	
	// 创建管道，其中fd[0]用于父进程读，fd[1]用于子进程写
    int pipe_fd[2];
	
	bool isClientWork;
	
	// 聊天信息
	Msg msg;
	
	//结构体要转换为字符串
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
	
    //用户连接的服务器 IP + port
    struct sockaddr_in serverAddr;
}

#endif CLIENT_H