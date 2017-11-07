#include "voice_recorder.h"
#include "bsp_init.h"

static void WAV_Head_Init(WaveHeader *pwavhead);
static uint8_t VAD(int16_t *VocBuf);


void Voice_Record(void)
{
	uint32_t i,j;
	WaveHeader *pwavhead;
	char *TempBuf;
	uint8_t Status = 0; //0:静音中 1:录音开始 2:录音结束
	uint8_t VocCnt = 0;	//连续语音帧数统计
	uint8_t SliCnt = 0;	//连续静音帧数统计
	uint8_t RecCnt = 0; //录音帧数统计

	WM8978_RecoMode();
	pwavhead = (WaveHeader*)malloc(sizeof(WaveHeader));	//为WAV头文件分配内存
	TempBuf = malloc(CACHE_SIZE/2);
	WAV_Head_Init(pwavhead);
	f_unlink("0:/IoT_Box/temp/rec");
	f_open(&fsrc,"0:/IoT_Box/temp/rec",FA_CREATE_ALWAYS | FA_WRITE);
	f_write(&fsrc,pwavhead,sizeof(WaveHeader),&br);			//写入头数据
	if(pwavhead)
	{
		I2S_RX_Start(I2S_Standard_Phillips,I2S_DataFormat_16b,I2S_AudioFreq_8k,I2S_Mode_MasterRx);
		I2S_RX_DMA_Init(Cache_0,CACHE_SIZE/2);
		DMA_Cmd(DMA1_Channel4,ENABLE);
		I2S_Trans = 0;
		while(1)
		{
			while(I2S_Trans == 0)
			{
				; //等待DMA传输完成;
			}
			I2S_Trans = 0;//清除标志

			if(I2S_Cache)
			{
				for(i=0,j=0; j<CACHE_SIZE; i+=2,j+=4)
				{
					TempBuf[i]	 = Cache_0[j];
					TempBuf[i+1] = Cache_0[j+1];
				}
			}
			else
			{
				for(i=0,j=0; j<CACHE_SIZE; i+=2,j+=4)
				{
					TempBuf[i]	 = Cache_1[j];
					TempBuf[i+1] = Cache_1[j+1];
				}
			}
			/*
				DMA_BUFSIZE/2 == 1152*2 Byte 音频采样率是8kHz,量化位数是16位，
				那么一位占用2个字节，所以传输一次用时(1152*2)/(8000*2)= 0.144s = 144ms
				stm32的CPU时钟72000000Hz 在0.144s内运算72000000*0.144 = 10368000次
				所有的运算和数据写入SD卡都要在这段时间内完成，否则数据会溢出。
			*/
			//语音判决部分，模式选择
			if(VAD((int16_t *)TempBuf))//只要有一帧达到要求，就进入录音模式
			{
				Status = 1;
				VocCnt++;
				SliCnt = 0;
			}
			else
			{
				SliCnt++;
				VocCnt = 0;
				if(Status == 1)					//在语音录制时
				{
					if(SliCnt > 3)				//如果连续静音帧数到达3帧以上，则判定语音结束
					{
						Status = 2;
					}
				}
			}
			//根据状态进行处理
			if(Status == 0)
			{
				LED_Control(0,0,0);			//关闭指示灯
			}
			else if(Status == 1)			//如果处于录音状态则将语音写入文件
			{
				RecCnt++;								//写入次数加一
				LED_Control(0,0,1);			//开启蓝色指示灯
				res = f_write(&fsrc,TempBuf,CACHE_SIZE/2,&br);			//写入文件
			}
			else if((Status==2 && RecCnt>5) || RecCnt > 50)				//有效语音结束或者语音时间过长(50*0.144ms)就结束录音
			{
				LED_Control(0,0,0);																	//关闭指示灯
				pwavhead->riff.ChunkSize = RecCnt*CACHE_SIZE/2+36;	//整个文件的大小-8;
				pwavhead->data.ChunkSize	=	RecCnt*CACHE_SIZE/2;		//数据大小
				f_lseek(&fsrc,0);																		//偏移到文件头.
				f_write(&fsrc,pwavhead,sizeof(WaveHeader),&br);			//写入头数据
				f_close(&fsrc);
				RecCnt=0;
				break;
			}
			else											//如果录音结束且语音帧小于5(5*144ms)
			{
				LED_Control(0,0,0);			//关闭指示灯
				f_close(&fsrc);					//关闭文件
				f_unlink("0:/IoT_Box/temp/rec");		//该文件无效，删除
				f_open(&fsrc,"0:/IoT_Box/temp/rec",FA_CREATE_ALWAYS | FA_WRITE);			//重新创建
				f_write(&fsrc,pwavhead,sizeof(WaveHeader),&br);												//写入头数据
				Status = 0;							//转为录音未开始模式

				RecCnt=0;								//写入次数归零
				VocCnt = 0;							//连续语音帧数归零
				SliCnt = 0;							//连续静音帧数归零
			}
		}
		DMA_Cmd(DMA1_Channel4,ENABLE);
		I2S_RX_Stop();
	}
	free(TempBuf);
	free(pwavhead);
}


static void WAV_Head_Init(WaveHeader *pwavhead) //初始化WAV头
{
	pwavhead->riff.ChunkID = 0x46464952;					//"RIFF"
	pwavhead->riff.ChunkSize = 0;									//还未确定,最后需要计算
	pwavhead->riff.Format = 0x45564157;						//"WAVE"
	pwavhead->fmt.ChunkID = 0x20746D66;						//"fmt "
	pwavhead->fmt.ChunkSize = 16;									//大小为16个字节
	pwavhead->fmt.AudioFormat = 0x01;							//0X01,表示PCM;0X11,表示IMA ADPCM
	pwavhead->fmt.NumOfChannels = 1;							//声道
	pwavhead->fmt.SampleRate = 8000;							// 采样速率
	pwavhead->fmt.ByteRate = pwavhead->fmt.SampleRate*2;	//字节速率=采样率*通道数*(ADC位数/8)
	pwavhead->fmt.BlockAlign = 2;									//块大小=通道数*(ADC位数/8)
	pwavhead->fmt.BitsPerSample = 16;							//16位PCM
	pwavhead->data.ChunkID = 0x61746164;					//"data"
	pwavhead->data.ChunkSize = 0;									//数据大小,还需要计算
}


static uint8_t VAD(int16_t *VocBuf)
{
	uint32_t i;
	uint32_t	Ene = 0;					//短时能量
	uint32_t	Zer = 0;					//短时过零率
	for(i=0,Ene = 0,Zer = 0; i<FRAME_LEN; i++)
	{
		Ene += (VocBuf[i] > 0) ? VocBuf[i] : -VocBuf[i];//计算短时能量
		if((VocBuf[i] * VocBuf[i+1]) < 0)								//计算短时过零率
		{
			Zer++;
		}
	}
	//printf("\r\nEne:%d\r\nZer:%d",Ene,Zer);
	if(Ene > ENE_CHA && Zer > ZER_CHA)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

