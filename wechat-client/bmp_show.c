#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

int *bmp_transition(int *bmp_buf,int row_length,int column_length,const char *mode);

int main(int argc, char const *argv[])
{
	int lcd;
	FILE *pic;
	int ret;
	int i,j,columnl,row;
	char bmp_buf[800*480*3];
	int lcd_buf[800*480];
	int show_buf[800*480];
	int *lcd_pointer = lcd_buf;

	
	//1.打开bmp文件，lcd文件
	lcd = open("/dev/fb0",O_RDWR);
	if(lcd < 0)
	{
		printf("open lcd error!\n");
	}
	
	pic = fopen("1.bmp", "r");//r:读取
	if(pic == NULL)
	{
		printf("fopen pic error!\n");
	}


	//2.偏移54个字节
	ret = fseek(pic,54,SEEK_SET);
	if(ret == -1)
	{
		printf("fseek error!\n");
	}
	
	//3.读取文件内容
	ret = fread(bmp_buf,sizeof(bmp_buf),1,pic);
	if(ret != 1)
	{
		printf("fread pic error!\n");
	}
	
	//4.转化RGB——>ARGB
	for(i=0,j=0;i<800*480;i++,j+=3)
	{
		lcd_buf[i] = (bmp_buf[j]<<0)|(bmp_buf[j+1]<<8)|(bmp_buf[j+2]<<16);
	}
/**/	
	//5.镜像翻转
	// for(row=0;row<480;row++)
	// {
	// 	for(columnl=0;columnl<800;columnl++)
	// 	{
	// 		show_buf[row*800+799-columnl] = lcd_buf[row*800+columnl];
	// 	}
	// }

	// for(row=0;row<480;row++)
	// {
	// 	for(columnl=0;columnl<800;columnl++)
	// 	{
	// 		lcd_buf[(479-row)*800+columnl] = show_buf[row*800+columnl];
	// 	}
	// }
	
	lcd_pointer = bmp_transition(lcd_pointer,480,800,argv[1]);  
	// lcd_pointer = bmp_transition(lcd_pointer,480,800,1);
	//6.写入数据
	ret = write(lcd,lcd_pointer,800*480*4);
	if(ret != sizeof(lcd_buf))
	{
		printf("write lcd error!  %d  %d \n",ret, sizeof(lcd_buf));
	}
	
	//7.关闭文件
	fclose(pic);
	close(lcd);
	return 0;
}
int *bmp_transition(int *bmp_buf,int row_length,int column_length,const char *mode)
{
	/*mode:
		'1'：上下翻转；
		'2'：镜像翻转
		'3'：旋转180° */
	int row,columnl;
	int new_buf[ row_length*column_length ];
	int *p = NULL;

	switch(*mode)
	{
		case '1':
			for(row = 0; row < row_length; row++)
			{
				for(columnl = 0; columnl < column_length; columnl++)
				{
					new_buf [ ( row_length - row - 1 )*column_length + columnl ] 
					= bmp_buf [ row*column_length + columnl ];
				}
			}
			p= new_buf;
			break;
		case '2':
			for(row=0;row<row_length;row++)
			{
				for(columnl = 0; columnl < column_length; columnl++)
				{
					new_buf [ row*column_length + column_length - columnl - 1 ]
					= bmp_buf [ row*column_length + columnl ];
				}
			}
			p= new_buf;
			break;
		case '3':
			for(row=0;row<row_length;row++)
			{
				for(columnl = 0; columnl < column_length; columnl++)
				{
					new_buf [ row*column_length + column_length - columnl - 1 ]
					= bmp_buf [ row*column_length + columnl ];
				}
			}
			for(row = 0; row < row_length; row++)
			{
				for(columnl = 0; columnl < column_length; columnl++)
				{
					bmp_buf [ ( row_length - row - 1 )*column_length + columnl ] 
					= new_buf [ row*column_length + columnl ];
				}
			}
			p= bmp_buf;
			break;
			break;
		default:
			p = bmp_buf;
	}
	return  p;

}