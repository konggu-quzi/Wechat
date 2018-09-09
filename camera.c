#include "camera.h"
#include "myjpeg.h"

//定义一个结构体，存放映射得到的每个缓冲块首地址以及每个缓冲区的大小


int camera_init(char *path)
{
	int camerafd;
	int ret;
	//打开摄像头的驱动
	camerafd=open(path,O_RDWR);
	if(camerafd==-1)
	{
		perror("打开摄像头失败!\n");
		return -1;
	}
	//获取摄像头功能参数
	struct v4l2_capability mycap;
	ret=ioctl(camerafd,VIDIOC_QUERYCAP,&mycap);
	if(ret==-1)
	{
		perror("获取摄像头功能参数失败!\n");
		return -1;
	}
	printf("摄像头驱动内核版本号:%x\n",mycap.version);
	//设置摄像头通道 
	int index=0;
	ret=ioctl(camerafd,VIDIOC_S_INPUT,&index);
	if(ret==-1)
	{
		perror("设置摄像头通道!\n");
		return -1;
	}
	//获取摄像头当前的采集格式 
	struct v4l2_format myfmt;
	bzero(&myfmt,sizeof(myfmt));
	myfmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret=ioctl(camerafd,VIDIOC_G_FMT,&myfmt);
	if(ret==-1)
	{
		perror("设置摄像头采集格式 !\n");
		return -1;
	}
	printf("目前摄像头采集的画面宽：%d 高：%d\n",myfmt.fmt.pix.width,myfmt.fmt.pix.height);
	
	return camerafd;
}

struct usrbuf *camera_LCD_show_other(int camerafd)
{	
	// printf("camerafd:%d\n",camerafd);
	int ret;
	int i;
	//申请缓冲块，用于等一会存放摄像头拍摄的画面
	struct v4l2_requestbuffers mybuf;
	bzero(&mybuf,sizeof(mybuf));
	mybuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	mybuf.memory=V4L2_MEMORY_MMAP;
	mybuf.count=4;//申请4个缓冲块
	ret=ioctl(camerafd,VIDIOC_REQBUFS,&mybuf);
	if(ret==-1)
	{
		perror("申请缓冲块!\n");
		exit(1);
	}
	//定义一个结构体指针存放4个缓存块的首地址和大小
	struct usrbuf *usraddr=calloc(4,sizeof(struct usrbuf));
	//分配你刚才申请的4个缓冲块
	for(i=0; i<mybuf.count; i++)
	{
		struct v4l2_buffer gbuf; 
		bzero(&gbuf,sizeof(gbuf));
		gbuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
		gbuf.memory=V4L2_MEMORY_MMAP;
		gbuf.index=i;
		ret=ioctl(camerafd,VIDIOC_QUERYBUF,&gbuf);
		if(ret==-1)
		{
			perror("分配缓冲块!\n");
			exit(1);
		}
		//将对应的缓冲块映射到用户空间
		usraddr[i].length=gbuf.length;
		usraddr[i].p=mmap(NULL,gbuf.length,PROT_READ|PROT_WRITE,MAP_SHARED,camerafd,gbuf.m.offset);
		//让画面入队
		ret=ioctl(camerafd,VIDIOC_QBUF,&gbuf);
		if(ret==-1)
		{
			perror("入队!\n");
			exit(1);
		} 
	}
		
	return usraddr;
}


void camera_icotl(int camerafd,struct usrbuf *usraddr,char mode)
{
	if(mode)
	{
		enum v4l2_buf_type mytype;
		mytype=V4L2_BUF_TYPE_VIDEO_CAPTURE;
		int ret=ioctl(camerafd,VIDIOC_STREAMON,&mytype);
		if(ret==-1)
		{
			perror("摄像头开始采集数据!\n");
			exit(1);
		}	
	}
	else if(mode == 2)
	{
		enum v4l2_buf_type mytype;
		mytype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		int ret = ioctl(camerafd,VIDIOC_STREAMOFF,&mytype);
		if(ret == -1)
		{
			perror("关闭摄像头采集!\n");
			exit(-1);
		}
		
	}
	else if(mode == 3)
	{
		enum v4l2_buf_type mytype;
		mytype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		int ret = ioctl(camerafd,VIDIOC_STREAMOFF,&mytype);
		if(ret == -1)
		{
			perror("关闭摄像头采集!\n");
			exit(-1);
		}
		
		for(int i = 0 ; i<4; i++)
			munmap(usraddr[i].p,usraddr[i].length);
		free(usraddr);
	}
}	






		
