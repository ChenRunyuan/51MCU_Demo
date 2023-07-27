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
/**********************����***********************
ESP8266:RX-PA2 TX-PA3 RST-PA5
DTH11:out-PC11
LED1:PB5  LED2:PB4 LED3:PE5
���ڵ��ԣ�RX-PA9 TX-PA10
*************************************************/
extern u8 send_flag;
void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϿ�������������
	delay_init();									//��ʱ������ʼ��
	LED_Init();
	uart_init(115200);	 	                        //����1��ʼ��Ϊ115200 		PA9-TX  PA10-RX
	Usart2_Init(115200);							//����2������SEND8266��      PA2-TX  PA3-RX
	DHT11_Init();
    printf("Hardware init OK\r\n");	
}
 
int main(void)
{	 
	unsigned short timeCount = 0;	//���ͼ������
	Hardware_Init();				//��ʼ����ΧӲ��
	SEND8266_Init();	
	TIM4_Int_Init(49999,7199);                      //10Khz�ļ���Ƶ�ʣ�������30000Ϊ3s 
	while(1)
	{
		control();//led�Ŀ��ƺ���
		if(send_flag==1)
		{
			printf("OneNet_GetData\r\n");	
			OneNet_GetData();
			send_flag = 0;
			SEND8266_Clear();
		}
		if(++timeCount >= 400)		//���ͼ��1s
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

