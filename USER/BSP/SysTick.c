#include "SysTick.h"

static volatile uint32_t TimingDelay;

//设置1us中断一次
void SysTick_Init(void)
{
	while(SysTick_Config(SystemCoreClock/1000000))
	{
		;
	}
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

void Delay_us(volatile uint32_t nTime)
{
	TimingDelay = nTime;
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
	while(TimingDelay != 0)
	{
		;
	}
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

void Delay_ms(volatile uint32_t nTime)
{
	TimingDelay = nTime*1000;
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
	while(TimingDelay != 0)
	{
		;
	}
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
	if (TimingDelay != 0)
	{
		TimingDelay--;
	}
}
