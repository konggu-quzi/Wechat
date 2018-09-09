#include "camera.h"
#include "myjpeg.h"
#include "lcd_app.h"
#include "head4animation.h"

#include <locale.h>
#include <assert.h>	
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include "tslib.h"
#include "kernel_list.h"

#define VIDEO   '1'
#define VOICE   '2'
#define MESSAGE '3'
#define IMG     '4'

#define VIDEO_send   "1"
#define VOICE_send   "2"
#define MESSAGE_send "3"
#define IMG_send     "4"

struct chatdata_node {	
	
	int mtime;
	// int data_offset;
	int type;
	int width;
	int height;
	char voice_name[3];
	unsigned char voice_length;
	
	struct list_head list;
};
int chat_times = 0;
struct chatdata_node *head_node;


struct image_info *usr_tool_imageinfo;
struct image_info *voice1_imageinfo;
struct image_info *voice2_imageinfo;
struct image_info *video_imageinfo;
struct image_info *msg_imageinfo;
struct image_info *micro_imageinfo;
struct image_info *me_imageinfo;
struct image_info *other_imageinfo;
struct image_info *voice_frame1_imageinfo;
struct image_info *voice_frame2_imageinfo;
struct image_info *voice_frame3_imageinfo;
struct image_info *voice_frame4_imageinfo;

extern int fb_fd;
extern unsigned char *fb_mem;
int tcpsock,tsfd,camerafd;
struct tsdev *TS;
int i=0; 

struct timeval start_tv,end_tv;
unsigned long int start,end;

struct usrbuf *usraddr;
struct _block block;
struct timeval timeout;
char voice_flag = 0,voice_stop_flag = 0;char video_flag = 0;
	
int frame=0;
char time_flag=1;


void safe_exit(int sig)							
{
	printf("sig:[%d]\n",sig);
	if(video_flag)
	{
		camera_icotl(camerafd,usraddr,3);
	}
	close(tcpsock);
	close(tsfd);
	close(camerafd);
	// lcd_clear_display(1);
	munmap(fb_mem,800*480*4);
	close(fb_fd);
	free(TS);
	exit(1);
}

void jpeg_init()
{
	block.x=50;
	block.y=310;
	block.width=160;
	block.height=120;
	// lcd_put_block(0 , 0 , 800 , 480 , 0xFFFFFF);//白底清屏
	usr_tool_imageinfo = jpeg_file_show("./usr_tool.jpg",1);
	voice1_imageinfo = jpeg_file_show("./voice1.jpg",1);//按钮
	voice2_imageinfo = jpeg_file_show("./voice2.jpg",1);//方块
	video_imageinfo = jpeg_file_show("./video.jpg",1);
	msg_imageinfo = jpeg_file_show("./msg.jpg",1);
	micro_imageinfo = jpeg_file_show("./microphone.jpg",1);
	
	voice_frame1_imageinfo = jpeg_file_show("./voice_frame1.jpg",1);
	voice_frame2_imageinfo = jpeg_file_show("./voice_frame2.jpg",1);
	voice_frame3_imageinfo = jpeg_file_show("./voice_frame3.jpg",1);
	voice_frame4_imageinfo = jpeg_file_show("./voice_frame4.jpg",1);
	
	me_imageinfo = jpeg_file_show("./me.jpg",1);
	other_imageinfo = jpeg_file_show("./other.jpg",1);
	
	write_lcd_block(usr_tool_imageinfo->bmp_buf,0,0,NULL, usr_tool_imageinfo, fb_mem);
	//背景色初始化;
	lcd_put_block(0 , 0 , 160 , 39 , 0xeeebe9);//顶部左状态栏背景
	lcd_put_block(0 , 40 , 160 , 480 , 0xDDDCDD);//好友列表背景
	lcd_put_block(160 , 0 , 800 , 39 , 0xF4F4F4);//顶部右状态栏背景
	lcd_put_block(0 , 39 , 800 , 40 , 0xD8D8D8);//顶部状态栏和（聊天框、好友列表）的--分隔线--
	lcd_put_block(160 , 40 , 800 , 430 , 0xEBEBEB);//聊天框背景
	lcd_put_block(160 , 430 , 800 , 431 , 0xD8D8D8);//聊天框和输入框的  --分隔线--
	lcd_put_block(160 , 431 , 800 , 480 , 0xF4F4F4);//输入框背景
	
	
	write_lcd_block(other_imageinfo->bmp_buf,732,300,NULL, other_imageinfo, fb_mem);
	
	lcd_put_block(248 + 50 +8 , 470 , 298 + 200 , 473 , 0xC8C8C8);//msg横线
	
	// write_lcd_block(voice1_imageinfo->bmp_buf,248,425,NULL, voice1_imageinfo, fb_mem);
	//****按住说话
	write_lcd_block(voice2_imageinfo->bmp_buf,558,430+8,NULL, voice2_imageinfo, fb_mem);
	write_lcd_block(video_imageinfo->bmp_buf,800-5-40,430+5,NULL, video_imageinfo, fb_mem);
	
}

void cameraa_init()
{
	camerafd = camera_init("/dev/video7");
	printf("camerafd:[%d]\n",camerafd);
	if(camerafd == -1)
		exit(1);
	usraddr = camera_LCD_show_other(camerafd);
	signal(SIGINT,safe_exit);
}

void ts_init()
{
	TS = init_ts();
	tsfd = ts_fd(TS);
	printf("ts_fd:[%d]\n",ts_fd);
}

void tcp_init()
{
	int ret;
	//定义ipv4地址结构体变量
	struct sockaddr_in bindaddr;
	bzero(&bindaddr,sizeof(bindaddr));
	bindaddr.sin_family=AF_INET;
	bindaddr.sin_addr.s_addr=htonl(INADDR_ANY); //操作系统中有定义一个宏：INADDR_ANY表示任意一个ip地址
	bindaddr.sin_port=htons(10001);//客户端的端口号
	
	struct sockaddr_in serveraddr;//服务器的ip和端口号
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	// serveraddr.sin_addr.s_addr=inet_addr("47.107.59.1");//服务器的ip，小端转大端
	serveraddr.sin_addr.s_addr=inet_addr("192.168.90.68");//服务器的ip，小端转大端
	serveraddr.sin_port=htons(10000);//服务器的端口号
	//创建tcp类型的套接字
	tcpsock=socket(AF_INET,SOCK_STREAM,0);
	printf("tcpsock:[%d]\n",tcpsock);
	if(tcpsock==-1)
	{
		perror("创建tcp套接字失败！\n");
		exit(1);
	}
	int sinsize = 1;//真
    setsockopt(tcpsock, SOL_SOCKET, SO_REUSEADDR, &sinsize, sizeof(int));//在socket()之后bind()之前使用
	//绑定ip和端口号
	ret=bind(tcpsock,(struct sockaddr *)&bindaddr,sizeof(bindaddr));
	if(ret==-1)
	{
		perror("绑定失败!\n");
		exit(1);
	}
	//连接服务器
	ret=connect(tcpsock,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
	if(ret==-1)
	{
		perror("连接服务器失败!\n");
		exit(1);
	}
}

int catch_action(struct tsdev *TS)
{
	// int mode = -1;
	struct ts_sample samp;
	ts_read(TS, &samp, 1);
	// printf("x:[%d]   y:[%d]   samp.pressure:[%d]\n",samp.x,samp.y,samp.pressure);
	if(samp.x > 558 && samp.x < 558 + 187 
			&& samp.y > 430 && samp.y < 480 && samp.pressure > 0)
	{
		//发送语音
		return 2;
	}
	else if(samp.x > 732-50 -30 && samp.x < 732-50 +30
			&& samp.y > 300+30-30 && samp.y < 300+30+30 && samp.pressure > 0 && 
			(voice_flag == 1)) 
	{	
		//播放语音
		return 3;	
		
	}
	else if(samp.x > 800-70 && samp.x < 800
			&& samp.y > 420 && samp.y < 480 && samp.pressure > 0)
	{
		//开启视频模式
		return 1;
	}
	// else if(samp.x > 800-70 && samp.x < 800
			// && samp.y > 420 && samp.y < 480 && samp.pressure > 0)
	// {
		// return 1;
	// }
	else
	{
		return -1;
	}
}

void get_voice(struct tsdev *TS)
{
	int ret;
	struct ts_sample samp;
	
	// printf("strat micro\n");
	write_lcd_block(micro_imageinfo->bmp_buf,430,200,NULL, micro_imageinfo, fb_mem);
		
	fd_set myset;
	FD_ZERO(&myset);
	FD_SET(tsfd,&myset);			
	struct timeval timeout;
	timeout.tv_sec = 0; 
	timeout.tv_usec = 10000;
	//******************************************
	chat_times++;
	//******初始化聊天信息结构体
	struct chatdata_node *voicedata_node = calloc(1,sizeof(struct chatdata_node));
	voicedata_node->type = 1; // 1 为自己的语音
	voicedata_node->width = 115;
	// voicedata_node->height = 30;	
	//  插入
	list_add_tail(&(voicedata_node->list),&(head_node->list));//添加节点到内核链表
	//******准备录音工作
	sprintf(voicedata_node->voice_name,"%d",chat_times);
	char voice_cmd[58+strlen(voicedata_node->voice_name)];
	sprintf(voice_cmd,"arecord -d61 -c1 -r16000 -twav -fS16_LE ./myvoice/example%s.wav &",voicedata_node->voice_name);
	// printf("[%s]\n",voice_cmd);
	
	gettimeofday(&start_tv,NULL);
	gettimeofday(&end_tv,NULL);
	int angle = 1;
	int time = 0,time_tmp =  1000*(end_tv.tv_sec - start_tv.tv_sec) + 
						(end_tv.tv_usec - start_tv.tv_usec)/1000;
	system(voice_cmd);
	while(1)
	{		
		FD_SET(tsfd,&myset);
		ret = select(tsfd+1,&myset,NULL,NULL,&timeout);//监测读就绪
		if(ret > 0)
		{
			ts_read(TS, &samp, 1);
			printf("samp.pressure:[%d]\n",samp.pressure);
			if(samp.pressure == 0)
			{
				system("killall -KILL arecord"); 
				lcd_put_block(450-40,180,450+150-40,180+150,0xEBEBEB);
				voicedata_node->voice_length = time/1000;
				printf("voice_length:[%d]\n",voicedata_node->voice_length);
				return;
			}
			
		}	
		gettimeofday(&end_tv,NULL);
		time = 1000*(end_tv.tv_sec - start_tv.tv_sec) + 
				   (end_tv.tv_usec - start_tv.tv_usec)/1000;
		
		if(time - time_tmp > 166)
		{
			lcd_put_circle(520-40,250,angle++,50,5,0xB5388F);
			// printf("time:[%d]\n",time/500*3);
			if(angle > 360)
			{
				usleep(200000);
				system("killall -KILL arecord"); 
				lcd_put_block(450-40,180,450+150-40,180+150,0xEBEBEB);
				voicedata_node->voice_length = time/1000;
				printf("voice_length:[%d]\n",voicedata_node->voice_length);
				return;
			}
			time_tmp = time;
		}
	}
}

int send_data(char mode)
{
	ssize_t size = 0,nread;
	switch(mode)
	{
		case VIDEO:
		{
			//************发送************
			//****报头
			send(tcpsock,VIDEO_send,1,0);
			//****jpeg length 
			send(tcpsock,&usraddr[i].length,4,0);
			//****jpeg
			int nsend=0;
			nsend = send(tcpsock,usraddr[i].p,usraddr[i].length,0);
			// printf("send-length:[%d]  acl-length:[%d]\n",nsend,usraddr[i].length);
			break;
		}
		case VOICE:
		{
			struct chatdata_node *first_node;
			first_node = list_entry(head_node->list.next , struct chatdata_node , list);
			//************发送************
			//****报头
			send(tcpsock,VOICE_send,1,0);
			//****voice 长度
			send(tcpsock,&first_node->voice_length,1,0);
			//****voice data
			char path[21+strlen(first_node->voice_name)];
			sprintf(path,"./myvoice/example%s.wav",first_node->voice_name);
			int fd = open(path,O_RDONLY);// printf("open fd:%d\n",fd);
			struct stat file_state;
			stat(path,&file_state);
			int data_length = file_state.st_size;
			send(tcpsock,&data_length,4,0);//send
			char data_buf[data_length];
			bzero(data_buf,data_length);
			while(data_length != 0)  //读取源文件的内容
			{
				nread = read(fd,data_buf,data_length);
				size += nread;
				data_length -= nread;
				send(tcpsock,data_buf,nread,0);
			}
			printf("send size:[%ld]\n",size);
			break;
		}	
		default:;
	}
}

int recv_data(void)
{
	char msg_header;
	unsigned char voice_length_sec;
	recv(tcpsock,&msg_header,1,0);
	switch(msg_header)
	{
		case VIDEO:
		{			
			// printf("VIDEO\n");
			break;
		}
		case VOICE:
		{
			//****voice 长度
			recv(tcpsock,&voice_length_sec,1,0);
			break;
		}
		default:;
	}
	
	int data_length;
	recv(tcpsock,&data_length,4,0);
	char data_buf[data_length];
	char *p = data_buf;
	bzero(data_buf,data_length);
	int nread;
	int size = 0;
	while(data_length != 0)
	{
		nread = recv(tcpsock,p,data_length,0);
		data_length -= nread;
		size += nread;
		p += nread;
		// printf("nread:[%u]\n",nread);
	}
	// printf("recv size:[%d]\n",size);
	
	switch(msg_header)
	{
		case VIDEO:
		{		
			struct image_info *recv_imageinfo = jpeg_decompress(data_buf,size,1);
			write_lcd_block(recv_imageinfo->bmp_buf,0,0,&block, recv_imageinfo, fb_mem);
			free(recv_imageinfo);
			break;
		}
		case VOICE:
		{
			
			int fd = open("./test/example.wav",O_CREAT|O_TRUNC|O_RDWR,0777);
			// printf("open fd:%d\n",fd);
			
			int nwrite;
			p = data_buf;
			while(size != 0)  //写入
			{	
				nwrite = write(fd,p,size);
				size -= nwrite;
				p += nwrite;
			}
			printf("write size:[%d]\n",p - data_buf);
			voice_flag = 1;
			voice_stop_flag == 0;
			lcd_put_circle(732-50,300+30,360,0,30,0xE50056);
			break;
		}
		default:;
	}
}

void start_video(int camerafd)
{
	camera_icotl(camerafd,NULL,1);
	int ret;
	struct ts_sample samp;
	struct image_info *imageinfo;
	
	fd_set myset;
	FD_ZERO(&myset);
	FD_SET(tsfd,&myset);			
	struct timeval timeout;
	timeout.tv_sec = 0; 
	timeout.tv_usec = 10000;
	int max = tcpsock > tsfd ? tcpsock:tsfd;
	char av_flag = 1;
	while(1)
	{
		FD_ZERO(&myset);
		FD_SET(camerafd,&myset);FD_SET(tsfd,&myset);FD_SET(tcpsock,&myset);
		ret = select(max+1,&myset,NULL,NULL,&timeout);//监测读就绪
		
		if(FD_ISSET(tcpsock,&myset)!=0)
		{
			// printf("22等待...\n");
			//**********接收摄像头数据************	
			recv_data();
			/* if(time_flag)
			{
				gettimeofday(&start_tv,NULL);
				time_flag = 0;
			}
			else
			{
				gettimeofday(&end_tv,NULL);
				if(end_tv.tv_sec-start_tv.tv_sec >= 1 && end_tv.tv_usec >= start_tv.tv_usec)
				{
					time_flag = 1;
					// printf("frame:[%d]\n",frame);
					printf("frame:[%d]   time:[%ld]\n",frame,
					1000*(end_tv.tv_sec-start_tv.tv_sec) + 
					(end_tv.tv_usec - start_tv.tv_usec)/1000);
					frame = 0;			
				} 
				else
					frame++;
				// printf("frame:[%d]\n",frame);
			}				 */
			// av_flag = 1;
		}
		else if(FD_ISSET(tsfd,&myset)!=0)
		{
			//收尾工作
			
			if(catch_action(TS) == 1)
			{
				camera_icotl(camerafd,NULL,2);
				// lcd_put_block(450,180,450+150,180+150,0xEBEBEB);
				time_flag = 1;
				break;
			}
		}
		else if(FD_ISSET(camerafd,&myset)!=0)
		{
			//**********发送摄像头数据************		
			struct v4l2_buffer outbuf; 
			bzero(&outbuf,sizeof(outbuf));
			outbuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
			outbuf.memory=V4L2_MEMORY_MMAP;
			outbuf.index=i;
			//将画面出队
			ret=ioctl(camerafd,VIDIOC_DQBUF,&outbuf);
			if(ret==-1)
			{
				perror("出队!\n");
				exit(1);
			}
			//让画面入队
			ret=ioctl(camerafd,VIDIOC_QBUF,&outbuf);
			if(ret==-1)
			{
				perror("入队!\n");
				exit(1);
			}
			
			//************本地显示************
			imageinfo = jpeg_decompress(usraddr[i].p,usraddr[i].length,4);
			write_lcd_block(imageinfo->bmp_buf,50,310,NULL, imageinfo, fb_mem);
			free(imageinfo);
			//************发送************
			send_data(VIDEO);
			
			if(i == 3)
				i = 0;
			else
				i++;
			// av_flag = 0;
		}
		
	}
	enum v4l2_buf_type mytype;
	mytype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(camerafd,VIDIOC_STREAMOFF,&mytype);
	if(ret == -1)
	{
		perror("关闭摄像头采集!\n");
		exit(-1);
	}
	//收尾
	for(i=0; i<4; i++)
		munmap(usraddr[i].p,usraddr[i].length);
	free(usraddr);
}

int update_chat_frame(char mode)
{
	switch(mode)
	{
		case VOICE:
		{
			struct chatdata_node *first_node;
			first_node = list_entry(head_node->list.next , struct chatdata_node , list);			
			if(first_node->type == 1)
			{
				write_lcd_block(me_imageinfo->bmp_buf,160+15,390,NULL, me_imageinfo, fb_mem);
				write_lcd_block(voice_frame4_imageinfo->bmp_buf,160+15+30+4,390,NULL,voice_frame4_imageinfo, fb_mem);
				
				char length_str[2] = {0};
				sprintf(length_str,"%d",first_node->voice_length);
				printf("length_str:[%s]\n",length_str);
				wchar_t *wstr;
				str_to_wstr(length_str,&wstr);
				// printf("wcslen(wstr):[%d]\n",wcslen( wstr ));
				// show_hanzi(wtext,30,0,0,1,0,50,1);//0xEBEBEB
				show_hanzi(wstr,20 , 0,0 ,1 ,160+15+30+4 +3  +115,390+10,1);//0xEBEBEB
				// show_hanzi(wstr,30 , 0,0 ,1 ,0,0,    1);//0xEBEBEB
				while(1);
				free(wstr);
			}
			else if(first_node->type == 2)
			{
				write_lcd_block(me_imageinfo->bmp_buf,160+15,390,NULL, me_imageinfo, fb_mem);
				write_lcd_block(voice_frame4_imageinfo->bmp_buf,160+15+30+4,390,NULL,voice_frame4_imageinfo, fb_mem);
				// voice_length
			}
			
			break;
		}
		default:;
	}
	return 1;
}

int main(int argc, char **argv)
{
	//初始化聊天信息头结点
	head_node = malloc(sizeof(struct chatdata_node));
	INIT_LIST_HEAD(&head_node->list);
	
	lcd_init();
	lcd_put_block(0, 0 ,800, 480, 0xFFFFFF);
	
	jpeg_init();
	// while(1)
	// {
		// lcd_put_block(248 + 50 + 8 + 5, 435 ,248 + 50 + 8 + 6, 435+30 , 0x040404);
		// usleep(500000);
		// lcd_put_block(248 + 50 + 8 + 5, 435 ,248 + 50 + 8 + 6, 435+30 , 0xD8D8D8);
		// usleep(500000);
	// }
	/**********摄像头初始化**************/
	// cameraa_init();
	/***********触摸屏初始化************/
	ts_init();
	// while(1)
	// {
		// ts_read(TS, &samp, 1);
		// printf("%d   %d\n",samp.x,samp.y);
	// }
	/***********tcp初始化************/
	tcp_init();

	fd_set myset;	
	timeout.tv_sec = 0; 
	timeout.tv_usec = 10000;
	int max = tcpsock > tsfd ? tcpsock:tsfd;
	int ret;
	while(1)
	{
		FD_ZERO(&myset);
		FD_SET(tcpsock,&myset);FD_SET(tsfd,&myset);
		// FD_SET(0,&myset);	
		ret=select(max+1,&myset,NULL,NULL,NULL);//监测读就绪，永远等待
		if(ret==-1)
		{
			perror("select失败!\n");
			exit(1);
		}
		if(FD_ISSET(tsfd,&myset)!=0)
		{
			// printf("解析用户输入!\n");
			//********解析用户输入
			switch(catch_action(TS))
			{
				case 1: 
					/*************视频模式************/ 
					start_video(camerafd);
					break;
				case 2:
					/*************录音模式************/ 
					get_voice(TS);
					// send_data(VOICE);
					update_chat_frame(VOICE);
					// while(1);
					break;
				case 3:
				{
					/*************播放语音************/ 
					// play_voice();
					// if(!voice_stop_flag)
					// {
						// system("aplay example.wav &");
						// lcd_put_block(732-50-30 , 300+30-30 , 732-50 +30 , 300+30+30 , 0xEBEBEB);
						// voice_stop_flag = 1;
						voice_flag == 0;
					// }
					// else
					// {
						// voice_flag = 0;
						// voice_stop_flag = 0;
						// system("killall -KILL aplay"); 
					// }
					break;
				}
				case 4: ;break;
				default:;
			}
				
		}
		//判断套接字是否发生了读就绪sdf 
		else if(FD_ISSET(tcpsock,&myset)!=0)
		{			
			recv_data();
			// write_lcd_block(me_imageinfo->bmp_buf,732,300,NULL, me_imageinfo, fb_mem);					
		}				
	}
	
}	
	// if(argc != 2)
	// {
		// printf("Usage: %s <jpeg image>\n", argv[0]);
		// exit(1);
	// }
	
	
			/* if(time_flag)
			{
				gettimeofday(&start_tv,NULL);
				time_flag = 0;
			}
			else
			{
				gettimeofday(&end_tv,NULL);
				if(end_tv.tv_sec-start_tv.tv_sec >= 1 && end_tv.tv_usec >= start_tv.tv_usec)
				{
					time_flag = 1;
					// printf("frame:[%d]   time:[%ld]\n",frame,
					// 1000*(end_tv.tv_sec-start_tv.tv_sec) + 
					// (end_tv.tv_usec - start_tv.tv_usec)/1000);
					frame = 0;			
				}
				else
					frame++;
				// printf("frame:[%d]\n",frame);
			} */	
	
	
	
	
	
	/**********键盘输入汉字**************/
	// wchar_t *wchar;
	// char ch_buf[500]={0};
	// fgets(ch_buf,500,stdin);
	
	// printf("-----------------------------------\n");
	// str_to_wstr(ch_buf,&wchar);
	
	// int size=25;
	// show_hanzi(wchar,size,0,0,1,0,50,1);//0xEBEBEB
	/**********键盘输入汉字**************/










