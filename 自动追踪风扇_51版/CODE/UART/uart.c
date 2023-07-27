#include "uart.h"
	
void delay(uint ms)
{
   uint i,j;
   for(i=0;i<ms;i++)
   for(j=0;j<120;j++);
}

void Uart_init()
{  
	TMOD= 0X21;		//T1  方式2  ，8位   自动重装   
	TH1=0Xfd;
	TL1=0Xfd;	    //9600
	TR1=1; 		    // 定时器1启动  
	SM0=0;	    // 设置串口的工作模式
	SM1=1; 	    //方式1
	REN=1; 		 // 允许串口接收数据 	 
	ES=1; 	     // 串口中断应许 
  TH0=0x4c;
	TL0=0x00;
	ET0=1;
	TR0 = 1;        //启动定时器
	EA=1;		 // 开启中断	
}

/****************************************************************************************************
 * 函数名称： void Send_Data(uchar *buf,uchar len)
 * 入口参数：uchar *buf uchar len
 * 返回  值：无
 * 功能说明：串口发送数据
 * 			 buf:发送区首地址
 *			 len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 ***************************************************************************************************/
void Send_Data(uchar *buf,uchar len)
{
	uchar t;
	delay(1);
	for(t=0;t<len;t++)		//循环发送数据
	{		
    SBUF=buf[t];	
  	while(!TI);
	  TI=0;		
	}	 
	delay(1);
	USART_RX_CNT=0;	  
}




