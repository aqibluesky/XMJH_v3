#include "I2S.h"
#include "bsp_init.h"

void I2S_RX_Start(uint16_t Standard, uint16_t WordLen, uint16_t AudioFreq, uint16_t Mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2S_InitTypeDef I2S_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | \
							RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);			//关闭JTAG调试功能

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);



	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	SPI_I2S_DeInit(SPI2);
	I2S_InitStructure.I2S_Mode = Mode;
	I2S_InitStructure.I2S_Standard = Standard;
	I2S_InitStructure.I2S_DataFormat = WordLen;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_Init(SPI2, &I2S_InitStructure);

	SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Rx,ENABLE);
	I2S_Cmd(SPI2, ENABLE);
}

void I2S_RX_Stop(void)
{
	I2S_Cmd(SPI2, DISABLE);
	SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Rx,DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,DISABLE);
}


void I2S_TX_Start(uint16_t Standard, uint16_t WordLen, uint16_t AudioFreq, uint16_t Mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2S_InitTypeDef I2S_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | \
	                       RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);         //关闭JTAG调试功能


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	SPI_I2S_DeInit(SPI3);
	I2S_InitStructure.I2S_Mode = Mode;
	I2S_InitStructure.I2S_Standard = Standard;
	I2S_InitStructure.I2S_DataFormat = WordLen;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_Init(SPI3, &I2S_InitStructure);

	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,ENABLE);
	I2S_Cmd(SPI3, ENABLE);
}

void I2S_TX_Stop(void)
{
	I2S_Cmd(SPI3, DISABLE);
	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,DISABLE);
}


void I2S_TX_DMA_Init(uint8_t *pBuf, uint16_t num)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);

	DMA_DeInit(DMA2_Channel2);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI3->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = num;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA2_Channel2, &DMA_InitStructure);

	DMA_ITConfig(DMA2_Channel2,DMA_IT_TC,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}



void DMA2_Channel2_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_IT_TC2) == SET)
	{
		if((DMA2_Channel2->CMAR)==((uint32_t)Cache_0))
		{
			DMA2_Channel2->CMAR=(uint32_t)Cache_1;
			I2S_Cache = 1;
		}
		else if((DMA2_Channel2->CMAR)==((uint32_t)Cache_1))
		{
			DMA2_Channel2->CMAR=(uint32_t)Cache_0;
			I2S_Cache = 0;
		}
		DMA_ClearITPendingBit(DMA2_IT_TC2);
		I2S_Trans = 1;
	}
}



void I2S_RX_DMA_Init(uint8_t *pBuf, uint16_t num)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

	DMA_DeInit(DMA1_Channel4);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = num;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel4, &DMA_InitStructure);

	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC4) == SET)
	{
		if((DMA1_Channel4->CMAR)==((uint32_t)Cache_0))
		{
			DMA1_Channel4->CMAR=(uint32_t)Cache_1;
			I2S_Cache = 0;
		}
		else if((DMA1_Channel4->CMAR)==((uint32_t)Cache_1))
		{
			DMA1_Channel4->CMAR=(uint32_t)Cache_0;
			I2S_Cache = 1;
		}
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		I2S_Trans = 1;
	}
}

