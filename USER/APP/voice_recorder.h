#ifndef __VOICE_RECORDER_H
#define __VOICE_RECORDER_H

#include "stm32f10x.h"
#include "music_player.h"

#define FRAME_LEN 1152		//用于VAD的一帧的语音长度

#define ENE_CHA 500000		//能量边界
#define ZER_CHA 100				//过零率边界

void Voice_Record(void);

#endif
