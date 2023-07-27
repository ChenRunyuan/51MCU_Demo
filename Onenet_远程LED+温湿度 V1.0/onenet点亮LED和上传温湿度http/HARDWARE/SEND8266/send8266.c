#include "stm32f10x.h"
#include "send8266.h"
#include "delay.h"
#include "usart.h"
#include "USART2.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define SEND8266_WIFI_INFO		"AT+CWJAP=\"ChinaNet-DHCN\",\"357933huang\"\r\n"
#define SEND8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n"

unsigned char send8266_buf[650];
unsigned short send8266_cnt = 0, send8266_cntPre = 0;
int led1,led2,led3; //��onenet��ȡLEDֵ
void SEND8266_Clear(void)
{

	memset(send8266_buf, 0, sizeof(send8266_buf));
	send8266_cnt = 0;

}

_Bool SEND8266_WaitRecive(void)
{

	if(send8266_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
		
	if(send8266_cnt == send8266_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		send8266_cnt = 0;							//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	send8266_cntPre = send8266_cnt;					//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־

}

_Bool SEND8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	Usart_SendString(USART2, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeOut--)
	{
		if(SEND8266_WaitRecive() == REV_OK)							//����յ�����
		{
			if(strstr((const char *)send8266_buf, res) != NULL)		//����������ؼ���
			{
				SEND8266_Clear();									//��ջ���
				
				return 0;
			}
		}
		
		delay_ms(10);
	}
	
	return 1;

}

void SEND8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	SEND8266_Clear();								//��ս��ջ���
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//��������
	if(!SEND8266_SendCmd(cmdBuf, ">"))				//�յ���>��ʱ���Է�������
	{
		Usart_SendString(USART2, data, len);		//�����豸������������
	}

}
void SEND8266_GetData(unsigned short timeOut)
{
	char *ptrLED1 = NULL;
	char *ptrLED2 = NULL;
	char *ptrLED3 = NULL;
	char *ptrclose= NULL;
	char led1_value[2];
	char led2_value[2];
	char led3_value[2];
	ptrclose = strstr((char *)send8266_buf, "CLOSE");
	ptrLED1 = strstr((char *)send8266_buf, "led1");
	ptrLED2 = strstr((char *)send8266_buf, "led2");
	ptrLED3 = strstr((char *)send8266_buf, "led3");
	do
	{
		if(SEND8266_WaitRecive() == REV_OK)						 //����������
		{   
			//printf("buf111:%s\r\n",send8266_buf);
			if(ptrLED1 != NULL)
			{
			    led1_value[0] = *(ptrLED1-10);					
			    led1 = atoi(led1_value);	
			    printf("led1:%d\r\n",led1);
			}
			if(ptrLED2 != NULL)
			{
			    led2_value[0] = *(ptrLED2-10);					
			    led2 = atoi(led2_value);
				printf("led2:%d\r\n",led2);
				//printf("led2_value:%c%c\r\n",led2_value[0],led2_value[1]);
			}
			if(ptrLED3 != NULL)
			{
			    led3_value[0] = *(ptrLED3-10);					
			    led3 = atoi(led3_value);
				printf("led3:%d\r\n",led3);				
			    //printf("buf111:%s\r\n",send8266_buf);
			}
		}
		
		delay_ms(5);													//��ʱ�ȴ�
	} while(timeOut--);
	if(ptrclose != NULL)
	{
		SEND8266_Init();
	}
}

void SEND8266_Init(void)
{
	
	GPIO_InitTypeDef GPIO_Initure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	//SEND8266��λ����
	GPIO_Initure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Initure.GPIO_Pin = GPIO_Pin_5;					//GPIOA5-��λ
	GPIO_Initure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_Initure);
	
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
	delay_ms(250);
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
	delay_ms(500);
	
	SEND8266_Clear();

	printf("1. AT\r\n");
	while(SEND8266_SendCmd("AT\r\n", "OK"))
		delay_ms(500);
	
	printf("2. CWMODE\r\n");
	while(SEND8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		delay_ms(500);
	
	printf("3. CWJAP\r\n");
	while(SEND8266_SendCmd(SEND8266_WIFI_INFO, "GOT IP"))
		delay_ms(500);
	
	printf("4. CIPSTART\r\n");
	while(SEND8266_SendCmd(SEND8266_ONENET_INFO, "CONNECT"))
		delay_ms(500);
	
	printf("5. SEND8266 Init OK\r\n");

}


void USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //�����ж�
	{
		if(send8266_cnt >= sizeof(send8266_buf))	send8266_cnt = 0; //��ֹ���ڱ�ˢ��
		send8266_buf[send8266_cnt++] = USART2->DR;
		
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

}
