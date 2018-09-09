
#include "lcd_app.h"
#include <locale.h>
#include <assert.h>

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
 	wchar_t *wstr_head = *wstr =(wchar_t *)malloc(wstr_length*4);
	
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

int main(void)
{
	wchar_t *wnums = L"我是";
	// wchar_t *wnums = L"1234567890";
	// wchar_t *wchar1 = L"我";

	wchar_t *wchar;
	char ch_buf[500]={0};
	fgets(ch_buf,500,stdin);
	// char *ch_buf="我是";
	printf("-----------------------------------\n");
	
	// enc_utf8_to_wstrode_one(ch_buf, &wchar);
	
	// printf("*  %x %x %x\n",ch_buf[0],ch_buf[1],ch_buf[2]);
	// printf("1  %x  \n\n",wnums[0]);//,wnums[1],wnums[2]);
	// printf("*  %x %x %x\n",ch_buf[3],ch_buf[4],ch_buf[5]);
	printf("1  %x  %x\n",wnums[0],wnums[1]);//,wchar[1],wchar[2]);
	// char *p = (char *)wnums;
	// printf("1  %p  %p\n",p,p+1);
	// printf("1  %x  %x\n",p[0],p[1]);
	str_to_wstr(ch_buf,&wchar);
	printf("2  %x  %x\n",wchar[0],wchar[1]);//,wchar[1],wchar[2]);
	// return 0;
	//,wchar[1],wchar[2]);
	
	lcd_init();
	printf("-----------------------------------\n");
	lcd_put_block(0 , 0 , 800 , 480 , 0xEBEBEB , 1);
	// lcd_put_block(100-16 , 50-16 , 100+14*25+16 , 50+25+16 , 0xA0E759 , 1);

	int size=25;
	// scanf("%d",&size);
	show_hanzi(wchar,size,0,0,1,0,50,1);//0xEBEBEB

	// show_hanzi(wchar1,50,0xff111f, 0,0,50);
	// show_hanzi(wchar2,50,0xff111f, 0,25,100);
	// show_hanzi(wchar3,50,0xff111f, 0,25,150);
	
	// int i,j;
	// for(i = 0;i < 64; i++)
	// {
		// for(j = 32;j < 64; j++)
			// show_put_pixel(i , j ,  0xf1111f);  //显示底色

	// }
	
	
	lcd_uninit();
	return 0;
}