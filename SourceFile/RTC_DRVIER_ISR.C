//---------------------------------------------------------
//功能说明：设定芯片内部 实时时钟 ，开启定时中断
//---------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"

#include "KeyEvent.H"
#include "RTC_DRVIER.H"
#include "DisplayUpdata.H"
//---------------------------------------------------------
xdata unsigned char TimeBuffer[3];//时，分，秒
////////////////闹钟时间/////////////////////////////
//AlaramTime[0] 第一组闹钟开关
//AlaramTime[1] 第一组小时
//AlaramTime[2] 第一组分钟

//AlaramTime[3] 第二组闹钟开关
//AlaramTime[4] 第二组小时
//AlaramTime[5] 第二组分钟
xdata unsigned char AlaramTime[6];
//---------------------------------------------------------
void Delay_50us(unsigned int Loop)
{unsigned char Count;
	do
	{	Count = 35;
		while (--Count);
	}while (--Loop);
}
//---------------------------------------------------------
//函数名:	RTC_TimeWrite
//功能说明: RTC写入小时
//输入参数: hour	小时值	//hour = 0~23
//输入参数: minute	分值	//minute = 0~59
//输入参数:	second	秒值 	//second = 0~59
//---------------------------------------------------------
void RTC_TimeWrite(unsigned char hour,unsigned char minute,unsigned char second)
{	RTCON |= RTCWE(1);
	Delay_50us(1);
	RTCH = (RTCH&0xE0)|hour;
	RTCM = minute;
	RTCS = second;
	Delay_50us(5);
	RTCON &= ~RTCWE(1);
}
//---------------------------------------------------------
//函数名:	RTC_TimeRead
//功能说明: RTC 时间读出
//输出参数: hour	小时值	//hour = 0~23
//输出参数: minute	分值	//minute = 0~59
//输出参数:	second	秒值 	//second = 0~59
//---------------------------------------------------------
void RTC_TimeRead(void)
{	TimeBuffer[0] = RTCS;
	TimeBuffer[1] = RTCM;
	TimeBuffer[2] = (RTCH &0x1F);
}
//---------------------------------------------------------
//函数名:	RTC_Config
//功能说明: 初始化RTC控制寄存器
//输入参数: rtce	RTC功能模块使能控制
//			mse		RTC毫秒中断使能控制
//			hse		RTC半秒中断使能控制
//功能说明: 配置RTC ALARM功能寄存器位
//输入参数: sce		秒比较使能控制
//			mce		分钟比较使能控制
//			hce		小时比较使能控制
//---------------------------------------------------------
void RTC_Config(RTCE_TypeDef rtce,MSE_TypeDef mse,HSE_TypeDef hse,
				SCE_TypeDef sce,MCE_TypeDef mce,HCE_TypeDef hce)
{
	RTCON = RTCE(rtce) | MSE(mse) | HSE(hse) | SCE(sce) | MCE(mce) | HCE(hce);
}
//---------------------------------------------------------
//函数名:	RTC_SetAlaramTime
//功能说明:	设置RTC ALARM时间
//输入参数:	al_hr		闹钟小时值
//			al_min		闹钟分钟值
//			al_sec		闹钟秒值
//---------------------------------------------------------
void RTC_SetAlaramTime(unsigned char al_hr,unsigned char al_min,unsigned char al_sec)
{	RTAH	=	al_hr;
	RTAM	=	al_min;
	RTAS	=	al_sec;
}

//---------------------------------------------------------
void RTC_InitialSet(void)
{	GPIO_Init(P72F,P72_XOSCL_IN_SETTING);
	GPIO_Init(P71F,P71_XOSCL_OUT_SETTING);//配置RTC 模块时钟
	//-------------------------------------------
	do{	Delay_50us(50);
		CKCON |= XLCKE;
	}while(!(CKCON & XLSTA));//开启 32.768KHz 晶振
	//-------------------------------------------
	RTC_Config(RTCE_On,MSE_Off,HSE_On,SCE_On,MCE_Off,HCE_Off);
	Delay_50us(300);//必须大于500uS,才能设置时间，否则可能导致时间写入失败
	//-------------------------------------------
	RTC_TimeWrite(0,0,0);//初始化时，分，秒
	//-------------------------------------------
	RTC_TimeRead();
	RTC_SetAlaramTime(0,0,59);
	INT8EN = 1;
	//-------------------------------------------
	AlaramTime[0] = 0x00;//关闭第一组闹钟
	AlaramTime[1] = 99;
	AlaramTime[2] = 59;
	AlaramTime[3] = 0x00;//关闭第二组闹钟
	AlaramTime[4] = 99;
	AlaramTime[5] = 59;
}
//---------------------------------------------------------
void RTC_ISR (void) interrupt 13
{unsigned int	AlarmClockTime;
	if(RTCIF & RTC_MF)	//毫秒中断
	{	RTCIF = RTC_MF;
	}
	if(RTCIF & RTC_HF)	//半秒中断--作为定时开机关机的基准时间
	{	//P6 ^=0x02;//测试定时中断

		RTCIF = RTC_HF;
	}
	if(RTCIF & RTC_AF)	//闹钟中断 一分钟一次
	{	RTCIF = RTC_AF;
		
		//------------------------------------------------------
		if((AlaramTime[0] == 0xAA)&&(DisplaySetIndex == 0x00))
		{	AlarmClockTime = ((unsigned int)AlaramTime[1] *60) + AlaramTime[2];//获得定时时间
			if(--AlarmClockTime == 0x00)
			{	AlaramTime[0] = 0x00;//闹钟失效
				AlaramTime[1] = 99;
				AlaramTime[2] = 59;//重设分钟
				SystemRunIndex = URAT_PowerOn;//开机
			}
			else
			{	AlaramTime[1] = AlarmClockTime /60;
				AlaramTime[2] = AlarmClockTime %60;//重设分钟
			}
		}//定时开机 被开启
		//------------------------------------------------------
		if((AlaramTime[3] == 0xAA)&&(DisplaySetIndex == 0x00))
		{	AlarmClockTime = ((unsigned int)AlaramTime[4] *60) + AlaramTime[5];//获得定时时间
			if(--AlarmClockTime == 0x00)
			{	AlaramTime[3] = 0x00;//闹钟失效
				AlaramTime[4] = 99;
				AlaramTime[5] = 59;//重设分钟
				SystemRunIndex = UART_PowerOff;//关机
			}
			else
			{	AlaramTime[4] = AlarmClockTime /60;
				AlaramTime[5] = AlarmClockTime %60;//重设分钟
			}
		}//定时关机 被开启
	}
}


