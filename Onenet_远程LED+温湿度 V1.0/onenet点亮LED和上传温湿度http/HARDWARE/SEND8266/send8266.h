#ifndef _SEND8266_H_
#define _SEND8266_H_





#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志


void SEND8266_Init(void);

void SEND8266_Clear(void);

void SEND8266_SendData(unsigned char *data, unsigned short len);

void SEND8266_GetData(unsigned short timeOut);


#endif
