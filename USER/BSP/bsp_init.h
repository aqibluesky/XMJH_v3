#ifndef __BSP_INIT_H
#define __BSP_INIT_H

#include "stm32f10x.h"
#include "SysTick.h"
#include "USART.h"
#include "timer.h"
#include "SD_Card.h"
#include "Esp8266.h"
#include "I2S.h"
#include "WM8978.h"

#include "ff.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define CACHE_SIZE 1152*4

extern volatile uint8_t I2S_Trans;
extern volatile uint8_t I2S_Cache;
extern volatile uint8_t USART_Trans;
extern volatile uint8_t USART_Cache;
//定义缓存
extern uint8_t Cache_0[CACHE_SIZE];
extern uint8_t Cache_1[CACHE_SIZE];

//FATFS文件系统相关变量
//---------------------------
extern FATFS fs;
extern FIL fsrc;
extern FRESULT res;
extern UINT br;
extern FILINFO finfo;
extern DIR dirs;
//---------------------------

void BSP_Init(void);

void LED_Init(void);
void LED_Control(uint8_t red, uint8_t green,uint8_t blue);


#endif
