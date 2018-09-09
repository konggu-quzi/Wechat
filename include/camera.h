#ifndef __CAMERA_H
#define __CAMERA_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pthread.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <poll.h>
#include <termios.h>
#include <stropts.h>
#include <sys/mman.h>
#include <linux/videodev2.h> //跟V4L2有关的头文件

struct usrbuf
{
	void *p; //首地址
	int length; //大小
};

extern int camera_init(char *path);

extern struct usrbuf *camera_LCD_show_other(int camerafd);

extern int camera_LCD_show_me(int camerafd);

extern void camera_icotl(int camerafd,struct usrbuf *usraddr,char mode);

#endif

