#ifndef __UART_H_
#define __UART_H_

#include "reg51.h"
#include "string.h"
#include "stdio.h"

#define uchar unsigned char
#define uint unsigned int 

static uchar USART_RX_CNT = 0;      //�����жϽ��յ����ݱ���Ļ�����
static uchar USART_RX_BUF[64];      //���ڱ���������õ�����
static uchar From_Flag = 0;         // ���ڱ�ʶ���ڽ������ݰ��Ƿ���ɱ�־
extern uchar RS485_RX_BUF[64];  	//���ջ���,���64���ֽ�.

void Uart_init();
void delay(uint ms);
void Send_Data(uchar *buf,uchar len);

//////////////����/////////////////
#endif