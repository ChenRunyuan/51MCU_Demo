#ifndef __UART_H_
#define __UART_H_

#include "reg51.h"
#include "string.h"
#include "stdio.h"

#define uchar unsigned char
#define uint unsigned int 

static uchar USART_RX_CNT = 0;      //串口中断接收到数据保存的缓冲区
static uchar USART_RX_BUF[64];      //用于保存读命令获得的数据
static uchar From_Flag = 0;         // 用于标识串口接收数据包是否完成标志
extern uchar RS485_RX_BUF[64];  	//接收缓冲,最大64个字节.

void Uart_init();
void delay(uint ms);
void Send_Data(uchar *buf,uchar len);

//////////////结束/////////////////
#endif