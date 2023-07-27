#include "LCD12864.h" //LCD12864液晶
#include "DS18B20.H" //18B20温度传感器
#include "Fount.h"   //字库
#include "string.h"
#include "stdio.h"

/******局域网进行数据对接时的配置 */
#define AT         "AT\r\n"	
#define RST        "AT+RST\r\n"	
#define CWSAP      "AT+CWSAP?\r\n"		//STA+AP模式
#define CIFSR      "AT+CIFSR\r\n"
#define CIPMUX     "AT+CIPMUX=1\r\n"    //IP  192.168.4.1
#define CIPSERVER     "AT+CIPSERVER=1,8080\r\n"
//----------------------------串口变量------------------------------------//
#define  Max_BUFF_Len 10               //数据缓存最大值

unsigned char Rec;                     //串口数据接收暂存位
unsigned char Rx_Buff[Max_BUFF_Len];   //缓存数组
unsigned int  Rx_Count;                //数据缓存存放地址
unsigned int  Count;
unsigned char Time;

unsigned char Message_length;

bit ok_flag=0;
bit AT_Mode=0;
char Message_Buf[20];  //用于存贮操作指令
unsigned int USART_RX_STA=0;       //接收状态标记	

//------------------------------------------------------------------------//
//----------------------时间、定时、温度变量-------------------------------//
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
unsigned char mode_flag=0; //模式
unsigned char Run_flag=0;  //运行
unsigned char Drive=0;    //档位
unsigned char mode=0;     //按键
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
void Fan_PWM(void);      //风扇速度
void Get_Wifi_Dat(void);    //获取Wifi发送来的数据
void Motor_Run_Time(void);  //风扇运行时间
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
	Rx_Count=0;	  
}

void ESP8266Mode_Init()
{
	int Delay = 1000;

	unsigned char Cmd[]="+++";//退出透传模式指令
	AT_Mode = 1;  //进入发AT指令模式

	Send_Data(Cmd,strlen((const char*)Cmd));//先行退出透传模式,再配置AT指令
	delay(100);				//等待退出透传
	
	SendCmd(AT,Delay);			//模块有效性检查

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
  TH0=0x4c;    //50ms定时
  TL0=0x00;
  Count++;
	if(Count==20)
	{ 
		Now_Time();
	  Count=0;	
    Motor_Run_Time();            //获取定时倒计时	
	}
	Fan_PWM();
}


void Usart() interrupt 4   //串口中断
{
   if(RI)
  {
	  RI=0;                   //接收中断清零
		Rec=SBUF;               //将接收的数据暂存到Rec
		Rx_Buff[Rx_Count]=Rec;  //将接收到的数据存入数组
/*  配置AT指令模式下 数据的处理 */
			if(AT_Mode==1)//配置AT指令下 数据的处理
			{
				Receive_AT_Data();  //处理AT指令模式下的数据
			}

		/*  连接平台后 数据的处理 */
			else if(AT_Mode==0)    //正常模式接收数据
			{			 	
				Receive_Normal_Data();  //正常模式下处理数据
			}
  }
}
	 
void LCD_Display_Init()   //显示初始化
{
  init_lcd();
	Clr_Scr();
//----------------第一行--------------------//
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
	
//----------------第二行--------------------//
	LCD_Write_Chinese(2,0,2,HZ_Chinese);  //温
	LCD_Write_Chinese(2,16,3,HZ_Chinese);  //度
  LCD_Write_Num(2,32,11,Num);//:
	LCD_Write_Chinese(2,64,8,HZ_Chinese);  //℃ 

//----------------第三行--------------------//
  LCD_Write_Chinese(4,0,4,HZ_Chinese);  //温
	LCD_Write_Chinese(4,16,5,HZ_Chinese);  //度
	LCD_Write_Num(4,32,11,Num);
	LCD_Write_Num(4,40,Hour/10,Num);LCD_Write_Num(4,48,Hour%10,Num);
	LCD_Write_Num(4,56,10,Num);
	LCD_Write_Num(4,64,Min/10,Num);LCD_Write_Num(4,72,Min%10,Num);
	LCD_Write_Num(4,80,10,Num);
	LCD_Write_Num(4,88,Sec/10,Num);LCD_Write_Num(4,96,Sec%10,Num);
	
//----------------第四行--------------------//
  LCD_Write_Chinese(6,0,6,HZ_Chinese);  //温
	LCD_Write_Chinese(6,16,7,HZ_Chinese);  //度
	LCD_Write_Num(6,32,11,Num);
	LCD_Write_Num(6,40,Motor_Hour/10,Num);LCD_Write_Num(6,48,Motor_Hour%10,Num);
	LCD_Write_Num(6,56,10,Num);
	LCD_Write_Num(6,64,Motor_Min/10,Num);LCD_Write_Num(6,72,Motor_Min%10,Num);
	LCD_Write_Num(6,80,10,Num);
	LCD_Write_Num(6,88,Motor_Sec/10,Num);LCD_Write_Num(6,96,Motor_Sec%10,Num);
}

void Display_Data()
{
	LCD_Write_Num(0,24,TH/10,Num);LCD_Write_Num(0,32,TH%10,Num);    //上限阈值
	LCD_Write_Num(0,88,TL/10,Num);LCD_Write_Num(0,96,TL%10,Num);    //下限阈值
	
 LCD_Write_Num(2,40,Temperature/10,Num);LCD_Write_Num(2,48,Temperature%10,Num);  //温度
	
	LCD_Write_Num(4,40,Hour/10,Num);LCD_Write_Num(4,48,Hour%10,Num); //时
	LCD_Write_Num(4,64,Min/10,Num);LCD_Write_Num(4,72,Min%10,Num);   //分
	LCD_Write_Num(4,88,Sec/10,Num);LCD_Write_Num(4,96,Sec%10,Num);   //秒
	 
	LCD_Write_Num(6,40,Motor_Hour/10,Num);LCD_Write_Num(6,48,Motor_Hour%10,Num); //定时-时
	LCD_Write_Num(6,64,Motor_Min/10,Num);LCD_Write_Num(6,72,Motor_Min%10,Num);   //定时-分
	LCD_Write_Num(6,88,Motor_Sec/10,Num);LCD_Write_Num(6,96,Motor_Sec%10,Num);   //定时-秒 
	
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

void Motor_Run_Time(void)  //风扇运行时间
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
		   Motor_Hour=Motor_Min=Motor_Sec=Run_flag=PWM=Beep=0; //定时时间到	 		  
		 }			   
	  }
  }
	}
  
}
void Receive_AT_Data()
{    //判断ESP8266是否返回'OK'、'>'、'ready' 
		if(strstr(Rx_Buff,"OK")||strstr(Rx_Buff,)||strstr(Rx_Buff,"ready"))    
		{
			 memset(Rx_Buff, 0, sizeof(Rx_Buff));  //清空Rx_Buff数组
			 ok_flag=1;			                //判断成功，标志位置1
			 Rx_Count=0;				
		}
		else                                   //否则继续接受数据
		{
			Rx_Count++;	
			if(Rx_Count>=150)                   //最多接收150个数据，数据溢出 清空数组
			{
				Rx_Count=0;
				memset(Rx_Buff, 0, sizeof(Rx_Buff));
			}
		}
}
//处理AT指令模式下的数据
void Receive_Normal_Data()
{
	  static int i;        //静态变量i
	
		if(Rx_Count==50)                   //最多接收150个数据，数据溢出 清空数组
		{
			Rx_Count=0;                 	
			memset(Rx_Buff, 0, sizeof(Rx_Buff));  //清空当前数组所有数据
		}

		if(Message_length>0)                   //表示可以开始存贮操作指令
		{
			Message_Buf[i]=Rx_Buff[Rx_Count];   //存贮操作指令数据
			i++;
			Message_length--;                  //存一个指令，剩余数量减一,判断操作指令是否存贮完成
		}
		 
		if(Rx_Count>3&&Rx_Buff[Rx_Count-2]==0x2b&&Rx_Buff[Rx_Count-1]==0x49&&Rx_Buff[Rx_Count]>0)   
		//如果当前接收到的数据大于0，并且这个数据的前两个数据为'+ I' 代表当前数据就是操作指令的长度。
		{
			 memset(Message_Buf, 0, sizeof(Message_Buf)); //清空存贮操作指令的数组，准备存贮新的操作指令
			
			 Message_length=Rx_Buff[Rx_Count];      //将接收到的数据存为操作指令长度。
			 i=0;                                   //清空i
		}
		Rx_Count++;                               //准备存储下一个数据 					
}

void SendCmd(char* cmd, int timeOut)
{
    while(1)
    {
		memset(Rx_Buff, 0, sizeof(Rx_Buff)); //发送数据之前，先情况接收数组，数据在串口中接收。
		Rx_Count=0;

		Send_Data((unsigned char *)cmd,strlen((const char *)cmd));   //用串口把cmd命令写给ESP8266

		delay(timeOut);                                          //延迟等待

		if(ok_flag==1)	//串口中断函数中，自行判断是否返回"OK"或">"，若符合条件，ok_flag则置1，跳出当前AT指令的配置，开始配置下一条
		{  
			ok_flag=0;   //清空标志
			break;
		}
		else
		{
			delay(100);		
		}
    }
}



void Fan_PWM(void)      //风扇速度
{
	static unsigned char countFan =0;
	countFan++;
	if(countFan<PWM)	   // 占空比调节
	{
		Fan=0;			  //打开
	}
	else if(countFan<=T)	//关闭时间段//**All notes can be deleted and modified**//
	{
	 	Fan=1;			//关闭
		if(countFan == T)	 countFan=0;  //一个周期结束
	}
 //----------手动调速模式-------------//	
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
 //----------自动调速模式-------------//
	  if(mode_flag == 1)
		{
			if((Temperature<=TH-15)&&(Temperature>=TL)) {PWM=First;Drive=1;}//PWM输出
			else if((Temperature<=TH-10)&&(Temperature>=TH-15)) {PWM=Second;Drive=2;}
			else if((Temperature>TH-10)&&(Temperature>=TL)) {PWM=Third;Drive=3;}
			else if((Temperature>TH)) {PWM=Fourth;Drive=4;}
			else if(Temperature<=TL) {PWM=Zero;Drive=0;}
		}	
}

void Get_Wifi_Dat(void)    //获取Wifi发送来的数据
{ 		
  /*用strstr函数来判断操作指令是否匹配 对该函数不了解的话请自行百度*/	 
	if(strstr((const char*)Message_Buf,"Mode0"))    //判断到远程操作指令
  {	
    mode_flag = 0;PWM=0;Run_flag=0;Drive=0;
		Motor_Hour=00;Motor_Min=30;Motor_Sec=00;
    delay(20);     //延迟
    memset(Message_Buf, 0, sizeof(Message_Buf));    //执行完指令 清空指令存贮空间 防止继续执行该指令
  }
	else if(strstr((const char*)Message_Buf,"Mode1"))    //判断到远程操作指令
  {	
    mode_flag = 1;PWM=0;Run_flag=0;Drive=0;
		Motor_Hour=00;Motor_Min=30;Motor_Sec=00;
    delay(20);     //延迟
    memset(Message_Buf, 0, sizeof(Message_Buf));    //执行完指令 清空指令存贮空间 防止继续执行该指令
  }
  else if(strstr((const char*)Message_Buf,"Mode2"))   //判断到自动档位操作指令
   {	
		 mode_flag = 2;PWM=0;Run_flag=1;
     delay(20);   //延迟
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
	   else if(strstr((const char*)Message_Buf,"Mode3"))   //判断到自动档位操作指令
   {	
		 mode_flag = 3;PWM=Zero;Run_flag=1;
     delay(20);   //延迟
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
	 else if(strstr((const char*)Message_Buf,"SD1"))   //判断到1挡操作指令
   {	
		 if(mode_flag==2) {PWM=First; Run_flag=1;Drive=1;}
     delay(20);   //延迟
     memset(Message_Buf, 0, sizeof(Message_Buf));   
   }
	 else if(strstr((const char*)Message_Buf,"SD2"))   //判断到2挡操作指令
   {	
		 if(mode_flag==2) {PWM=Second;Run_flag=1;Drive=2;}
     delay(20);   //延迟
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
	 else if(strstr((const char*)Message_Buf,"SD3"))   //判断到3挡操作指令
   {	
		 if(mode_flag==2) {PWM=Third;;Drive=3;}
     delay(20);   //延迟
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }else if(strstr((const char*)Message_Buf,"SD4"))   //判断到4挡操作指令
   {	
		 if(mode_flag==2) {PWM=Fourth;Drive=4;}
     delay(20);   //延迟
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
	 else if(strstr((const char*)Message_Buf,"Stop"))   //判断到停止操作指令
   {	
		if(mode_flag==2) {PWM=Zero;Run_flag=0;mode_flag=0;Drive=0;}
		 Motor_Hour=00;Motor_Min=30;Motor_Sec=00;
     delay(20);   //延迟
     memset(Message_Buf, 0, sizeof(Message_Buf));    
   }
   
}

void Key_Scan(void)  //按键扫描
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
