#include "LCD12864.h" //LCD12864配置
#include "uart.h"    //串口配置
#include "Fount.h"   //字库

//#define DeBug       //调试串口时需要打开这行测序  打开后才能运行下面的Send_Data("Usart Test OK\r\n",15);
#define Strstr       //使用strstr命令匹配数据

sbit LED1=P1^0;  //轻度
sbit LED2=P1^1;  //清醒
sbit LED3=P1^2;  //重度

sbit BEEP=P2^0;  //报警

void main()
{
	init_lcd();
	Clr_Scr();
	Uart_init();
	
	LCD_Write_Chinese(0,16,0,Chinese); //健
	LCD_Write_Chinese(0,32,1,Chinese); //康
	LCD_Write_Chinese(0,48,2,Chinese); //检
	LCD_Write_Chinese(0,64,3,Chinese); //测
  LCD_Write_Chinese(0,80,6,Chinese); //系
  LCD_Write_Chinese(0,96,7,Chinese); //统
	
	LCD_Write_Chinese(4,0,15,Chinese); //状
	LCD_Write_Chinese(4,16,16,Chinese);//态	
	LCD_Write_Chinese(4,32,17,Chinese);//：

  while(1)
	{
	  
#ifdef Strstr
		
	    if(strstr((char *)Rx_Buff,"QX"))        //清醒
     {	
		 LED1=0;LED2=1;LED3=1; memset(Rx_Buff,0,sizeof(Rx_Buff));Send_Data("QX\r\n",4);//更新上位机清醒LED状态
     LCD_Write_Chinese(4,64,20,Chinese);LCD_Write_Chinese(4,48,8,Chinese);	//显示"清  醒"
     LCD_Write_Chinese(4,80,9,Chinese);	LCD_Write_Chinese(4,96,20,Chinese);				 
     }
	   else if(strstr((char *)Rx_Buff,"QD"))    //轻度
    {	
		 LED1=1;LED2=0;LED3=1; memset(Rx_Buff,0,sizeof(Rx_Buff));Send_Data("QD\r\n",4);//更新上位机轻度LED状态
     LCD_Write_Chinese(4,48,10,Chinese);LCD_Write_Chinese(4,64,12,Chinese);	       //显示"轻度疲劳"
     LCD_Write_Chinese(4,80,18,Chinese);	 LCD_Write_Chinese(4,96,19,Chinese);	 			
    }
	   else if(strstr((char *)Rx_Buff,"ZD"))    //重度
   {  	
		 LED1=1;LED2=1;LED3=0; memset(Rx_Buff,0,sizeof(Rx_Buff));Send_Data("ZD\r\n",4);//更新上位机重度LED状态
     LCD_Write_Chinese(4,48,11,Chinese);LCD_Write_Chinese(4,64,12,Chinese);       //显示"重度疲劳"
     LCD_Write_Chinese(4,80,18,Chinese);	 LCD_Write_Chinese(4,96,19,Chinese);			 
   }
#endif
	 
#ifdef DeBug
	   Send_Data("Usart Test OK\r\n",15);
	   delayms(500);
#endif
	  
	}
}

void Timer0() interrupt 1 using 1
{
  TH0=0x28;    //5ms定时
  TL0=0x00;
	if(!LED3) BEEP=0; else BEEP=1;  //检测到重度疲劳报警
}

void Usart() interrupt 4   //串口中断
{
   if(RI)
  {
	  RI=0;                   //接收中断清零
		Res=SBUF;               //将接收的数据暂存到Rec
		
		Rx_Buff[Rx_Count] = Res;		// 把数据放到接收缓冲区
			
	 	if(Rx_Count<Max_BUFF_Len) 	// 接收未完成
		{						 
			 if(Res==0x0a) {Rx_Count=0;memset(Rx_Buff,0,sizeof(Rx_Buff));}  //接收到回车符结束一轮接收 准备接收下个数据
			 else Rx_Count++;
		}
		else Rx_Count=0;  //超过最大接收字节数则清零

  }
}