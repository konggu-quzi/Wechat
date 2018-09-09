// lcd_test.c 
#include "lcd_app.h"
#include <math.h>

#define pi 3.14159


int fb_fd;
int hzk_fd;
struct stat hzk_stat;
unsigned char *hzk_mem;

unsigned char *fb_mem;
struct fb_var_screeninfo var;	/* Current var */
struct fb_fix_screeninfo fix;	/* Current fix */

int x_res, y_res;
int line_length;
int screen_size;
int pixel_length;


// freetype relattived parameters
FT_Library    library;
FT_Face       face;
FT_GlyphSlot  slot;
FT_Matrix     matrix;                 /* transformation matrix */
FT_Vector     pen;                    /* untransformed origin  */
FT_Error      error;

//
unsigned int  n ;

int enc_utf8_to_wstrode_one(const char *utf_8, wchar_t *utf_16,char utf8_length)
{
    // b1 表示UTF-8编码的str中的高字节, b2 表示次高字节, ...
    char b1, b2, b3, b4, b5, b6;
	char *output = (char *)utf_16;	
	
    switch ( utf8_length )
    {
        case 1:
            *output     = *utf_8;
            break;
        case 2:
            b1 = *utf_8;
            b2 = *(utf_8 + 1);
            if ( (b2 & 0xE0) != 0x80 )
                return 0;
            *output     = (b1 << 6) + (b2 & 0x3F);
            *(output+1) = (b1 >> 2) & 0x07;
            break;
        case 3:
            b1 = *utf_8;
            b2 = *(utf_8 + 1);
            b3 = *(utf_8 + 2);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )
                return 0;
            *output     = (b2 << 6) + (b3 & 0x3F);
            *(output+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);
			// printf("%x  %x\n",output[0],output[1]);
			// printf("%p  %p\n",output,output+1);
			// printf("[%x]\n",utf_16[0]);
			// printf("2--p--  %p\n",utf_16);
			// (wchar_t *)output;
            break;
        case 4:
            b1 = *utf_8;
            b2 = *(utf_8 + 1);
            b3 = *(utf_8 + 2);
            b4 = *(utf_8 + 3);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) )
                return 0;
            *output     = (b3 << 6) + (b4 & 0x3F);
            *(output+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
            *(output+2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);
            break;
        case 5:
            b1 = *utf_8;
            b2 = *(utf_8 + 1);
            b3 = *(utf_8 + 2);
            b4 = *(utf_8 + 3);
            b5 = *(utf_8 + 4);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )
                return 0;
            *output     = (b4 << 6) + (b5 & 0x3F);
            *(output+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);
            *(output+2) = (b2 << 2) + ((b3 >> 4) & 0x03);
            *(output+3) = (b1 << 6);
            break;
        case 6:
            b1 = *utf_8;
            b2 = *(utf_8 + 1);
            b3 = *(utf_8 + 2);
            b4 = *(utf_8 + 3);
            b5 = *(utf_8 + 4);
            b6 = *(utf_8 + 5);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)
                    || ((b6 & 0xC0) != 0x80) )
                return 0;
            *output     = (b5 << 6) + (b6 & 0x3F);
            *(output+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);
            *(output+2) = (b3 << 2) + ((b4 >> 4) & 0x03);
            *(output+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
            break;
        default:
            return 0;
            break;
    }
}


void str_to_wstr(unsigned char *str, wchar_t **wstr)
{
	unsigned long int str_length = strlen(str);
    unsigned long int wstr_length = 0;
    
 	printf("str_length:[%d]\n", str_length);
	if(str[str_length-1] == '\n')
		str[str_length-1] = '\0';
 	unsigned long int i = 0;
 	while(i < str_length)
 	{
 		wstr_length++;
 		if(str[i] > 0x7f)
 		{
 			if(str[i] > 0xdf)
 			{
 				if(str[i] > 0xef)
	 			{
	 				if(str[i] > 0xf7)
		 			{
		 				if(str[i] > 0xfb)
			 			{
			 				if(str[i] > 0xfd)
				 			{
				 				//????
				 			}
				 			else
				 				i+=6;
			 			}
			 			else
			 				i+=5;
		 			}
		 			else	
		 				i+=4;
	 			}
 				else
	 				i+=3;
 			}
 			else
				i+=2;
 		}
 		else
 			i++;
 	}
 	printf("wstr_length:[%ld]\n", wstr_length);
 	wchar_t *wstr_head = *wstr =(wchar_t *)calloc(1,wstr_length*4);
	
	// printf("--head--  %p\n",*wstr);
	i=0;
	while(i < str_length)
 	{
 		if(str[i] > 0x7f)
 		{
 			if(str[i] > 0xdf)
 			{
 				if(str[i] > 0xef)
	 			{
	 				if(str[i] > 0xf7)
		 			{
		 				if(str[i] > 0xfb)
			 			{
			 				if(str[i] > 0xfd)
				 			{
				 				//????
				 			}
				 			else
							{
								enc_utf8_to_wstrode_one(str+i,*wstr,6);
								i+=6;
							}
			 			}
			 			else
						{
							enc_utf8_to_wstrode_one(str+i,*wstr,5);
							i+=5;
						}
		 			}
		 			else	
					{
						enc_utf8_to_wstrode_one(str+i,*wstr,4);
						i+=4;
					}
	 			}
 				else
				{
					enc_utf8_to_wstrode_one(str+i,*wstr,3);
					i+=3;
				}
 			}
 			else
			{
				enc_utf8_to_wstrode_one(str+i,*wstr,2);
				i+=2;
			}	
 		}
 		else
		{
			enc_utf8_to_wstrode_one(str+i,*wstr,1);
			i++;
		}
		// printf("1--head--  %p\n",*wstr);
		(*wstr)++;
		// printf("2--head--  %p\n",*wstr);
 	}
	
	*wstr = wstr_head;
}


void lcd_init(void)
{
	int ret;
	/* 打开 framebuffer */
	fb_fd = open("/dev/fb0" , O_RDWR);
	if(fb_fd < 0)
	{
		printf("neo: cannot open the fb device\n");
		return ;
	}

	/*获得固定参数 和 变化参数 */
	ret = ioctl(fb_fd , FBIOGET_VSCREENINFO , &var);
	if(ret)
	{
		printf("neo: get FBIOGET_VSCREENINFO args error");
		return  ;
	}
	
	ret = ioctl(fb_fd , FBIOGET_FSCREENINFO , &fix);
	if(ret)
	{
		printf("neo: get FBIOGET_FSCREENINFO args error");
		return  ;
	}

	line_length	 =  fix.line_length;
	screen_size  =  fix.smem_len;
	pixel_length =  var.bits_per_pixel / 8;
	x_res = var.xres;
	y_res = var.yres;
	
	// printf("fix.line_length 	= %d\n",fix.line_length);
	// printf("fix.smem_len 		= %d\n",fix.smem_len);
	// printf("var.bits_per_pixel 	= %d\n",var.bits_per_pixel);
	// printf("var.xres = %d\n",var.xres);
	// printf("var.yres = %d\n",var.yres);
	// printf("screen_size = %d\n",screen_size); //3072000  800*960
	/* 映射 framebuffer 地址 */
    fb_mem =  (unsigned char*)mmap(NULL, 800*480*4,PROT_READ | PROT_WRITE,MAP_SHARED,fb_fd, 0);
	if(fb_mem == MAP_FAILED)
	{
		printf("mmap failure!\n");
		return;
	}	
	// memset(fb_mem , 0xffffff , screen_size);
}

void lcd_uninit(void)
{
	close(fb_fd);

	munmap(fb_mem,screen_size);
}
 

void lcd_clear_display(unsigned int c)
{
	memset(fb_mem , 0 , screen_size);
}

void show_put_pixel(int x ,int y , unsigned int color)
{
	unsigned int *pen_32 = (unsigned int *)(fb_mem + y*line_length + x*pixel_length);
	if(var.bits_per_pixel != 32)
	{
		printf(" sorry ! only support 32 bit\n");
		return ;
	}
	*pen_32 = color ;
}


void lcd_put_block(int x1, int y1 , int x2, int y2 , unsigned int color)
{
	int i,j;
	for ( i = x1; i < x2; i++)
	{
		for ( j = y1; j < y2; j++)
			show_put_pixel(i , j,  color);
	}
	
}


void lcd_put_circle(int center_x,int center_y,int angle_end,int radius,int radius_vary,unsigned int color)
{
	float angle;
	int x,y,v;
	for ( angle = 0; angle < angle_end; angle+=0.5)
	{
		for(v = 0; v < radius_vary ; v++)
		{			
			x = center_x-(radius + v) * sinf(angle*pi/180);
			y = center_y+(radius + v) * cosf(angle*pi/180);
			show_put_pixel(x,y,color);
			// show_put_pixel(x+1,y+1,color);
			// show_put_pixel(x-1,y-1,color);
			// show_put_pixel(x-1,y,color);
			// show_put_pixel(x,y-1,color);
			// show_put_pixel(x+1,y,color);
			// show_put_pixel(x,y+1,color);
		}				
	}
	
}

void draw_bitmap(int font_color,int bg_color,int flag, FT_Bitmap*  bitmap, FT_Int  x, FT_Int y,int start_x,int start_y,int size,char mode)
{
	FT_Int  i, j, p, q;
	// printf("x:[%u]    y:[%u]\n",x,y);
	// printf("width:[%u]   rows:[%u]\n",bitmap->width, bitmap->rows);
	FT_Int 	x_end,y_end,char_x_end,char_y_end,char_x_start,char_y_start,x_start,y_start;
	if(mode == 2)
	{			
		x_end = start_x + ((x-start_x)/(size/2) + 1)*size;
		y_end = size + start_y;
		
		x_start = x_end-size;
		y_start = start_y;
		
		char_x_end = x_end - size + size/4 + bitmap->width;
		char_y_end = y + bitmap->rows;
		
		char_x_start = char_x_end - bitmap->width;
		char_y_start = char_y_end - bitmap->rows;
	}
	else
	{
		x_start = x;
		y_start = y;
		
		x_end = x + bitmap->width;
		y_end = y + bitmap->rows;
		
		char_x_end = x + bitmap->width;
		char_y_end = y + bitmap->rows;
		
		char_x_start = char_x_end - bitmap->width;
		char_y_start = char_y_end - bitmap->rows;
	}
	
	// printf("x_end:[%u]   ",x_end);
	// printf("y_end:[%u] \n",y_end);
	int nums=0;
	for ( i = x_start, p = 0; i < x_end; i++)
	{
		if(i > char_x_start) p++;
		for ( j = y_start, q = 0; j < y_end; j++)
		{
			if(j > char_y_start) q++;
			if ( i < 0 || j < 0 || i >=x_res || j >= x_res )
				continue;	
			
			if(bitmap->buffer[q * bitmap->width + p]   && 
									  i < char_x_end   && 
									  j < char_y_end   &&
									  i > char_x_start &&
									  j > char_y_start  )
			{
				show_put_pixel(i , j,  font_color);
				
			}			
			else
			{	
				nums++;
				// printf("%d     %d\n",i,j);
				if(flag == 0)
					show_put_pixel(i , j ,  bg_color);  //显示底色
				else
				{
					//空，则无底色
				}
			}
			//     image[j][i] |= bitmap->buffer[q * bitmap->width + p];
		}
	}
	
	// printf("nums:[%u] \n",nums);
}


//flag:1 不显示底色
void show_hanzi(wchar_t *wtext,int size,int font_color,int bg_color,int flag,int start_x,int start_y,char mode)
{
	int ret;
	/* 显示汉字  中国*/	
	hzk_fd  = open("HZK16" , O_RDWR); // 打开汉字库
	ret = fstat(hzk_fd, &hzk_stat); // 获得汉字库大小
	if(ret)
	{
		printf("can't  open the hanziku\n");
	}
	//hzk_mem =  (unsigned char*)mmap(NULL, hzk_stat.st_size,PROT_READ | PROT_WRITE,MAP_SHARED,hzk_fd, 0);

	//printf("打印出 中国的 GBk code \n");
	//printf("GBK code :  %x , %x  , %x ,%x \n" , str1[0] , str1[1] ,str2[0] , str2[1] );
	//printf("GBK code :  %x , %x  , %x ,%x \n" , str3[0] , str3[1] ,str4[0] , str4[1] );	
	//lcd_put_gbk(250 ,200 , str1); // 此时中字采用 gbk码 保存的 所以为 d6 d0
	//lcd_put_gbk(500 ,200 , str2);

	//while(1);
	
	/*显示宋体中国*/
	error = FT_Init_FreeType( &library );              /* initialize library */

	error = FT_New_Face( library, "./1.ttf", 0, &face ); /* create face object    这个打开字体文件 */
	slot = face->glyph;

	error = FT_Set_Pixel_Sizes(face, size, 0);    /* set font size */

	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	pen.x = start_x * 64;
	pen.y = ( y_res - start_y - size) * 64;

	for ( n = 0; n < wcslen( wtext ); n++ )
	{
		/* set transformation */
		FT_Set_Transform( face, 0, &pen );
		// if(wtext[n])
		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Char( face, wtext[n], FT_LOAD_RENDER );
		if ( error )
		  continue;                 /* ignore errors */

		/* now, draw to our target surface (convert position) */
		
		draw_bitmap(font_color,bg_color,flag, &slot->bitmap,
		             slot->bitmap_left,
		             y_res - slot->bitmap_top -2,start_x,start_y,size,mode);

		/* increment pen position */
		pen.x += slot->advance.x;
	}	
	
	close(hzk_fd);
}


/* void draw_bitmap(int font_color,int bg_color,int flag, FT_Bitmap*  bitmap, FT_Int  x, FT_Int y,int start_x,int start_y,int size,char mode)
{
	FT_Int  i, j, p, q;
	printf("x:[%u]    y:[%u]\n",x,y);
	printf("width:[%u]   rows:[%u]\n",bitmap->width, bitmap->rows);
	FT_Int 	x_end,y_end,char_x_end,char_y_end,char_x_start,char_y_start;
	if(mode == 2)
	{			
		x_end = start_x + ((x-start_x)/(size/2) + 1)*size;
		y_end = size + start_y;
		
		char_x_end = x_end - size + size/4 + bitmap->width;
		char_y_end = y + bitmap->rows;
		
		char_x_start = char_x_end - bitmap->width;
		char_y_start = char_y_end - bitmap->rows;
	}
	else
	{
		x_end = start_x + ((x-start_x)/size + 1)*size;
		y_end = size + start_y;
		
		char_x_end = x + bitmap->width;
		char_y_end = y + bitmap->rows;
		
		char_x_start = char_x_end - bitmap->width;
		char_y_start = char_y_end - bitmap->rows;
	}
	
	printf("x_end:[%u]   ",x_end);
	printf("y_end:[%u] \n",y_end);
	int nums=0;
	for ( i = x_end-size, p = 0; i < x_end; i++)
	{
		if(i > char_x_start) p++;
		for ( j = start_y, q = 0; j < y_end; j++)
		{
			if(j > char_y_start) q++;
			if ( i < 0 || j < 0 || i >=x_res || j >= x_res )
				continue;	
			
			if(bitmap->buffer[q * bitmap->width + p]   && 
									  i < char_x_end   && 
									  j < char_y_end   &&
									  i > char_x_start &&
									  j > char_y_start  )
			{
				show_put_pixel(i , j,  font_color);
				
			}			
			else
			{	
				nums++;
				// printf("%d     %d\n",i,j);
				if(flag == 0)
					show_put_pixel(i , j ,  bg_color);  //显示底色
				else
				{
					//空，则无底色
				}
			}
			//     image[j][i] |= bitmap->buffer[q * bitmap->width + p];
		}
	}
	
	printf("nums:[%u] \n",nums);
} */






