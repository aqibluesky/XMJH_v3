#include "music_player.h"
#include "mp3dec.h"
#include "net_link.h"
#include "bsp_init.h"

static WAV_Para wavpara;//WAV文件音频参数结构体
static MP3_Para mp3para;//WAV文件音频参数结构体

static uint8_t MP2_ID3V1_Decode(uint8_t *buf);
static uint8_t MP2_ID3V2_Decode(uint8_t *buf,UINT br);
static uint8_t MP3_Get_Info(char *PathBuf);

static uint8_t WAV_Get_Info(char *PathBuf);
static uint8_t Word_Cmpare(char *pText,char *pKey);

uint8_t Music_Play(void)
{
	uint8_t fun_res = 0;
	char* PathBuf = NULL;

	WM8978_PlayMode();
	//为长文件名分配内存
	finfo.lfname = malloc(_MAX_LFN*2 + 1);
	finfo.lfsize = _MAX_LFN*2 + 1;
	//为路径字符串分配内存
	PathBuf = malloc(512);
	if(!(PathBuf && finfo.lfname))
	{
		fun_res = 1;
		goto end_point;
	}
	res = f_opendir(&dirs,"0:/music");
	while(f_readdir(&dirs,&finfo) == FR_OK)
	{
		if(finfo.fattrib & AM_ARC)	//如果文件属性为档案
		{
			if(!finfo.fname[0])		//如果文件名为空则退出
				break;
			if( strstr(*finfo.lfname ? finfo.lfname : finfo.fname, ".wav") != NULL)
			{
				strcpy(PathBuf,"0:/music");
				strcat(PathBuf,"/");
				strcat(PathBuf,*finfo.lfname ? finfo.lfname : finfo.fname);
				printf("%s\r\n", PathBuf);
				fun_res = WAV_Play(PathBuf);
			}
			if( strstr(*finfo.lfname ? finfo.lfname : finfo.fname, ".mp3") != NULL)
			{
				strcpy(PathBuf,"0:/music");
				strcat(PathBuf,"/");
				strcat(PathBuf,*finfo.lfname ? finfo.lfname : finfo.fname);
				printf("%s\r\n", PathBuf);
				fun_res = MP3_Play(PathBuf);
			}
		}
	}

end_point:
	free(PathBuf);
	free(finfo.lfname);
	PathBuf = NULL;
	finfo.lfname = NULL;
	return fun_res;
}


static uint8_t Word_Cmpare(char *pText,char *pKey)
{
	char GBKTemp[2];//用于缓存一个GBK字符
	char UNITemp[2];//用于存放转换后的UNI格式字符
	char UTFTemp[3];//用于存放转换后的UTF格式字符
	char *pK = NULL;
	char *pT = NULL;
	char *pS = NULL;
	uint8_t KeyCnt = 0;//用于统计PKey的长度
	uint8_t error = 0;//不匹配字符统计

	for(pT = pText, pK = pKey; *pK != '\0'; pK+=2)
	{
		GBKTemp[0] = *pK;
		GBKTemp[1] = *(pK+1);
		while((uint8_t)*pK < 0x81 && *pK != '\0')//GBK字符的有效范围是8140-FEFE，这里只做简单的判断
		{
			GBKTemp[0] = *pK+1;
			GBKTemp[1] = *(pK+2);
			pK++;
		}
		if(*pK == '\0')
		{
			break;
		}
		KeyCnt++;
		Gbk2Unicode(GBKTemp,2,UNITemp);
		UTF_Encode(UNITemp,2,UTFTemp);
		pS = strstr(pT,UTFTemp);
		if(!pS)
		{
			error++;
		}
		else
		{
			pT = pS + 3;
		}
	}
	if(error >= KeyCnt/2)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


uint8_t Music_Found(char *pText)
{
	uint8_t fun_res = 0;
	char* PathBuf = NULL;

	WM8978_PlayMode();
	//为长文件名分配内存
	finfo.lfname = malloc(_MAX_LFN*2 + 1);
	finfo.lfsize = _MAX_LFN*2 + 1;
	//为路径字符串分配内存
	PathBuf = malloc(512);
	if(! PathBuf && finfo.lfname)
	{
		fun_res = 1;
		goto end_point;
	}
	res = f_opendir(&dirs,"0:/music");
	while(f_readdir(&dirs,&finfo) == FR_OK)
	{
		if(finfo.fattrib & AM_ARC)	//如果文件属性为档案
		{
			if(!finfo.fname[0])		//如果文件名为空则退出
				break;

			if(strstr(*finfo.lfname ? finfo.lfname : finfo.fname, ".mp3") != NULL)
			{
				strcpy(PathBuf,"0:/music");
				strcat(PathBuf,"/");
				strcat(PathBuf,*finfo.lfname ? finfo.lfname : finfo.fname);
				//printf("%s\r\n", PathBuf);
				if(!Word_Cmpare(pText,*finfo.lfname ? finfo.lfname : finfo.fname))
				{
					fun_res = MP3_Play(PathBuf);
				}
			}
			if(strstr(*finfo.lfname ? finfo.lfname : finfo.fname, ".wav") != NULL)
			{
				strcpy(PathBuf,"0:/music");
				strcat(PathBuf,"/");
				strcat(PathBuf,*finfo.lfname ? finfo.lfname : finfo.fname);
				//printf("%s\r\n", PathBuf);
				if(!Word_Cmpare(pText,*finfo.lfname ? finfo.lfname : finfo.fname))
				{
					fun_res = WAV_Play(PathBuf);
				}
			}
		}
	}

end_point:
	free(PathBuf);
	free(finfo.lfname);
	PathBuf = NULL;
	finfo.lfname = NULL;
	return fun_res;
}

//WAV解析初始化
//返回值:0,成功;1,内存分配失败;2,文件不正常;3,DATA区域未找到.
static uint8_t WAV_Get_Info(char *PathBuf)
{
	uint8_t fun_res = 0;
	uint8_t * temp_buf = NULL;
	uint32_t br=0;

	ChunkRIFF *riff = NULL;
	ChunkFMT *fmt = NULL;
	ChunkFACT *fact = NULL;
	ChunkDATA *data = NULL;

	temp_buf = malloc(512);

	if(!temp_buf)
	{
		fun_res = 1;
		goto end_point;
	}
	res = f_open(&fsrc,PathBuf,FA_READ);
	f_read(&fsrc, temp_buf, 512, &br);
	riff = (ChunkRIFF *)temp_buf;	//获取RIFF块
	if(riff->Format==0X45564157)	//是WAV文件
	{
		fmt = (ChunkFMT *)(temp_buf+12);	//获取FMT块
		fact = (ChunkFACT *)(temp_buf+12+8+fmt->ChunkSize);//读取FACT块
		if(fact->ChunkID==0X74636166 || fact->ChunkID==0X5453494C)
		{
			wavpara.datastart=12+8+fmt->ChunkSize+8+fact->ChunkSize;
		}
		else
		{
			wavpara.datastart=12+8+fmt->ChunkSize;
		}
		data=(ChunkDATA *)(temp_buf+wavpara.datastart);	//读取DATA块
		if(data->ChunkID==0X61746164)								//解析成功!
		{
			wavpara.audioformat=fmt->AudioFormat;			//音频格式
			wavpara.nchannels=fmt->NumOfChannels;			//通道数
			wavpara.samplerate=fmt->SampleRate;				//采样率
			wavpara.bitrate=fmt->ByteRate*8;					//得到位速
			wavpara.blockalign=fmt->BlockAlign;				//块对齐
			wavpara.bps=fmt->BitsPerSample;						//位数,16/24/32位
			wavpara.datasize=data->ChunkSize;					//数据块大小
			wavpara.datastart=wavpara.datastart+8;		//数据流开始的地方
//			printf("wavx->audioformat:%d\r\n",wavpara.audioformat);
//			printf("wavx->nchannels:%d\r\n",wavpara.nchannels);
//			printf("wavx->samplerate:%d\r\n",wavpara.samplerate);
//			printf("wavx->bitrate:%d\r\n",wavpara.bitrate);
//			printf("wavx->blockalign:%d\r\n",wavpara.blockalign);
//			printf("wavx->bps:%d\r\n",wavpara.bps);
//			printf("wavx->datasize:%d\r\n",wavpara.datasize);
//			printf("wavx->datastart:%d\r\n",wavpara.datastart);
		}
		else
		{
			fun_res = 3;
			goto end_point;
		}
	}
	else
	{
		fun_res = 2;
		goto end_point;
	}

end_point:
	f_close(&fsrc);
	free(temp_buf);
	return fun_res;
}


static uint32_t WAV_Buffill(uint8_t *Buf, uint16_t Size)
{
	uint32_t NeedReadSize = 0;
	uint32_t ReadSize = 0;
	uint32_t i = 0;
	uint8_t *p = NULL; //用于指向TempBuf
	uint8_t *TempBuf = NULL;

	TempBuf = malloc(CACHE_SIZE);
	if(! TempBuf)
	{
		goto end_point;
	}
	//stm32是小端模式,Wave文件低地址位上放的是音频数据的低位
	//先放左声道，再放右声道，I2S传输时，先传数据的高位，再传低位以16位为单位传输
	//双声道
	if(wavpara.nchannels == 2)
	{
		if(wavpara.bps == 16)
		{
			f_read(&fsrc,Buf,Size,(UINT*)&ReadSize);
		}
		//8位数据是无符号的整型数，如果不加处理绘制出来的图形是以0x80为中心上下波动的波形
		//16位、24位和32位数据都是有符号的整形数，这样的数据绘制的图形是以0值为中心上下波动的波形
		else
		{
			goto end_point;
		}
	}
	//单声道，调整为双声道数据进行播放
	else
	{
		if(wavpara.bps == 16)
		{
			NeedReadSize = Size/2;
			//此次要读取的字节数
			f_read(&fsrc,TempBuf,NeedReadSize,(UINT*)&ReadSize);
			p = TempBuf;
			ReadSize=ReadSize*2;
			//填充后的大小.
			for(i=0; i<ReadSize;)
			{
				Buf[i+0] = p[0];
				Buf[i+1] = p[1];
				Buf[i+2] = p[0];
				Buf[i+3] = p[1];
				i+=4;
				p=p+2;
			}
		}
		else
		{
			goto end_point;
		}

	}
	if(ReadSize < Size)//不够数据了,补充0
	{
		for(i = ReadSize; i < Size-ReadSize; i++)
		{
			Buf[i]=0;
		}
	}

end_point:
	free(TempBuf);
	TempBuf = NULL;
	return ReadSize;
}


uint8_t WAV_Play(char *PathBuf)
{
	uint8_t fun_res = 0;
	uint32_t fillnum = 0;

	WM8978_PlayMode();
	fun_res = WAV_Get_Info(PathBuf);
	I2S_TX_Start(I2S_Standard_Phillips,I2S_DataFormat_16b,wavpara.samplerate,I2S_Mode_MasterTx);
	//配置TX DMA,前面设置着SPI2为一个字，这里是字节，所以传输的数量要除以2
	I2S_TX_DMA_Init(Cache_0,CACHE_SIZE/2);
	DMA_Cmd(DMA2_Channel2,DISABLE);
	res = f_open(&fsrc,PathBuf,FA_READ);			//打开文件
	res = f_lseek(&fsrc,wavpara.datastart);		//跳过文件头

	//填充缓冲区
	fillnum = WAV_Buffill(Cache_0,CACHE_SIZE);
	fillnum = WAV_Buffill(Cache_1,CACHE_SIZE);
	DMA_Cmd(DMA2_Channel2,ENABLE);

	I2S_Trans = 0;
	while(res == 0)
	{
		while(I2S_Trans == 0)
		{
			; //等待DMA传输完成;
		}
		I2S_Trans = 0;
		if(I2S_Cache)
		{
			fillnum = WAV_Buffill(Cache_0,CACHE_SIZE);
		}
		else
		{
			fillnum = WAV_Buffill(Cache_1,CACHE_SIZE);
		}
		if(fillnum != CACHE_SIZE) //播放结束
		{
			break;
		}
	}
	while(I2S_Trans == 0)
	{
		; //等待DMA传输完成;
	}
	I2S_Trans = 0;
	while(DMA2_Channel2->CNDTR > CACHE_SIZE-br)//最后一次读多少传输多少
	{
		;
	}
	DMA_Cmd(DMA2_Channel2,DISABLE);
	I2S_TX_Stop();
	return fun_res;
}


//解析ID3V1
//buf:输入数据缓存区(大小固定是128字节)
//pctrl:MP3控制器
//返回值:0,获取正常 其他,获取失败
static uint8_t MP2_ID3V1_Decode(uint8_t *buf)
{
	ID3V1_Tag *id3v1tag = NULL;
	id3v1tag = (ID3V1_Tag*)buf;

	if (strncmp("TAG",(char*)id3v1tag->id,3) == 0)//是MP3 ID3V1 TAG
	{
		if(id3v1tag->title[0])
		{
			strncpy((char*)mp3para.title,(char*)id3v1tag->title,30);
		}
		if(id3v1tag->artist[0])
		{
			strncpy((char*)mp3para.artist,(char*)id3v1tag->artist,30);
		}
	}
	else
	{
		return 1;
	}
	return 0;
}


//解析ID3V2
//buf:输入数据缓存区
//size:数据大小
//pctrl:MP3控制器
//返回值:0,获取正常 其他,获取失败
static uint8_t MP2_ID3V2_Decode(uint8_t *buf, UINT br)
{
	ID3V2_TagHead *taghead = NULL;
	ID3V23_FrameHead *framehead = NULL;
	uint32_t t = 0;
	uint32_t tagsize = 0;		//tag大小
	uint32_t frame_size = 0;	//帧大小

	taghead = (ID3V2_TagHead*)buf;
	if(strncmp("ID3",(const char*)taghead->id,3)==0)//存在ID3?
	{
		tagsize = ((uint32_t)taghead -> size[0]<<21) | ((uint32_t)taghead -> size[1]<<14) | ((uint16_t)taghead -> size[2]<<7) | taghead -> size[3];		//得到tag 大小
		mp3para.datastart = tagsize;		//得到mp3数据开始的偏移量
		if(tagsize > br)
		{
			tagsize = br;		//tagsize大于输入bufsize的时候,只处理输入大小的数据
		}
		if(taghead -> mversion < 3)
		{
			printf("not supported mversion!\r\n");
			return 1;
		}
		t=10;
		while(t < tagsize)
		{
			framehead = (ID3V23_FrameHead*)(buf+t);
			frame_size = ((uint32_t)framehead -> size[0]<<24)|((uint32_t)framehead -> size[1]<<16) | ((uint32_t)framehead -> size[2]<<8) | framehead -> size[3];	//得到帧大小
			if (strncmp("TT2", (char*)framehead -> id,3) == 0 || strncmp("TIT2", (char*)framehead -> id, 4) == 0)//找到歌曲标题帧,不支持unicode格式!!
			{
				strncpy((char*)mp3para.title,(char*)(buf+t+sizeof(ID3V23_FrameHead)+1),AUDIO_MIN(frame_size-1,MP3_TITSIZE_MAX-1));
			}
			if (strncmp("TP1",(char*)framehead->id,3)==0||strncmp("TPE1",(char*)framehead->id,4)==0)//找到歌曲艺术家帧
			{
				strncpy((char*)mp3para.artist, (char*)(buf+t+sizeof(ID3V23_FrameHead)+1),AUDIO_MIN(frame_size-1, MP3_ARTSIZE_MAX-1));
			}
			t += frame_size+sizeof(ID3V23_FrameHead);
		}
	}
	else
	{
		mp3para.datastart = 0;//不存在ID3,mp3数据是从0开始
	}
	return 0;
}


static uint8_t MP3_Get_Info(char *PathBuf)
{
	HMP3Decoder	Mp3Decoder = NULL;
	MP3FrameInfo mp3frameinfo;
	MP3_FrameXing *fxing = NULL;
	MP3_FrameVBRI *fvbri = NULL;

	uint8_t *temp_buf = NULL;
	int offset = 0;
	uint32_t p = 0;
	short samples_per_frame = 0;					//一帧的采样个数
	uint32_t totframes = 0;							//总帧数
	uint8_t fun_res = 0;

	temp_buf = malloc(MP3_DECODE_SIZE);				//为音乐文件解码分配缓存
	if(!temp_buf)
	{
		fun_res = 1;
		goto end_point;
	}
	res = f_open(&fsrc, PathBuf, FA_READ);	//打开文件
	res = f_read(&fsrc, temp_buf, MP3_DECODE_SIZE, &br);

	fun_res = MP2_ID3V2_Decode(temp_buf, br);	//解析ID3V2数据
	f_lseek(&fsrc,(&fsrc)->fsize-128);	//偏移到倒数128的位置
	f_read(&fsrc, temp_buf, 128, &br);//读取128字节
	fun_res = MP2_ID3V1_Decode(temp_buf);	//解析ID3V1数据
	Mp3Decoder = MP3InitDecoder(); 		//MP3解码申请内存
	f_lseek(&fsrc,mp3para.datastart);	//偏移到数据开始的地方
	f_read(&fsrc,temp_buf,MP3_DECODE_SIZE,&br);	//读取5K字节mp3数据
	offset = MP3FindSyncWord(temp_buf,br);	//查找帧同步信息

	if((offset >= 0) && (MP3GetNextFrameInfo(Mp3Decoder,&mp3frameinfo,&temp_buf[offset]) == 0))//找到帧同步信息了,且下一阵信息获取正常
	{
		p=offset+4+32;
		fvbri=(MP3_FrameVBRI*)(temp_buf+p);
		if(strncmp("VBRI",(char*)fvbri->id,4)==0)//存在VBRI帧(VBR格式)
		{
			if(mp3frameinfo.version==MPEG1)
			{
				samples_per_frame=1152;//MPEG1,layer3每帧采样数等于1152
			}
			else
			{
				samples_per_frame=576;//MPEG2/MPEG2.5,layer3每帧采样数等于576
			}
			totframes = ((uint32_t)fvbri -> frames[0]<<24) | ((uint32_t)fvbri -> frames[1]<<16) | ((uint16_t)fvbri -> frames[2]<<8) | fvbri -> frames[3];//得到总帧数
			mp3para.totsec = totframes*samples_per_frame/mp3frameinfo.samprate;//得到文件总长度
		}
		else//不是VBRI帧,尝试是不是Xing帧(VBR格式)
		{
			if(mp3frameinfo.version == MPEG1)	//MPEG1
			{
				p = (mp3frameinfo.nChans == 2)?32:17;
				samples_per_frame = 1152;	//MPEG1,layer3每帧采样数等于1152
			}
			else
			{
				p = mp3frameinfo.nChans == 2?17:9;
				samples_per_frame = 576;		//MPEG2/MPEG2.5,layer3每帧采样数等于576
			}
			p += offset+4;
			fxing = (MP3_FrameXing*)(temp_buf + p);
			if(strncmp("Xing",(char*)fxing->id,4) == 0 || strncmp("Info",(char*)fxing->id,4) == 0)//是Xng帧
			{
				if(fxing -> flags[3] & 0X01)//存在总frame字段
				{
					totframes = ((uint32_t)fxing -> frames[0]<<24) | ((uint32_t)fxing -> frames[1] << 16) | ((uint16_t)fxing -> frames[2]<<8) | fxing->frames[3];//得到总帧数
					mp3para.totsec = totframes*samples_per_frame/mp3frameinfo.samprate;//得到文件总长度
				}
				else	//不存在总frames字段
				{
					mp3para.totsec = fsrc.fsize/(mp3frameinfo.bitrate/8);
				}
			}
			else 		//CBR格式,直接计算总播放时间
			{
				mp3para.totsec = fsrc.fsize/(mp3frameinfo.bitrate/8);
			}
		}
		mp3para.bitrate = mp3frameinfo.bitrate;			//得到当前帧的码率
		mp3para.samplerate = mp3frameinfo.samprate; 	//得到采样率.
		if(mp3frameinfo.nChans == 2)
		{
			mp3para.outsamples = mp3frameinfo.outputSamps; //输出PCM数据量大小
		}
		else
		{
			mp3para.outsamples = mp3frameinfo.outputSamps*2;
		}
		//输出PCM数据量大小,对于单声道MP3,直接*2,补齐为双声道输出
//		printf("title:%s\r\n",mp3para.title);
//		printf("artist:%s\r\n",mp3para.artist);
//		printf("bitrate:%dbps\r\n",mp3para.bitrate);
//		printf("samplerate:%d\r\n", mp3para.samplerate);
//		printf("totalsec:%d\r\n",mp3para.totsec);
	}
	else
	{
		fun_res = 1;
		goto end_point;
	}

end_point:
	f_close(&fsrc);
	MP3FreeDecoder(Mp3Decoder);//释放内存
	free(temp_buf);
	Mp3Decoder = NULL;
	Mp3Decoder = NULL;
	return fun_res;
}


//填充PCM数据到DAC
//buf:PCM数据首地址
//size:pcm数据量(16位为单位)
//nch:声道数(1,单声道,2立体声)
static void MP3_Buffill(uint16_t *buf, uint16_t size, uint8_t nch)
{
	uint16_t i;
	uint16_t *p;
	while(I2S_Trans == 0)//等待传输完成
	{
		;
	}
	I2S_Trans = 0;
	if(I2S_Cache == 0)
	{
		p = (uint16_t*)Cache_1;
	}
	else
	{
		p = (uint16_t*)Cache_0;
	}
	if(nch==2)
	{
		for(i=0; i<size; i++)
		{
			p[i] = buf[i];
		}
	}
	else	//单声道
	{
		for(i=0; i<size; i++)
		{
			p[2*i] = buf[i];
			p[2*i+1] = buf[i];
		}
	}
}

uint8_t MP3_Play(char *PathBuf)
{
	HMP3Decoder mp3decoder = NULL;
	MP3FrameInfo mp3frameinfo;

	uint8_t *temp_buf = NULL;		//mp3文件暂存字符串
	uint8_t *dec_buf = NULL;		//mp3解码后存放的字符串
	uint8_t *readptr = NULL;		//MP3解码读指针
	int offset = 0;					//偏移量
	int outofdata = 0;				//超出数据范围
	int bytesleft = 0;				//buffer还剩余的有效数据
	int err = 0;
	uint8_t fun_res = 0;

	WM8978_PlayMode();
	fun_res = MP3_Get_Info(PathBuf);
	temp_buf = malloc(MP3_DECODE_SIZE);
	dec_buf = malloc(CACHE_SIZE);
	if(! (dec_buf && temp_buf))
	{
		fun_res = 1;
		goto end_point;
	}

	I2S_TX_Start(I2S_Standard_Phillips, I2S_DataFormat_16b, mp3para.samplerate, I2S_Mode_MasterTx);
	memset(Cache_0,0,CACHE_SIZE);//清空缓存，防止出现杂音。
	I2S_TX_DMA_Init(Cache_0,mp3para.outsamples);
	DMA_Cmd(DMA2_Channel2,DISABLE);

	mp3decoder = MP3InitDecoder();
	if(! mp3decoder)
	{
		fun_res = 1;
		goto end_point;
	}
	res = f_open(&fsrc,PathBuf,FA_READ);
	f_lseek(&fsrc,mp3para.datastart);	//跳过文件头中tag信息
	DMA_Cmd(DMA2_Channel2,ENABLE);
	while(res == 0)
	{
		readptr = temp_buf;	//MP3读指针指向buffer
		offset = 0;		//偏移量为0
		outofdata = 0;	//数据正常
		bytesleft = 0;
		res = f_read(&fsrc,temp_buf,MP3_DECODE_SIZE,&br); //一次读取MP3_FILE_BUF_SZ字节
		if(res)//读数据出错了
		{
			break;
		}
		if(br==0)		//读数为0,说明解码完成了.
		{
			break;
		}
		bytesleft += br;	//buffer里面有多少有效MP3数据?
		err = 0;
		while(!outofdata)//没有出现数据异常(即可否找到帧同步字符)
		{
			offset = MP3FindSyncWord(readptr,bytesleft);//在readptr位置,开始查找同步字符
			if(offset < 0)	//没有找到同步字符,跳出帧解码循环
			{
				outofdata=1;//没找到帧同步字符
			}
			else	//找到同步字符了
			{
				readptr += offset;		//MP3读指针偏移到同步字符处.
				bytesleft -= offset;		//buffer里面的有效数据个数,必须减去偏移量
				err = MP3Decode(mp3decoder,&readptr,&bytesleft,(short*)dec_buf,0);//解码一帧MP3数据
				if(err!=0)
				{
					printf("decode error:%d\r\n",err);
					break;
				}
				else
				{
					MP3GetLastFrameInfo(mp3decoder,&mp3frameinfo);	//得到刚刚解码的MP3帧信息
					if(mp3para.bitrate != mp3frameinfo.bitrate)		//更新码率
					{
						mp3para.bitrate = mp3frameinfo.bitrate;
					}
					MP3_Buffill((uint16_t*)dec_buf,mp3frameinfo.outputSamps,mp3frameinfo.nChans);//填充pcm数据
				}
				if(bytesleft<MAINBUF_SIZE*2)//当数组内容小于2倍MAINBUF_SIZE的时候,必须补充新的数据进来.
				{
					memmove(temp_buf,readptr,bytesleft);//移动readptr所指向的数据到buffer里面,数据量大小为:bytesleft
					f_read(&fsrc,temp_buf+bytesleft,MP3_DECODE_SIZE-bytesleft,&br);//补充余下的数据
					if(br < MP3_DECODE_SIZE-bytesleft)
					{
						memset(temp_buf+bytesleft+br,0,MP3_DECODE_SIZE-bytesleft-br);
					}
					bytesleft = MP3_DECODE_SIZE;
					readptr = temp_buf;
				}
			}
		}
	}
	I2S_Trans = 0;
	while(I2S_Trans == 0)
	{
		; //等待DMA传输完成;
	}
	I2S_Trans = 0;
	while(DMA2_Channel2->CNDTR > CACHE_SIZE-br)//最后一次读多少传输多少
	{
		;
	}
	DMA_Cmd(DMA2_Channel2,DISABLE);
	I2S_TX_Stop();

end_point:
	f_close(&fsrc);
	free(temp_buf);
	free(dec_buf);
	MP3FreeDecoder(mp3decoder);
	return fun_res;
}
