#include<reg51.h>

#define uchar unsigned char
#define uint unsigned int 
	
void delay(uint ms)
{
        uint i,j;
        for(i=0;i<ms;i++)
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
	TH0=0x4c;    //50ms定时
	TL0=0x00;
	ET0=1;
	TR0 = 1;    //开定时器0
	EA=1;		 // 开总中断
}

 void Send_byte(unsigned char d)
{	
	SBUF=d;
	while(!TI);
	TI=0;
}

void Send_String(unsigned char *str)	
{
	while(*str)
	{
		Send_byte(*str) ;
		str++;
	}
}