#include "Server.h"
#include <iostream>
using namespace std;

Server::Server()
{
	//初始化服务器地址和端口
	/**
	相关头文件中的定义：AF = Address Family
						PF = Protocol Family
						AF_INET = PF_INET
	//理论上建立socket时是指定协议，应该用PF_xxxx，设置地址时应该用AF_xxxx。
	//当然AF_INET和PF_INET的值是相同的，混用也不会有太大的问题。
	*/
	serverAddr.sin_family = PF_INET;
	
	//在Linux和Windows网络编程时需要用到htons和htonl函数，用来将主机字节顺序转换为网络字节顺序。
	//小端字节序：低字节存于内存低地址；
	//大端字节序：高字节存于内存低地址；
	//UDP/TCP/IP协议规定:把接收到的第一个字节当作高位字节看待,这就要求发送端发送的第一个字节是高位字节;
	//而发送的第一个字节是该数值在内存中的起始地址处对应的那个字节,(即:高位字节存放在低地址处);
	//由此可见,网络字节序是大端字节序;
    serverAddr.sin_port = htons(SERVER_PORT);
	
	///将一个点分十进制的IP转换成一个长整数型数（u_long类型）
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
}

/**
	#include<sys/types.h>
	#include<sys/socket.h>
	int socket(int domain, int type, int protocol);
	domain用于设置网络通信的域
	PF_UNIX,PF_LOCAL	本地通信
	AF_INET,PF_INET		IPv4 Internet协议
	
	type用于设置套接字通信的类型
	主要有SOCKET_STREAM（TCP流式套接字）、SOCK——DGRAM（UDP数据包套接字）等。
	
	protocol用于制定某个协议的特定类型，即type类型中的某个类型。
	通常某协议中只有一种特定类型，这样protocol参数仅能设置为0；
	但是有些协议有多种特定的类型，就需要设置这个参数来选择特定的类型。

    类型为SOCK_STREAM的套接字表示一个双向的字节流，与管道类似。
	流式的套接字在进行数据收发之前必须已经连接，连接使用connect()函数进行。
	一旦连接，可以使用read()或者write()函数进行数据的传输。
	流式通信方式保证数据不会丢失或者重复接收，当数据在一段时间内任然没有接受完毕，可以将这个连接人为已经死掉。
	*/
void Server::Init()
{
    cout << "Init Server..." << endl;
	
	//创建监听socket
	
	listener = socket(PF_INET, SOCK_STREAM, 0);
	if(listener < 0) 
	{
		perror("listener"); 
		exit(-1);
	}
	
	//绑定地址
	if(bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0))
	{
		perror("bind err"); 
		exit(-1);
	}
	
	//监听
	//第二个参数是在进入 队列中允许的连接数目
	//进入的连接是在队列中一直等待直 到你接受 (accept())
	//它们的数目限制于队列的允许。 大多数系统的允许数目是20，你也可以设置为5到10。
	int ret = listen(listener, 5);
	if(ret < 0)
	{
		perror("liste err"); 
		exit(-1);
	}
	cout << "Start to listen: " << SERVER_IP << endl;
	
	///在内核创建epoll事件表
	epfd = epoll_creat(EPOLL_SIZE)
	if(epfd < 0)
	{
		perror("epfd err"); 
		exit(-1);
	}
	
	//往事件表里面添加监听事件
	addfd(epfd, listener, true);
}

void Server::Close()
{
	//关闭socket
	close(listener);
	
	//关闭epoll监听
	close(epfd);
}

// 发送广播消息给所有客户端
int Server::SendBroadcastMessage(int clientfd)
{
    // buf[BUF_SIZE] 接收新消息
    // message[BUF_SIZE] 保存格式化的消息
    char recv_buf[BUF_SIZE];
    char send_buf[BUF_SIZE];
    Msg msg;
    bzero(recv_buf, BUF_SIZE);
	
    // 接收新消息
    cout << "read from client(clientID = " << clientfd << ")" << endl;
    int len = recv(clientfd, recv_buf, BUF_SIZE, 0);
	
    //清空结构体，把接受到的字符串转换为结构体
    memset(&msg,0,sizeof(msg));
    memcpy(&msg,recv_buf,sizeof(msg));
	
    //判断接受到的信息是私聊还是群聊
    msg.fromID=clientfd;
    if(msg.content[0]=='\\'&&isdigit(msg.content[1]))
	{
        msg.type=1;
        msg.toID=msg.content[1]-'0';
        memcpy(msg.content,msg.content+2,sizeof(msg.content));
    }
    else
        msg.type=0;
    // 如果客户端关闭了连接
    if(len == 0) 
    {
        close(clientfd);
        
        // 在客户端列表中删除该客户端
        clients_list.remove(clientfd);
        cout << "ClientID = " << clientfd 
             << " closed.\n now there are " 
             << clients_list.size()
             << " client in the char room"
             << endl;
 
    }
    // 发送广播消息给所有客户端
    else 
    {
        // 判断是否聊天室还有其他客户端
        if(clients_list.size() == 1){ 
            // 发送提示消息
            memcpy(&msg.content,CAUTION,sizeof(msg.content));
            bzero(send_buf, BUF_SIZE);
            memcpy(send_buf,&msg,sizeof(msg));
            send(clientfd, send_buf, sizeof(send_buf), 0);
            return len;
        }
        //存放格式化后的信息
        char format_message[BUF_SIZE];
        //群聊
        if(msg.type==0){
            // 格式化发送的消息内容 #define SERVER_MESSAGE "ClientID %d say >> %s"
            sprintf(format_message, SERVER_MESSAGE, clientfd, msg.content);
            memcpy(msg.content,format_message,BUF_SIZE);
            // 遍历客户端列表依次发送消息，需要判断不要给来源客户端发
            list<int>::iterator it;
            for(it = clients_list.begin(); it != clients_list.end(); ++it) {
               if(*it != clientfd){
                    //把发送的结构体转换为字符串
                    bzero(send_buf, BUF_SIZE);
                    memcpy(send_buf,&msg,sizeof(msg));
                    if( send(*it,send_buf, sizeof(send_buf), 0) < 0 ) {
                        return -1;
                    }
               }
            }
        }
        //私聊
        if(msg.type==1){
            bool private_offline=true;
            sprintf(format_message, SERVER_PRIVATE_MESSAGE, clientfd, msg.content);
            memcpy(msg.content,format_message,BUF_SIZE);
            // 遍历客户端列表依次发送消息，需要判断不要给来源客户端发
            list<int>::iterator it;
            for(it = clients_list.begin(); it != clients_list.end(); ++it) {
               if(*it == msg.toID){
                    private_offline=false;
                    //把发送的结构体转换为字符串
                    bzero(send_buf, BUF_SIZE);
                    memcpy(send_buf,&msg,sizeof(msg));
                    if( send(*it,send_buf, sizeof(send_buf), 0) < 0 ) {
                        return -1;
                    }
               }
            }
            //如果私聊对象不在线
            if(private_offline){
                sprintf(format_message,SERVER_PRIVATE_ERROR_MESSAGE,msg.toID);
                memcpy(msg.content,format_message,BUF_SIZE);
                bzero(send_buf,BUF_SIZE);
                memcpy(send_buf,&msg,sizeof(msg));
                if(send(msg.fromID,send_buf,sizeof(send_buf),0)<0)
                    return -1;
            }
        }
    }
    return len;
}

//启动服务器
void Server::Start()
{
    // epoll 事件队列
	static struct epoll_event events[EPOLL_SIZE];
	
	// 初始化服务端
    Init();
	
	//主循环
    while(1)
    {
        //epoll_events_count表示就绪事件的数目
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
 
        if(epoll_events_count < 0) {
            perror("epoll failure");
            break;
        }
 
        cout << "epoll_events_count =\n" << epoll_events_count << endl;
 
        //处理这epoll_events_count个就绪事件
        for(int i = 0; i < epoll_events_count; ++i)
        {
            int sockfd = events[i].data.fd;
            //新用户连接
            if(sockfd == listener)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
				
                int clientfd = accept( listener, ( struct sockaddr* )&client_address, &client_addrLength );
 
                cout << "client connection from: "
                     << inet_ntoa(client_address.sin_addr) << ":"
                     << ntohs(client_address.sin_port) << ", clientfd = "
                     << clientfd << endl;
 
                addfd(epfd, clientfd, true);
 
                // 服务端用list保存用户连接
                clients_list.push_back(clientfd);
                cout << "Add new clientfd = " << clientfd << " to epoll" << endl;
                cout << "Now there are " << clients_list.size() << " clients int the chat room" << endl;
 
                // 服务端发送欢迎信息  
                cout << "welcome message" << endl;                
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                sprintf(message, SERVER_WELCOME, clientfd);
                int ret = send(clientfd, message, BUF_SIZE, 0);
                if(ret < 0) {
                    perror("send error");
                    Close();
                    exit(-1);
                }
            }
            //处理用户发来的消息，并广播，使其他用户收到信息
            else 
			{   
                int ret = SendBroadcastMessage(sockfd);
                if(ret < 0) {
                    perror("error");
                    Close();
                    exit(-1);
                }
            }
        }
    }
	// 关闭服务
	//使用close中止一个连接，但它只是减少描述符的参考数，并不直接关闭连接，只有当描述符的参考数为0时才关闭连接。
	//如果有五个进程使用该 socket id 则这个socket 描述符参考数则有五个，
	//当所有进程都close()了这样参考数就等于0，该socket 就真的关闭释放了。
    Close();
}
