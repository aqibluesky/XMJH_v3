#include "timer.h"

volatile uint8_t TIM2_Flag = 0;
volatile uint8_t TIM3_Flag = 0;

/*
 *通用定时器中断初始化
 *这里始终选择为APB1的2倍，而APB1为36M
 *period：自动重装值。
 *prescaler：时钟预分频数
 *(9999,7199);		//1000ms中断
*/
void TIM2_Init(uint16_t period,uint16_t prescaler)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = period;
	TIM_TimeBaseStructure.TIM_Prescaler =prescaler;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0x00 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}



void TIM2_Config(uint8_t state)
{
	if(state)
	{
		TIM_SetCounter(TIM2,0);
		TIM_Cmd(TIM2, ENABLE);
	}
	else
		TIM_Cmd(TIM2, DISABLE);
}


void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM2_Flag++;
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

/**
 *通用定时器中断初始化
 *这里始终选择为APB1的2倍，而APB1为36M
 *period：自动重装值。
 *prescaler：时钟预分频数
 *999,7199);		//36000000*2/72000Hz 100ms中断
*/
void TIM3_Init(uint16_t period,uint16_t prescaler)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = period;
	TIM_TimeBaseStructure.TIM_Prescaler =prescaler;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0x00 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}



void TIM3_Config(uint8_t state)
{
	if(state)
	{
		TIM_SetCounter(TIM3,0);
		TIM_Cmd(TIM3, ENABLE);
	}
	else
		TIM_Cmd(TIM3, DISABLE);
}


void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM3_Flag++;
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}
