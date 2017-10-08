#ifndef __IIC_H
#define __IIC_H

#include "stm32f10x.h"

#define IIC_WR	0
#define IIC_RD	1

#define SDA_HIGH GPIO_SetBits(GPIOB, GPIO_Pin_10);
#define SDA_LOW GPIO_ResetBits(GPIOB, GPIO_Pin_10);
#define SCL_HIGH GPIO_SetBits(GPIOB, GPIO_Pin_11);
#define SCL_LOW GPIO_ResetBits(GPIOB, GPIO_Pin_11);

//控制SCL的比特率在400kHz以下
#define IIC_DELAY Delay_us(3)

uint8_t IIC_Init(uint8_t Addr);

void IIC_Start(void);
void IIC_Stop(void);

void IIC_SendByte(uint8_t Data);
uint8_t IIC_ReadByte(void);

uint8_t IIC_WaitAck(void);
void IIC_Ack(void);
void IIC_NAck(void);

#endif
