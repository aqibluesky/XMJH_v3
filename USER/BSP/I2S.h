#ifndef __I2S_H
#define __I2S_H

#include "stm32f10x.h"


void I2S_RX_Start(uint16_t Standard, uint16_t WordLen, uint16_t AudioFreq, uint16_t Mode);
void I2S_RX_Stop(void);

void I2S_TX_Start(uint16_t Standard, uint16_t WordLen, uint16_t AudioFreq, uint16_t Mode);
void I2S_TX_Stop(void);

void I2S_RX_DMA_Init(uint8_t *pBuf, uint16_t num);
void I2S_TX_DMA_Init(uint8_t *pBuf, uint16_t num);
void DMA2_Channel2_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);

#endif
