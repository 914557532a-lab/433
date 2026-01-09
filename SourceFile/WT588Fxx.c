//-----------------------------------------------------------------------------
//串口发送程序
//-----------------------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

#include "flash.h"

#include "DisplayUpdata.H"
#include "WT588Fxx.h"

/*********************************************************************************************/
#define GPIO_WT588F_CLK_H 	P05 = 1
#define GPIO_WT588F_CLK_L 	P05 = 0

#define GPIO_WT588F_SDA_H	P06 = 1
#define GPIO_WT588F_SDA_L	P06 = 0

#define GPIO_WT588F_BUSY_IN 	P04

//-----------------------------------------------------------------------------

xdata Uion_Audio	AudioData;
/*********************************************************************************************/
// 函数名称  : Line_IIC_WT588F
// 功能描述  : 实现二线串口通信函数
// 输入参数  : Data -- 为发送数据.
// 输出参数  : None.
// 返回参数  : None.
/*********************************************************************************************/
void Line_IIC_WT588F(unsigned char Data)
{unsigned char Count;

	GPIO_WT588F_CLK_L; // 时钟线
	GPIO_WT588F_SDA_H; // 数据线
	Delay_100us(50);// 延时 5ms

	for(Count = 0; Count < 8; Count++) {
		GPIO_WT588F_CLK_L; // 拉低
        if(Data & 0x01) {
            GPIO_WT588F_SDA_H;
        } else {
            GPIO_WT588F_SDA_L;
        }
		Delay_100us(5);// 延时200uS
		GPIO_WT588F_CLK_H; // 拉高
		Delay_100us(5);// 延时200uS
		Data = Data >> 1;
	}
}

/*********************************************************************************************/
// 函数名称  : WT588F_Play_Initial
// 功能描述  : 检查 插入 播放语音
// 输入参数  : audio_index -- 语音索引.
// 输出参数  : None.
// 返回参数  : None.
/*********************************************************************************************/
void WT588F_Play_Initial(void)
{	AudioData.AudioSwitch = 0xAA;//默认上电开启语音播放
	AudioData.AudioIndex = 0x00;
	AudioData.AudioPlayCount = 0;
    // 播完语音数据线拉低
    GPIO_WT588F_CLK_H;
    GPIO_WT588F_SDA_H;
}

/*********************************************************************************************/
// 函数名称  : WT588F_Play_Index
// 功能描述  : 检查 插入 播放语音
// 输入参数  : audio_index -- 语音索引.
// 输出参数  : None.
// 返回参数  : None.
/*********************************************************************************************/
void WT588F_Play_Index(unsigned char Index)
{	if(AudioData.AudioIndex != Index)
	{	AudioData.AudioVolume = 0xEF;//音量最大
		AudioData.AudioIndex = Index;
	#if		(LanguageType == Multilingual)	//多种语言
		if((Index >5)&&(Index <16))
	#else //if	(LanguageType == Chinese)	//单独中文语音
		if((Index >4)&&(Index <15))
	#endif
		{	AudioData.AudioPlayCount = 248;	}
		else
		{	AudioData.AudioPlayCount = 8;	}
	}//最大允许缓存10条连续语音
}
/*********************************************************************************************/
// 函数名称  : WT588F_AutoPlay_Audio
// 功能描述  : 播放语音
// 输入参数  : 
// 输出参数  : None.
// 返回参数  : None.
/*********************************************************************************************/
void WT588F_AutoPlay_Audio(void)
{unsigned char audio_index;

	if(Config.Struct.ProductLanguage >3){	Config.Struct.ProductLanguage = 0;	}//默认为英语

#if	(LanguageType == Multilingual)	//多种语言
	Config.Struct.ProductLanguage = 0x00;//测试声音
#endif

	if(AudioData.AudioSwitch == 0x00){//允许关闭语音播放
		AudioData.AudioPlayCount = 0;
		AudioData.AudioIndex = 0;
	}

	if((AudioData.AudioPlayCount >0)&&((AudioData.AudioPlayCount &0x07) ==0))
	{	audio_index = (Config.Struct.ProductLanguage * 23);//获得当前语言的，语音首地址 1种语音占 23个播放段
		audio_index += AudioData.AudioIndex;//加上播放地址
        if(GPIO_WT588F_BUSY_IN == 1)
		{   Line_IIC_WT588F(0xFE);//停止播放当前语音
            Delay_100us(30);// 延时 3ms
            Line_IIC_WT588F(AudioData.AudioVolume);//音量
            Delay_100us(30);// 延时 3ms
            Line_IIC_WT588F(0xF3);//连码播放
            Delay_100us(30);// 延时 3ms
            Line_IIC_WT588F(audio_index);

			AudioData.AudioPlayCount--;//等待播放完成
        }
    }
	//Line_IIC_WT588F(0xAA);//停止播放当前语音
}


