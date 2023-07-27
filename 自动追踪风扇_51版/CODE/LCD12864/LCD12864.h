#ifndef __LCD12864_H__
#define __LCD12864_H__

#include "reg51.h"

#define LCDRSsp_Off   0x3e
#define LCDRSsp_On    0x3f 
#define Page_Add      0xb8//页地址
#define LCDCol_Add    0x40//列地址
#define Start_Line    0xC0//行地址

/*****液晶显示器的端口定义*****/
#define data_ora P0 /*液晶数据总线*/
sbit CS1=P2^4 ;    /*片选1*/
sbit CS2=P2^3 ;    /*片选2*/
sbit RS=P2^2 ;     /*数据/指令 选择*/
sbit RW=P2^1 ;     /*读/写 选择*/
sbit EN=P2^0 ;     /*读/写 使能*/

void LCDdelay(unsigned int t);
void CheckState();
void init_lcd();
void Clr_Scr();
void write_com(unsigned char cmdcode);
void write_data(unsigned char RSspdata);
void LCD_Write_X_Y(unsigned char x,unsigned char y);
void LCD_Write_Chinese(unsigned char x,unsigned char y, unsigned char hz,unsigned char *dat);
void LCD_Write_Num(unsigned char x,unsigned char y, unsigned char en,unsigned char *dat);
void LCD_Display_Init();
#endif

