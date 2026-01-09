//-----------------------------------------------------------------------------
//功能说明：单片机直接驱动液晶程序
//创建日期：
//-----------------------------------------------------------------------------
#ifndef _MAIN_C_
#define _MAIN_C_
#endif
//-----------------------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

#include "pwm.h"
#include "flash.h"
#include "WT588Fxx.h"
//-----------------------------------------------------------------------------
#include "LCD_DRIVER.H"
#include "Timer0_ISR.H"
#include "UART2_ISR.h"

#include "RTC_DRVIER.H"
#include "HP_03N_DRV.H"

#ifdef Enable_CMT2300	//打开双向遥控
	#include "Radio.H"
#endif

#include "DisplayUpdata.H"
#include "KeyEvent.H"

#include "RF_Receiver.H"
//-----------------------------------------------------------------------------
xdata unsigned char Timer_0_Count;

//-----------------------------------------------------------------------------
#pragma optimize(0)
void Delay_100us(unsigned int Loop)
{unsigned char Count;
	do
	{	Count = 120;
		while (--Count);
	}while (--Loop);
}
//-----------------------------------------------------------------------------
void System_Clock_Setup(void)
{
	EA = 0;//关总中断

	CKCON |= IHCKE; //打开IRCH时钟

	while(!(CKCON & XLSTA)); //等待XOSCL时钟稳定
	RCTAGH = ((4000000 *400)/32768)/256; //设置目标频率
	RCTAGL = ((4000000 *400)/32768)%256;

	VCKDH = 400/256; //设置参考时钟分频，分频后的频率为81.92HZ
	VCKDL = 400%256;
	RCCON = MODE(3) | MSEX(0) | CKSS(0); //设置目标时钟为IRCH，参考时钟为XOSCL，设置校正方式， //启动校正模式
	while(RCCON&0xC0); //等待校正完成

	CKSEL = (CKSEL &0xF8) | CK_IRCH;//选择 ICRH 时钟

	PLLCON = PLLON(1) | MULFT(1);//打开PLL并设置 3倍频
	while(!(PLLCON & PLSTA)); //等待PLL时钟稳定
	CKSEL = (CKSEL&0xF8) | CK_PLL; //设置时钟为PLL

}

//-----------------------------------------------------------------------------
void BackLightControl(void)
{	//----- DivDat  周期 -- DutyDat 占空比 --
    PWM_CfgDivDuty(PWM_CH6, PWM_DivDat, PWM_DutyDat);
    //------------------------------------------------
    PWM_Update((1 << PWM_CH6));
    WaitPWMUpdateComplete();
	//------------------------------------------------
    PWM_CfgDivDuty(PWM_CH7, PWM_DivDat, PWM_DutyDat);
    //------------------------------------------------
    PWM_Update((1 << PWM_CH7));
    WaitPWMUpdateComplete();
//	
    PWM_EnContrl((1 << PWM_CH6) |(1 << PWM_CH7));//开启 PWM
}
//-----------------------------------------------------------------------------
void PWM_ControlInitial(void)
{	PWM_DivDat = 1800;//周期
	PWM_DutyDat = 200;//占空比
	BackLightTime = 500;//设置背光高亮时间
	//------------------------------------------------
	PWM_InterruptCfg(PWM_CH6,TIE_Off,ZIE_Off,PIE_Off,NIE_Off,0);//PWM 中断关闭
	PWM_InterruptCfg(PWM_CH7, TIE_Off, ZIE_Off, PIE_Off, NIE_Off, 0);//PWM 中断关闭
#if		(SYSCLK_SRC	== CK_IRCH)//IRCH
    PWM_init(PWM_CH6, Edge_Align, CKS_IH, Toggle_Off, 0, 0);
    PWM_init(PWM_CH7, Edge_Align, CKS_IH, 0, 0, Complementary_Off);

#else
    PWM_init(PWM_CH6, Edge_Align, CKS_PLL, Toggle_Off, 0, 0);
    PWM_init(PWM_CH7, Edge_Align, CKS_PLL, 0, 0, Complementary_Off);
#endif
	//------------------------------------------------
	BackLightControl();//调节背光亮度
}
//-----------------------------------------------------------------------------
//#pragma optimize(0)
void SystemTimeUpdate(void)
{	if(InterruptCount >=20)//50mS中断
	{	InterruptCount = 0x00;
		//---------------------------------------
        if(++OneSecondTime >20)//秒计时
        {   OneSecondTime = 0x00;
			if((DisplayStateIndex == 0)&&(DisplaySetIndex == 0))
			{	RTC_TimeRead();		}//获取时钟
			//-----------------------------------
			if(ExitSetupTime >0x00)//退出设置状态计时
			{	if(--ExitSetupTime ==0)
				{   DisplaySetIndex = 0x09;
					DisplayStateIndex = 0x00;
				}//退出设置菜单
			}
		//---------------------------------------
			if(AudioData.AudioPlayCount >0)
			{	AudioData.AudioPlayCount--;		}
		//---------------------------------------
			if(UART_ERROR_Time >0x00)
			{	UART_ERROR_Time--;
				if(ErrorIndex == 0x08)
				{	ErrorIndex = 0x00;	}
			}
			else
			{	ErrorIndex = 0x08;		}
		//---------------------------------------
		}
		//---------------------------------------
		if(++HP20X_ReadTime > 10)//500mS转换一次
		{	HP20X_ReadTime = 0x00;		}
		//---------------------------------------
		if(++FlashDisplayTime >20)//控制显示0.5秒闪
		{	FlashDisplayTime = 0x00;
			if(RF_LearnAddrssTime > 0x00)//在5S学习时间内
			{	RF_LearnAddrssTime--;		}
		#ifdef Enable_CMT2300	//打开双向遥控
			if(CMT2300_PairingTime > 0x00)//在5S学习时间内
			{	CMT2300_PairingTime--;		}
		#endif
		}
		//---------------------------------------
		if(SystemStateTime > 0x00)
		{	SystemStateTime--;			}
        //----------------------------------
		if(DataChangeUpdataTime >0)		//调试
		{	
			if(--DataChangeUpdataTime ==0)
			{	Data_Area_ReadWrite(WRITE_DATA,0,&Config.Data[0],ROM_DATA_1_LENGHT);
			}//参数被更改，重新回存
		}
		//---------------------------------------
        if(BackLightTime >0)
        {	if(--BackLightTime ==0x00)
			{	PWM_DutyDat = (PWM_DivDat -(((unsigned long)PWM_DivDat *20)/100));	}//占空比20%
			else
			{	PWM_DutyDat = (PWM_DivDat -(((unsigned long)PWM_DivDat *90)/100));	}//占空比90%
			BackLightControl();//调节背光亮度
        }
//-----------------------------------------------------------------------------
		if(OutHoldTime > 0x00)
		{	OutHoldTime--;			}
		else
		{	RF_KEY_Hold[0] = 0x00;
			RF_KEY_Hold[1] = 0x00;
		}
        //----------------------------------
	#ifdef Enable_CMT2300	//打开双向遥控
		if(g_nTx_RxTimeout >0)//射频发送超时时间
		{	g_nTx_RxTimeout--;		}
	#endif
	}
}
//-----------------------------------------------------------------------------
void ResetVolatgeSet(void)
{
	LVDCON = 0xC9;//设置低电压复位门限为 3.6V
//	LVDCON = 0xCA;//设置低电压复位门限为 3.8V
//	LVDCON = 0xCB;//设置低电压复位门限为 4.0V
//	LVDCON = 0xCC;//设置低电压复位门限为 4.2V
//	LVDCON = 0xCD;//设置低电压复位门限为 4.4V
}
void LDO_PowerSetup(void)
{
//	PWCON = (PWCON &0xF0) |0x0F;//设置LDO 为高功率模式--1.73V
	PWCON = (PWCON &0xF0) |0x0E;//设置LDO 为高功率模式--1.67V
//	PWCON = (PWCON &0xF0) |0x0D;//设置LDO 为高功率模式--1.61V
//	PWCON = (PWCON &0xF0) |0x0C;//设置LDO 为高功率模式--1.55V
}
//-----------------------------------------------------------------------------



void PowerOn_Initial(void)
{	P0 = 0x00;
	GPIO_Init(P04F,OUTPUT );//WT588_DAT
	GPIO_Init(P05F,INPUT);//WT588_BUSY
	GPIO_Init(P06F,OUTPUT );//WT588_CLK
//-------------------------------------
	P1 = 0x00;
	P2 = 0x00;
//-------------------------------------
	P3 = 0xFF;
	GPIO_Init(P32F,P32_PWM7_SETTING);
	GPIO_Init(P33F,P33_PWM6_SETTING);

	GPIO_Init(P34F,OUTPUT |PD_EN);
	GPIO_Init(P34C,General_IO);
//-------------------------------------
	P4 = 0xF7;
	
	GPIO_Init(P43F,INPUT );
	GPIO_Init(P44F,OUTPUT );
	GPIO_Init(P45F,OUTPUT |OP_EN);
	GPIO_Init(P46F,OUTPUT |OP_EN);
	GPIO_Init(P47F,OUTPUT |OP_EN);
//-------------------------------------
	P5 = 0xFF;
	GPIO_Init(P50F,INPUT |PU_EN);
	GPIO_Init(P51F,INPUT |PU_EN);
	GPIO_Init(P52F,INPUT |PU_EN);
	GPIO_Init(P53F,INPUT |PU_EN);
	GPIO_Init(P54F,INPUT |PU_EN);//按键输入

	GPIO_Init(P50C,General_IO);
	GPIO_Init(P51C,General_IO);
	GPIO_Init(P52C,General_IO);
	GPIO_Init(P53C,General_IO);
	GPIO_Init(P54C,General_IO);
//-------------------------------------
	P6 = 0xC0;
	GPIO_Init(P60F,INPUT |PU_EN);
	GPIO_Init(P61F,OUTPUT |PU_EN);
	GPIO_Init(P62F,OUTPUT |PU_EN);//RXD 上拉
	
//-------------------------------------
	P7 = 0xFF;//注意P71,P72,P73,P74 -- 外部时钟端口	
//	GPIO_Init(P75F,OUTPUT);
//-------------------------------------
	System.DATA = 0x00;//清除所有按键
	SystemRunState = TASK_IdleMode;
	DataChangeUpdataTime = 0x00;
	OilpumpEnableState = 0xAA;//允许泵油条件

}
//-----------------------------------------------------------------------------
void main(void)
{	Delay_100us(800);
	ResetVolatgeSet();
	LDO_PowerSetup();
	PowerOn_Initial();
	RTC_InitialSet();//时间模块初始化
	Delay_100us(350);
	System_Clock_Setup();//设置倍频时钟
	Delay_100us(200);
	//-----------------------------------------------------
	WT588F_Play_Initial();
	LCD_PORT_Initial();
	PWM_ControlInitial();
	Uart2_Initial(7500);
	Timer0_Initial();
	HP20X_Initial();//初始化传感器
	//-----------------------------------------------------
	DisplayDataInitial();//准备初始状态
	RunData_Initial();//准备所有运行数据
//-----------------------------------------------------
	RF_Receiver_Initial();
#ifdef Enable_CMT2300	//打开双向遥控
	CMT2300_SetInitial();//2300A 初始化
#endif
	//-----------------------------------------------------
	EA = 1;//开总中断
	//-----------------------------------------------------
	Delay_100us(1800);
#if		(LanguageType == Multilingual)	//多种语言
	WT588F_Play_Index(0x01);//开机提示
#elif	(LanguageType == Chinese)	//单独中文语音
	WT588F_Play_Index(61);//开机提示
#endif
	WT588F_AutoPlay_Audio();
	//-----------------------------------------------------
	while(1)
	{	SystemTimeUpdate();
		WirelessDecode();
		KeyEventProcess();//处理按键
		
		WT588F_AutoPlay_Audio();
		HP20X_DataProcess();
		DisplayDataProcess();
		LCD_DisplayUpdate();//刷新显示
		//=================================================
		UART2_DataProcess();
		UART2_DataTransmit();//下发数据
		//=================================================
	#ifdef Enable_CMT2300	//打开双向遥控
		RunStateControl();
		CMT2300A_RF_Process();
	#endif
	}
}
