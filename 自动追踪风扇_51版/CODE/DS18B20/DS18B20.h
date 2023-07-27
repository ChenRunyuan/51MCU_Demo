#ifndef _DS18B20_H
#define _DS18B20_H

#include "reg51.h"  

#define uchar unsigned char
#define uint  unsigned int
	
#define OW_SKIP_ROM 0xcc
#define DS18B20_CONVERT 0x44
#define DS18B20_READ 0xbe

extern uchar Temperature;
extern uchar TH;
extern uchar TL;
//IC引脚定义
sbit DQ = P2^7;

//函数声明
void Delay_OneWire(unsigned int t);    
void Write_DS18B20(unsigned char dat);
void Init_DS18B20(void);
unsigned char Read_DS18B20(void);
extern uchar readtemp();

#endif
