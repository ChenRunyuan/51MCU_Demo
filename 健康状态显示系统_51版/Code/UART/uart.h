#ifndef __UART_H_
#define __UART_H_

#include "reg51.h"  
#include "string.h"
#include "stdio.h"
	
#define Max_BUFF_Len  64

extern  unsigned char Rx_Count;      //�����жϽ��յ����ݱ���Ļ�����
extern  unsigned char Rx_Buff[Max_BUFF_Len];      //���ڱ���������õ�����
extern  unsigned char Res;         // ���ڱ�ʶ���ڽ������ݰ��Ƿ���ɱ�־

void Uart_init();
void delayms(unsigned int xms);
void Send_Data(unsigned char *buf,unsigned char len);

//////////////����/////////////////
#endif