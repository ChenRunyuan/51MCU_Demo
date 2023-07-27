#ifndef __UART_H_
#define __UART_H_

#include "reg51.h"  
#include "string.h"
#include "stdio.h"
	
#define Max_BUFF_Len  64

extern  unsigned char Rx_Count;      //串口中断接收到数据保存的缓冲区
extern  unsigned char Rx_Buff[Max_BUFF_Len];      //用于保存读命令获得的数据
extern  unsigned char Res;         // 用于标识串口接收数据包是否完成标志

void Uart_init();
void delayms(unsigned int xms);
void Send_Data(unsigned char *buf,unsigned char len);

//////////////结束/////////////////
#endif