#ifndef __NET_LINK_H
#define __NET_LINK_H

#include "bsp_init.h"
#include "music_player.h"

#define TXT_LEN 64		//一次转换最多的中文字数


//用于连接wifi
uint8_t Wifi_Connect(void);
//用于根据识别结果控制物联网设备
uint8_t Device_Config(char* pText);

//获取天气
uint8_t Get_Weather(char *pText);
//用于实现语音转文字以及文字转语音
uint8_t Audio2Text(char* pText);
uint8_t Text2Audio(char* pText);


//以下的函数用于不用的编码格式的转换，部分函数需要FATFS文件系统中的cc936.c文件支持
//版权@terry chen
uint8_t URL_Encode(const char *utf, uint16_t num, char *url);
uint8_t UTF_Encode(const char *uincode, uint16_t num, char *utf);
uint8_t UTF_Decode(const char *utf, uint16_t num, char *uincode);
uint8_t Gbk2Unicode(const char *gbk, uint16_t num, char *uincode);
uint8_t Unicode2Gbk(const char *uincode, uint16_t num, char *gbk);


#endif
