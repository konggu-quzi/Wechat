#include "myhead.h"
/*
	通过select多路复用函数去实现tcp单向通信
	客户端代码 
	思路：
	    第一步：你要想清楚究竟要监测哪些文件描述符
		        tcp套接字
        第二步：定义集合变量，将你要监测的文件描述符添加到集合
        第三步：调用select去监测集合中所有文件描述符的状态
		第四步：判断是否发生了你想要监测的状态变化
              通过 FD_ISSET(int fd, fd_set *fdset)来判断
*/
int main(int argc,char **argv)
{
	int tcpsock;
	int ret;
	char msg[50];
	//定义ipv4地址结构体变量
	struct sockaddr_in bindaddr;
	bzero(&bindaddr,sizeof(bindaddr));
	bindaddr.sin_family=AF_INET;
	bindaddr.sin_addr.s_addr=htonl(INADDR_ANY); //操作系统中有定义一个宏：INADDR_ANY表示任意一个ip地址
	bindaddr.sin_port=htons(10001);//客户端的端口号
	
	struct sockaddr_in serveraddr;//服务器的ip和端口号
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=inet_addr("192.168.90.68");//服务器的ip，小端转大端
	serveraddr.sin_port=htons(10000);//服务器的端口号
	//创建tcp类型的套接字
	tcpsock=socket(AF_INET,SOCK_STREAM,0);
	if(tcpsock==-1)
	{
		perror("创建tcp套接字失败！\n");
		return -1;
	}
	int sinsize = 1;//真
    setsockopt(tcpsock, SOL_SOCKET, SO_REUSEADDR, &sinsize, sizeof(int));//在socket()之后bind()之前使用
	//绑定ip和端口号
	ret=bind(tcpsock,(struct sockaddr *)&bindaddr,sizeof(bindaddr));
	if(ret==-1)
	{
		perror("绑定失败!\n");
		return -1;
	}
	//连接服务器
	ret=connect(tcpsock,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
	if(ret==-1)
	{
		perror("连接服务器失败!\n");
		return -1;
	}
	//定义文件描述符集合变量，存放你要监测的文件描述符
	fd_set myset;
	//往集合中添加你想监测的文件描述符
	printf("请输入要广播/点播的信息! 格式如下：\n");
	printf("broadcast:真实信息     广播\n");
	printf("端口号/ip:真实信息     点播\n");
	while(1)
	{
		bzero(msg,50);
		FD_ZERO(&myset);
		FD_SET(tcpsock,&myset);
		FD_SET(0,&myset);
		//调用select去监测集合中所有文件描述符的状态
		ret=select(tcpsock+1,&myset,NULL,NULL,NULL);//监测读就绪，永远等待
		if(ret==-1)
		{
			perror("select失败!\n");
			return -1;
		}
		//判断套接字是否发生了读就绪
		if(FD_ISSET(tcpsock,&myset)!=0)
		{
			ret=recv(tcpsock,msg,50,0);
			if(ret==0){
				printf("\n\t服务器连接异常\n\n");
				return -1;
			}	
			printf("接收的内容是:%s\n",msg);
		}
		else if(FD_ISSET(0,&myset)!=0)
		{
			
			printf("请输入要广播/点播的信息! 格式如下：\n");
			printf("broadcast:真实信息     广播\n");
			printf("端口号/ip:真实信息     点播\n");
			scanf("%s",msg);
			send(tcpsock,msg,50,0);
			if(strcmp(msg,"get") == 0)
			{
				int fd = open("./example.wav",O_RDONLY);
				printf("open fd:%d\n",fd);
				bzero(msg,50);
				int nread;
				ssize_t size=0;
				char data_buf[4096];bzero(data_buf,4096);
				while((nread = read(fd,data_buf,4096)) > 0)  //读取源文件的内容
				{
					size += nread;
					send(tcpsock,data_buf,nread,0);
					bzero(data_buf,4096);
				}
				// sleep(1);
				send(tcpsock,"end",3,0);
				printf("size:%ld\n",size);
				close(fd);
			}
			
		}
		
	}
	
	//挂机
	close(tcpsock);
	return 0;
}