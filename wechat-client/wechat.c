#include "myhead.h"
/*
	微信群程序
*/
int udpsock;//UDP套接字
char send_buf[250]={0};
char usrname[10]={0};
char _switch = 1;
char flag = 1;	


void *recv_anyone(void *arg)
{
	struct sockaddr_in *otheraddr = (struct sockaddr_in *)arg;
	int addrsize=sizeof(*otheraddr);
	ssize_t ret;
	char re_buf[250]={0};
	while(1)
	{
		//接收组播信息
		bzero(re_buf,250);
		ret=recvfrom(udpsock,re_buf,250,0,(struct sockaddr *)otheraddr,&addrsize);
		fflush(stdout);
		// printf("send_buf-[%s]\n",send_buf);
		// printf("re_buf-[%s]\n",re_buf);
		fflush(stdout);
		if(strcmp(re_buf,send_buf) != 0)
		{
			if(flag){
				printf("\n----------------\n");
				printf("[%s]\n",re_buf);
				printf("----------------\n");
				flag = 0;
			}
			else
			{
				printf("[%s]\n",re_buf);
				printf("----------------\n");	
			}	
			
			fflush(stdout);
		}
		_switch = 0;
		usleep(10000);
	}
}

int main()
{
	
	int ret;
	
	struct sockaddr_in bindaddr;
	bzero(&bindaddr,sizeof(bindaddr));
	bindaddr.sin_family=AF_INET;
	bindaddr.sin_addr.s_addr=htonl(INADDR_ANY); //操作系统中有定义一个宏：INADDR_ANY表示任意一个ip地址
	bindaddr.sin_port=htons(10000);
	
	struct sockaddr_in otheraddr;
	bzero(&otheraddr,sizeof(otheraddr));
	otheraddr.sin_family=AF_INET;
	otheraddr.sin_addr.s_addr=inet_addr("224.10.10.90"); //组播发送给224.10.10.90  D类地址（任意的）
	otheraddr.sin_port=htons(10000);
	
	struct sockaddr_in otheraddr2;
	bzero(&otheraddr2,sizeof(otheraddr2));
	int addrsize=sizeof(otheraddr2);
	
	//创建udp套接字
	udpsock=socket(PF_INET,SOCK_DGRAM,0);
	if(udpsock==-1)
	{
		perror("udp失败!\n");
		return -1;
	}
	//屏蔽掉端口号被占用后的连接限制
	int sinsize = 1;//真
    setsockopt(udpsock, SOL_SOCKET, SO_REUSEADDR, &sinsize, sizeof(int));//在socket()之后bind()之前使用
	//绑定
	ret=bind(udpsock,(struct sockaddr *)&bindaddr,sizeof(bindaddr));
	if(ret==-1)
	{
		perror("绑定失败！\n");
		return -1;
	}
	int on = 1;
	setsockopt(udpsock, SOL_SOCKET, SO_BROADCAST, &on, sizeof on);
	
	//设置该接收端添加到组播组里面
	struct ip_mreq mreq; //存放组播信息的结构体
	bzero(&mreq, sizeof mreq);
	mreq.imr_multiaddr.s_addr = inet_addr("224.10.10.90"); // 组播ip地址，必须跟发送端使用的那个D类地址一样
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);  // 必须用这个宏定义
	setsockopt(udpsock, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof mreq);//将目前这个接收端添加到224.10.10.90代表的组里面
	
	
	printf("请输入网上冲浪的ID:\n");
	scanf("%s",usrname);
	system("clear");
	pthread_t display_tid;
	pthread_create(&display_tid, NULL, recv_anyone , &otheraddr2);
	
	//组播信息给所有的在这个组里面的所有接收端
	char tmp_buf[250];
	getchar();
	while(1)
	{
		
		bzero(tmp_buf,250);
		bzero(send_buf,250);
		// while(getchar() != '\n');
		printf("----------------\n%s:",usrname);	
		fflush(stdout);
		// scanf("%s",tmp_buf);
		fgets(tmp_buf,250,stdin);
		tmp_buf[strlen(tmp_buf)-1] = '\0';
		_switch=1;
		// printf("----------------\n");
		sprintf(send_buf,"%s:%s",usrname,tmp_buf);
		// printf("send:[%s]\n",send_buf);
		//组播  D类地址
	    ret=sendto(udpsock,send_buf,250,0,(struct sockaddr *)&otheraddr,sizeof(otheraddr));
	    flag=1;
		while(_switch);
	}
	//关机
	close(udpsock);
	return 0;
}