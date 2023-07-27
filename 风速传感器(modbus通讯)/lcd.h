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
   busy=1;      //��æ��־λ��1
   lcd1602_en=0;       //��ʹ�ܶ���0
   lcd1602_rs=0;      //��rs����0��Ϊָ�����
   lcd1602_rw=1;      //Ϊ������
   lcd1602_en=1;       //����ʹ�ܶ�
   _nop_();      //�ȴ�
   while(busy==1);    //���æ��־λ��
   lcd1602_en=0;       //æ��־λΪ0�����ʹ������
}

//--------------------------дָ��---------------------------
void write_1602com(uchar com)//****Һ��д��ָ���****
{
	  busy_lcd();      //æ��־���
  lcd1602_rs=0;        //ָ�����
  lcd1602_rw=0;        //д�����
  lcd1602_en=1;        //����ʹ�ܶ�
  P0=com;      //��������
  _nop_();       //�ȴ�
  lcd1602_en=0;        //����ʹ�ܶˣ�����д��Һ��
}

//-------------------------д����-----------------------------
void write_1602dat(uchar dat)//***Һ��д�����ݺ���****
{
  busy_lcd();      //æ��־���
  lcd1602_rs=1;        //���ݲ���
  lcd1602_rw=0;        //д�����
  lcd1602_en=1;        //����ʹ�ܶ�
  P0=dat;      //��������
  _nop_();       //�ȴ�
  lcd1602_en=0;        //����ʹ�ܶˣ�����д��Һ��
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
//-------------------------��ʼ��-------------------------
void lcd_init(void)
{
	write_1602com(0x38);//����Һ������ģʽ����˼��16*2����ʾ��5*7����8λ����
	write_1602com(0x0c);//����ʾ����ʾ���
	write_1602com(0x06);//�������ƶ�������Զ�����
	write_1602com(0x01);//����ʾ
	write_string(0x80," Welcome  User  ",0);
	write_string(0xc0,"000.0 M/s        ",0);
 
}