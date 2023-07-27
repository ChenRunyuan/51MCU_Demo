#ifndef _USART2_H_
#define _USART2_H_


#include "stm32f10x.h"


void Usart2_Init(unsigned int baud);

void Usart_SendString(USART_TypeDef *USARTx, unsigned char *str, unsigned short len);

void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...);

#endif
