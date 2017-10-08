#include "SoftIIC.h"
#include "SysTick.h"

static uint8_t SDA_Value(void);


static uint8_t SDA_Value(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	return(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10));
}



void IIC_Start(void)
{
	//当SCL高电平时，SDA出现一个下跳沿表示IIC总线启动信号
	SDA_HIGH
	SCL_HIGH
	IIC_DELAY;
	SDA_LOW
	IIC_DELAY;
	SCL_LOW//拉低时钟线，等待传输数据
	IIC_DELAY;
}


void IIC_Stop(void)
{
	//当SCL高电平时，SDA出现一个上跳沿表示IIC总线停止信号
	SDA_LOW
	SCL_HIGH
	IIC_DELAY;
	SDA_HIGH//结束了就结束了
}


void IIC_SendByte(uint8_t Data)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if (Data & 0x80)
		{
			SDA_HIGH
		}
		else
		{
			SDA_LOW
		}
		IIC_DELAY;
		SCL_HIGH
		IIC_DELAY;
		SCL_LOW
		if (i == 7)
		{
			SDA_HIGH
		}
		Data <<= 1;
		IIC_DELAY;
	}
}


uint8_t IIC_ReadByte(void)
{
	uint8_t i;
	uint8_t value;

	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		SCL_HIGH
		IIC_DELAY;
		if (SDA_Value())
		{
			value++;
		}
		SCL_LOW
		IIC_DELAY;
	}
	return value;
}


uint8_t IIC_WaitAck(void)
{
	uint8_t res;

	SDA_HIGH//释放数据总线
	IIC_DELAY;
	SCL_HIGH
	IIC_DELAY;
	if(SDA_Value())
	{
		res = 1;
	}
	else
	{
		res = 0;
	}
	SCL_LOW
	IIC_DELAY;
	return res;
}


void IIC_Ack(void)
{
	SDA_LOW
	IIC_DELAY;
	SCL_HIGH
	IIC_DELAY;
	SCL_LOW
	IIC_DELAY;
	SDA_HIGH
}


void IIC_NAck(void)
{
	SDA_HIGH
	IIC_DELAY;
	SCL_HIGH
	IIC_DELAY;
	SCL_LOW
	IIC_DELAY;
}




uint8_t IIC_Init(uint8_t Addr)
{
	uint8_t Ack;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//如若外部接上拉电阻，可以改为开漏输出，这样可以不改变
	//引脚的模式就得到输入的状态，这里改为推挽输出，当需要读取
	//数据时改为上拉输入。
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	IIC_Start();
	IIC_SendByte(Addr | IIC_WR);
	Ack = IIC_WaitAck();
	IIC_Stop();
	return Ack;
}

