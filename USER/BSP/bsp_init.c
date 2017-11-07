#include "bsp_init.h"

volatile uint8_t I2S_Trans = 0;	//I2S传输完成标志，0表示正在传输，1表示传输完毕
volatile uint8_t I2S_Cache = 0;	//指示正在使用那个缓存

volatile uint8_t USART_Trans = 0; //USART传输完成标志，0表示正在传输，1表示传输完毕
volatile uint8_t USART_Cache = 0; //指示正在使用那个缓存

//定义缓存
uint8_t Cache_0[CACHE_SIZE];
uint8_t Cache_1[CACHE_SIZE];

//FATFS文件系统相关变量
//---------------------------
FATFS fs;
FIL fsrc;
FRESULT res;
UINT br,bw;			//用于记录具体到底读出或者写入了多少个字节
FILINFO finfo;
DIR dirs;
//---------------------------

static void File_Mount(void);
const static char FileConfig[] = {"#Voice IoT Controller \r\n#Firmware version: 0.3\r\n#Copyright statement: \r\n#This device and code can only be used for learning and presentation.\r\n#For business cooperation, please send E-mail to chenqt123@qq.com.\r\n#Copyright 2017 Terry Chen.\r\n\r\n\r\nWIFI:\r\n>>\r\n-S YourSsid -P YourPassword\r\n<<\r\n"};		//配置文件示例

void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SysTick_Init();
	USART1_Init(115200);
	LED_Init();
	TIM2_Init(9999,7199);
	SD_Init();
	Esp_Init();
	WM8978_Init();
	File_Mount();
}

void File_Mount(void)
{
	f_mount(&fs, "0:", 1);//挂载SD卡
	res = f_opendir(&dirs, "0:/IoT_Box");
	if (res == FR_NO_PATH)
	{
		f_mkdir("0:/IoT_Box");
		f_mkdir("0:/IoT_Box/temp");
	}
	res = f_open(&fsrc, "0:/IoT_Box/config", FA_CREATE_NEW | FA_WRITE);//创建配置文件
	if (res == FR_OK)
	{
		f_write(&fsrc,FileConfig,sizeof(FileConfig),&br);
		f_close(&fsrc);
	}
	res = f_opendir(&dirs, "0:/music");
	if (res == FR_NO_PATH)
	{
		f_mkdir("0:/music");
	}
}

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	//红绿蓝
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);
}


void LED_Control(uint8_t red, uint8_t green,uint8_t blue)
{
	if(red)
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_1);
	}
	else
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_1);
	}
	if(green)
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_2);
	}
	else
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_2);
	}
	if(blue)
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_3);
	}
	else
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_3);
	}
}


#if 0
/* LSI: 40 000Hz 4s*/
static void prvIWDG_Init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	IWDG_SetReload(625);
	IWDG_ReloadCounter();
	IWDG_Enable();
}


void vIWDG_Feed(void)
{
	IWDG_ReloadCounter();
}

/*--------------------------------------------------*/
#define WWDG_COUNTER			0x7F
#define WWDG_WINDOW_VALUE		0x41

/* Fwwdg = 36 000 000 / (4096 * Prescaler). */
void vWWDG_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	WWDG_SetPrescaler(WWDG_Prescaler_8);		/* 1096Hz */
	WWDG_SetWindowValue(WWDG_WINDOW_VALUE);
	WWDG_Enable(WWDG_COUNTER);
	WWDG_ClearFlag();
	
	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
	NVIC_Init(&NVIC_InitStructure);
	
	WWDG_EnableIT();  
}

void WWDG_IRQHandler(void)
{
	WWDG_SetCounter(WWDG_COUNTER);
	WWDG_ClearFlag();
	printf("Entry \"WWDG_IRQHandler\" !r\n");
}
/*--------------------------------------------------*/
#endif

void vBspFLASH_Protection(void)
{
	if (FLASH_GetReadOutProtectionStatus() != SET)
	{
		FLASH_Unlock();
		if (FLASH_ReadOutProtection(ENABLE) != FLASH_COMPLETE)
		{
			NVIC_SystemReset();
		}
		else
		{
			/*protection set --> reset device to enable protection*/
			NVIC_SystemReset();
		}
	}
}


void vBspDelayByDiv(void)
{
	float x = 50.0f;

	while (x > 0.0001f)
	{
		x = x/1.0001f;
	}
}


#ifdef USE_FULL_ASSERT
#include "printf-stdarg.h"
/**
* @brief Reports the name of the source file and the source line number
* where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	printf("Wrong parameters value: file %s on line %d\r\n", file, (int)line);
	__set_PRIMASK(1);
	for (;;)
	{
		;
	}
}
#endif

