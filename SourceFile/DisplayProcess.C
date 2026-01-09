//-----------------------------------------------------------------------------
//功能说明：风暖面板，显示数据更新
//创建日期：
//-----------------------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

//-----------------------------------------------------------------------------
#include "flash.h"
#include "WT588Fxx.h"

#include "LCD_DRIVER.H"
#include "RTC_DRVIER.H"
#include "KeyEvent.H"
#include "UART2_ISR.h"
#include "HP_03N_DRV.H"
#include "DisplayUpdata.H"

#ifdef Enable_CMT2300	//打开双向遥控
	#include "Radio.H"
#endif

#include "RF_Receiver.H"
//---- 笔段排列顺序 --- 0 -- 7 == C G B   D E F A
//-----------------------------------0----1----2----3----4----5----6----7----8----9-----------
unsigned char code LCD_NUM_CODE[]={0xF5,0x05,0xB6,0x97,0x47,0xD3,0xF3,0x85,0xF7,0xD7,\
//-----------------------------------H----F----o----N----O----C----P----U----E--  -  ---------
								   0x67,0xE2,0x33,0xE5,0xC6,0xF0,0xE6,0x75,0xF2,0x02,\
//-----------------------------------A ---L---
								   0xE7,0x70};
//-----------------------------------------------------------------------------
xdata unsigned int BackLightTime;
xdata unsigned char DisplayIndex;
xdata unsigned char DisplaySetIndex;
xdata unsigned char DisplayStateIndex;
xdata unsigned char FlashDisplayTime;
/////
xdata unsigned char Fan_DisplayTime;
xdata unsigned char Fan_DisplayHold;

xdata unsigned char Fan_DisplayIndex;
//////
xdata unsigned char SystemRunIndex;
xdata unsigned char SystemRunState;
//////
xdata unsigned char SystemStateTime;
xdata unsigned char SystemStateHold;
////////////////////////////////////////////////////////////////
#define MainPassword  0x9009//超级密码
xdata unsigned int InputPassword;

//////
xdata union{
	unsigned char Array[2];
	unsigned int Data;
}RunVoltage;
////////////////////////////////////////////////////////////////
xdata union{
	unsigned char TempArray[2];
	signed int Temperature;
}SW;
xdata union{
	unsigned char TempArray[2];
	signed int Temperature;
}HY;
xdata union{
	unsigned char Array[2];
	unsigned int Data;
}RotateSpeed;
xdata union{
	unsigned char Array[2];
	unsigned int Data;
}Set_Speed;
xdata union{
    unsigned char Array[2];
	unsigned int Data;
}Fan_Vot;
xdata union{
    unsigned int DutyCycle;
    unsigned char Data[2];
}Ignition_Vot;
xdata union{
    unsigned int Current;
    unsigned char Data[2];
}Ignition_CUR;

xdata union{
    unsigned char Array[2];
	unsigned int Data;
}WaterPump_CUR;
xdata unsigned char ErrorIndex;
xdata unsigned char ErrorIndexHold;
xdata unsigned char SetupTemperatureHold;

xdata unsigned char OilpumpSetFrequ;//油泵设定频率
xdata unsigned char OilPumpFrequency;//油泵频率
xdata unsigned char WorkingTime;//剩余工作时间
xdata unsigned char RemainingWorkingTime;//剩余工作时间
//-----------------------------------------------------------------------------
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//喇叭 图标
		unsigned char BIT_2:1;//海拔高度 m
		unsigned char BIT_3:1;//加热图标
		unsigned char BIT_4:1;//故障图标
		unsigned char BIT_5:1;//油箱图标
		unsigned char BIT_6:1;//转速 RPM
		unsigned char BIT_7:1;//电压单位 V
		unsigned char BIT_8:1;//百分比 %
	}BIT;
}LCD_00;//第一行，第一个8字
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//1C
		unsigned char BIT_2:1;//1G
		unsigned char BIT_3:1;//1B
		unsigned char BIT_4:1;//Hz
		unsigned char BIT_5:1;//1D
		unsigned char BIT_6:1;//1E
		unsigned char BIT_7:1;//1F
		unsigned char BIT_8:1;//1A
	}BIT;
}LCD_01;//第二行，第二个8字
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//2C
		unsigned char BIT_2:1;//2G
		unsigned char BIT_3:1;//2B
		unsigned char BIT_4:1;//第3位 下点
		unsigned char BIT_5:1;//2D
		unsigned char BIT_6:1;//2E
		unsigned char BIT_7:1;//2F
		unsigned char BIT_8:1;//2A
	}BIT;
}LCD_02;//第二行，第二个8字
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//第2位 下点
		unsigned char BIT_2:1;//秒点
		unsigned char BIT_3:1;//电池图标
		unsigned char BIT_4:1;//第3位 上点
		unsigned char BIT_5:1;//飘带 10		电源-右下角
		unsigned char BIT_6:1;//飘带 9 
		unsigned char BIT_7:1;//飘带 8
		unsigned char BIT_8:1;//飘带 7
	}BIT;
}LCD_03;//第二行，第一个8字
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//飘带 3		箭头-右下角
		unsigned char BIT_2:1;//飘带 4		
		unsigned char BIT_3:1;//飘带 5
		unsigned char BIT_4:1;//飘带 6
		unsigned char BIT_5:1;//关机 图标
		unsigned char BIT_6:1;//飘带 2
		unsigned char BIT_7:1;//飘带 线条
		unsigned char BIT_8:1;//飘带 1
	}BIT;
}LCD_04;//第三行，第一个8字
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//下调 图标
		unsigned char BIT_2:1;//闹钟图标
		unsigned char BIT_3:1;//AT 图标
		unsigned char BIT_4:1;//MT 图标
		unsigned char BIT_5:1;//上调 图标
		unsigned char BIT_6:1;//断线 图标
		unsigned char BIT_7:1;//温度传感器 图标
		unsigned char BIT_8:1;//点火塞 图标
	}BIT;
}LCD_05;//第四行，第二个8字
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//扇叶 1
		unsigned char BIT_2:1;//扇叶 中点
		unsigned char BIT_3:1;//扇叶 2
		unsigned char BIT_4:1;//设置 图标
		unsigned char BIT_5:1;//油泵 图标
		unsigned char BIT_6:1;//水泵 图标
		unsigned char BIT_7:1;//扇叶 4
		unsigned char BIT_8:1;//扇叶 3
	}BIT;
}LCD_06;//第四行，第一个8字
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//天线 主线
		unsigned char BIT_2:1;//天线 1
		unsigned char BIT_3:1;//天线 2
		unsigned char BIT_4:1;//天线 3
		unsigned char BIT_5:1;//4D
		unsigned char BIT_6:1;//4E
		unsigned char BIT_7:1;//4F
		unsigned char BIT_8:1;//4A
	}BIT;
}LCD_07;//第五行，第一个8字
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//4C
		unsigned char BIT_2:1;//4G
		unsigned char BIT_3:1;//4B
		unsigned char BIT_4:1;//天线 4
		unsigned char BIT_5:1;//3D
		unsigned char BIT_6:1;//3E
		unsigned char BIT_7:1;//3F
		unsigned char BIT_8:1;//3A
	}BIT;
}LCD_08;//第五行，第一个8字
xdata union{
	unsigned char DATA;
	struct{
		unsigned char BIT_1:1;//3C
		unsigned char BIT_2:1;//3G
		unsigned char BIT_3:1;//3B
		unsigned char BIT_4:1;//海拔高度 图标
		unsigned char BIT_5:1;//
		unsigned char BIT_6:1;//
		unsigned char BIT_7:1;//
		unsigned char BIT_8:1;//
	}BIT;
}LCD_09;//第五行，第一个8字
//10101010	
//-----------------------------------------------------------------------------
void ClearDisplay(void)
{	LCD_00.DATA = 0x00;
	LCD_01.DATA = 0x00;
	LCD_02.DATA = 0x00;
	LCD_03.DATA = 0x00;
	LCD_04.DATA = 0x00;
	LCD_05.DATA = 0x00;
	LCD_06.DATA = 0x00;
	LCD_07.DATA = 0x00;
	LCD_08.DATA = 0x00;
}
//-----------------------------------------------------------------------------
void ClearBitDisplay(void)
{
	LCD_00.DATA &= 0x1D;	//0001 1011
	LCD_00.BIT.BIT_3 = EQUAL_L;//水暖图标
	LCD_01.BIT.BIT_4 = EQUAL_L;//Hz 图标
	LCD_02.BIT.BIT_4 = EQUAL_L;//第3位 下点
	LCD_03.BIT.BIT_1 = EQUAL_L;//第2位 下点
	LCD_03.BIT.BIT_2 = EQUAL_L;//秒点
	LCD_03.BIT.BIT_4 = EQUAL_L;//第3位 上点
	LCD_06.BIT.BIT_4 = EQUAL_L;//设置 图标
	LCD_09.BIT.BIT_4 = EQUAL_L;//海拔高度
	

}
//-----------------------------------------------------------------------------
void DisplayDataInitial(void)
{	ClearDisplay();//清除全部显示
//----------------------------------------------------------
	ErrorIndex = 0x00;
	UART_ERROR_Time = 100;//通信错误故障
	
	WorkingTime = 45;//预热时间默认45分钟

	RunVoltage.Data = 0x0000;
	Fan_Vot.Data = 0x0000;
	RotateSpeed.Data = 0x0000;

	Ignition_Vot.DutyCycle = 0x0000;
	Ignition_CUR.Current = 0x0000;

	OilPumpFrequency = 0x00;

	SW.Temperature = 0x0000;
	HY.Temperature = 0x0000;
	WaterPump_CUR.Data = 0x0000;
//----------------------------------------------------------
	SystemRunIndex = 0x00;//运行状态
	SystemStateHold = SystemRunState;
//----------------------------------------------------------
	Fan_DisplayIndex = 0x00;

	DisplayIndex  = 0x00;
	DisplaySetIndex = 0x09;//从读取参数开始
	DisplayStateIndex = 0x00;
	System.BIT.BIT_04 = EQUAL_L;//清除 密码更新状态
	
	//----------------------------------------------------------
//	Config.Struct.RF_ModeSelect = Select_OneWay_RF;

}
//-----------------------------------------------------------------------------
//#pragma optimize(0)
//-----------------------------------------------------------------------------

void Number_1_Display(unsigned char Type,unsigned char Number)
{	LCD_07.DATA &= 0x0F;
	LCD_08.DATA &= 0xF8;//默认千位数字关闭
	//--------------------------------------
	switch(Type)
	{case 0x02:
			if(FlashDisplayTime >10)//秒闪
			{	break;		}
	 case 0x01:
			LCD_07.DATA |= (LCD_NUM_CODE[Number] &0xF0);
			LCD_08.DATA |= (LCD_NUM_CODE[Number] &0x07);
		break;
	}
}
void Number_2_Display(unsigned char Type,unsigned char Number)
{	LCD_08.DATA &= 0x0F;
	LCD_09.DATA &= 0xF8;//默认百位数字关闭
	//--------------------------------------
	switch(Type)
	{case 0x02:
			if(FlashDisplayTime >10)//秒闪
			{	break;		}
	 case 0x01:
			LCD_08.DATA |= (LCD_NUM_CODE[Number] &0xF0);
			LCD_09.DATA |= (LCD_NUM_CODE[Number] &0x07);
		break;
	}
}

void Number_3_Display(unsigned char Type,unsigned char Number)
{	LCD_02.DATA &= 0x08;//默认十位数字关闭
	//--------------------------------------
	switch(Type)
	{case 0x02:
			if(FlashDisplayTime >10)//秒闪
			{	break;		}
	 case 0x01:
			LCD_02.DATA |= LCD_NUM_CODE[Number];
		break;
	}
}
void Number_4_Display(unsigned char Type,unsigned char Number)
{	LCD_01.DATA &= 0x08;//默认个位数字关闭
	//--------------------------------------
	switch(Type)
	{case 0x02:
			if(FlashDisplayTime >10)//秒闪
			{	break;		}
	 case 0x01:
			LCD_01.DATA |= LCD_NUM_CODE[Number];
		break;
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Fan_Display(unsigned char Type)
{	switch(Type)
	{case 0x02://故障报警
			if(FlashDisplayTime >10)//秒闪
			{	LCD_06.DATA &= 0x38;
			}
			else
			{	LCD_06.DATA |= 0xC7;
			}
			Fan_DisplayIndex = 0x00;
			Fan_DisplayTime = 0x00;
	   	break;
	 case 0x01:
			if(Fan_DisplayTime >Fan_DisplayHold)
			{	Fan_DisplayTime = 0x00;
				if(Fan_DisplayHold >3)//逐渐加速
				{	Fan_DisplayHold--;		}
			}
			else{	break;		}
			//----------------------------------------
			LCD_06.DATA |= 0xC7;
			//----------------------------------------
			switch(Fan_DisplayIndex++)
			{case 0x04:
					LCD_06.BIT.BIT_7 = EQUAL_L;//风扇叶片 4
				break;
			 case 0x03:
					LCD_06.BIT.BIT_8 = EQUAL_L;//风扇叶片 3
				break;
			 case 0x02:
					LCD_06.BIT.BIT_3 = EQUAL_L;//风扇叶片 2
				break;
			 case 0x01:
					LCD_06.BIT.BIT_1 = EQUAL_L;//风扇叶片 1
				break;
			 default:////初始状态
					Fan_DisplayIndex = 0x01;
					Fan_DisplayHold = 40;//设定加速时间100mS
				break;
			}
		break;
	 default:////默认风扇图标全显示	
			LCD_06.DATA &= 0x38;
			//----------------------------------------
			Fan_DisplayIndex = 0x00;
			Fan_DisplayTime = 0x00;
			Fan_DisplayHold = 40;
		break;
	}
}
//-----------------------------------------------------------------------------

void Gear_Display(char Gear)
{
	if(Gear < 0 && Gear > 9)
	{	return ;}
	//----------------------------------------
	LCD_03.DATA &= ~(0xF0);
	LCD_04.DATA &= ~(0xAF);
	//----------------------------------------
	switch(Gear)
	{
		case 0:	LCD_04.DATA |= 0x80;	break;
		case 1:	LCD_04.DATA |= 0xA0;	break;
		case 2:	LCD_04.DATA |= 0xA1;	break;
		case 3:	LCD_04.DATA |= 0xA3;	break;
		case 4:	LCD_04.DATA |= 0xA7;	break;
		case 5:	LCD_04.DATA |= 0xAF;	break;
		case 6:	LCD_04.DATA |= 0xAF;
				LCD_03.DATA |= 0x80;	break;	
		case 7:	LCD_04.DATA |= 0xAF;
				LCD_03.DATA |= 0xC0;	break;	
		case 8:	LCD_04.DATA |= 0xAF;
				LCD_03.DATA |= 0xE0;	break;
		case 9:LCD_04.DATA |= 0xAF;
				LCD_03.DATA |= 0xF0;	break;
		default:	break;
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void RunStateDisplay(void)//实时显示运行状态
{	if(FlashDisplayTime <10)//秒 闪
	{	LCD_03.BIT.BIT_3 = EQUAL_H;			}//电压正常
	else
	{	if((ErrorIndex == 0x02)||(ErrorIndex == 0x03))
		{	LCD_03.BIT.BIT_3 = EQUAL_L;	}//电池 图标
	}
    //-------------------------------------------
	if(ErrorIndex != 0x04)//点火塞故障
	{	if(Ignition_CUR.Current >80)
		{	LCD_05.BIT.BIT_8 = EQUAL_H;		}//点火塞
		else
		{	LCD_05.BIT.BIT_8 = EQUAL_L;		}
	}
	else
	{	if(FlashDisplayTime <10)//秒 闪
		{	LCD_05.BIT.BIT_8 = EQUAL_H;			}
		else
		{	LCD_05.BIT.BIT_8 = EQUAL_L;		}
	}
    //-------------------------------------------
	if(ErrorIndex != 0x05)
	{	if(OilPumpFrequency != 0x00)
		{	LCD_06.BIT.BIT_5 = EQUAL_H;		}//油泵 图标
		else
		{	LCD_06.BIT.BIT_5 = EQUAL_L;		}//油泵 图标
	}
	else
	{	if(FlashDisplayTime <10)//秒 闪
		{	LCD_06.BIT.BIT_5 = EQUAL_H;		}//油泵 图标
		else
		{	LCD_06.BIT.BIT_5 = EQUAL_L;		}//油泵 图标
	}
    //-------------------------------------------
	/*
	if(WaterPump_CUR.Data > 35)//水泵工作电压大于 0.35 A		//未改
	{	LCD_06.BIT.BIT_7 = EQUAL_H;//水泵图标
	}
	else
	{	LCD_06.BIT.BIT_7 = EQUAL_L;//水泵图标
	}
	*/
    //-------------------------------------------
	if(FlashDisplayTime <10)//秒 闪
	{	LCD_05.BIT.BIT_7 = EQUAL_H;	}//温度传感器
	else
	{	if((ErrorIndex == 0x06)||(ErrorIndex == 0x0A))
		{	LCD_05.BIT.BIT_7 = EQUAL_L;}//温度传感器
	}
    //-------------------------------------------
	if(ErrorIndex == 0x07)//电机故障报警
	{	Fan_Display(2);		}
	else if((SystemRunState > TASK_IdleMode)&&(SystemRunState < TASK_Shutdown))
    {	Fan_Display(1);		}//风扇叶轮转动
    else//----------------------OF
	{	Fan_Display(0);		}//风扇叶轮停止
    //-------------------------------------------
    //-------------------------------------------
	if(Config.Struct.RF_Address.DATA != 0x00)//遥控已匹配
	{	LCD_07.DATA |= 0x0F;
		LCD_08.BIT.BIT_4 = EQUAL_H;//天线 图标	
	}
	else
	{	LCD_07.DATA &= 0xF0;
		LCD_08.BIT.BIT_4 = EQUAL_L;//天线 图标
	}
    //-------------------------------------------	//补充
	if(FlashDisplayTime >10)//秒 闪		//未改
	{	LCD_05.BIT.BIT_6 = EQUAL_L;		}
	else
	{	if(ErrorIndex == 0x08)
		{	LCD_05.BIT.BIT_6 = EQUAL_H;		}//断线 图标
	}
	//-----------------------------------
	if((AlaramTime[0] != 0x00)||(AlaramTime[3] != 0x00))
	{	LCD_05.BIT.BIT_2 = EQUAL_H;		}//定时 闹钟
	else
	{	LCD_05.BIT.BIT_2 = EQUAL_L;		}
//	if(AudioData.AudioSwitch == 0x00)
//	{	LCD_00.BIT.BIT_1 = EQUAL_L;		}//喇叭 图标 关
//	else
//	{	LCD_00.BIT.BIT_1 = EQUAL_H;		}//喇叭 图标 开
	//-------------------------------------------
	if(Config.Struct.TemperatureType != 0x01)//改变控温方式
	{	LCD_05.BIT.BIT_4 = EQUAL_L;//手动
		LCD_05.BIT.BIT_3 = EQUAL_H;//自动
	}
	else//按照档位显示
	{	LCD_05.BIT.BIT_3 = EQUAL_L;//自动
		LCD_05.BIT.BIT_4 = EQUAL_H;//手动
	}
	//-------------------------------------------
	if(Config.Struct.TemperatureType == 0x01)
	{
		Gear_Display(Config.Struct.EnergyLevel);
	}
	//-------------------------------------------
	if(SystemStateHold != SystemRunState)
	{	SystemStateHold = SystemRunState;
		if((SystemRunState ==TASK_Self_Test)||(SystemRunState ==TASK_Ignition)
		||(SystemRunState ==TASK_Turn_Off)||(SystemRunState ==TASK_Wait_Off))
		{	SystemStateTime = 90;
			DisplayStateIndex = 0x00;//优先显示 开 关机状态
		}//4.5秒
	}
	if(SystemStateTime > 0x00)
	{	if((SystemRunState ==TASK_Self_Test)||(SystemRunState ==TASK_Ignition))
		{	Number_1_Display(0,0);
			Number_2_Display(0,0);
			Number_3_Display(2,12);
			Number_4_Display(2,13);//开机提示 ON
		#if		(LanguageType == Multilingual)	//多种语言
			WT588F_Play_Index(0x02);//开机
		#elif	(LanguageType == Chinese)	//单独中文语音
			WT588F_Play_Index(1);//提示开机
		#endif
		}
		if((SystemRunState ==TASK_Turn_Off)||(SystemRunState ==TASK_Wait_Off))
		{	Number_1_Display(0,0);
			Number_2_Display(2,12);
			Number_3_Display(2,11);
			Number_4_Display(2,11);//关机提示 OF
		#if		(LanguageType == Multilingual)	//多种语言
			WT588F_Play_Index(0x05);//开始关机
		#elif	(LanguageType == Chinese)	//单独中文语音
			WT588F_Play_Index(4);//提示关机
		#endif
		}
		LCD_03.BIT.BIT_2 = EQUAL_L;//时间 秒点
		LCD_02.BIT.BIT_4 = EQUAL_L;//3 下点
		LCD_03.BIT.BIT_4 = EQUAL_L;//3 上点
		LCD_03.BIT.BIT_1 = EQUAL_L;//2 下点
	}
//	else
//	{	if(SetupTemperatureHold != Config.Struct.SetupTemperature)
//		{	SetupTemperatureHold = Config.Struct.SetupTemperature;
//		#if		(LanguageType == Multilingual)	//多种语言
//			if(Config.Struct.TemperatureType == 0xCD)
//			{	WT588F_Play_Index(((Config.Struct.SetupTemperature -8) /3) +21);	}
//			else
//			{	WT588F_Play_Index(23 + Config.Struct.SetupTemperature);		}
//		#endif
//		}
//		//-----------------------------------
//		if(ErrorIndex >0x01)
//		{	if(ErrorIndexHold != ErrorIndex)
//			{	ErrorIndexHold = ErrorIndex;
//				DisplayStateIndex = 0x00;
//				DisplaySetIndex = 0x00;
//			}
//		#if		(LanguageType == Multilingual)	//多种语言
//			WT588F_Play_Index((ErrorIndex -0x01) +5);//故障码播报
//		#else
//			WT588F_Play_Index((ErrorIndex -0x01) +4);//故障码播报
//		#endif
//		}//故障码出现时优先显示故障码
//	}
}
//-----------------------------------------------------------------------------
void  DisplayDataProcess(void)
{signed long int_Temp_1,int_Temp_2,int_Temp_3; signed char Temp_0 , Temp_1;
	switch(DisplaySetIndex)
    {case 0x0E://手动开启水泵
			if((SystemRunState > TASK_Self_Test)&&(SystemRunState < TASK_Shutdown))
			{	DisplayStateIndex = 0x02;		}//退出手动泵油状态
			//--------------------------------------------------
			ExitSetupTime = 00;//以秒为单位（禁止自动退出）
			//--------------------------------------------------
			switch(DisplayStateIndex)
			{case 0x02:
					SystemRunIndex = UART_StopWaterPump;
					DisplaySetIndex = 0x00;//下一项
					DisplayStateIndex = 0x00;
				break;
			 case 0x01:
					Number_1_Display(1,16);
					Number_2_Display(0,0);
					if(WaterPump_CUR.Data > 35)
					{	Number_3_Display(2,12);
						Number_4_Display(2,13);
					}
					else
					{	Number_3_Display(2,12);
						Number_4_Display(2,11);
					}
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						SystemRunIndex = UART_RunWaterPump;
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						SystemRunIndex = UART_StopWaterPump;
					}
//					if(System.BIT.BIT_01 == EQUAL_H)//确认
//					{   System.BIT.BIT_01 = EQUAL_L;
//						DisplayStateIndex++;//下一项
//					}
				break;
			 default:
					ClearBitDisplay();
					//---------------------------
					DisplayStateIndex = 0x01;
					SystemRunIndex = 0x00;
				break;
			}
        break;
     case 0x0D://手动泵油
			if((SystemRunState > TASK_Self_Test)&&(SystemRunState < TASK_Shutdown))
			{	DisplayStateIndex = 0x02;		}//退出手动泵油状态
			//--------------------------------------------------
			ExitSetupTime = 00;//以秒为单位（禁止自动退出）
			//--------------------------------------------------
			switch(DisplayStateIndex)
			{case 0x02:
					DisplaySetIndex = 0x00;//下一项
					DisplayStateIndex = 0x00;
				
				break;
			 case 0x01:
					Number_1_Display(1,10);
					Number_2_Display(0,0);
					if(OilPumpFrequency != 0x00)
					{	Number_3_Display(2,12);
						Number_4_Display(2,13);
					}
					else
					{	Number_3_Display(2,12);
						Number_4_Display(2,11);
					}
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						SystemRunIndex = UART_PumpOil;
					#if		(LanguageType == Multilingual)	//多种语言
						WT588F_Play_Index(0x14);//开始手动泵油
					#elif	(LanguageType == Chinese)	//单独中文语音
						WT588F_Play_Index(0x13);//开始手动泵油
					#endif
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						SystemRunIndex = UART_ExitPumpOil;
					#if		(LanguageType == Multilingual)	//多种语言
						WT588F_Play_Index(0x15);//停止手动泵油
					#elif	(LanguageType == Chinese)	//单独中文语音
						WT588F_Play_Index(0x14);//停止手动泵油
					#endif
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
						SystemRunIndex = UART_ExitPumpOil;
					#if		(LanguageType == Multilingual)	//多种语言
						WT588F_Play_Index(0x15);//停止手动泵油
					#elif	(LanguageType == Chinese)	//单独中文语音
						WT588F_Play_Index(0x14);//停止手动泵油
					#endif
					}
				break;
			 default:
					ClearBitDisplay();
					//--------------------------------
					DisplayStateIndex = 0x01;
				break;
			}
        break;
     case 0x0C://遥控地址码设置
			switch(DisplayStateIndex)
			{case 0x01:

					Number_1_Display(2,10);
					Number_2_Display(2,11);//闪烁
					Number_3_Display(2,20);//闪烁		
					Number_4_Display(2,19);//闪烁		

				//---------------------------------------------------------
//					Number_1_Display(2,10);
//					Number_2_Display(2,11);//闪烁
//					Number_3_Display(2,19);//闪烁		
//					if(Config.Struct.RF_ModeSelect == Select_OneWay_RF)//单向
//					{	Number_4_Display(2,1);		}
//					else
//					{	Number_4_Display(2,2);		}
//					//---------------------------
//					if(System.BIT.BIT_02 == EQUAL_H)//上调
//					{   System.BIT.BIT_02 = EQUAL_L;
//						//-----------------------
//						Config.Struct.RF_ModeSelect = Select_TwoWay_RF;
//						CMT2300_SetInitial();//重新配置 无线芯片
//						CMT2300_PairingTime = CMT2300_ADDRLEARNTIME;
//						RF_LearnAddrssTime = 0;
//					}
//					if(System.BIT.BIT_03 == EQUAL_H)//下调
//					{   System.BIT.BIT_03 = EQUAL_L;
//						//-----------------------
//						Config.Struct.RF_ModeSelect = Select_OneWay_RF;
//						CMT2300_SetInitial();//重新配置 无线芯片
//						RF_LearnAddrssTime = RF_ADDRLEARNTIME; 
//						CMT2300_PairingTime = 0;
//					}
					//---------------------------
					if((RF_LearnAddrssTime == 0x00)&&(CMT2300_PairingTime == 0x00))//完成学习后退出
					{	DisplaySetIndex = 0x00;
                    	DisplayStateIndex = 0x00;

						RF_LearnAddrssTime = 0x00;
						CMT2300_PairingTime = 0x00;
					}
				break;
			 default:
					ClearBitDisplay();
					//--------------------------------
          DisplayStateIndex = 0x01;
					//--------------------------------
					Config.Struct.RF_ModeSelect = Select_OneWay_RF;		//固定为单向
					//--------------------------------
					if(Config.Struct.RF_ModeSelect == Select_OneWay_RF)//单向
					{	RF_LearnAddrssTime = RF_ADDRLEARNTIME;
						CMT2300_PairingTime = 0x00;
					}//配对等待时间
					else
					{	CMT2300_PairingTime = CMT2300_ADDRLEARNTIME;
						RF_LearnAddrssTime = 0x00;
					}
					//--------------------------------
					CMT2300_SetInitial();//重新配置 无线芯片
					//--------------------------------
				#if		(LanguageType == Multilingual)	//多种语言
					WT588F_Play_Index(0x10);//遥控配对
				#elif	(LanguageType == Chinese)	//单独中文语音
					WT588F_Play_Index(15);//遥控配对
				#endif
				break;
			}
        break;
///////////////////////////////////////////////////////////////////////////////////////////////////
     case 0x0A://参数存储
			RunDataUpdate();//存储设置参数
	 case 0x09://更新参数
			RunData_Initial();//取得设置参数
			ClearBitDisplay();//清除各种状态显示标志	//补充
	//----------------------------------------------------------
			System.BIT.BIT_04 = EQUAL_L;//清除 密码更新状态
 			DisplaySetIndex = 0x00;//退出
			DisplayStateIndex = 0x00;
		break;
     case 0x08://电压，点火功率，工作模式(单次加热-循环加热)，密码
			switch(DisplayStateIndex)
			{case 0x05://修改密码
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						DisplayStateIndex = 0x04;
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplaySetIndex = 0x03;//进入密码修改状态
						System.BIT.BIT_01 = EQUAL_L;
						System.BIT.BIT_02 = EQUAL_L;
						System.BIT.BIT_03 = EQUAL_L;
						System.BIT.BIT_04 = EQUAL_H;
						DisplayStateIndex = 0x40;//可更改密码
					}
					Number_1_Display(0,0);
					Number_2_Display(0,0);
					Number_3_Display(1,12);
					Number_4_Display(1,13);
				break;
			 case 0x04:
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						DisplayStateIndex = 0x05;
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplaySetIndex = 0x0A;//更新参数退出
					}
					Number_1_Display(0,0);
					Number_2_Display(0,0);
					Number_3_Display(1,12);
					Number_4_Display(1,11);
				break;
			 case 0x03://工作模式(单次加热-循环加热)
					Number_1_Display(1,18);
					Number_2_Display(1,17);
					Number_3_Display(1,19);
					Number_4_Display(2,Config.Struct.RunStateType %10);
					//---------------------------
					LCD_00.BIT.BIT_7 = EQUAL_L;//电压 图标
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(Config.Struct.RunStateType  <2)
						{	Config.Struct.RunStateType ++;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(Config.Struct.RunStateType  >1)
						{	Config.Struct.RunStateType --;		}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x02://点火塞功率选择
					Number_1_Display(1,16);
					Number_2_Display(1,11);
					Number_3_Display(1,19);
					Number_4_Display(2,Config.Struct.Ignition_Power %10);
					//---------------------------
					LCD_00.BIT.BIT_7 = EQUAL_L;//电压 图标
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(Config.Struct.Ignition_Power  <6)
						{	Config.Struct.Ignition_Power ++;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(Config.Struct.Ignition_Power  >1)
						{	Config.Struct.Ignition_Power --;		}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x01://工作电压
					Number_1_Display(1,17);
					Number_2_Display(1,19);
					Number_3_Display(2,(Config.Struct.SystemPowerVolatge/10) /10);
					Number_4_Display(2,(Config.Struct.SystemPowerVolatge/10) %10);
					//---------------------------
					LCD_00.BIT.BIT_7 = EQUAL_H;//电压 图标
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						Config.Struct.SystemPowerVolatge = 240;
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						Config.Struct.SystemPowerVolatge = 120;
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 default:
					ClearBitDisplay();
			 		LCD_00.BIT.BIT_6 = EQUAL_L;//转速 图标
					LCD_06.BIT.BIT_4 = EQUAL_H;//设置 图标
					DisplayStateIndex = 0x01;
				break;
			}
	   	break;
	 case 0x07://终止转速设定
			Set_Speed.Array[0] = Config.Struct.MAX_FanSpeed.Array[0];
			Set_Speed.Array[1] = Config.Struct.MAX_FanSpeed.Array[1];
			switch(DisplayStateIndex)
			{case 0x04:
					DisplaySetIndex++;//下一项
					DisplayStateIndex = 0x00;
				break;
			 case 0x03:
					int_Temp_1 = Set_Speed.Data /100;//分解
					int_Temp_3 = Set_Speed.Data %10;
					//---------------------------
					Number_1_Display(1,Set_Speed.Data /1000);
					Set_Speed.Data %= 1000;
					Number_2_Display(1,Set_Speed.Data /100);
					Set_Speed.Data %= 100;
					Number_3_Display(2,Set_Speed.Data /10);
					int_Temp_2 = Set_Speed.Data /10;//权值
					Number_4_Display(1,Set_Speed.Data %10);
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++int_Temp_2 > 9)
						{	int_Temp_2 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(int_Temp_2 >0)
						{	int_Temp_2--;		}
						else
						{	int_Temp_2 = 9;		}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
					//---------------------------
					Set_Speed.Data = (int_Temp_1 *100)+(int_Temp_2 *10)+int_Temp_3;//合成
				break;
			 case 0x02:
					int_Temp_1 = Set_Speed.Data /1000;//分解
					int_Temp_3 = Set_Speed.Data %100;
					//---------------------------
					Number_1_Display(1,Set_Speed.Data /1000);
					Set_Speed.Data %= 1000;
					int_Temp_2 = Set_Speed.Data /100;//权值
					Number_2_Display(2,Set_Speed.Data /100);
					Set_Speed.Data %= 100;
					Number_3_Display(1,Set_Speed.Data /10);
					Number_4_Display(1,Set_Speed.Data %10);
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++int_Temp_2 > 9)
						{	int_Temp_2 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(int_Temp_2 >0)
						{	int_Temp_2--;		}
						else
						{	int_Temp_2 = 9;		}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
					//---------------------------
					Set_Speed.Data = (int_Temp_1 *1000)+(int_Temp_2 *100)+int_Temp_3;//合成
				break;
			 case 0x01:
					int_Temp_1 = Set_Speed.Data /1000;//分解
					int_Temp_2 = Set_Speed.Data %1000;
					//---------------------------
					Number_1_Display(2,Set_Speed.Data /1000);
					Set_Speed.Data %= 1000;
					Number_2_Display(1,Set_Speed.Data /100);
					Set_Speed.Data %= 100;
					Number_3_Display(1,Set_Speed.Data /10);
					Number_4_Display(1,Set_Speed.Data %10);
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++int_Temp_1 > 9)
						{	int_Temp_1 = 6;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--int_Temp_1 < 1)
						{	int_Temp_1 = 9;		}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
					//---------------------------
					Set_Speed.Data = (int_Temp_1 *1000) +int_Temp_2;//合成
				break;
			 default:
					ClearBitDisplay();
			 		LCD_00.BIT.BIT_6 = EQUAL_H;//转速 图标
					LCD_06.BIT.BIT_4 = EQUAL_H;//设置 图标
					DisplayStateIndex = 0x01;
				break;
			}
			if(Set_Speed.Data < (Config.Struct.MIN_FanSpeed.DATA +200)){	Set_Speed.Data = (Config.Struct.MIN_FanSpeed.DATA +200);	}
			if(Set_Speed.Data > 9900){	Set_Speed.Data = 9900;	}//限制
			Config.Struct.MAX_FanSpeed.DATA = Set_Speed.Data;
	   	break;
	 case 0x06://起始转速设定
			Set_Speed.Array[0] = Config.Struct.MIN_FanSpeed.Array[0];
			Set_Speed.Array[1] = Config.Struct.MIN_FanSpeed.Array[1];
			switch(DisplayStateIndex)
			{case 0x04:
					DisplaySetIndex++;//下一项
					DisplayStateIndex = 0x00;
				break;
			 case 0x03:
					int_Temp_1 = Set_Speed.Data /100;//分解
					int_Temp_3 = Set_Speed.Data %10;
					//---------------------------
					Number_1_Display(1,Set_Speed.Data /1000);
					Set_Speed.Data %= 1000;
					Number_2_Display(1,Set_Speed.Data /100);
					Set_Speed.Data %= 100;
					Number_3_Display(2,Set_Speed.Data /10);
					int_Temp_2 = Set_Speed.Data /10;//权值
					Number_4_Display(1,Set_Speed.Data %10);
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++int_Temp_2 > 9)
						{	int_Temp_2 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(int_Temp_2 >0)
						{	int_Temp_2--;		}
						else
						{	int_Temp_2 = 9;		}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
					//---------------------------
					Set_Speed.Data = (int_Temp_1 *100)+(int_Temp_2 *10)+int_Temp_3;//合成
				break;
			 case 0x02:
					int_Temp_1 = Set_Speed.Data /1000;//分解
					int_Temp_3 = Set_Speed.Data %100;
					//---------------------------
					Number_1_Display(1,Set_Speed.Data /1000);
					Set_Speed.Data %= 1000;
					int_Temp_2 = Set_Speed.Data /100;//权值
					Number_2_Display(2,Set_Speed.Data /100);
					Set_Speed.Data %= 100;
					Number_3_Display(1,Set_Speed.Data /10);
					Number_4_Display(1,Set_Speed.Data %10);
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++int_Temp_2 > 9)
						{	int_Temp_2 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(int_Temp_2 >0)
						{	int_Temp_2--;		}
						else
						{	int_Temp_2 = 9;		}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
					//---------------------------
					Set_Speed.Data = (int_Temp_1 *1000)+(int_Temp_2 *100)+int_Temp_3;//合成
				break;
			 case 0x01:
					int_Temp_1 = Set_Speed.Data /1000;//分解
					int_Temp_2 = Set_Speed.Data %1000;
					//---------------------------
					Number_1_Display(2,Set_Speed.Data /1000);
					Set_Speed.Data %= 1000;
					Number_2_Display(1,Set_Speed.Data /100);
					Set_Speed.Data %= 100;
					Number_3_Display(1,Set_Speed.Data /10);
					Number_4_Display(1,Set_Speed.Data %10);
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++int_Temp_1 > 6)
						{	int_Temp_1 = 1;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(int_Temp_1 >1)
						{	int_Temp_1--;		}
						else
						{	int_Temp_1 = 6;		}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
					//---------------------------
					Set_Speed.Data = (int_Temp_1 *1000) +int_Temp_2;//合成
				break;
			 default:
					ClearBitDisplay();
					//---------------------------
			 		LCD_00.BIT.BIT_6 = EQUAL_H;//转速 图标
					LCD_06.BIT.BIT_4 = EQUAL_H;//设置 图标
					DisplayStateIndex = 0x01;
				break;
			}
			if(Set_Speed.Data < 1300){	Set_Speed.Data = 1300;	}
			if(Set_Speed.Data > 6000){	Set_Speed.Data = 6000;	}//限制
			Config.Struct.MIN_FanSpeed.DATA = Set_Speed.Data;
	   	break;
	 case 0x05://油泵频率
			switch(DisplayStateIndex)
			{case 0x02://油泵终止频率
					Number_1_Display(1,16);
					Number_2_Display(1,19);
					Number_3_Display(2,Config.Struct.MAX_OilPumpCycle /10);
					Number_4_Display(2,Config.Struct.MAX_OilPumpCycle %10);
					//---------------------------
					if(FlashDisplayTime <10)
					{	LCD_02.BIT.BIT_4 = EQUAL_H;		}//3 下点
					else
					{	LCD_02.BIT.BIT_4 = EQUAL_L;		}//3 下点
					//---------------------------
					LCD_01.BIT.BIT_4 = EQUAL_H;//频率 图标
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(Config.Struct.MAX_OilPumpCycle <99)
						{	Config.Struct.MAX_OilPumpCycle++;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						Config.Struct.MAX_OilPumpCycle--;
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplaySetIndex++;//下一项
						DisplayStateIndex = 0x00;
					}
					if(Config.Struct.MAX_OilPumpCycle <(Config.Struct.MIN_OilPumpCycle +10))//限制下限值
					{	Config.Struct.MAX_OilPumpCycle = (Config.Struct.MIN_OilPumpCycle +10);		}
				break;
			 case 0x01://油泵起始频率
					Number_1_Display(1,16);
					Number_2_Display(1,19);
					Number_3_Display(2,Config.Struct.MIN_OilPumpCycle /10);
					Number_4_Display(2,Config.Struct.MIN_OilPumpCycle %10);
					if(FlashDisplayTime <10)	//秒 冒号
					{	LCD_02.BIT.BIT_4 = EQUAL_H;		}//3 下点
					else
					{	LCD_02.BIT.BIT_4 = EQUAL_L;		}//3 下点
					//---------------------------
					LCD_01.BIT.BIT_4 = EQUAL_H;//频率 图标
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(Config.Struct.MIN_OilPumpCycle <65)
						{	Config.Struct.MIN_OilPumpCycle++;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(Config.Struct.MIN_OilPumpCycle >8)
						{	Config.Struct.MIN_OilPumpCycle--;		}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 default:
					ClearBitDisplay();
					LCD_06.BIT.BIT_4 = EQUAL_H;//设置 图标
					DisplayStateIndex = 0x01;
				break;
			}
	   	break;
	 case 0x04://燃油类型，温度下限，温度上限
			switch(DisplayStateIndex)
			{case 0x05://温度上限，个位
					Temp_0 = Config.Struct.SetupTemper_H_Limit /10;
					Temp_1 = Config.Struct.SetupTemper_H_Limit %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_1 >9){	Temp_1 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_1 <0){	Temp_1 = 9;		}
					}
					Config.Struct.SetupTemper_H_Limit = (Temp_0 *10) + Temp_1;
					if(Config.Struct.SetupTemper_H_Limit <(Config.Struct.SetupTemper_L_Limit +5))
					{	Config.Struct.SetupTemper_H_Limit = (Config.Struct.SetupTemper_L_Limit +5);	}
					//--------------------------------------
					Number_1_Display(1,10);
					Number_2_Display(1,19);
					Number_3_Display(1,Config.Struct.SetupTemper_H_Limit /10);
					Number_4_Display(2,Config.Struct.SetupTemper_H_Limit %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplaySetIndex++;//下一项
						DisplayStateIndex = 0x00;
					}
				break;
			 case 0x04://温度上限，十位 >= 50度
					Temp_0 = Config.Struct.SetupTemper_H_Limit /10;
					Temp_1 = Config.Struct.SetupTemper_H_Limit %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_0 > 9){   Temp_0 = 5;	}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_0 < 5){   Temp_0 = 9;	}
					}
					Config.Struct.SetupTemper_H_Limit = (Temp_0 *10) + Temp_1;
					if(Config.Struct.SetupTemper_H_Limit <(Config.Struct.SetupTemper_L_Limit +5))
					{	Config.Struct.SetupTemper_H_Limit = (Config.Struct.SetupTemper_L_Limit +5);	}
					//--------------------------------------
					Number_1_Display(1,10);
					Number_2_Display(1,19);
					Number_3_Display(2,Config.Struct.SetupTemper_H_Limit /10);
					Number_4_Display(1,Config.Struct.SetupTemper_H_Limit %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x03://温度下限，个位
					Temp_0 = Config.Struct.SetupTemper_L_Limit /10;
					Temp_1 = Config.Struct.SetupTemper_L_Limit %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_1 >9){	Temp_1 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_1 <0){	Temp_1 = 9;		}
					}
					Config.Struct.SetupTemper_L_Limit = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,21);
					Number_2_Display(1,19);
					Number_3_Display(1,Config.Struct.SetupTemper_L_Limit /10);
					Number_4_Display(2,Config.Struct.SetupTemper_L_Limit %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x02://温度下限，十位 < 80度
					Temp_0 = Config.Struct.SetupTemper_L_Limit /10;
					Temp_1 = Config.Struct.SetupTemper_L_Limit %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_0 > 7){   Temp_0 = 0;	}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_0 < 0){   Temp_0 = 7;	}
					}
					Config.Struct.SetupTemper_L_Limit = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,21);
					Number_2_Display(1,19);
					Number_3_Display(2,Config.Struct.SetupTemper_L_Limit /10);
					Number_4_Display(1,Config.Struct.SetupTemper_L_Limit %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x01://油品类型
					Number_1_Display(1,11);//F
					Number_2_Display(1,17);//U
					Number_3_Display(1,19);
					Number_4_Display(2,Config.Struct.SetupFuelType  %10);
					//---------------------------
					LCD_06.BIT.BIT_5 = EQUAL_L;//电压 图标
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						Config.Struct.SetupFuelType = 2;
						Config.Struct.MIN_OilPumpCycle = 25;//最小泵油量
						Config.Struct.MAX_OilPumpCycle = 66;//最大泵油量
						Config.Struct.MIN_FanSpeed.DATA = 2300;//最小风速
						Config.Struct.MAX_FanSpeed.DATA = 8650;//最大风速
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						Config.Struct.SetupFuelType = 1;
						Config.Struct.MIN_OilPumpCycle = 18;//最小泵油量
						Config.Struct.MAX_OilPumpCycle = 58;//最大泵油量
						Config.Struct.MIN_FanSpeed.DATA = 3000;//最小风速
						Config.Struct.MAX_FanSpeed.DATA = 8650;//最大风速
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 default:
					ClearBitDisplay();
					LCD_06.BIT.BIT_4 = EQUAL_H;//设置 图标
					DisplayStateIndex = 0x01;
					//---------------------------
					if(Config.Struct.SetupFuelType == 0x01)//燃油类型	1柴油，2汽油，3甲醛
					{	Config.Struct.MIN_OilPumpCycle = 18;//最小泵油量
						Config.Struct.MAX_OilPumpCycle = 58;//最大泵油量
						Config.Struct.MIN_FanSpeed.DATA = 3000;//最小风速
						Config.Struct.MAX_FanSpeed.DATA = 8650;//最大风速
					}
					else
					{	Config.Struct.MIN_OilPumpCycle = 25;//最小泵油量
						Config.Struct.MAX_OilPumpCycle = 66;//最大泵油量
						Config.Struct.MIN_FanSpeed.DATA = 2300;//最小风速
						Config.Struct.MAX_FanSpeed.DATA = 8650;//最大风速
					}
				break;
			}
	   	break;
	 case 0x03://密码管理-合格转入参数设定
			switch(DisplayStateIndex)
			{case 0x04:
					Number_1_Display(1,19);
					Number_2_Display(1,19);
					Number_3_Display(1,19);
					if(System.BIT.BIT_08 == EQUAL_L)
					{	Number_4_Display(2,InputPassword &0x000F);	}
					else
					{	Number_4_Display(2,19);		}
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						System.BIT.BIT_08 = EQUAL_L;
						//-----------------------
						if((InputPassword &0x000F) < 0x0009)
						{	InputPassword += 0x0001;	}
						else
						{	InputPassword &= 0xFFF0;	}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						System.BIT.BIT_08 = EQUAL_L;
						//-----------------------
						if((InputPassword &0x000F) > 0x0000)
						{	InputPassword -= 0x0001;		}
						else
						{	InputPassword &= 0xFFF0;
							InputPassword |= 0x0009;
						}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						if(System.BIT.BIT_04 == EQUAL_L)
						{	if((Config.Struct.Password.DATA == InputPassword)
							 ||(MainPassword == InputPassword))//超级密码
							{	DisplaySetIndex++;	//下一项
								DisplayStateIndex = 0x00;
							}
							else
							{	DisplayStateIndex = 0x00;	}//密码错误重来
						}
						else//更新管理密码
						{	Config.Struct.Password.DATA  = InputPassword;
							RunDataUpdate();//存储设置参数
							DisplaySetIndex = 0x0A;//更新参数退出
						}
					}
				break;
			 case 0x03:
					Number_1_Display(1,19);
					Number_2_Display(1,19);
					if(System.BIT.BIT_07 == EQUAL_L)
					{	Number_3_Display(2,(InputPassword >>4) &0x000F);		}
					else
					{	Number_3_Display(2,19);		}
					Number_4_Display(2,19);
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						System.BIT.BIT_07 = EQUAL_L;
						//-----------------------
						if((InputPassword &0x00F0) < 0x0090)
						{	InputPassword += 0x0010;		}
						else
						{	InputPassword &= 0xFF0F;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						System.BIT.BIT_07 = EQUAL_L;
						//-----------------------
						if((InputPassword &0x00F0) > 0x0000)
						{	InputPassword -= 0x0010;		}
						else
						{	InputPassword &= 0xFF0F;
							InputPassword |= 0x0090;
						}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x02:
					Number_1_Display(1,19);
					if(System.BIT.BIT_06 == EQUAL_L)
					{	Number_2_Display(2,(InputPassword >>8) &0x000F);		}
					else
					{	Number_2_Display(2,19);	}
					Number_3_Display(2,19);
					Number_4_Display(2,19);
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						System.BIT.BIT_06 = EQUAL_L;
						//-----------------------
						if((InputPassword &0x0F00) < 0x0900)
						{	InputPassword += 0x0100;		}
						else
						{	InputPassword &= 0xF0FF;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						System.BIT.BIT_06 = EQUAL_L;
						//-----------------------
						if((InputPassword &0x0F00) > 0x0000)
						{	InputPassword -= 0x0100;		}
						else
						{	InputPassword &= 0xF0FF;
							InputPassword |= 0x0900;
						}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x01:
					if(System.BIT.BIT_05 == EQUAL_L)
						{	Number_1_Display(2,(InputPassword >>12) &0x000F);		}
						else
						{	Number_1_Display(2,19);		}
						Number_2_Display(2,19);
						Number_3_Display(2,19);
						Number_4_Display(2,19);
					//---------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						System.BIT.BIT_05 = EQUAL_L;
						//-----------------------
						if((InputPassword &0xF000) < 0x9000)
						{	InputPassword += 0x1000;		}
						else
						{	InputPassword &= 0x0FFF;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						System.BIT.BIT_05 = EQUAL_L;
						//-----------------------
						if((InputPassword &0xF000) > 0x0000)
						{	InputPassword -= 0x1000;		}
						else
						{	InputPassword &= 0x0FFF;
							InputPassword |= 0x9000;
						}
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 default:
					System.BIT.BIT_04 = EQUAL_L;//正常设置不更改 密码
			 case 0x40:
					ClearBitDisplay();
					DisplayStateIndex = 0x01;
					LCD_06.BIT.BIT_4 = EQUAL_H;//设置 图标
					//--------------------------------
					InputPassword = 0x5555;
					System.BIT.BIT_05 = EQUAL_H;
					System.BIT.BIT_06 = EQUAL_H;
					System.BIT.BIT_07 = EQUAL_H;
					System.BIT.BIT_08 = EQUAL_H;//密码标志位
				break;
			}
		break;
	 case 0x02://定时-开关机设定
			switch(DisplayStateIndex)
			{case 0x14://关机时间
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					Temp_0 = AlaramTime[5] /10;
					Temp_1 = AlaramTime[5] %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_1 >9){	Temp_1 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_1 <0){	Temp_1 = 9;		}
					}
					AlaramTime[5] = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,AlaramTime[4] /10);
					Number_2_Display(1,AlaramTime[4] %10);
					Number_3_Display(1,AlaramTime[5] /10);
					Number_4_Display(2,AlaramTime[5] %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplaySetIndex++;//下一项
						DisplayStateIndex = 0x00;
					}
				break;
			 case 0x13:
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					Temp_0 = AlaramTime[5] /10;
					Temp_1 = AlaramTime[5] %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_0 > 5){   Temp_0 = 0;	}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_0 < 0){   Temp_0 = 5;	}
					}
					AlaramTime[5] = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,AlaramTime[4] /10);
					Number_2_Display(1,AlaramTime[4] %10);
					Number_3_Display(2,AlaramTime[5] /10);
					Number_4_Display(1,AlaramTime[5] %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x12:
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					Temp_0 = AlaramTime[4] /10;
					Temp_1 = AlaramTime[4] %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_1 >9){	Temp_1 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_1 <0){	Temp_1 = 9;		}
					}
					AlaramTime[4] = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,AlaramTime[4] /10);
					Number_2_Display(2,AlaramTime[4] %10);
					Number_3_Display(1,AlaramTime[5] /10);
					Number_4_Display(1,AlaramTime[5] %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x11:
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					Temp_0 = AlaramTime[4] /10;
					Temp_1 = AlaramTime[4] %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_0 > 9){   Temp_0 = 0;	}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_0 < 0){   Temp_0 = 9;	}
					}
					AlaramTime[4] = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(2,AlaramTime[4] /10);
					Number_2_Display(1,AlaramTime[4] %10);
					Number_3_Display(1,AlaramTime[5] /10);
					Number_4_Display(1,AlaramTime[5] %10);//显示时间
					//--------------------------------------
					LCD_02.BIT.BIT_1 = EQUAL_H;//时间 秒点
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x10://第二组
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					//------------------------------------------
					Number_1_Display(1,2);
					Number_2_Display(0,0);
					if(AlaramTime[3] == 0x00)// OF
					{	if(FlashDisplayTime <10)//秒 闪
						{	Number_3_Display(1,12);
							Number_4_Display(1,11);
						}
						else
						{	Number_3_Display(0,0);
							Number_4_Display(0,0);
						}
					}
					else
					{	if(FlashDisplayTime <10)//秒 闪
						{	Number_3_Display(1,12);
							Number_4_Display(1,13);
						}
						else
						{	Number_3_Display(0,0);
							Number_4_Display(0,0);
						}
					}
					//------------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						AlaramTime[3] = 0xAA;
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						AlaramTime[3] = 0x00;
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						if(AlaramTime[3] != 0x00)
						{	DisplayStateIndex++;//下一项
						}
						else
						{	DisplaySetIndex++;
							DisplayStateIndex = 0x00;
						}
					}
				break;
			 case 0x04://开机时间
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					Temp_0 = AlaramTime[2] /10;
					Temp_1 = AlaramTime[2] %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_1 >9){	Temp_1 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_1 <0){	Temp_1 = 9;		}
					}
					AlaramTime[2] = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,AlaramTime[1] /10);
					Number_2_Display(1,AlaramTime[1] %10);
					Number_3_Display(1,AlaramTime[2] /10);
					Number_4_Display(2,AlaramTime[2] %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex = 0x10;//下一项
						ClearBitDisplay();
					}
				break;
			 case 0x03:
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					Temp_0 = AlaramTime[2] /10;
					Temp_1 = AlaramTime[2] %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_0 > 5){   Temp_0 = 0;	}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_0 < 0){   Temp_0 = 5;	}
					}
					AlaramTime[2] = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,AlaramTime[1] /10);
					Number_2_Display(1,AlaramTime[1] %10);
					Number_3_Display(2,AlaramTime[2] /10);
					Number_4_Display(1,AlaramTime[2] %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x02:
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					Temp_0 = AlaramTime[1] /10;
					Temp_1 = AlaramTime[1] %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_1 >9){	Temp_1 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_1 <0){	Temp_1 = 9;		}
					}
					AlaramTime[1] = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,AlaramTime[1] /10);
					Number_2_Display(2,AlaramTime[1] %10);
					Number_3_Display(1,AlaramTime[2] /10);
					Number_4_Display(1,AlaramTime[2] %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 case 0x01:
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					Temp_0 = AlaramTime[1] /10;
					Temp_1 = AlaramTime[1] %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_0 > 9){   Temp_0 = 0;	}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_0 < 0){   Temp_0 = 9;	}
					}
					AlaramTime[1] = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(2,AlaramTime[1] /10);
					Number_2_Display(1,AlaramTime[1] %10);
					Number_3_Display(1,AlaramTime[2] /10);
					Number_4_Display(1,AlaramTime[2] %10);//显示时间
					//--------------------------------------
					LCD_02.BIT.BIT_1 = EQUAL_H;//时间 秒点
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 default:
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					//------------------------------------------
					Number_1_Display(1,1);
					Number_2_Display(0,0);
					if(AlaramTime[0] == 0x00)// OF
					{	if(FlashDisplayTime <10)//秒 闪
						{	Number_3_Display(1,12);
							Number_4_Display(1,11);
						}
						else
						{	Number_3_Display(0,0);
							Number_4_Display(0,0);
						}
					}
					else
					{	if(FlashDisplayTime <10)//秒 闪
						{	Number_3_Display(1,12);
							Number_4_Display(1,13);
						}
						else
						{	Number_3_Display(0,0);
							Number_4_Display(0,0);
						}
					}
					//------------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						AlaramTime[0] = 0xAA;
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						AlaramTime[0] = 0x00;
					}
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						if(AlaramTime[0] != 0x00)
						{	DisplayStateIndex++;		}//下一项
						else
						{	DisplayStateIndex = 0x10;	}
					}
				break;
			}
	   	break;
	 case 0x01://预热时间设定
			switch(DisplayStateIndex)
			{case 0x02:
					Temp_0 = WorkingTime /10;
					Temp_1 = WorkingTime %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_1 >9){	Temp_1 = 0;		}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_1 <0){	Temp_1 = 9;		}
					}
					WorkingTime = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,15);
					Number_2_Display(1,19);
					Number_3_Display(1,WorkingTime /10);
					Number_4_Display(2,WorkingTime %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplaySetIndex++;//下一项
						DisplayStateIndex = 0x00;
						ClearBitDisplay();
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					}
				break;
			 case 0x01:
					Temp_0 = WorkingTime /10;
					Temp_1 = WorkingTime %10;
					//--------------------------------------
					if(System.BIT.BIT_02 == EQUAL_H)//上调
					{   System.BIT.BIT_02 = EQUAL_L;
						//-----------------------
						if(++Temp_0 > 9){   Temp_0 = 0;	}
					}
					if(System.BIT.BIT_03 == EQUAL_H)//下调
					{   System.BIT.BIT_03 = EQUAL_L;
						//-----------------------
						if(--Temp_0 < 0){   Temp_0 = 9;	}
					}
					WorkingTime = (Temp_0 *10) + Temp_1;
					//--------------------------------------
					Number_1_Display(1,15);
					Number_2_Display(1,19);
					Number_3_Display(2,WorkingTime /10);
					Number_4_Display(1,WorkingTime %10);//显示时间
					//--------------------------------------
					if(System.BIT.BIT_01 == EQUAL_H)//确认
					{   System.BIT.BIT_01 = EQUAL_L;
						DisplayStateIndex++;//下一项
					}
				break;
			 default:
					ClearBitDisplay();
					LCD_05.BIT.BIT_2 = EQUAL_H;//闹钟图标
					DisplayStateIndex++;//下一项
				break;
			}
		break;
	 default:////轮流显示--时间，当前环境温度
			ClearBitDisplay();
			//--------------------------------------------------
			switch(DisplayStateIndex)
			{
			 case 0x05://故障码显示
					int_Temp_1 = RotateSpeed.Data;//显示转速
					if(int_Temp_1 >9999){	int_Temp_1 = 9999;	}//防止显示溢出
					//--------------------------------------------
					Number_1_Display(1,int_Temp_1 /1000);
					int_Temp_1 %=	1000;
					Number_2_Display(1,int_Temp_1 /100);
					int_Temp_1 %=	100;
					Number_3_Display(1,int_Temp_1 /10);
					Number_4_Display(1,int_Temp_1 %10);
					
					LCD_00.BIT.BIT_6 = EQUAL_H;//转速 RPM
				break;
			 case 0x04://海拔高度
					if(AvgAltitude.Data & 0x80000000)//负值，补码转换
					{	int_Temp_1 = (~AvgAltitude.Data +1) &0x00FFFFFF;
						Temp_0 = 0x01;//负数标志
						Number_1_Display(1,19);//显示 负号
					}
					else
					{	int_Temp_1 = AvgAltitude.Data;
						Temp_0 = 0x00;
					}
					//-----------------------------------
					if(int_Temp_1 <10000)//显示小数
					{	int_Temp_1 = int_Temp_1 /10;//海拔高度
						int_Temp_2 = int_Temp_1;//备份
						if(Temp_0 == 0x00)
						{	if(int_Temp_2 >999)
							{	Number_1_Display(1,int_Temp_1 /1000);	}
							else
							{	Number_1_Display(0,0);		}
						}
						//---------------------------
						int_Temp_1 %= 1000;
						if(int_Temp_2 >99)
						{	Number_2_Display(1,int_Temp_1 /100);	}
						else
						{	Number_2_Display(0,0);		}
						//---------------------------
						int_Temp_1 %= 100;
						Number_3_Display(1,int_Temp_1 /10);
						Number_4_Display(1,int_Temp_1 %10);
						//---------------------------
						LCD_02.BIT.BIT_4 = EQUAL_H;//3 下点
					}
					else
					{	int_Temp_1 = int_Temp_1 /100;//海拔高度
						int_Temp_2 = int_Temp_1;//备份
						if(Temp_0 == 0x00)
						{	if(int_Temp_2 >999)
							{	Number_1_Display(1,int_Temp_1 /1000);	}
							else
							{	Number_1_Display(0,0);		}
						}
						//---------------------------
						int_Temp_1 %= 1000;
						if(int_Temp_2 >99)
						{	Number_2_Display(1,int_Temp_1 /100);	}
						else
						{	Number_2_Display(0,0);		}
						//---------------------------
						int_Temp_1 %= 100;
						Number_3_Display(1,int_Temp_1 /10);
						Number_4_Display(1,int_Temp_1 %10);
						//---------------------------
						LCD_02.BIT.BIT_4 = EQUAL_H;//3 下点
					}
					LCD_09.BIT.BIT_4 = EQUAL_H;//海拔高度
					LCD_00.BIT.BIT_2 = EQUAL_H;//海拔高度 m
				break;
			 case 0x03://工作电压
					int_Temp_1 = RunVoltage.Data;
					if(int_Temp_1 >999){	int_Temp_1 =999;	}
					//---------------------------
					Number_1_Display(0,0);//关闭
					Number_2_Display(1,int_Temp_1 /100);
			 		int_Temp_1 %= 100;
					Number_3_Display(1,int_Temp_1 /10);
					//---------------------------
					Number_4_Display(1,int_Temp_1 %10);
					//---------------------------
					LCD_02.BIT.BIT_4 = EQUAL_H;//3 下点
					LCD_00.BIT.BIT_7 = EQUAL_H;//电压 图标
				break;
			 case 0x02://显示工作档位
					if(Config.Struct.TemperatureType == 0x01)//控温方式
					{	Number_1_Display(1,21);
						Number_2_Display(1,19);
						Number_3_Display(1,Config.Struct.EnergyLevel /10);
						Number_4_Display(1,Config.Struct.EnergyLevel %10);
					}
					else//显示剩余工作时间
					{	Number_1_Display(1,10);// H
						Number_2_Display(2,19);// -
					//--------------------------------------------
						Number_3_Display(1,RemainingWorkingTime /10);
						Number_4_Display(1,RemainingWorkingTime %10);
					}
			 
				break;
			 case 0x01://环境温度
					if(HY.Temperature <0)
					{	int_Temp_1 = (0 - HY.Temperature);//温度显示
						Number_1_Display(1,0x13);//显示负号
					}
					else
					{	int_Temp_1 = HY.Temperature;//温度显示
						if(int_Temp_1 >99)
						{	Number_1_Display(1,int_Temp_1 /100);	}
						else
						{	Number_1_Display(0,0);		}
						int_Temp_1 %= 100;
					}
					Number_2_Display(1,int_Temp_1 /10);
					Number_3_Display(1,int_Temp_1 %10);
					Number_4_Display(1,15);
					//---------------------------
					LCD_03.BIT.BIT_4 = EQUAL_H;//温度上点
//					LCD_00.BIT.BIT_1 = EQUAL_H;//火焰温度 图标
			 
				break;
			 default:////时钟显示
					if(ErrorIndex > 0x01)
					{	int_Temp_1 = ErrorIndex -0x01;//显示错误状态
						Number_1_Display(1,0x12);// E
						Number_2_Display(2,0x13);// -	//闪烁
						Number_3_Display(1,int_Temp_1 /10);
						Number_4_Display(1,int_Temp_1 %10);
					}
					else//壳体温度、开关机显示
					{	if(SW.Temperature <0)
						{	int_Temp_1 = (0 - SW.Temperature);//温度显示
							Number_1_Display(1,0x13);//显示负号
						}
						else
						{	int_Temp_1 = SW.Temperature;//温度显示
							if(int_Temp_1 >99)
							{	Number_1_Display(1,int_Temp_1 /100);	}
							else
							{	Number_1_Display(0,0);		}
							int_Temp_1 %= 100;
						}
						Number_2_Display(1,int_Temp_1 /10);
						Number_3_Display(1,int_Temp_1 %10);
						Number_4_Display(1,15);
					//---------------------------
						LCD_03.BIT.BIT_4 = EQUAL_H;//温度上点
						LCD_00.BIT.BIT_3 = EQUAL_H;
//						LCD_00.BIT.BIT_2 = EQUAL_H;//水温温度 图标
					}
				break;
			}
            //-----------------------------------
			if((AlaramTime[0] != 0x00)||(AlaramTime[3] != 0x00))
			{	LCD_05.BIT.BIT_2 = EQUAL_H;		}//定时 闹钟
			else
			{	LCD_05.BIT.BIT_2 = EQUAL_L;		}
            //-----------------------------------
			DisplaySetIndex = 0x00;
		break;
	}
    //---------------------------------------------------------------
	LCD_04.BIT.BIT_7 = EQUAL_H;////飘带 线条
	RunStateDisplay();//实时显示运行状态
}
//-----------------------------------------------------------------------------
void LCD_DisplayUpdate(void)
{	switch(DisplayIndex++)
	{
	case 0x03:
			LCD_WriteLram(0x02,LCD_01.DATA &0x0F);
			LCD_WriteLram(0x03,LCD_01.DATA >>4);
			LCD_WriteLram(0x04,LCD_02.DATA &0x0F);
			LCD_WriteLram(0x05,LCD_02.DATA >>4);
			LCD_WriteLram(0x0E,LCD_07.DATA &0x0F);
			LCD_WriteLram(0x0F,LCD_07.DATA >>4);
	case 0x02:

			LCD_WriteLram(0x10,LCD_08.DATA &0x0F);
			LCD_WriteLram(0x11,LCD_08.DATA >>4);
			LCD_WriteLram(0x12,LCD_09.DATA &0x0F);
			LCD_WriteLram(0x13,LCD_09.DATA >>4);

		break;
	 case 0x01:
			LCD_WriteLram(0x06,LCD_03.DATA &0x0F);
			LCD_WriteLram(0x07,LCD_03.DATA >>4);
			LCD_WriteLram(0x08,LCD_04.DATA &0x0F);
			LCD_WriteLram(0x09,LCD_04.DATA >>4);
			LCD_WriteLram(0x0A,LCD_05.DATA &0x0F);
			LCD_WriteLram(0x0B,LCD_05.DATA >>4);
		break;
	 default:
			LCD_WriteLram(0x0C,LCD_06.DATA &0x0F);
			LCD_WriteLram(0x0D,LCD_06.DATA >>4);
			LCD_WriteLram(0x00,LCD_00.DATA &0x0F);
			LCD_WriteLram(0x01,LCD_00.DATA >>4);
	 		DisplayIndex = 0x01;
		break;
	}
	
	
	
}

void Display_Test(void)
{
	LCD_00.DATA = 0xFF;
	LCD_01.DATA = 0xFF;
	LCD_02.DATA = 0xFF;
	LCD_03.DATA = 0xFF;
	LCD_04.DATA = 0xFF;
	LCD_05.DATA = 0xFF;
	LCD_06.DATA = 0xFF;
	LCD_07.DATA = 0xFF;
	LCD_08.DATA = 0xFF;
	LCD_09.DATA = 0xFF;

	
			LCD_WriteLram(0x00,LCD_00.DATA &0x0F);
			LCD_WriteLram(0x01,LCD_00.DATA >>4);
			LCD_WriteLram(0x02,LCD_01.DATA &0x0F);
			LCD_WriteLram(0x03,LCD_01.DATA >>4);
			LCD_WriteLram(0x04,LCD_02.DATA &0x0F);
			LCD_WriteLram(0x05,LCD_02.DATA >>4);
			LCD_WriteLram(0x06,LCD_03.DATA &0x0F);
			LCD_WriteLram(0x07,LCD_03.DATA >>4);
			LCD_WriteLram(0x08,LCD_04.DATA &0x0F);
			LCD_WriteLram(0x09,LCD_04.DATA >>4);
			LCD_WriteLram(0x0A,LCD_05.DATA &0x0F);
			LCD_WriteLram(0x0B,LCD_05.DATA >>4);
			LCD_WriteLram(0x0C,LCD_06.DATA &0x0F);
			LCD_WriteLram(0x0D,LCD_06.DATA >>4);
			LCD_WriteLram(0x0E,LCD_07.DATA &0x0F);
			LCD_WriteLram(0x0F,LCD_07.DATA >>4);
			LCD_WriteLram(0x10,LCD_08.DATA &0x0F);
			LCD_WriteLram(0x11,LCD_08.DATA >>4);
			LCD_WriteLram(0x12,LCD_09.DATA &0x0F);
			LCD_WriteLram(0x13,LCD_09.DATA >>4);



}
