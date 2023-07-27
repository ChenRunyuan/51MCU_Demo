#include "uart.h"

unsigned char Rx_Count = 0;      //�����жϽ��յ����ݱ���Ļ�����
unsigned char Rx_Buff[64];      //���ڱ���������õ�����
unsigned char Res;              //���ڱ�����յ�ʵʱ����

void delayms(unsigned int xms)
{
   unsigned int i,j;
   for(i=0;i<xms;i++)
   for(j=0;j<120;j++);
}

void Uart_init()
{  
	TMOD|= 0X21;		   
	TH1=0Xfd;
	TL1=0Xfd;	    //���ò�����9600
	TR1=1; 		    // ���ж�1 
	SM0=0;	    
	SM1=1; 	    
	REN=1; 		  	 
	ES=1; 	     
	TH0=0x28;    //5ms��ʱ
	TL0=0x00;
	ET0=1;
	TR0 = 1;    //����ʱ��0
	EA=1;		// �����ж�
}

/****************************************************************************************************
 * �������ƣ� void Send_Data(uchar *buf,uchar len)
 * ��ڲ�����uchar *buf uchar len
 * ����  ֵ����
 * ����˵�������ڷ�������
 * 			 buf:�������׵�ַ
 *			 len:���͵��ֽ���(Ϊ�˺ͱ�����Ľ���ƥ��,���ｨ�鲻Ҫ����64���ֽ�)
 ***************************************************************************************************/
void Send_Data(unsigned char *buf,unsigned char len)
{
	unsigned char t;
	delayms(1);
	for(t=0;t<len;t++)		//ѭ����������
	{		
    SBUF=buf[t];	     //Ҫ���͵�����
  	while(!TI);    
	  TI=0;		
	}	 
	delayms(1);
	Rx_Count=0;	  //������ɺ��ؽӽ�������
}

