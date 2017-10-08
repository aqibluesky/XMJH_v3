#include "usart.h"
#include "BSP_Init.h"

volatile uint8_t IDLE_Flag = 0;

void USART1_Init(uint32_t BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate            = BaudRate;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_Cmd(USART1, ENABLE);
}


void USART2_Init(uint32_t BaudRate)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate            = BaudRate;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART2,USART_IT_TC,DISABLE);
	USART_ITConfig(USART2,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);

	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);

	USART_Cmd(USART2, ENABLE);
}


void USART2_IRQHandler(void)
{
	//防止编译器优化
	volatile uint32_t temp;
	//总线空闲中断，初始化完毕后会进入一次，发送完毕后会进入一次，接受完毕后也会进入一次
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)//总线空闲中断，表示数据已经发送完成
	{
		//USART_ClearFlag，这个是不能清除IDLE标志的，IDLE 该位由软件序列清零（读入 USART_SR 寄存器，然后读入 USART_DR 寄存器）。
		//USART_ClearFlag(USART1,USART_IT_IDLE);
		//清USART_IT_IDLE标志
		temp = USART2->SR;
		temp = USART2->DR;
		IDLE_Flag++;
	}
}


void USART2_TX_DMA_Init(uint8_t *pBuf, uint16_t num)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = num;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);

	DMA_ITConfig(DMA1_Channel7,DMA_IT_TC,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void DMA1_Channel7_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC7) == SET)
	{
//		发送数据暂时用不到双缓存
//		if((DMA1_Channel7->CMAR)==((uint32_t)Cache_0))
//		{
//			DMA1_Channel7->CMAR=(uint32_t)Cache_1;
//			USART_Cache = 1;
//		}
//		else if((DMA1_Channel7->CMAR)==((uint32_t)Cache_1))
//		{
//			DMA1_Channel7->CMAR=(uint32_t)Cache_0;
//			USART_Cache = 0;
//		}
		DMA_ClearITPendingBit(DMA1_IT_TC7);
		USART_Trans = 1;
	}
}


void USART2_RX_DMA_Init(uint8_t *pBuf, uint16_t num)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = num;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);

	DMA_ITConfig(DMA1_Channel6,DMA_IT_TC,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void DMA1_Channel6_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC6) == SET)
	{
		if((DMA1_Channel6->CMAR)==((uint32_t)Cache_0))
		{
			DMA1_Channel6->CMAR=(uint32_t)Cache_1;
			USART_Cache = 1;
		}
		else if((DMA1_Channel6->CMAR)==((uint32_t)Cache_1))
		{
			DMA1_Channel6->CMAR=(uint32_t)Cache_0;
			USART_Cache = 0;
		}
		DMA_ClearITPendingBit(DMA1_IT_TC6);
		USART_Trans = 1;
	}
}


void USART_SendByte(USART_TypeDef *USARTx, char Data)
{
	USART_ClearFlag(USARTx,USART_FLAG_TC);
	USART_SendData(USARTx,Data);
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET)
	{
		;
	}
}


char USART_ReceiveByte(USART_TypeDef *USARTx)
{
	while((USART_GetFlagStatus(USARTx,USART_FLAG_RXNE)) == RESET)
	{
		;
	}
	return USART_ReceiveData(USARTx);
}


void USART_SendStr(USART_TypeDef *USARTx, const char *pData)
{
	USART_ClearFlag(USARTx,USART_FLAG_TC);
	while(*pData)
	{
		USART_SendData(USARTx, *(pData));
		pData ++;
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET)
		{
			;
		}
	}
}

//重定向c标准库的frintf函数和scanf函数
int fputc(int ch, FILE *f)
{
	USART_ClearFlag(USART1,USART_FLAG_TC);
	USART_SendData(USART1, (uint8_t) ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{
		;
	}
	return (ch);
}


int fgetc(FILE *f)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
	{
		;
	}
	return (int)USART_ReceiveData(USART1);
}
