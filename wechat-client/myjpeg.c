#include "myjpeg.h"



extern unsigned char *fb_mem;

// 将bmp_buffer中的24bits的RGB数据，写入LCD的32bits的显存中
void write_lcd_block(unsigned char *bmp_buffer,int x_start,int y_start,struct _block *block,
			struct image_info *imageinfo,
			unsigned char *FB)
{
	// int nums = 0,nums1 = 0;
	int x_lcd, y_lcd;
	int x_bmp, y_bmp;
	
	if(block != NULL)
	{
		for(y_lcd=y_start,y_bmp=0; y_lcd < LCD_HEIGHT && y_bmp<imageinfo->height; y_lcd++,y_bmp++)
		{
			for(x_lcd=x_start,x_bmp=0; x_lcd < LCD_WIDTH && x_bmp<imageinfo->width; x_lcd++,x_bmp++)
			{
				if(x_lcd == block->x && y_lcd > block->y - 1 && y_lcd < block->y + block->height)
				{
					x_lcd+=block->width-1;
					x_bmp+=block->width-1;
					continue;
				}	
				unsigned long lcd_offset = (LCD_WIDTH*y_lcd + x_lcd) * 4; //
				unsigned long bmp_offset = (imageinfo->width*y_bmp+x_bmp) *
								imageinfo->pixel_size;  //
				
				memset(FB + lcd_offset + 3 , 0 , 1);
				
				memcpy(FB + lcd_offset + 2,
					   bmp_buffer + bmp_offset + 0, 1);//R
				memcpy(FB + lcd_offset + 1,
					   bmp_buffer + bmp_offset + 1, 1);
				memcpy(FB + lcd_offset + 0,
					   bmp_buffer + bmp_offset + 2, 1);
									
			}
		}
	}
	else
	{
		// printf("width:[%d]   height:[%d]\n",imageinfo->width,imageinfo->height);
		for(y_lcd=y_start,y_bmp=0; y_lcd < LCD_HEIGHT && y_bmp < imageinfo->height; y_lcd++,y_bmp++)
		{
			for(x_lcd=x_start,x_bmp=0; x_lcd < LCD_WIDTH && x_bmp < imageinfo->width; x_lcd++,x_bmp++)
			{	
				unsigned long lcd_offset = (LCD_WIDTH*y_lcd + x_lcd) * 4; //
				unsigned long bmp_offset = (imageinfo->width*y_bmp+x_bmp) *
								imageinfo->pixel_size;  //
				
				memset(FB + lcd_offset + 3 , 0 , 1);
				
				memcpy(FB + lcd_offset + 2,
					   bmp_buffer + bmp_offset + 0, 1);//R
				memcpy(FB + lcd_offset + 1,
					   bmp_buffer + bmp_offset + 1, 1);
				memcpy(FB + lcd_offset + 0,
					   bmp_buffer + bmp_offset + 2, 1);
				// nums1++;		
			}
			// printf("nums1:[%d]\n",nums1);
			// nums++;
			// nums1=0;
		}
		// printf("nums:[%d]   nums1:[%d]\n",nums,nums1);
	}		
}



// 将jpeg文件的压缩图像数据读出，放到jpg_buffer中去等待解压
unsigned long read_image_from_file(int fd,
				   unsigned char *jpg_buffer,
				   unsigned long jpg_size)
{
	unsigned long nread = 0;
	unsigned long total = 0;

	while(jpg_size > 0)
	{
		nread = read(fd, jpg_buffer, jpg_size);
		if(nread == -1)
		{
			perror("read jpeg-file failed");
			exit(1);
		}

		jpg_size -= nread;
		jpg_buffer += nread;
		total += nread;
	}
	close(fd);

	return total;
}

int Stat(const char *filename, struct stat *file_info)
{
	int ret = stat(filename, file_info);

	if(ret == -1)
	{
		fprintf(stderr, "[%d]: stat failed: "
			"%s\n", __LINE__, strerror(errno));
		exit(1);
	}

	return ret;
}

int Open(const char *filename, int mode)
{
	int fd = open(filename, mode);
	if(fd == -1)
	{
		fprintf(stderr, "[%d]: open failed: "
			"%s\n", __LINE__, strerror(errno));
		exit(1);
	}

	return fd;
}
int nums = 0;
struct image_info *jpeg_decompress(unsigned char *jpg_buffer,unsigned long int size,int scale)
{
	/*************************/
	// 声明解压缩结构体，
	struct jpeg_decompress_struct cinfo;
	//以及错误管理结构体
	struct jpeg_error_mgr jerr;

	// 使用缺省的出错处理来初始化解压缩结构体
	// printf("*****1*****\n");
	cinfo.err = jpeg_std_error(&jerr);
	// printf("*****2*****\n");
	jpeg_create_decompress(&cinfo);
	// printf("*****3*****\n");
	// 配置该cinfo，使其从jpg_buffer中读取jpg_size个字节
	// 这些数据必须是完整的JPEG数据
	// printf("*****4*****\n");
	jpeg_mem_src(&cinfo, jpg_buffer, size);
	// printf("*****5*****\n");
	// 读取JPEG文件的头，并判断其格式是否合法
	int ret = jpeg_read_header(&cinfo, true);
	if(ret != 1)
	{
		fprintf(stderr, "[%d]: jpeg_read_header failed: "
			"%s\n", __LINE__, strerror(errno));
		exit(1);
	}
	// printf("*****6*****\n");
	cinfo.scale_num=1;
	
	// scanf("%d",&cinfo.scale_denom);	
	cinfo.scale_denom=scale;

	// 开始解压
	jpeg_start_decompress(&cinfo);
	// printf("*****7*****\n");
	struct image_info *imageinfo = malloc(sizeof(struct image_info));
	imageinfo->width = cinfo.output_width;
	imageinfo->height = cinfo.output_height;
	imageinfo->pixel_size = cinfo.output_components;//RGB = 3

	int row_stride = imageinfo->width * imageinfo->pixel_size;

	// 根据图片的尺寸大小，分配一块相应的内存bmp_buffer
	// 用来存放从jpg_buffer解压出来的图像数据
	unsigned long bmp_size;
	bmp_size = imageinfo->width *
			imageinfo->height * imageinfo->pixel_size;
	
	// printf("pixel_size:[%d]   bmp_size:[%d]\n",imageinfo->pixel_size,bmp_size);
	
	unsigned char *bmp_buffer = (unsigned char *)calloc(1, bmp_size);
	
	// 循环地将图片的每一行读出并解压到bmp_buffer中
	int line = 0;
	// printf("*****8*****\n");
	while(cinfo.output_scanline < cinfo.output_height)
	{
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer +
				(cinfo.output_scanline) * row_stride;
		jpeg_read_scanlines(&cinfo, buffer_array, 1);
	}
	// printf("*****9*****\n");
	// 解压完了，将jpeg相关的资源释放掉
 	jpeg_finish_decompress(&cinfo);
	// printf("*****10*****\n");
	jpeg_destroy_decompress(&cinfo);
	// printf("*****11*****\n");
	// free(jpg_buffer);
	
	imageinfo->bmp_buf = bmp_buffer;
	
	return imageinfo;

}



struct image_info *jpeg_file_show(char *argv,int scale)
{
	
	// 读取图片文件属性信息
	// 并根据其大小分配内存缓冲区jpg_buffer
	struct stat file_info;
	Stat(argv, &file_info);
	int fd = Open(argv, O_RDONLY);

	unsigned char *jpg_buffer;
	jpg_buffer = (unsigned char *)calloc(1, file_info.st_size);
	read_image_from_file(fd, jpg_buffer, file_info.st_size);
	// printf("2-length:%d\n",file_info.st_size);
	struct _block block;
	block.x=50;
	block.y=310;
	block.width=160;	
	block.height=120;
	struct image_info *imageinfo = jpeg_decompress(jpg_buffer,file_info.st_size,scale);
	
	close(fd);
	return imageinfo;
}






