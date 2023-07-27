#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "USART2.h"		
#include "send8266.h"
#include "dht11.h"
#include "onenet.h"
#include "timer.h"
#include <string.h>
#include <stdio.h>
/**********************接线***********************
ESP8266:RX-PA2 TX-PA3 RST-PA5
DTH11:out-PC11
LED1:PB5  LED2:PB4 LED3:PE5
串口调试：RX-PA9 TX-PA10
*************************************************/
extern u8 send_flag;
void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置
	delay_init();									//延时函数初始化
	LED_Init();
	uart_init(115200);	 	                        //串口1初始化为115200 		PA9-TX  PA10-RX
	Usart2_Init(115200);							//串口2，驱动SEND8266用      PA2-TX  PA3-RX
	DHT11_Init();
    printf("Hardware init OK\r\n");	
}
 
int main(void)
{	 
	unsigned short timeCount = 0;	//发送间隔变量
	Hardware_Init();				//初始化外围硬件
	SEND8266_Init();	
	TIM4_Int_Init(49999,7199);                      //10Khz的计数频率，计数到30000为3s 
	while(1)
	{
		control();//led的控制函数
		if(send_flag==1)
		{
			printf("OneNet_GetData\r\n");	
			OneNet_GetData();
			send_flag = 0;
			SEND8266_Clear();
		}
		if(++timeCount >= 400)		//发送间隔1s
		{
			printf("OneNet_SendData\r\n");	
			OneNet_SendData();
			timeCount = 0;
			SEND8266_Clear();
		}
			SEND8266_GetData(0);
		delay_ms(10);
	}
}

