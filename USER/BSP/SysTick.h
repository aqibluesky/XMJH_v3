#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"

void SysTick_Handler(void);
void SysTick_Init(void);

void Delay_us(volatile uint32_t nTime);
void Delay_ms(volatile uint32_t nTime);


#endif
