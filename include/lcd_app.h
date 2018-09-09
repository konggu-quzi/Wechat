#ifndef __LCD_APP_H
#define __LCD_APP_H

// ��ʾ ���� ���� �� �ַ� ���� ���� ͼ���
#include <sys/mman.h>
#include <stdio.h>
#include <linux/fb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <wchar.h>
#include <string.h>
#include <sys/ioctl.h>
#include "ascii.h"

#include <ft2build.h>
#include FT_FREETYPE_H


extern int enc_utf8_to_wstrode_one(const char *utf_8, wchar_t *utf_16,char utf8_length);

extern void str_to_wstr(unsigned char *str, wchar_t **wstr);

//LCD��ʼ��
extern void lcd_init(void);
//����LCD��ʼ��
extern void lcd_uninit(void);

//����Ļ��ɺ�ɫ
extern void lcd_clear_display(unsigned int c);
//LCD��㺯��
extern void show_put_pixel(int x ,int y , unsigned int color);

//��ʾ��״����
extern void lcd_put_block(int x1, int y1 , int x2, int y2 , unsigned int color);

//��Բ����
void lcd_put_circle(int center_x,int center_y,int angle_end,int radius,int radius_vary,unsigned int color);

//Ŀ�����ص���ʾ
extern void draw_bitmap(int font_color,int bg_color,int flag, FT_Bitmap*  bitmap, FT_Int  x, FT_Int y,int start_x,int start_y,int size,char mode);


/*******************************************
����˵������ʾ�����ַ�
wchar_t *wtext:����ĺ��ֻ����ַ�
int size	  �������С     
int color	  ��������ɫ
int flag	  �����޵�ɫ��0��ʾ��ɫ 1 �޵�ɫ
int start_x   ����ʾ���������X��
int start_y   ����ʾ���������y��

********************************************/
extern void show_hanzi(wchar_t *wtext,int size,int font_color,int bg_color,int flag,int start_x,int start_y,char mode);

/****************************************
����˵��������λ����ʾһ��С��800*480 ��24λbmpͼƬ
const char *pathname��ͼƬ·��
int start_x��ͼƬ��ʼ����X��
int start_y��ͼƬ��ʼ����Y��
***************************************/
void show_bmp(const char *pathname,int start_x,int start_y);


#endif