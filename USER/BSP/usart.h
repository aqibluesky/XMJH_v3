#ifndef __USART_H
#define	__USART_H

#include "stm32f10x.h"
#include <stdio.h>

extern volatile uint8_t IDLE_Flag;

void USART1_Init(uint32_t BaudRate);
void USART2_Init(uint32_t BaudRate);

void USART2_IRQHandler(void);
void USART2_TX_DMA_Init(uint8_t *pBuf, uint16_t num);
void DMA1_Channel7_IRQHandler(void);
void USART2_RX_DMA_Init(uint8_t *pBuf, uint16_t num);
void DMA1_Channel6_IRQHandler(void);

void USART_SendByte(USART_TypeDef *USARTx, char Data);
char USART_ReceiveByte(USART_TypeDef *USARTx);
void USART_SendStr(USART_TypeDef *USARTx, const char *pData);

#endif
