#include "myhead.h"
#include "kernel_list.h"

#define VIDEO   '1'
#define VOICE   '2'
#define MESSAGE '3'
#define IMG     '4'
#define VIDEO_send   "1"
#define VOICE_send   "2"
#define MESSAGE_send "3"
#define IMG_send     "4"

//封装保存客户端信息的内核链表
struct clientlist
{
	//有效数据
	char ip[16];
	unsigned short someport;
	int somesock;
	//保存指针的小结构体
	struct list_head mylist;
};
struct clientlist *myhead;


//封装初始化内核链表表头的函数
struct clientlist *list_init_()
{
	struct clientlist *head=malloc(sizeof(struct clientlist));
	INIT_LIST_HEAD(&(head->mylist));
	return head;
}

void transmit_data(struct clientlist *pos,char mode)
{
	int ret;	
	int video_length;
	switch(mode)
	{
		case VIDEO:
			printf("VIDEO\n");
			send(pos->somesock,VIDEO_send,1,0);
			break;
		case VOICE:
			// printf("VOICE\n");
			send(pos->somesock,VOICE_send,1,0);
			//*********接收声音时长
			unsigned char voice_length;
			ret = recv(pos->somesock,&voice_length,1,0);
			if(ret == 0)
			{
				printf("IP:[%s]   PORT:[%hu]   disconnect...\n",pos->ip,pos->someport);
				list_del(&(pos->mylist));
				free(pos);
				return;
			}
			send(pos->somesock,&voice_length,1,0);	
			break;
		case 2:;break;
		case 3:;break;
		default:;
	}
	int data_length;
	ret = recv(pos->somesock,&data_length,4,0);
	// printf("data_length:[%d]\n",data_length);
	if(ret == 0)
	{
		printf("IP:[%s]   PORT:[%hu]   disconnect...\n",pos->ip,pos->someport);
		list_del(&(pos->mylist));
		free(pos);
		return;
	}
	send(pos->somesock,&data_length,4,0);
	unsigned char data_buf[data_length];
	bzero(data_buf,data_length);
	int nread;
	int size = 0;
	while(data_length != 0)
	{
		nread = recv(pos->somesock,data_buf,data_length,0);
		data_length -= nread;
		size += nread;
		// printf("nread:[%u]\n",nread);
		// printf("end:[%s]\n",data_buf + nread - 7);
		// if()
		send(pos->somesock,data_buf,nread,0);	
	}
	printf("finish_transmit_size:[%d]\n",size);
}

int main(int argc,char **argv)
{
	int tcpsock;
	int newsock;
	int ret;
	//定义ipv4地址结构体变量
	struct sockaddr_in bindaddr;
	bzero(&bindaddr,sizeof(bindaddr));
	bindaddr.sin_family=AF_INET;
	bindaddr.sin_addr.s_addr=htonl(INADDR_ANY); //操作系统中有定义一个宏：INADDR_ANY表示任意一个ip地址
	// inet_aton("192.168.90.68",&(bindaddr.sin_addr));//服务器自己的ip，小端转大端
	bindaddr.sin_port=htons(atoi("10000"));//服务器的端口号
	
	struct sockaddr_in clientaddr;
	int addrsize=sizeof(clientaddr);
	
	//初始化内核链表的表头
	myhead=list_init_();
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
	//监听
	ret=listen(tcpsock,5);
	if(ret==-1)
	{
		perror("监听失败!\n");
		return -1;
	}
	
	fd_set  myset;
	struct clientlist *pos;
	int MAX = tcpsock;
	while(1)
	{
		
		FD_ZERO(&myset);
		FD_SET(tcpsock,&myset); 
		list_for_each_entry(pos,&(myhead->mylist),mylist)
		{
			if(pos->somesock > MAX)	MAX = pos->somesock;
			FD_SET(pos->somesock,&myset);

		}
		ret=select(MAX+1,&myset,NULL,NULL,NULL);//监测读就绪，永远等待
		if(ret==-1)
		{
			perror("select失败!\n");
			return -1;
		}
		
		if(FD_ISSET(tcpsock,&myset)!=0)
		{
			//接受客户端的连接请求
			newsock=accept(tcpsock,(struct sockaddr *)&clientaddr,&addrsize);
			if(newsock==-1)
			{
				perror("接收连接请求失败!\n");
				return -1;
			}
			
			//将目前连接成功的这个客户端信息(ip，端口号，已连接套接字)保存到内核链表
			struct clientlist *newnode=malloc(sizeof(struct clientlist));//初始化新节点
			strcpy(newnode->ip,inet_ntoa(clientaddr.sin_addr));
			newnode->someport=ntohs(clientaddr.sin_port);
			newnode->somesock=newsock;
			list_add_tail(&(newnode->mylist),&(myhead->mylist));//添加节点到内核链表
			//打印新客户端信息
			printf("IP:[%s]   PORT:[%hu]   connect success...\n",newnode->ip,newnode->someport);
			
		}
		else
		{
			list_for_each_entry(pos,&(myhead->mylist),mylist)
			{
				
				if(FD_ISSET(pos->somesock,&myset) != 0)
				{
					
					char msg_header;				
					ret=recv(pos->somesock,&msg_header,1,0);//有消息就接收，没有就阻塞
					if(ret==0)//客户端断开连接 ----------释放
					{
						printf("IP:[%s]   PORT:[%hu]   disconnect...\n",pos->ip,pos->someport);
						list_del(&(pos->mylist));
						free(pos);					
					}
					else	
					{	
						
						switch(msg_header)
						{
							case VIDEO:
								printf("1-VIDEO from IP:[%s]   PORT:[%hu]...\n",pos->ip,pos->someport);
								transmit_data(pos,VIDEO);
								;					
								break;
							case VOICE:
								printf("2-VOICE from IP:[%s]   PORT:[%hu]...\n",pos->ip,pos->someport);
								transmit_data(pos,VOICE);
								break;
							case MESSAGE:;
								break;
							case IMG:;
								break;
							default:;
						}
						
						
						//判断服务器收到的信息格式
						//切割字符串  sscanf()   strtok() 

						
						/* strcpy(keybuf,strtok(buf,":"));
						strcpy(truemsg,strtok(NULL,":"));
						printf("keybuf is:%s truemsg is:%s\n",keybuf,truemsg);
						struct clientlist *_pos;
						if(strcmp(keybuf,"broadcast")==0)//广播 
						{	
							//遍历内核链表，将信息转发给所有其他的客户端
							list_for_each_entry(_pos,&(myhead->mylist),mylist)
							{
								//判断，不要发给自己
								if(_pos->somesock!=pos->somesock)
								{
									send(_pos->somesock,truemsg,strlen(truemsg),0);
								}
							}
						}
						else //点播
						{
							//遍历内核链表，将信息转发给指定的客户端
							list_for_each_entry(_pos,&(myhead->mylist),mylist)
							{
								//if(strcmp(_pos->ip,keybuf)==0)//找到了指定的客户端
								if(_pos->someport==atoi(keybuf))
								{
									send(_pos->somesock,truemsg,strlen(truemsg),0);
								}
							}
						} */
					}
					break;
				}
			}		
			
		}
		
	}
	//挂机
	close(tcpsock);
	close(newsock);
	return 0;
}