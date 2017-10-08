#ifndef __MUSIC_PLAYER_H
#define __MUSIC_PLAYER_H

#include "stm32f10x.h"


//MP3文件相关定义
#define MP3_TITSIZE_MAX		40		//歌曲名字最大长度
#define MP3_ARTSIZE_MAX		40		//歌曲名字最大长度
#define MP3_DECODE_SIZE		2*1024

#define AUDIO_MIN(x,y)	((x)<(y)? (x):(y))

//ID3V1 标签
typedef __packed struct
{
	uint8_t id[3];		   	//ID,TAG三个字母
	uint8_t title[30];		//歌曲名字
	uint8_t artist[30];		//艺术家名字
	uint8_t year[4];			//年代
	uint8_t comment[30];		//备注
	uint8_t genre;			//流派
} ID3V1_Tag;

//ID3V2 标签头
typedef __packed struct
{
	uint8_t id[3];		   	//ID
	uint8_t mversion;		//主版本号
	uint8_t sversion;		//子版本号
	uint8_t flags;			//标签头标志
	uint8_t size[4];			//标签信息大小(不包含标签头10字节).所以,标签大小=size+10.
} ID3V2_TagHead;

//ID3V2.3 版本帧头
typedef __packed struct
{
	uint8_t id[4];		   	//帧ID
	uint8_t size[4];			//帧大小
	uint16_t flags;			//帧标志
} ID3V23_FrameHead;

//MP3 Xing帧信息(没有全部列出来,仅列出有用的部分)
typedef __packed struct
{
	uint8_t id[4];		   	//帧ID,为Xing/Info
	uint8_t flags[4];		//存放标志
	uint8_t frames[4];		//总帧数
	uint8_t fsize[4];		//文件总大小(不包含ID3)
} MP3_FrameXing;

//MP3 VBRI帧信息(没有全部列出来,仅列出有用的部分)
typedef __packed struct
{
	uint8_t id[4];		   	//帧ID,为Xing/Info
	uint8_t version[2];		//版本号
	uint8_t delay[2];		//延迟
	uint8_t quality[2];		//音频质量,0~100,越大质量越好
	uint8_t fsize[4];		//文件总大小
	uint8_t frames[4];		//文件总帧数
} MP3_FrameVBRI;


//MP3控制结构体
typedef __packed struct
{
	uint8_t title[MP3_TITSIZE_MAX];		//歌曲名字
	uint8_t artist[MP3_ARTSIZE_MAX];	//艺术家名字
	uint32_t totsec ;									//整首歌时长,单位:秒
	uint32_t bitrate;	   							//比特率
	uint32_t samplerate;							//采样率
	uint16_t outsamples;							//PCM输出数据量大小(以16位为单位),单声道MP3,则等于实际输出*2(方便DAC输出)
	uint32_t datastart;								//数据帧开始的位置(在文件里面的偏移)
} MP3_Para;


//WAV文件相关定义
//RIFF块
typedef __packed struct
{
	uint32_t ChunkID;		   	//chunk id;这里固定为"RIFF",即0X46464952
	uint32_t ChunkSize ;		   	//集合大小;文件总大小-8
	uint32_t Format;	   			//格式;WAVE,即0X45564157
} ChunkRIFF ;
//fmt块
typedef __packed struct
{
	uint32_t ChunkID;		   	//chunk id;这里固定为"fmt ",即0X20746D66
	uint32_t ChunkSize ;		   //子集合大小(不包括ID和Size);
	uint16_t AudioFormat;	  	//音频格式;0X01,表示线性PCM;0X11表示IMA ADPCM
	uint16_t NumOfChannels;		//通道数量;1,表示单声道;2,表示双声道;
	uint32_t SampleRate;			//采样率;0X1F40,表示8Khz
	uint32_t ByteRate;			//字节速率;
	uint16_t BlockAlign;			//块对齐(字节);
	uint16_t BitsPerSample;		//单个采样数据大小;4位ADPCM,设置为4
//	uint16_t ByteExtraData;		//附加的数据字节;2个; 线性PCM,没有这个参数
} ChunkFMT;
//fact块
typedef __packed struct
{
	uint32_t ChunkID;		   	//chunk id;这里固定为"fact",即0X74636166;
	uint32_t ChunkSize ;		   	//子集合大小(不包括ID和Size);这里为:4.
	uint32_t NumOfSamples;	  	//采样的数量;
} ChunkFACT;
//LIST块
typedef __packed struct
{
	uint32_t ChunkID;		   	//chunk id;这里固定为"LIST",即0X74636166;
	uint32_t ChunkSize ;		   	//子集合大小(不包括ID和Size);这里为:4.
} ChunkLIST;

//data块
typedef __packed struct
{
	uint32_t ChunkID;		   	//chunk id;这里固定为"data",即0X5453494C
	uint32_t ChunkSize ;		   	//子集合大小(不包括ID和Size)
} ChunkDATA;

//wav头
typedef __packed struct
{
	ChunkRIFF riff;	//riff块
	ChunkFMT fmt;  	//fmt块
//	ChunkFACT fact;	//fact块 线性PCM,没有这个结构体
	ChunkDATA data;	//data块
} WaveHeader;


//WAV播放参数结构体
typedef __packed struct
{
	uint16_t audioformat;			//音频格式;0X01,表示线性PCM;0X11表示IMA ADPCM
	uint16_t nchannels;				//通道数量;1,表示单声道;2,表示双声道;
	uint16_t blockalign;				//块对齐(字节);
	uint32_t datasize;				//WAV数据大小
	uint32_t totsec ;				//整首歌时长,单位:秒
	uint32_t cursec ;				//当前播放时长
	uint32_t bitrate;	   			//比特率(位速)
	uint32_t samplerate;				//采样率
	uint16_t bps;					//位数,比如16bit,24bit,32bit
	uint32_t datastart;				//数据帧开始的位置(在文件里面的偏移)
} WAV_Para;


uint8_t Music_Play(void);
uint8_t Music_Found(char *pText);
uint8_t WAV_Play(char *PathBuf);
uint8_t MP3_Play(char *PathBuf);


#endif
