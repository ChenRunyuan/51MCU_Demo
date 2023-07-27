#include "uart.h"

unsigned char Rx_Count = 0;      //串口中断接收到数据保存的缓冲区
unsigned char Rx_Buff[64];      //用于保存读命令获得的数据
unsigned char Res;              //用于保存接收的实时数据

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
	TL1=0Xfd;	    //设置波特率9600
	TR1=1; 		    // 开中断1 
	SM0=0;	    
	SM1=1; 	    
	REN=1; 		  	 
	ES=1; 	     
	TH0=0x28;    //5ms定时
	TL0=0x00;
	ET0=1;
	TR0 = 1;    //开定时器0
	EA=1;		// 开总中断
}

/****************************************************************************************************
 * 函数名称： void Send_Data(uchar *buf,uchar len)
 * 入口参数：uchar *buf uchar len
 * 返回  值：无
 * 功能说明：串口发送数据
 * 			 buf:发送区首地址
 *			 len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 ***************************************************************************************************/
void Send_Data(unsigned char *buf,unsigned char len)
{
	unsigned char t;
	delayms(1);
	for(t=0;t<len;t++)		//循环发送数据
	{		
    SBUF=buf[t];	     //要发送的数据
  	while(!TI);    
	  TI=0;		
	}	 
	delayms(1);
	Rx_Count=0;	  //发送完成后重接接收数据
}

