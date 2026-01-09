#ifndef __WT588FXX_H
#define __WT588FXX_H

#define AUDIO_REPEAT_TIME        1000  // 重复语音将在时间段内屏蔽

/*********************************************************************************************/
#define Enable_Play		0xAA//开启语言播报
#define Disable_Play	0x00//关闭语言播报

/*********************************************************************************************/
typedef struct{
	unsigned char AudioSwitch;
	unsigned char AudioVolume;
	unsigned char AudioIndex;
	unsigned char AudioPlayCount;
}Uion_Audio;

extern xdata Uion_Audio	AudioData;

void Line_IIC_WT588F(unsigned char Data);

void WT588F_Play_Index(unsigned char Index);
void WT588F_AutoPlay_Audio(void);
void WT588F_Play_Initial(void);
/*********************************************************************************************/
#endif


