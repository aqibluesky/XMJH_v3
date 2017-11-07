#include "net_link.h"

//该函数用于获取访问百度语音服务器的令牌
static uint8_t Get_Token(char *pToken);
//用于获取wifi配置
static uint8_t WIFI_Config(void);

const char GET_TOKEN[] = {"GET /oauth/2.0/token?grant_type=client_credentials&client_id=SlHEpH9hw92SNsrpjGszg7RU&client_secret=5d8da4956a8c104ebb7d9d43c6604554 HTTP/1.1\r\nHost: openapi.baidu.com\r\n\r\n"};
const char AUDIO2TEXT[] = {"POST /server_api?lan=zh&cuid=chenqt123&token=%s HTTP/1.1\r\nHost: vop.baidu.com\r\nContent-Type: wav;rate=8000\r\nContent-Length: %s\r\n\r\n"};
const char TEXT2AUDIO[]	= {"GET /text2audio?tex=%s&lan=zh&cuid=001&ctp=1&tok=%s HTTP/1.1\r\nHost: tsn.baidu.com\r\n\r\n"};
#if 0
const char IOT_CMD[] = {"POST /v1/datastreams/plug-status/datapoint/?deliver_to_device=true HTTP/1.1\r\nHost: iot.espressif.cn\r\nContent-Type: application/json\r\nAuthorization: token %s\r\ncontent-length: 21\r\n\r\n{\"datapoint\":{\"x\":%1d}}\r\n\r\n"};
const char TIME_API[] = {"GET /?app=life.time&amp;appkey=10003&amp;sign=b59bc3ef6191eb9f747dd4e83c99f2a4&amp;format=json\r\nHTTP/1.1\r\nHost: api.k780.com:88\r\n\r\n"};
const char WEATHER_API[] = {"GET /data/cityinfo/101210101.html HTTP/1.1\r\nHost: www.weather.com.cn\r\n\r\n"};
#endif

static uint8_t Get_Token(char *pToken)
{
	char *pS = NULL;

	Wifi_Connect();
	Esp_LinkServer("openapi.baidu.com","80");
	Esp_TransStart();
	USART_SendStr(USART2,(char *)GET_TOKEN);

	USART2_RX_DMA_Init(Cache_0,CACHE_SIZE);
	memset(Cache_0,0,CACHE_SIZE);
	DMA_Cmd(DMA1_Channel6,ENABLE);

	IDLE_Flag = 0;
	TIM2_Flag = 0;
	TIM2_Config(1);
	while(IDLE_Flag == 0)
	{
		if(TIM2_Flag >= 1)
		{
			break;
		}
	}
	IDLE_Flag = 0;
	TIM2_Flag = 0;
	TIM2_Config(0);
	DMA_Cmd(DMA1_Channel6,DISABLE);
	Esp_BreakTrans();
	Esp_ExitServer();

	pS = strstr((char *)Cache_0,"\"access_token\"");
	if(pS != NULL)
	{
		strncpy(pToken,(char *)(pS + 16),69);
		*(pToken+69) = '\0';
	}
	return 0;
}



uint8_t Audio2Text(char *pText)
{
	char pToken[69];
	char ConLen[10];
	char *pS = NULL;
	char *pE = NULL;

	Wifi_Connect();
	Get_Token(pToken);
	printf("识别开始\r\n");
	res = f_open(&fsrc,"0:/IoT_Box/temp/rec",FA_READ);
	memset(ConLen,0,sizeof(ConLen));
	sprintf(ConLen,"%d",(int)fsrc.fsize);
	memset((char *)Cache_0,0,CACHE_SIZE);
	sprintf((char *)Cache_0,AUDIO2TEXT,pToken,ConLen);
	Esp_LinkServer("vop.baidu.com","80");
	Esp_TransMode(0);
	if(Esp_TransBuf((char *)Cache_0, strlen((char *)Cache_0)))
	{
		Esp_ExitServer();
		return 1;
	}
	while(1)
	{
		f_read(&fsrc,Cache_0,2048,&br);
		while(Esp_TransBuf((char *)Cache_0, br))
		{
			break;
		}
		if(br < 2048)
		{
			break;
		}
	}
	f_close(&fsrc);

	memset((char *)Cache_0,0,CACHE_SIZE);
	USART2_RX_DMA_Init(Cache_0,CACHE_SIZE);
	DMA_Cmd(DMA1_Channel6,ENABLE);
	TIM2_Flag = 0;
	IDLE_Flag = 0;
	TIM2_Config(1);
	while(IDLE_Flag == 0)
	{
		if(TIM2_Flag >= 2)
		{
			TIM2_Config(0);
			break;
		}
	}
	TIM2_Flag = 0;
	IDLE_Flag = 0;
	TIM2_Config(0);
	DMA_Cmd(DMA1_Channel6,DISABLE);

	Esp_ExitServer();

	pS = strstr((char*)Cache_0,"\"result\":[\"");
	pE = strstr((char*)Cache_0,"\"],\"sn\"");
	if(pE>pS && pE-pS <= TXT_LEN*3)
	{
		strncpy(pText,pS+11,(pE-pS)-11);
		*(pText+(pE-pS)-11) = '\0';
	}
	else
	{
		printf("识别失败\r\n");
	}
	return 0;
}



uint8_t Text2Audio(char *pText)
{
	char pToken[69];
	char *URL = NULL;

	Wifi_Connect();
	Get_Token(pToken);
	URL = malloc(TXT_LEN*9);
	memset((char *)Cache_0,0,CACHE_SIZE);
	memset(URL,0,TXT_LEN*9);

	URL_Encode(pText,strlen(pText),URL);

	res = f_unlink("0:/IoT_Box/temp/voc");
	res = f_open(&fsrc,"0:/IoT_Box/temp/voc",FA_CREATE_ALWAYS | FA_WRITE);

	sprintf((char *)Cache_0,TEXT2AUDIO,(char *)URL,pToken);


	free(URL);
	URL = NULL;

	Esp_LinkServer("tsn.baidu.com","80");
	Esp_TransStart();

	USART_SendStr(USART2,(char *)Cache_0);
	memset(Cache_0,0,CACHE_SIZE);
	USART2_RX_DMA_Init(Cache_0,CACHE_SIZE);
	DMA_Cmd(DMA1_Channel6,ENABLE);

	USART_Trans = 0;
	TIM2_Flag = 0;
	TIM2_Config(1);
	while(1)
	{
		if(USART_Trans == 0)
		{
			if(TIM2_Flag >= 1)//如果在1000ms内没有发生DMA传送完毕中断，则判定数据接收完毕
			{
				if(USART_Cache)
				{
					res = f_write(&fsrc,Cache_1,CACHE_SIZE,&br);
				}
				else
				{
					res = f_write(&fsrc,Cache_0,CACHE_SIZE,&br);
				}
				f_close(&fsrc);
				TIM2_Config(0);
				break;
			}
		}
		else
		{
			USART_Trans = 0;//清除标志
			TIM2_Flag = 0;
			if(USART_Cache)
			{
				res = f_write(&fsrc,Cache_0,CACHE_SIZE,&br);
				memset(Cache_0,0,CACHE_SIZE);
			}
			else
			{
				res = f_write(&fsrc,Cache_1,CACHE_SIZE,&br);
				memset(Cache_1,0,CACHE_SIZE);
			}
		}
	}
	DMA_Cmd(DMA1_Channel6,DISABLE);

	Esp_BreakTrans();
	Esp_ExitServer();
	return 0;
}




static uint8_t WIFI_Config(void)
{
	char *pN = NULL;
	char *pS = NULL;
	char *pP = NULL;
	char *ReadTemp = NULL;

	char SSID[64];
	char PASSWORD[64];

	uint16_t i = 0;
	uint8_t fun_res = 1;

	res = f_open(&fsrc,"0:/IoT_Box/config",FA_READ);
	ReadTemp = malloc(1024);
	memset(ReadTemp,0,1024);
	while(f_gets(ReadTemp,1024,&fsrc))
	{
		if(strstr(ReadTemp,"WIFI:") == ReadTemp)
		{
			memset(ReadTemp,0,1024);
			while(f_gets(ReadTemp,1024,&fsrc))
			{
				if(strstr(ReadTemp,">>") == ReadTemp)
				{
					memset(ReadTemp,0,1024);
					while(f_gets(ReadTemp,1024,&fsrc))
					{
						if(strstr(ReadTemp,"<<") == ReadTemp)
						{
							goto config_end;
						}
						else if(strstr(ReadTemp,"-S") && strstr(ReadTemp,"-P"))
						{
							pS = strstr(ReadTemp,"-S");
							pP = strstr(ReadTemp,"-P");
							pN = pS+2;
							memset(SSID,0,sizeof(SSID));
							i = 0;
							while(pN < pP)
							{
								if(*pN != ' ' && i < 64)
								{
									SSID[i] = *pN;
									i++;
									pN++;
								}
								else
								{
									pN++;
								}
							}
							pN = pP+2;
							memset(PASSWORD,0,sizeof(PASSWORD));
							i = 0;
							while(*pN && i < 64)
							{
								if(*pN != ' ' && *pN != '\r' && *pN != '\n')
								{
									PASSWORD[i] = *pN;
									i++;
									pN++;
								}
								else
								{
									pN++;
								}
							}
						}
						if(strlen(SSID) >= 1 && strlen(PASSWORD) >= 8)
						{
							if(!Esp_JoinAP(SSID,PASSWORD))
							{
								fun_res = 0;
								goto config_end;
							}
						}
					}
				}
			}
		}
		else
		{
			memset(ReadTemp,0,1024);
		}
	}

config_end:
	free(ReadTemp);
	ReadTemp = NULL;
	return fun_res;
}



#if 0
uint8_t Device_Config(char *pText)
{
	
}

uint8_t Get_Time(void)
{
	
}

static uint8_t Iot_SendCmd(char *Token, uint8_t status)
{
	char *pData = NULL;

	pData = malloc(512);

	Wifi_Connect();
	sprintf(pData,IOT_CMD,Token,status);
	Esp_LinkServer("iot.espressif.cn","80");
	Esp_TransStart();
	USART_SendStr(USART2,pData);
	Esp_BreakTrans();
	Esp_ExitServer();

	free(pData);
	pData = NULL;
	return 0;
}
#endif


uint8_t Wifi_Connect(void)
{
	uint8_t TryCnt = 0;
	
	while (1)
	{
		if (Esp_APtest())
		{
			LED_Control(1,0,0);
			///MP3_Play("0:/Iot_Box/temp/ner");
			if (WIFI_Config())
			{
				Esp_Reset();
				Esp_Init();
				TryCnt++;
				if (++TryCnt > 3)
				{
					Esp_Restore();
					Esp_Init();
					TryCnt = 0;
				}
			}
			else
			{
				//MP3_Play("0:/Iot_Box/temp/rea");
				LED_Control(0,0,0);
				break;
			}
		}
		else
		{
			break;
		}
	}
	return 0;
}


uint8_t URL_Encode(const char *utf, uint16_t num, char *url)
{
	uint16_t i=0;
	uint16_t j=0;
	const char HEX_Code[16]= {"0123456789ABCDEF"};
	for (i=0,j=0; i<num; i++,j+=3)
	{
		url[j+0] = '%';
		url[j+1] = HEX_Code[(uint8_t)utf[i] >> 4];
		url[j+2] = HEX_Code[(uint8_t)utf[i] & 15];
	}
	return 0;
}


uint8_t UTF_Encode(const char *uincode, uint16_t num, char *utf)
{
	uint16_t i=0;
	uint16_t j=0;
	uint16_t C_temp;
	for (i=0,j=0; j<num; i+=3,j+=2)
	{
		C_temp = (((uint8_t)uincode[j])<<8) + (uint8_t)uincode[j+1];
		utf[i+0] = (char)(((C_temp >> 0x0C) & 0x3F) | 0xE0);
		utf[i+1] = (char)(((C_temp >> 0x06) & 0x3F) | 0x80);
		utf[i+2] = (char)(((C_temp >> 0x00) & 0x3F) | 0x80);
	}
	return 0;
}


uint8_t UTF_Decode(const char *utf, uint16_t num, char *uincode)
{
	uint16_t i=0;
	uint16_t j=0;
	for (i=0,j=0; j<num; i+=2,j+=3)
	{
		uincode[i+0] = (char)(((((uint8_t)utf[j+0])&0x0F)<<0x04) + ((((uint8_t)utf[j+1])&0x3C)>>0x02));
		uincode[i+1] = (char)(((((uint8_t)utf[j+1])&0x03)<<0x06) + ((((uint8_t)utf[j+2])&0x3F)>>0x00));
	}
	return 0;
}


uint8_t Gbk2Unicode(const char *gbk, uint16_t num, char *uincode)
{
	uint16_t i;
	uint16_t C_temp;
	for (i=0; i<num; i+=2)
	{
		C_temp = (((uint8_t)gbk[i])<<8) + (uint8_t)gbk[i+1];
		C_temp = ff_convert(C_temp, 1);
		uincode[i] =(char)(C_temp >> 8);
		uincode[i+1] =(char)(C_temp & 0xFF);
	}
	return 0;
}


uint8_t Unicode2Gbk(const char *uincode, uint16_t num, char *gbk)
{
	uint16_t i;
	uint16_t C_temp;
	for (i=0; i<num; i+=2)
	{
		C_temp = (((uint8_t)uincode[i])<<8) + (uint8_t)uincode[i+1];
		C_temp = ff_convert(C_temp, 0);
		gbk[i] =(char)(C_temp >> 8);
		gbk[i+1] =(char)(C_temp & 0xFF);
	}
	return 0;
}

