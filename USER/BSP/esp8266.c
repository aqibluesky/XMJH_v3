#include "esp8266.h"
#include "SysTick.h"
#include "usart.h"
#include "timer.h"
#include "bsp_init.h"
#include <string.h>

extern volatile uint8_t USART_Trans;


//用于向8266发送AT指令的函数
static uint8_t Esp_SendCmd(char *pCmd, uint8_t waittime, char *pRes);

void Esp_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_0 | GPIO_Pin_1);
	
	USART2_Init(115200);
	
	Delay_ms(250);//从8266上电开始到能开始接受AT指令至少需等待200毫秒
	Esp_SetBaudRate(1152000);
	Delay_ms(50);	//需等待一段时间让8266反应过来
	Esp_StationMode();
}


void Esp_Reset(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	Delay_ms(100);
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
	Delay_ms(250);
}


void Esp_Sleep(uint8_t mode)
{
	if(mode)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);
		Delay_ms(100);
	}
	else
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_0);
		Delay_ms(250);
	}
}


static uint8_t Esp_SendCmd(char *pCmd, uint8_t waittime, char *pRes)
{
	char RecTemp[256];
	uint8_t i = 0;
	
	TIM2_Flag = 0;//清空标志并开启定时器，用于超时自己推出而不是站着傻等
	TIM2_Config(1);
	memset(RecTemp,0,sizeof(RecTemp));
	USART_SendStr(USART2,pCmd);
	while(*pRes)
	{
		while((USART_GetFlagStatus(USART2,USART_FLAG_RXNE)) == RESET)
		{
			if(TIM2_Flag >= waittime)
			{
				TIM2_Flag = 0;
				TIM2_Config(0);
				return 1;
			}
		}
		RecTemp[i] = USART_ReceiveData(USART2);
		if(*pRes == RecTemp[i])
		{
			pRes ++;
		}
		i++;
	}
	TIM2_Flag = 0;
	TIM2_Config(0);
	return 0;
}


uint8_t Esp_ATtest(void)
{
	return(Esp_SendCmd("AT\r\n",1,"OK"));
}


uint8_t Esp_Restore(void)
{
	return(Esp_SendCmd("AT+RESTORE\r\n",1,"OK"));
}


uint8_t Esp_EnterSmartConfig(void)
{
	return(Esp_SendCmd("AT+CWSTARTSMART\r\n",1,"OK"));
}

uint8_t Esp_ExitSmartConfig(void)
{
	return(Esp_SendCmd("AT+CWSTOPSMART\r\n",1,"OK"));
}


uint8_t Esp_StationMode(void)
{
	return(Esp_SendCmd("AT+CWMODE=1\r\n",1,"OK"));
}


uint8_t Esp_SetBaudRate(uint32_t BaudRate)
{
	char cmd [128];
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd, "AT+UART_CUR=%d,8,1,0,0\r\n",BaudRate);
	if(Esp_SendCmd(cmd,1,"OK"))
	{
		return 1;
	}
	else
	{
		USART2_Init(BaudRate);
		return 0;
	}
}


uint8_t Esp_LinkServer(const char* ip, const char* port)
{
	char cmd [128];
	uint8_t fun_res;
	
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",ip,port);
	fun_res = Esp_SendCmd(cmd,3,"OK");
	if(fun_res == 0)
	{
		LED_Control(0,1,0);
	}
	return fun_res;
}


uint8_t Esp_TransStart(void)
{
	Esp_TransMode(1);
	return(Esp_SendCmd("AT+CIPSEND\r\n",1,"OK\r\n>"));
}


uint8_t Esp_ExitServer(void)
{
	uint8_t fun_res;
	
	fun_res = Esp_SendCmd("AT+CIPCLOSE\r\n",1,"OK");
	if(fun_res == 0)
	{
		LED_Control(0,0,0);
	}
	return fun_res;
}


uint8_t Esp_APtest(void)
{
	return(Esp_SendCmd("AT+CWJAP?\r\n",1,"+CWJAP:OK"));
}


uint8_t Esp_BreakTrans(void)
{
	Delay_ms(100);
	USART_SendStr(USART2,"+++");//必须是单独的一串+++，所以和前后数据要隔开一定的时间
	Delay_ms(100);
	return(Esp_ATtest());
}


uint8_t Esp_JoinAP (const char *pSSID, const char *pPassWord)
{
	char cmd[128];
	
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", pSSID, pPassWord);
	return(Esp_SendCmd(cmd,15,"WIFIGOTIPOK"));
}


uint8_t Esp_SmartConfig(void)
{
	char Res[] = {"WIFIGOTIPOK"};
	char *pRes;
	
	pRes = Res;
	Esp_StationMode();
	Esp_EnterSmartConfig();
	while(*pRes)
	{
		while((USART_GetFlagStatus(USART2,USART_FLAG_RXNE)) == RESET)
		{
			;
		}
		if(*pRes == USART_ReceiveData(USART2))
		{
			pRes++;
		}
	}
	Esp_ExitSmartConfig();
	return 0;
}


uint8_t Esp_TransMode(uint8_t mode)
{
	if(mode)
	{
		return(Esp_SendCmd("AT+CIPMODE=1\r\n",1,"OK"));
	}
	else
	{
		return(Esp_SendCmd("AT+CIPMODE=0\r\n",1,"OK"));
	}
}


uint8_t Esp_TransBuf(char *pBuf, uint32_t length)
{
	char cmd[128];
	
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"AT+CIPSENDBUF=%d\r\n", length);
	
	if(Esp_SendCmd(cmd,1,"OK\r\n>"))
	{
		return 1;
	}
	USART2_TX_DMA_Init((uint8_t *)pBuf,length);
	DMA_Cmd(DMA1_Channel7,ENABLE);
	USART_Trans = 0;
	while(USART_Trans == 0)
	{
		; //等待DMA传输完成;
	}
	DMA_Cmd(DMA1_Channel7,DISABLE);
	USART_Trans = 0;
	return(Esp_SendCmd("\0",1,"SEND OK"));
}
