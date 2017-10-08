#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"

extern volatile uint8_t TIM2_Flag;
extern volatile uint8_t TIM3_Flag;

void TIM2_Init(uint16_t period, uint16_t prescaler);
void TIM2_Config(uint8_t state);
void TIM2_IRQHandler(void);

void TIM3_Init(uint16_t period, uint16_t prescaler);
void TIM3_Config(uint8_t state);
void TIM3_IRQHandler(void);

#endif
