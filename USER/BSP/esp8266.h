#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"


void Esp_Init(void);
void Esp_Reset(void);
void Esp_Sleep(uint8_t mode);


uint8_t Esp_ATtest(void);
uint8_t Esp_Restore(void);
uint8_t Esp_EnterSmartConfig(void);
uint8_t Esp_ExitSmartConfig(void);
uint8_t Esp_StationMode(void);
uint8_t Esp_SetBaudRate(uint32_t BaudRate);
uint8_t Esp_LinkServer(const char *ip, const char *port);
uint8_t Esp_TransStart(void);
uint8_t Esp_ExitServer(void);
uint8_t Esp_APtest(void);
uint8_t Esp_BreakTrans(void);
uint8_t Esp_JoinAP (const char *pSSID, const char *pPassWord);
uint8_t Esp_SmartConfig(void);
uint8_t Esp_TransMode(uint8_t mode);
uint8_t Esp_TransBuf(char *pBuf, uint32_t length);


#endif
