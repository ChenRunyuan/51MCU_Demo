#include "stm32f10x.h"
#include "send8266.h"
#include "onenet.h"
#include "usart.h"
#include "USART2.h"
#include "delay.h"
#include "dht11.h"
#include <string.h>
#include <stdio.h>


#define DEVID   "596876537"   
#define APIKEY	"QF=QF=8zJajGpEfxsClo9xDIhpkRuY="

void OneNet_FillBuf(char *buf)
{
	
	char text[24];
	char buf1[256];
	u8 temp,humi;
	DHT11_Read_Data(&temp,&humi);   
	memset(text, 0, sizeof(text));
	memset(buf1, 0, sizeof(buf1));
	
	strcpy(buf1, "{");
	memset(text, 0, sizeof(text));
	delay_ms(10);
	
	sprintf(text, "\"HUMI\":%d,",humi);
	strcat(buf1, text);
	memset(text, 0, sizeof(text));
	
	delay_ms(10);
	sprintf(text, "\"TEMP\":%d",temp);
	strcat(buf1, text);
	memset(text, 0, sizeof(text));
	
	strcat(buf1, "}");
	
	sprintf(buf, "POST /devices/%s/datapoints?type=3 HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\n"
					"Content-Length:%d\r\n\r\n",
	
					DEVID, APIKEY, strlen(buf1));
					
	strcat(buf, buf1);

}

void OneNet_SendData(void)
{
	
	char buf[512];
	
	memset(buf, 0, sizeof(buf));
	
	OneNet_FillBuf(buf);									//封装数据流
	
	SEND8266_SendData((unsigned char *)buf, strlen(buf));	//上传数据
	
}

void OneNet_RevPro(unsigned char *dataPtr)
{

	if(strstr((char *)dataPtr, "CLOSED"))
	{
		printf( "TCP CLOSED\r\n");
	}
	else
	{
		//这里用来检测是否发送成功
		if(strstr((char *)dataPtr, "succ"))
		{
			printf( "Tips:	Send OK\r\n"); 
		}  
		else
		{
			printf( "Tips:	Send Err\r\n");
		}
	}
	SEND8266_Clear();
}



void OneNet_Getbuf(char *buf_GET)
{
	
	sprintf(buf_GET, "GET /devices/%s/datapoints HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\n\r\n",DEVID, APIKEY);					

}

void OneNet_GetData(void)
{
	
	char buf[256];
	
	memset(buf, 0, sizeof(buf));
	
	OneNet_Getbuf(buf);									//封装数据流
	
	SEND8266_SendData((unsigned char *)buf, strlen(buf));	//上传数据
	
}


