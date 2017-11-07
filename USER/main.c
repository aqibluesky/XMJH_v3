#include "bsp_init.h"

#include "music_player.h"
#include "net_link.h"
#include "voice_recorder.h"


char pText[TXT_LEN*3];//用于存放识别出的UTF-8格式的编码一个中文字符占用三个字节

int main(void)
{
	BSP_Init();
	Wifi_Connect();
	while(1)
	{
		memset(pText,0,sizeof(pText));
		Voice_Record();						//开始录音
		Audio2Text(pText);
		
		if(strlen(pText) >= 9)
		{
			Text2Audio(pText);			//调用文字转语音API
			MP3_Play("0:/Iot_Box/temp/voc");//将识别结果复述一遍
			Music_Found(pText);			//寻找有无这个mp3文件，有就播放
		}
	}
}

