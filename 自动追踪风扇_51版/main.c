#include "LCD12864.h" //LCD12864Һ��
#include "DS18B20.H" //18B20�¶ȴ�����
#include "Fount.h"   //�ֿ�
#include "string.h"
#include "stdio.h"

/******�������������ݶԽ�ʱ������ */
#define AT         "AT\r\n"	
#define RST        "AT+RST\r\n"	
#define CWSAP      "AT+CWSAP?\r\n"		//STA+APģʽ
#define CIFSR      "AT+CIFSR\r\n"
#define CIPMUX     "AT+CIPMUX=1\r\n"    //IP  192.168.4.1
#define CIPSERVER     "AT+CIPSERVER=1,8080\r\n"
//----------------------------���ڱ���------------------------------------//
#define  Max_BUFF_Len 10               //���ݻ������ֵ

unsigned char Rec;                     //�������ݽ����ݴ�λ
unsigned char Rx_Buff[Max_BUFF_Len];   //��������
unsigned int  Rx_Count;                //���ݻ����ŵ�ַ
unsigned int  Count;
unsigned char Time;

unsigned char Message_length;

bit ok_flag=0;
bit AT_Mode=0;
char Message_Buf[20];  //���ڴ�������ָ��
unsigned int USART_RX_STA=0;       //����״̬���	

//------------------------------------------------------------------------//
//----------------------ʱ�䡢��ʱ���¶ȱ���-------------------------------//
uchar TH=45,TL=25,Temperature=85;
uchar Hour=21,Min=42,Sec=38;
uchar Motor_Hour=00,Motor_Min=00,Motor_Sec=10;
//------------------------------------------------------------------------//
sbit Fan=P1^0;
sbit Beep=P1^2;
unsigned int PWM=0;
unsigned int T=450;
#define Zero   0;
#define First  150;
#define Second 200;
#define Third  350;
#define Fourth 450;
unsigned char mode_flag=0; //ģʽ
unsigned char Run_flag=0;  //����
unsigned char Drive=0;    //��λ
unsigned char mode=0;     //����
//------------------------------------------------------------------------//
sbit Key_Mode= P1^4;
sbit Key_ADD = P1^5;
sbit Key_Dec = P1^6;
sbit Key_Memu= P1^7;

void Now_Time();
void Display_Data();
void Receive_AT_Data();
void Receive_Normal_Data();
void SendCmd(char* cmd, int timeOut);
void Fan_PWM(void);      //�����ٶ�
void Get_Wifi_Dat(void);    //��ȡWifi������������
void Motor_Run_Time(void);  //��������ʱ��
void Key_Scan(void) ;
void Usart();

void delay(uint ms)
{
   uint i,j;
   for(i=0;i<ms;i++)
   for(j=0;j<120;j++);
}

void Uart_init()
{  
	TMOD= 0X21;		//T1  ��ʽ2  ��8λ   �Զ���װ   
	TH1=0Xfd;
	TL1=0Xfd;	    //9600
	TR1=1; 		    // ��ʱ��1����  
	SM0=0;	    // ���ô��ڵĹ���ģʽ
	SM1=1; 	    //��ʽ1
	REN=1; 		 // �����ڽ������� 	 
	ES=1; 	     // �����ж�Ӧ�� 
  TH0=0x4c;
	TL0=0x00;
	ET0=1;
	TR0 = 1;        //������ʱ��
	EA=1;		 // �����ж�	
}

/****************************************************************************************************
 * �������ƣ� void Send_Data(uchar *buf,uchar len)
 * ��ڲ�����uchar *buf uchar len
 * ����  ֵ����
 * ����˵�������ڷ�������
 * 			 buf:�������׵�ַ
 *			 len:���͵��ֽ���(Ϊ�˺ͱ�����Ľ���ƥ��,���ｨ�鲻Ҫ����64���ֽ�)
 ***************************************************************************************************/
void Send_Data(uchar *buf,uchar len)
{
	uchar t;
	delay(1);
	for(t=0;t<len;t++)		//ѭ����������
	{		
    SBUF=buf[t];	
  	while(!TI);
	  TI=0;		
	}	 
	delay(1);
	Rx_Count=0;	  
}

void ESP8266Mode_Init()
{
	int Delay = 1000;

	unsigned char Cmd[]="+++";//�˳�͸��ģʽָ��
	AT_Mode = 1;  //���뷢ATָ��ģʽ

	Send_Data(Cmd,strlen((const char*)Cmd));//�����˳�͸��ģʽ,������ATָ��
	delay(100);				//�ȴ��˳�͸��
	
	SendCmd(AT,Delay);			//ģ����Ч�Լ��

	SendCmd(CWSAP,Delay);	
//SendCmd(CIFSR,Delay);	
	SendCmd(CIPMUX,Delay);	
	SendCmd(CIPSERVER,Delay);

	AT_Mode = 0;
}

void main()
{	
  LCD_Display_Init();
	Uart_init();
	delay(500);
	ESP8266Mode_Init();
	while(1)
  {	
	  Temperature=readtemp();
	  Display_Data();
		Get_Wifi_Dat();
		Key_Scan();
	}
}

void Timer0() interrupt 1 using 1
{
  TH0=0x4c;    //50ms��ʱ
  TL0=0x00;
  Count++;
	if(Count==20)
	{ 
		Now_Time();
	  Count=0;	
    Motor_Run_Time();            //��ȡ��ʱ����ʱ	
	}
	Fan_PWM();
}


void Usart() interrupt 4   //�����ж�
{
   if(RI)
  {
	  RI=0;                   //�����ж�����
		Rec=SBUF;               //�����յ������ݴ浽Rec
		Rx_Buff[Rx_Count]=Rec;  //�����յ������ݴ�������
/*  ����ATָ��ģʽ�� ���ݵĴ��� */
			if(AT_Mode==1)//����ATָ���� ���ݵĴ���
			{
				Receive_AT_Data();  //����ATָ��ģʽ�µ�����
			}

		/*  ����ƽ̨�� ���ݵĴ��� */
			else if(AT_Mode==0)    //����ģʽ��������
			{			 	
				Receive_Normal_Data();  //����ģʽ�´�������
			}
  }
}
	 
void LCD_Display_Init()   //��ʾ��ʼ��
{
  init_lcd();
	Clr_Scr();
//----------------��һ��--------------------//
	LCD_Write_Num(0,0,0,English);//T
	LCD_Write_Num(0,8,1,English);//H
	LCD_Write_Num(0,16,11,Num);//:
	LCD_Write_Num(0,24,TH/10,Num);
	LCD_Write_Num(0,32,TH%10,Num);
  LCD_Write_Chinese(0,40,8,HZ_Chinese); //C 	
	LCD_Write_Num(0,64,0,English);//T
	LCD_Write_Num(0,72,2,English); //L
	LCD_Write_Num(0,80,11,Num);//://:
	LCD_Write_Num(0,88,TL/10,Num);
	LCD_Write_Num(0,96,TL%10,Num);
  LCD_Write_Chinese(0,104,8,HZ_Chinese);//C
	
//----------------�ڶ���--------------------//
	LCD_Write_Chinese(2,0,2,HZ_Chinese);  //��
	LCD_Write_Chinese(2,16,3,HZ_Chinese);  //��
  LCD_Write_Num(2,32,11,Num);//:
	LCD_Write_Chinese(2,64,8,HZ_Chinese);  //�� 

//----------------������--------------------//
  LCD_Write_Chinese(4,0,4,HZ_Chinese);  //��
	LCD_Write_Chinese(4,16,5,HZ_Chinese);  //��
	LCD_Write_Num(4,32,11,Num);
	LCD_Write_Num(4,40,Hour/10,Num);LCD_Write_Num(4,48,Hour%10,Num);
	LCD_Write_Num(4,56,10,Num);
	LCD_Write_Num(4,64,Min/10,Num);LCD_Write_Num(4,72,Min%10,Num);
	LCD_Write_Num(4,80,10,Num);
	LCD_Write_Num(4,88,Sec/10,Num);LCD_Write_Num(4,96,Sec%10,Num);
	
//----------------������--------------------//
  LCD_Write_Chinese(6,0,6,HZ_Chinese);  //��
	LCD_Write_Chinese(6,16,7,HZ_Chinese);  //��
	LCD_Write_Num(6,32,11,Num);
	LCD_Write_Num(6,40,Motor_Hour/10,Num);LCD_Write_Num(6,48,Motor_Hour%10,Num);
	LCD_Write_Num(6,56,10,Num);
	LCD_Write_Num(6,64,Motor_Min/10,Num);LCD_Write_Num(6,72,Motor_Min%10,Num);
	LCD_Write_Num(6,80,10,Num);
	LCD_Write_Num(6,88,Motor_Sec/10,Num);LCD_Write_Num(6,96,Motor_Sec%10,Num);
}

void Display_Data()
{
	LCD_Write_Num(0,24,TH/10,Num);LCD_Write_Num(0,32,TH%10,Num);    //������ֵ
	LCD_Write_Num(0,88,TL/10,Num);LCD_Write_Num(0,96,TL%10,Num);    //������ֵ
	
 LCD_Write_Num(2,40,Temperature/10,Num);LCD_Write_Num(2,48,Temperature%10,Num);  //�¶�
	
	LCD_Write_Num(4,40,Hour/10,Num);LCD_Write_Num(4,48,Hour%10,Num); //ʱ
	LCD_Write_Num(4,64,Min/10,Num);LCD_Write_Num(4,72,Min%10,Num);   //��
	LCD_Write_Num(4,88,Sec/10,Num);LCD_Write_Num(4,96,Sec%10,Num);   //��
	 
	LCD_Write_Num(6,40,Motor_Hour/10,Num);LCD_Write_Num(6,48,Motor_Hour%10,Num); //��ʱ-ʱ
	LCD_Write_Num(6,64,Motor_Min/10,Num);LCD_Write_Num(6,72,Motor_Min%10,Num);   //��ʱ-��
	LCD_Write_Num(6,88,Motor_Sec/10,Num);LCD_Write_Num(6,96,Motor_Sec%10,Num);   //��ʱ-�� 
	
	LCD_Write_Num(2,108,mode,Num);
	LCD_Write_Num(4,108,mode_flag,Num);
	LCD_Write_Num(6,108,Drive,Num);
	
	
}

void Now_Time()
{
 if(Sec<59) Sec++; 
	 else {
     Sec=0;  
     if(Min<59) Min++;
   else{
     Min=0;
     if(Hour<23) Hour++;
   else  Hour=Min=Sec=0;
	  }
	}
}

void Motor_Run_Time(void)  //��������ʱ��
{
  if(PWM!=0)
	{
	  if(Motor_Sec>0) Motor_Sec--;
	  else {
      Motor_Sec=59;  
    if(Motor_Min>0) Motor_Min--;
    else{
     Motor_Min=59;
   if(Motor_Hour>0) Motor_Hour--;
    else  
		 {
		   Motor_Hour=Motor_Min=Motor_Sec=Run_flag=PWM=Beep=0; //��ʱʱ�䵽	 		  
		 }			   
	  }
  }
	}
  
}
void Receive_AT_Data()
{    //�ж�ESP8266�Ƿ񷵻�'OK'��'>'��'ready' 
		if(strstr(Rx_Buff,"OK")||strstr(Rx_Buff,)||strstr(Rx_Buff,"ready"))    
		{
			 memset(Rx_Buff, 0, sizeof(Rx_Buff));  //���Rx_Buff����
			 ok_flag=1;			                //�жϳɹ�����־λ��1
			 Rx_Count=0;				
		}
		else                                   //���������������
		{
			Rx_Count++;	
			if(Rx_Count>=150)                   //������150�����ݣ�������� �������
			{
				Rx_Count=0;
				memset(Rx_Buff, 0, sizeof(Rx_Buff));
			}
		}
}
//����ATָ��ģʽ�µ�����
void Receive_Normal_Data()
{
	  static int i;        //��̬����i
	
		if(Rx_Count==50)                   //������150�����ݣ�������� �������
		{
			Rx_Count=0;                 	
			memset(Rx_Buff, 0, sizeof(Rx_Buff));  //��յ�ǰ������������
		}

		if(Message_length>0)                   //��ʾ���Կ�ʼ��������ָ��
		{
			Message_Buf[i]=Rx_Buff[Rx_Count];   //��������ָ������
			i++;
			Message_length--;                  //��һ��ָ�ʣ��������һ,�жϲ���ָ���Ƿ�������
		}
		 
		if(Rx_Count>3&&Rx_Buff[Rx_Count-2]==0x2b&&Rx_Buff[Rx_Count-1]==0x49&&Rx_Buff[Rx_Count]>0)   
		//�����ǰ���յ������ݴ���0������������ݵ�ǰ��������Ϊ'+ I' ����ǰ���ݾ��ǲ���ָ��ĳ��ȡ�
		{
			 memset(Message_Buf, 0, sizeof(Message_Buf)); //��մ�������ָ������飬׼�������µĲ���ָ��
			
			 Message_length=Rx_Buff[Rx_Count];      //�����յ������ݴ�Ϊ����ָ��ȡ�
			 i=0;                                   //���i
		}
		Rx_Count++;                               //׼���洢��һ������ 					
}

void SendCmd(char* cmd, int timeOut)
{
    while(1)
    {
		memset(Rx_Buff, 0, sizeof(Rx_Buff)); //��������֮ǰ��������������飬�����ڴ����н��ա�
		Rx_Count=0;

		Send_Data((unsigned char *)cmd,strlen((const char *)cmd));   //�ô��ڰ�cmd����д��ESP8266

		delay(timeOut);                                          //�ӳٵȴ�

		if(ok_flag==1)	//�����жϺ����У������ж��Ƿ񷵻�"OK"��">"��������������ok_flag����1��������ǰATָ������ã���ʼ������һ��
		{  
			ok_flag=0;   //��ձ�־
			break;
		}
		else
		{
			delay(100);		
		}
    }
}



void Fan_PWM(void)      //�����ٶ�
{
	static unsigned char countFan =0;
	countFan++;
	if(countFan<PWM)	   // ռ�ձȵ���
	{
		Fan=0;			  //��
	}
	else if(countFan<=T)	//�ر�ʱ���//**All notes can be deleted and modified**//
	{
	 	Fan=1;			//�ر�
		if(countFan == T)	 countFan=0;  //һ�����ڽ���
	}
 //----------�ֶ�����ģʽ-------------//	
	if(mode_flag==0)
	{
	  switch(Drive)   
	  {
			case 0:PWM=Zero;break;
		  case 1:PWM=First;break;
		  case 2:PWM=Second;break;
		  case 3:PWM=Third;break;
			case 4:PWM=Fourth;break;
		}
	
	}
 //----------�Զ�����ģʽ-------------//
	  if(mode_flag == 1)
		{
			if((Temperature<=TH-15)&&(Temperature>=TL)) {PWM=First;Drive=1;}//PWM���
			else if((Temperature<=TH-10)&&(Temperature>=TH-15)) {PWM=Second;Drive=2;}
			else if((Temperature>TH-10)&&(Temperature>=TL)) {PWM=Third;Drive=3;}
			else if((Temperature>TH)) {PWM=Fourth;Drive=4;}
			else if(Temperature<=TL) {PWM=Zero;Drive=0;}
		}	
}

void Get_Wifi_Dat(void)    //��ȡWifi������������
{ 		
  /*��strstr�������жϲ���ָ���Ƿ�ƥ�� �Ըú������˽�Ļ������аٶ�*/	 
	if(strstr((const char*)Message_Buf,"Mode0"))    //�жϵ�Զ�̲���ָ��
  {	
    mode_flag = 0;PWM=0;Run_flag=0;Drive=0;
		Motor_Hour=00;Motor_Min=30;Motor_Sec=00;
    delay(20);     //�ӳ�
    memset(Message_Buf, 0, sizeof(Message_Buf));    //ִ����ָ�� ���ָ������ռ� ��ֹ����ִ�и�ָ��
  }
	else if(strstr((const char*)Message_Buf,"Mode1"))    //�жϵ�Զ�̲���ָ��
  {	
    mode_flag = 1;PWM=0;Run_flag=0;Drive=0;
		Motor_Hour=00;Motor_Min=30;Motor_Sec=00;
    delay(20);     //�ӳ�
    memset(Message_Buf, 0, sizeof(Message_Buf));    //ִ����ָ�� ���ָ������ռ� ��ֹ����ִ�и�ָ��
  }
  else if(strstr((const char*)Message_Buf,"Mode2"))   //�жϵ��Զ���λ����ָ��
   {	
		 mode_flag = 2;PWM=0;Run_flag=1;
     delay(20);   //�ӳ�
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
	   else if(strstr((const char*)Message_Buf,"Mode3"))   //�жϵ��Զ���λ����ָ��
   {	
		 mode_flag = 3;PWM=Zero;Run_flag=1;
     delay(20);   //�ӳ�
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
	 else if(strstr((const char*)Message_Buf,"SD1"))   //�жϵ�1������ָ��
   {	
		 if(mode_flag==2) {PWM=First; Run_flag=1;Drive=1;}
     delay(20);   //�ӳ�
     memset(Message_Buf, 0, sizeof(Message_Buf));   
   }
	 else if(strstr((const char*)Message_Buf,"SD2"))   //�жϵ�2������ָ��
   {	
		 if(mode_flag==2) {PWM=Second;Run_flag=1;Drive=2;}
     delay(20);   //�ӳ�
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
	 else if(strstr((const char*)Message_Buf,"SD3"))   //�жϵ�3������ָ��
   {	
		 if(mode_flag==2) {PWM=Third;;Drive=3;}
     delay(20);   //�ӳ�
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }else if(strstr((const char*)Message_Buf,"SD4"))   //�жϵ�4������ָ��
   {	
		 if(mode_flag==2) {PWM=Fourth;Drive=4;}
     delay(20);   //�ӳ�
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
	 else if(strstr((const char*)Message_Buf,"Stop"))   //�жϵ�ֹͣ����ָ��
   {	
		if(mode_flag==2) {PWM=Zero;Run_flag=0;mode_flag=0;Drive=0;}
		 Motor_Hour=00;Motor_Min=30;Motor_Sec=00;
     delay(20);   //�ӳ�
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
   
}

void Key_Scan(void)  //����ɨ��
{
	static unsigned char Pause=0;
	static bit flag=0;
	
	if(Key_Mode==0)
	{
	  delay(10);
		if(Key_Mode==0)
		{
			if(Beep==0)  {Motor_Hour=00;Motor_Min=30;Motor_Sec=00;Beep=1;Drive=0;}
			else if(mode<8)  mode++; else mode=0;	
		}
		while(!Key_Mode);
	}
	
	if(Key_ADD==0)  
	{
	  delay(10);
		if(Key_ADD==0)
	  {
			if(Beep==1)
			{
		   switch(mode)
			{ 
				case 0:if(mode_flag==0&&Drive<4) Drive++; else Drive=0;break;
			  case 1:if(Motor_Sec<59)Motor_Sec++;else Motor_Sec=0;break;
			  case 2:if(Motor_Min<59)Motor_Min++;else Motor_Min=0;break;
				case 3:if(Motor_Hour<23)Motor_Hour++;else Motor_Hour=0;break;
				case 4:if(Sec<59)Sec++;else Sec=0;break;
			  case 5:if(Min<59)Min++;else Min=0;break;
				case 6:if(Hour<23)Hour++;else Hour=0;break;
				case 7:if(TH<100) TH++;else TH=100;break;
				case 8:if(TL<TH) TL++; else TL=TH;break;
			 }	
		 }			
		}
		if(mode==0||mode==4||mode==5) while(!Key_ADD);
	}

		if(Key_Dec==0)
	{
	  delay(10);
		if(Key_Dec==0)
	  {
			if(Beep==1)
			{
		  switch(mode)
			{
				case 0:if((mode_flag==0)&&(Drive>0)) Drive--; else Drive=4; break;
			  case 1:if(Motor_Sec>0)Motor_Sec--;else Motor_Sec=59;break;
			  case 2:if(Motor_Min>0)Motor_Min--;else Motor_Min=59;break;
				case 3:if(Motor_Hour>0)Motor_Hour--;else Motor_Hour=59;break;
				case 4:if(Sec>0)Sec--;else Sec=59;break;
			  case 5:if(Min>0)Min--;else Min=59;break;
				case 6:if(Hour>0)Hour--;else Hour=59;break;
				case 7:if(TH>TL) TH--;else TH=TL;break;
				case 8:if(TL>0) TL--; else TL=0;break;
			}
      }			
		}
		if(mode==0||mode==4||mode==5) while(!Key_Dec);
	}
	
	if(Key_Memu==0)
	{
	 delay(10);
	if(Key_Memu==0)
	{
		while(!Key_Memu);
	   if(mode==0)
     {
			 if(mode_flag<3) mode_flag++;else mode_flag=0;Drive=PWM=0;
			
		  if(Run_flag==1) 
		 {
			 flag=!flag;
		   if(flag==1) {Pause=PWM;PWM=0;}
			 else  PWM=Pause;
		 }
		 }			
		 else mode=0; 
	}
	 
	}

}
