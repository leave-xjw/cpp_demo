/**
定义一些共用的宏定义，包括一些共用的网络编程相关头文件。

1）定义一个函数将文件描述符fd添加到epfd表示的内核事件表中供客户端和服务端两个类使用。

2）定义一个信息数据结构，用来表示传送的信息，结构体包括发送方fd, 接收方fd,用来表示消息类别的type,还有文字信息。

函数recv() send() write() read() 参数传递是字符串，所以在传送前/接受后要把结构体转换为字符串/字符串转换为结构体。
*/

#ifndef COMMON_H
#define COMMON_H

 
#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 默认服务器端IP地址
#define SERVER_IP "127.0.0.1"
 
// 服务器端口号
#define SERVER_PORT 8888
 
// int epoll_create(int size)中的size
// 为epoll支持的最大句柄数
#define EPOLL_SIZE 5000
 
// 缓冲区大小65535
#define BUF_SIZE 0xFFFF
    
// 新用户登录后的欢迎信息
#define SERVER_WELCOME "Welcome you join to the chat room! Your chat ID is: Client #%d"
 
// 其他用户收到消息的前缀
#define SERVER_MESSAGE "ClientID %d say >> %s"
#define SERVER_PRIVATE_MESSAGE "Client %d say to you privately >> %s"
#define SERVER_PRIVATE_ERROR_MESSAGE "Client %d is not in the chat room yet~"
// 退出系统
#define EXIT "EXIT"
 
// 提醒你是聊天室中唯一的客户
#define CAUTION "There is only one int the char room!"

//注册新的fd到epoll中
//参数enable_et表示是否启用ET模式，如果为True则启用，否则使用LT模式

///使用static用于函数定义时，对函数的连接方式产生影响，
///使得函数只在本文件内部有效，对其他文件是不可见的。这样的函数又叫作静态函数。
static void addfd( int epollfd, int fd, bool enable_et )
{
	struct epoll_event ev;
	ev.data.fd = fd;
    ev.events = EPOLLIN;
	
    if( enable_et )
        ev.events = EPOLLIN | EPOLLET;
	
	//epfd：由 epoll_create 生成的epoll专用的文件描述符；
	//EPOLL_CTL_ADD 注册、 EPOLL_CTL_MOD 修改、EPOLL_CTL_DEL 删除
	//fd：关联的文件描述符
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	
    // 设置socket为非阻塞模式
    // 套接字立刻返回，不管I/O是否完成，该函数所在的线程会继续运行
    //eg. 在recv(fd...)时，该函数立刻返回，在返回时，内核数据还没准备好会返回WSAEWOULDBLOCK错误代码
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)| O_NONBLOCK);
    cout << "fd added to epoll!" << endl;
	
//定义信息结构，在服务端和客户端之间传送
struct Msg
{
    int type;
    int fromID;
    int toID;
    char content[BUF_SIZE];
};
}







#endif //COMMON_H