#include "net_link.h"


//该函数用于获取访问百度语音服务器的令牌
static uint8_t Get_Token(char *pToken);
//用于获取wifi配置
static uint8_t WIFI_Config(void);
//gbk字符对比函数
static uint8_t Word_Cmpare(char *pText,char *pKey);
//用此函数通过HTTP协议向IOT服务器发送数据
static uint8_t Iot_SendCmd(char *Token, uint8_t status);


const char GET_TOKEN[] = {"GET /oauth/2.0/token?grant_type=client_credentials&client_id=SlHEpH9hw92SNsrpjGszg7RU&client_secret=5d8da4956a8c104ebb7d9d43c6604554 HTTP/1.1\r\nHost: openapi.baidu.com\r\n\r\n"};

const char AUDIO2TEXT[] = {"POST /server_api?lan=zh&cuid=chenqt123&token=%s HTTP/1.1\r\nHost: vop.baidu.com\r\nContent-Type: wav;rate=8000\r\nContent-Length: %s\r\n\r\n"};

const char TEXT2AUDIO[]	= {"GET /text2audio?tex=%s&lan=zh&cuid=001&ctp=1&tok=%s HTTP/1.1\r\nHost: tsn.baidu.com\r\n\r\n"};

const char IOT_CMD[] = {"POST /v1/datastreams/plug-status/datapoint/?deliver_to_device=true HTTP/1.1\r\nHost: iot.espressif.cn\r\nContent-Type: application/json\r\nAuthorization: token %s\r\ncontent-length: 21\r\n\r\n{\"datapoint\":{\"x\":%1d}}\r\n\r\n"};

const char TIME_API[] = {"GET /?app=life.time&amp;appkey=10003&amp;sign=b59bc3ef6191eb9f747dd4e83c99f2a4&amp;format=json\r\nHTTP/1.1\r\nHost: api.k780.com:88\r\n\r\n"};

const char WEATHER_API[] = {"GET /data/cityinfo/101210101.html HTTP/1.1\r\nHost: www.weather.com.cn\r\n\r\n"};



static uint8_t Word_Cmpare(char *pText,char *pKey)
{
	char GBKTemp[2];//用于缓存一个GBK字符
	char UNITemp[2];//用于存放转换后的UNI格式字符
	char UTFTemp[3];//用于存放转换后的UTF格式字符
	char *pK = NULL;

	for(pK = pKey; *pK != '\0'; pK+=2)
	{
		GBKTemp[0] = *pK;
		GBKTemp[1] = *(pK+1);
		while((uint8_t)*pK < 0x81 && *pK != '\0')//GBK字符的有效范围是8140-FEFE，这里只做简单的判断
		{
			GBKTemp[0] = *pK+1;
			GBKTemp[1] = *(pK+2);
			pK++;
		}
		Gbk2Unicode(GBKTemp,2,UNITemp);
		UTF_Encode(UNITemp,2,UTFTemp);
		if(!strstr(pText,UTFTemp))
		{
			return 1;
		}
	}
	return 0;
}




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
	res = f_open(&fsrc,"0:/IoT_Box/temp/test",FA_READ);
	//res = f_open(&fsrc,"0:/IoT_Box/temp/rec",FA_READ);
	memset(ConLen,0,sizeof(ConLen));
	sprintf(ConLen,"%d",(int)fsrc.fsize);
	memset((char *)Cache_0,0,CACHE_SIZE);
	sprintf((char *)Cache_0,AUDIO2TEXT,pToken,ConLen);
	//Esp_LinkServer("vop.baidu.com","80");
	Esp_LinkServer("192.168.191.1","80");
	Esp_TransMode(0);
//	if(Esp_TransBuf((char *)Cache_0, strlen((char *)Cache_0)))
//	{
//		Esp_ExitServer();
//		return 1;
//	}
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

	res = f_open(&fsrc,"0:/IoT_Box/config.txt",FA_READ);
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


uint8_t Device_Config(char *pText)
{
	char *pN = 0;
	char *pW = 0;
	char *pT = 0;
	char *pS = 0;
	char *ReadTemp = 0;
	char *Text = 0;
	char *Token = 0;

	uint8_t status = 0;
	uint8_t res = 1;
	uint16_t i = 0;

	Wifi_Connect();
	res = f_open(&fsrc,"0:/IoT_Box/config.txt",FA_READ);
	ReadTemp = malloc(1024);
	Text = malloc(TXT_LEN*2);
	Token = malloc(40);

	memset(ReadTemp,0,1024);
	memset(Token,0,40);

	while(f_gets(ReadTemp,1024,&fsrc))
	{
		if(strstr(ReadTemp,"DEVICE:") == ReadTemp)
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
						else if(strstr(ReadTemp,"-W") && strstr(ReadTemp,"-T") && strstr(ReadTemp,"-S"))
						{
							pW = strstr(ReadTemp,"-W");
							pT = strstr(ReadTemp,"-T");
							pS = strstr(ReadTemp,"-S");
							pN = pW+2;
							memset(Text,0,TXT_LEN*2);
							i = 0;
							while(pN < pT)
							{
								if(*pN != ' ' && i < TXT_LEN*2 )
								{
									*(Text+i) = *pN;
									*(Text+i+1) = *(pN+1);
									i+=2;
									pN+=2;
								}
								else
								{
									pN++;
								}
							}
							pN = pT+2;
							memset(Token,0,40);
							i = 0;
							while(pN < pS)
							{
								if(*pN != ' ')
								{
									Token[i] = *pN;
									i++;
									pN++;
								}
								else
								{
									pN++;
								}
							}
							pN = pS+2;
							while(*pS)
							{
								if(*pN != ' ')
								{
									if(*pN == '0')
									{
										status = 0;
										break;
									}
									if(*pN == '1')
									{
										status = 1;
										break;
									}
									else
									{
										goto config_end;
									}
								}
								else
								{
									pN++;
								}
							}
						}
						if(strlen(Text) >= 2 && strlen(Token) == 40)
						{
							if(!Word_Cmpare(pText,Text))
							{
								Iot_SendCmd(Token,status);
								res = 0;
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
	free(Text);
	free(Token);
	return res;
}


const char WEATHER_RD[] =
{
	0xE4, 0xBB, 0x8A, 0xE6, 0x97, 0xA5,  '%',  's', 0xE5, 0xA4, 0xA9, 0xE6, 0xB0, 0x94,
	0xE6, 0x9C, 0x80, 0xE4, 0xBD, 0x8E, 0xE6, 0xB8, 0xA9, 0xE5, 0xBA, 0xA6,  '%',  's',
	0xE6, 0x91, 0x84, 0xE6, 0xB0, 0x8F, 0xE5, 0xBA, 0xA6,
	0xE6, 0x9C, 0x80, 0xE9, 0xAB, 0x98, 0xE6, 0xB8, 0xA9, 0xE5, 0xBA, 0xA6,  '%',  's',
	0xE6, 0x91, 0x84, 0xE6, 0xB0, 0x8F, 0xE5, 0xBA, 0xA6,
	'%',  's'
};//今日天气最低温度摄氏度最高温度摄氏度


uint8_t Get_Weather(char *pText)
{
	char *pS = NULL;
	char *pE = NULL;
	char City[16];
	char Temp1[16];
	char Temp2[16];
	char Weather[16];
	uint8_t CopyNum = 0;

	Wifi_Connect();
	Esp_LinkServer("www.weather.com.cn","80");
	Esp_TransStart();
	USART_SendStr(USART2,(char *)WEATHER_API);

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

	//拷贝城市
	pS = strstr((char *)Cache_0,"\"city\":\"");
	pE = strstr((char *)Cache_0,"\",\"cityid\":");
	if(pE > pS)
	{
		memset(City, 0, sizeof(City));
		CopyNum = (uint8_t)(pE - (pS + 8)) < 16 ? (uint8_t)(pE - (pS + 8)) : 16;
		strncpy(City,(char *)(pS + 8),CopyNum);
		pS = NULL;
		pE = NULL;
	}
	//拷贝最低温度
	pS = strstr((char *)Cache_0,"\"temp1\":\"");
	pE = strstr((char *)Cache_0,"\",\"temp2\":\"");
	if(pE > pS)
	{
		memset(Temp1, 0, sizeof(Temp1));
		CopyNum = (uint8_t)(pE - (pS + 9 + 3)) < 16 ? (uint8_t)(pE - (pS + 9 + 3)) : 16;
		strncpy(Temp1,(char *)(pS + 9),CopyNum);
		pS = NULL;
		pE = NULL;
	}
	//拷贝最高温度
	pS = strstr((char *)Cache_0,"\"temp2\":\"");
	pE = strstr((char *)Cache_0,"\",\"weather\":\"");
	if(pE > pS)
	{
		memset(Temp2, 0, sizeof(Temp2));
		CopyNum = (uint8_t)(pE - (pS + 9 + 3)) < 16 ? (uint8_t)(pE - (pS + 9 + 3)) : 16;
		strncpy(Temp2,(char *)(pS + 9),CopyNum);
		pS = NULL;
		pE = NULL;
	}
	//拷贝天气
	pS = strstr((char *)Cache_0,"\"weather\":\"");
	pE = strstr((char *)Cache_0,"\",\"img1\":\"");
	if(pE > pS)
	{
		memset(Weather, 0, sizeof(Weather));
		CopyNum = (uint8_t)(pE - (pS + 11)) < 16 ? (uint8_t)(pE - (pS + 11)) : 16;
		strncpy(Weather,(char *)(pS + 11),CopyNum);
		pS = NULL;
		pE = NULL;
	}
	snprintf(pText,TXT_LEN*2,WEATHER_RD ,City,Temp1,Temp2,Weather);
	return 0;
}


//uint8_t Get_Time(void)
//{
//
//}



uint8_t Wifi_Connect(void)
{
	uint8_t TryCnt = 0;

	while(1)
	{
		if(Esp_APtest())
		{
			LED_Control(1,0,0);
			MP3_Play("0:/Iot_Box/temp/ner");
			if(WIFI_Config())
			{
				Esp_Reset();
				Esp_Init();
				TryCnt++;
				if(++TryCnt > 3)
				{
					Esp_Restore();
					Esp_Init();
					TryCnt = 0;
				}
			}
			else
			{
				MP3_Play("0:/Iot_Box/temp/rea");
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
	for(i=0,j=0; i<num; i++,j+=3)
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
	for(i=0,j=0; j<num; i+=3,j+=2)
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
	for(i=0,j=0; j<num; i+=2,j+=3)
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
	for(i=0; i<num; i+=2)
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
	for(i=0; i<num; i+=2)
	{
		C_temp = (((uint8_t)uincode[i])<<8) + (uint8_t)uincode[i+1];
		C_temp = ff_convert(C_temp, 0);
		gbk[i] =(char)(C_temp >> 8);
		gbk[i+1] =(char)(C_temp & 0xFF);
	}
	return 0;
}





