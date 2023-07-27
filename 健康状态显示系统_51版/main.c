#include "LCD12864.h" //LCD12864����
#include "uart.h"    //��������
#include "Fount.h"   //�ֿ�

//#define DeBug       //���Դ���ʱ��Ҫ�����в���  �򿪺�������������Send_Data("Usart Test OK\r\n",15);
#define Strstr       //ʹ��strstr����ƥ������

sbit LED1=P1^0;  //���
sbit LED2=P1^1;  //����
sbit LED3=P1^2;  //�ض�

sbit BEEP=P2^0;  //����

void main()
{
	init_lcd();
	Clr_Scr();
	Uart_init();
	
	LCD_Write_Chinese(0,16,0,Chinese); //��
	LCD_Write_Chinese(0,32,1,Chinese); //��
	LCD_Write_Chinese(0,48,2,Chinese); //��
	LCD_Write_Chinese(0,64,3,Chinese); //��
  LCD_Write_Chinese(0,80,6,Chinese); //ϵ
  LCD_Write_Chinese(0,96,7,Chinese); //ͳ
	
	LCD_Write_Chinese(4,0,15,Chinese); //״
	LCD_Write_Chinese(4,16,16,Chinese);//̬	
	LCD_Write_Chinese(4,32,17,Chinese);//��

  while(1)
	{
	  
#ifdef Strstr
		
	    if(strstr((char *)Rx_Buff,"QX"))        //����
     {	
		 LED1=0;LED2=1;LED3=1; memset(Rx_Buff,0,sizeof(Rx_Buff));Send_Data("QX\r\n",4);//������λ������LED״̬
     LCD_Write_Chinese(4,64,20,Chinese);LCD_Write_Chinese(4,48,8,Chinese);	//��ʾ"��  ��"
     LCD_Write_Chinese(4,80,9,Chinese);	LCD_Write_Chinese(4,96,20,Chinese);				 
     }
	   else if(strstr((char *)Rx_Buff,"QD"))    //���
    {	
		 LED1=1;LED2=0;LED3=1; memset(Rx_Buff,0,sizeof(Rx_Buff));Send_Data("QD\r\n",4);//������λ�����LED״̬
     LCD_Write_Chinese(4,48,10,Chinese);LCD_Write_Chinese(4,64,12,Chinese);	       //��ʾ"���ƣ��"
     LCD_Write_Chinese(4,80,18,Chinese);	 LCD_Write_Chinese(4,96,19,Chinese);	 			
    }
	   else if(strstr((char *)Rx_Buff,"ZD"))    //�ض�
   {  	
		 LED1=1;LED2=1;LED3=0; memset(Rx_Buff,0,sizeof(Rx_Buff));Send_Data("ZD\r\n",4);//������λ���ض�LED״̬
     LCD_Write_Chinese(4,48,11,Chinese);LCD_Write_Chinese(4,64,12,Chinese);       //��ʾ"�ض�ƣ��"
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
  TH0=0x28;    //5ms��ʱ
  TL0=0x00;
	if(!LED3) BEEP=0; else BEEP=1;  //��⵽�ض�ƣ�ͱ���
}

void Usart() interrupt 4   //�����ж�
{
   if(RI)
  {
	  RI=0;                   //�����ж�����
		Res=SBUF;               //�����յ������ݴ浽Rec
		
		Rx_Buff[Rx_Count] = Res;		// �����ݷŵ����ջ�����
			
	 	if(Rx_Count<Max_BUFF_Len) 	// ����δ���
		{						 
			 if(Res==0x0a) {Rx_Count=0;memset(Rx_Buff,0,sizeof(Rx_Buff));}  //���յ��س�������һ�ֽ��� ׼�������¸�����
			 else Rx_Count++;
		}
		else Rx_Count=0;  //�����������ֽ���������

  }
}