//-----------------------------------------------------------------------------
//功能说明：按键处理
//-----------------------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

#include "flash.h"
#include "WT588Fxx.h"
#include "KeyEvent.H"
#include "DisplayUpdata.H"


#ifdef Enable_CMT2300	//打开双向遥控
	#include "Radio.H"
#endif

//-----------------------------------------------------------------------------
xdata unsigned char ExitSetupTime;//退出设置计时

xdata unsigned char KeyState;
xdata unsigned char KeyEvent;
xdata unsigned int KeyCount_Time;//长按计时
xdata unsigned char KeyTimeLimit;
xdata unsigned char KeyEventCount;
//--------------------------------------------------------------
xdata union {
	unsigned char DATA;
	struct{
		unsigned char BIT_01:1;//确认
		unsigned char BIT_02:1;//上调
		unsigned char BIT_03:1;//下调
		unsigned char BIT_04:1;//修改密码
		unsigned char BIT_05:1;//密码 个位
		unsigned char BIT_06:1;//密码 十位
		unsigned char BIT_07:1;//密码 百位
		unsigned char BIT_08:1;//密码 千位
	}BIT;
}System;

xdata unsigned char DataChangeUpdataTime;
xdata unsigned char OilpumpEnableState;//允许泵油条件
//-----------------------------------------------------------------------------
//P54 开机	0x10	0xEF
//P53 设置	0x08	0xF7
//P52 确定	0x04	0xFB
//P51 上调	0x02	0xFD
//P50 下调	0x01	0xF3
void KeyStateScan(void)
{unsigned char KeyPortState = 0xFF;
	if(P54 == 0){	KeyPortState &= 0xFE;	}	//FE = 开关，
	if(P50 == 0){	KeyPortState &= 0xFD;	}	//FB = 下调
	if(P51 == 0){	KeyPortState &= 0xFB;	}	//FD = 上调，
	if(P53 == 0){	KeyPortState &= 0xFC;	}	//FC = 设置
	if(P52 == 0){	KeyPortState &= 0xFA;	}	//FA = 确定

	if(KeyEvent != KeyPortState)
	{	if(++KeyCount_Time >KEY_HOLDTIME)
		{	KeyEvent = KeyPortState;

			if(KeyEvent == 0xFA)
			{	KeyEventCount++;//连续短按计数
				KeyTimeLimit = 200;//750毫秒
			}
		}
	}
	else
	{	//	开机键
		if((KeyEvent == 0xFE)|| (KeyEvent == 0xFB) || (KeyEvent == 0xFD))
		{	if((KeyCount_Time < KeyCount_HOLD)&&(DisplaySetIndex == 0x00))		//
			{	if(++KeyCount_Time ==KeyCount_HOLD)
				{	KeyState = 0xFF;	
				}//处理长按
			}
		}
		else if((KeyEvent == 0xFC) )
		{
			if(KeyCount_Time < KeyCount_HOLD)		//
			{	if(++KeyCount_Time ==KeyCount_HOLD)
				{	KeyState = 0xFF;	
				}//处理长按
			}
		}
		else//按键释放后，才检测 短按次数
		{	KeyCount_Time = 0x00;
		//-------------------------------------------------
			if(KeyTimeLimit >0)//快速连续 短按计数
			{	if(--KeyTimeLimit == 0x00)
				{	
					if(KeyEventCount ==1)//确认键
					{	KeyEvent = 0xF2;	KeyState = 0xFF;	}
					else if((KeyEventCount ==2)&&(DisplaySetIndex == 0x00))
					{	KeyEvent = 0xF5;	KeyState = 0xFF;	}		//控温模式
					
					else if((KeyEventCount ==3)&&(DisplaySetIndex == 0x00))//开启水泵
					{	KeyEvent = 0xF3;	KeyState = 0xFF;	}//等同于长按
//					else if((KeyEventCount ==4))//进入设置
//					{	KeyEvent = 0xF4;	KeyState = 0xFF;	}
//					else if((KeyEventCount ==5)&&(DisplaySetIndex == 0x00))//控温方式
//					{	KeyEvent = 0xF5;	KeyState = 0xFF;	}
					KeyEventCount = 0x00;
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------
void KeyEventProcess(void)
{	if((SystemRunState >=TASK_RapidWarming)&&(SW.Temperature >50))
	{	OilpumpEnableState = 0x00;		}//正常开机,检测到温升后禁止手动泵油
	//--------------------------------------
	if(KeyState != KeyEvent)
	{	KeyState = KeyEvent;//单次执行
		BackLightTime = 500;//设置背光高亮时间
	//--------------------------------------
		if(KeyCount_Time >= KeyCount_HOLD)//长按键处理
		{	switch(KeyEvent)
			{case 0xFE://开关机
					if(SystemRunState == TASK_IdleMode)//ON
					{	if(((RunVoltage.Data >100)&&(RunVoltage.Data <170))
						 ||((RunVoltage.Data >190)&&(RunVoltage.Data <320)))//电压限制
						{	SystemRunIndex = URAT_PowerOn;		}//启动
					}
					else
					{	SystemRunIndex = UART_PowerOff;		}//关闭
				break;

			 case 0xFB://遥控对码//下调
					if(SystemRunState == TASK_IdleMode)
					{	if(DisplaySetIndex != 0x0C)
						{	DisplaySetIndex = 0x0C;
							DisplayStateIndex = 0x00;
							ExitSetupTime = 0;
						}//禁止按键超时退出，遥控学习时间超时才退出
					}
				break;
			 case 0xFD://手动泵油//上调
					if((SystemRunState == TASK_IdleMode)&&(OilpumpEnableState == 0xAA))
					{	if(DisplaySetIndex != 0x0D)
						{	DisplayStateIndex = 0x00;
							DisplaySetIndex = 0x0D;
							ExitSetupTime = 00;//以秒为单位（禁止自动退出）
						}
					}
				break;
					


			 case 0xFC:	//退出设置
				 if((DisplaySetIndex > 0x00 && DisplaySetIndex < 0x09) || (DisplaySetIndex >= 0x0C && DisplaySetIndex < 0x0D))
				 {
					 DisplaySetIndex = 0x00;	
					 DisplayStateIndex = 0x00;
				 }
				 else if(DisplaySetIndex == 0x0E)
				 {
					 DisplayStateIndex++;
				 }
				 else if(DisplaySetIndex == 0x0D)
				 {
					 System.BIT.BIT_01 = EQUAL_H;
				 }
				 break;
			}
		}
		else//----------------------------短按键处理
		{	switch(KeyEvent)
			{case 0xFB://下调	
					if(DisplaySetIndex == 0x00)
					{	if(Config.Struct.EnergyLevel >1)//输出功率 0-9 共10级
						{   Config.Struct.EnergyLevel--;
							DataChangeUpdataTime = 6;//设定参数回存时间
						}
						DisplayStateIndex = 0x02;//显示设定温度
						ExitSetupTime = 3;//以秒为单位
						
					//------------------------------------------
				#ifdef Enable_CMT2300	//打开双向遥控
						if((Config.Struct.RF_Address.DATA != 0x00000000) && (Config.Struct.RF_Address.DATA != 0xFFFFFFFF))
						{	CMT2300_T_Array[0] =0xAA;	}//准备发送数据
				#endif
					}
					else
					{   
						System.BIT.BIT_01 = EQUAL_L;
						System.BIT.BIT_02 = EQUAL_L;
						//--------------------
						System.BIT.BIT_03 = EQUAL_H;
						FlashDisplayTime = 0x00;
						ExitSetupTime = 15;//以秒为单位
					}//参数 下调
				break;
			 case 0xFD://上调	0xF7
					if(DisplaySetIndex == 0x00)
					{	if(Config.Struct.EnergyLevel <9)//输出功率 0-9 共10级
						{   Config.Struct.EnergyLevel++;
							DataChangeUpdataTime = 6;//设定参数回存时间
						}
						DisplayStateIndex = 0x02;//显示设定温度
						ExitSetupTime = 3;//以秒为单位
						
					//------------------------------------------
				#ifdef Enable_CMT2300	//打开双向遥控
						if((Config.Struct.RF_Address.DATA != 0x00000000) && (Config.Struct.RF_Address.DATA != 0xFFFFFFFF))
						{	CMT2300_T_Array[0] =0xAA;	}//准备发送数据
				#endif
					}
					else
					{   
						System.BIT.BIT_01 = EQUAL_L;
						System.BIT.BIT_03 = EQUAL_L;
						//--------------------
						System.BIT.BIT_02 = EQUAL_H;
						FlashDisplayTime = 0x00;
						ExitSetupTime = 15;//以秒为单位
					}//上调
				break;
			 case 0xF2://确认	0xFD
					if(DisplaySetIndex == 0x00)
					{   if(++DisplayStateIndex >4)
						{   DisplayStateIndex = 0x00;
						}
						ExitSetupTime = 00;//以秒为单位（禁止自动退出）
					}
					else
					{   
						System.BIT.BIT_02 = EQUAL_L;
						System.BIT.BIT_03 = EQUAL_L;
						//--------------------
						System.BIT.BIT_01 = EQUAL_H;
						ExitSetupTime = 15;//以秒为单位
					//------------------------------------------
				#ifdef Enable_CMT2300	//打开双向遥控
					if(Config.Struct.RF_Address.DATA != 0xFFFFFFFF)
					{	CMT2300_T_Array[0] =0xAA;	}//准备发送数据
				#endif
					}//确认

				break;
					
			 case 0xF4://设置	0xEF

						if(++DisplaySetIndex >7)
						{   DisplaySetIndex = 0x00;
							InputPassword = 0x0000;
						}
						if(Config.Struct.Password.DATA != InputPassword)
						{	if(DisplaySetIndex >3)
							{	DisplaySetIndex = 0x00;		}
						}	
						DisplayStateIndex = 0x00;
					System.BIT.BIT_01 = EQUAL_L;
					System.BIT.BIT_02 = EQUAL_L;
					System.BIT.BIT_03 = EQUAL_L;
					ExitSetupTime = 15;//以秒为单位
				break;
			 case 0xF5://控温方式选择
					if(Config.Struct.TemperatureType == 0x02)//改变控温方式
					{	Config.Struct.TemperatureType = 0x01;	}
					else
					{	Config.Struct.TemperatureType = 0x02;	}
					DataChangeUpdataTime = 6;//设定参数回存时间
					//-----------------------------------------
					DisplayStateIndex = 0x02;//显示设定温度
					ExitSetupTime = 3;//以秒为单位
				break;
					
			 case 0xF3://开启水泵//连续 短按 3次
					if(SystemRunState == TASK_IdleMode)
					{	if(DisplaySetIndex != 0x0E)
						{	DisplaySetIndex = 0x0E;
							DisplayStateIndex = 0x00;
							ExitSetupTime = 0;
						}//禁止按键超时退出
					}
				break;
					
			case 0xFC:		//进入设置
				if(DisplaySetIndex >= 0x01 && DisplaySetIndex < 0x03)
				{
					DisplaySetIndex ++;
					DisplayStateIndex = 0x00;
				}
				else if(DisplaySetIndex == 0x00)
				{
					DisplaySetIndex = 0x01;
					DisplayStateIndex = 0x00;
					ExitSetupTime = 15;
				}
				 break;
			}
		}
	}
}

