#ifndef __MYJPEG_H
#define __MYJPEG_H


// #include "myhead.h"

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

#include <jpeglib.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


struct image_info
{
	int width;
	int height;
	int pixel_size;
	unsigned char *bmp_buf;
};

struct _block
{
	int x;
	int y;
	int width;
	int height;	
};

#define LCD_WIDTH 800
#define LCD_HEIGHT 480


// 451010 18 1 1 108 18
// 4510101811108
// 将bmp_buffer中的24bits的RGB数据，写入LCD的32bits的显存中
void write_lcd_block(unsigned char *bmp_buffer,int x_start,int y_start,struct _block *block,
			struct image_info *imageinfo,
			unsigned char *FB);

// 将jpeg文件的压缩图像数据读出，放到jpg_buffer中去等待解压
unsigned long read_image_from_file(int fd,
				   unsigned char *jpg_buffer,
				   unsigned long jpg_size);

int Stat(const char *filename, struct stat *file_info);

int Open(const char *filename, int mode);

struct image_info *jpeg_decompress(unsigned char *jpg_buffer,unsigned long int size,int scale);

struct image_info *jpeg_file_show(char *argv,int scale);







#endif

