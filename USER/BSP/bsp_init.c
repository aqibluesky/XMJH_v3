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

const static char FileConfig[] = {""};

void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SysTick_Init();
	USART1_Init(115200);
	LED_Init();
	TIM2_Init(9999,7199);
	SD_Init();
	File_Mount();
	Esp_Init();
	WM8978_Init();
}

void File_Mount(void)
{
	f_mount(&fs, "0:", 1);//挂载SD卡
	res = f_opendir(&dirs,"0:/IoT_Box");
	if(res == FR_NO_PATH)
	{
		f_mkdir("0:/IoT_Box");
		f_mkdir("0:/IoT_Box/temp");
	}
	res = f_open(&fsrc,"0:/IoT_Box/config.txt",FA_CREATE_NEW | FA_WRITE);
	if(res == FR_OK)
	{
		f_write(&fsrc,FileConfig,sizeof(FileConfig),&br);
		f_close(&fsrc);
	}
	res = f_opendir(&dirs,"0:/music");
	if(res == FR_NO_PATH)
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

