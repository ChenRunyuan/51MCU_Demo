#include "intrins.h"
//sbit lcd1602_rs=P2^6;
//sbit lcd1602_rw=P2^5;
//sbit lcd1602_en=P2^7;

sbit lcd1602_rs=P3^5;
sbit lcd1602_rw=P3^6;
sbit lcd1602_en=P3^4;
sbit busy=P0^7;
void busy_lcd(void)
{
   busy=1;      //把忙标志位置1
   lcd1602_en=0;       //把使能端置0
   lcd1602_rs=0;      //把rs端置0，为指令操作
   lcd1602_rw=1;      //为读操作
   lcd1602_en=1;       //拉高使能端
   _nop_();      //等待
   while(busy==1);    //检测忙标志位，
   lcd1602_en=0;       //忙标志位为0，则把使能拉低
}

//--------------------------写指令---------------------------
void write_1602com(uchar com)//****液晶写入指令函数****
{
	  busy_lcd();      //忙标志检测
  lcd1602_rs=0;        //指令操作
  lcd1602_rw=0;        //写入操作
  lcd1602_en=1;        //拉高使能端
  P0=com;      //放上数据
  _nop_();       //等待
  lcd1602_en=0;        //拉低使能端，数据写入液晶
}

//-------------------------写数据-----------------------------
void write_1602dat(uchar dat)//***液晶写入数据函数****
{
  busy_lcd();      //忙标志检测
  lcd1602_rs=1;        //数据操作
  lcd1602_rw=0;        //写入操作
  lcd1602_en=1;        //拉高使能端
  P0=dat;      //放上数据
  _nop_();       //等待
  lcd1602_en=0;        //拉低使能端，数据写入液晶
}
void write_string(uchar dat,uchar *str,uchar dat1)
{
 if(dat)
 write_1602com(dat);
 while(*str!='\0' && (*str!='\n') && (*str!='\r'))
 {
 write_1602dat(*str);
  str++;
  if(str>dat1&&dat1!=0)
  break;
 }
}
//-------------------------初始化-------------------------
void lcd_init(void)
{
	write_1602com(0x38);//设置液晶工作模式，意思：16*2行显示，5*7点阵，8位数据
	write_1602com(0x0c);//开显示不显示光标
	write_1602com(0x06);//整屏不移动，光标自动右移
	write_1602com(0x01);//清显示
	write_string(0x80," Welcome  User  ",0);
	write_string(0xc0,"000.0 M/s        ",0);
 
}