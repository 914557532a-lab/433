//************************************************************************
//功能说明：利用单片机定时中断，主动采样方式，读取信号,解码无线遥控发射码
//			采样时间120--130uS
//************************************************************************
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

#include "flash.h"
#include "KeyEvent.H"
#include "UART2_ISR.h"
#include "RF_Receiver.H"
#include "DisplayUpdata.H"
#include "WT588Fxx.h"
//------------------------------------------------------------------------
#define  RF_OLD_CODE	//兼容原精一公司遥控编码
//------------------------------------------------------------------------
//数据格式 3字节(地址码) +1字节功能码(2位标志 00手动 11自动，6位按键功能) + 1字节 温度值(8 -- 35度)
#ifdef  RF_OLD_CODE
	#define NBIT         31 // 数据长度
#else
	#define NBIT         40 // 数据长度
#endif
///////
#define LONG_HEAD    100	// long  head
#define SHORT_HEAD   50		// short head
#define HIGH_TO     -16		// high periods
#define LOW_TO       16		// low  periods
//------------------------------------------------------------------------
#define RF_RESET	0x00
#define RF_SYNCHRO  0x01
#define RF_0_TO_1	0x02
#define RF_1_TO_0	0x03
//------------------------------------------------------------------------
#define RF_SampledPort	(P4 &0x08)

//------------------------------------------------------------------------
xdata struct {
	unsigned char RF_Sampled:1;	// sampled RF signal
	unsigned char RF_BufferFull:1;	// buffer full
}RF_Bit;
//------------------------------------------------------------------------
xdata signed char RF_Count;  // timer counter
xdata unsigned char RF_State;// receive state
xdata unsigned char BitCount;// receive bits counter
////////
xdata unsigned char RF_Index;// receive buffer pointer

#define RF_BUFFER_LENGHT	6
xdata unsigned char RF_BUFFER[RF_BUFFER_LENGHT];// receive buffer
////////
// receive buffer map  --  S.DDDDDDDD.DDDDDDDD.DDDDKKKK.
//						        0        1        2
//S --> Head Synchro	--> Ignore
//D --> Serial Number	--> 16 Bit
//K --> Key Map			--> 8  Bit
//------------------------------------------------------------------------
#define OUTACTIONHOLD   3	//output delay 
xdata unsigned char OutHoldTime;// output timer

xdata unsigned char RF_LearnAddrssTime;//用于学习地址码
//------------------------------------------------------------------------

xdata unsigned char RF_KEY_Hold[2];//按键值备份
//************************************************************************
void RF_Receiver_Initial(void)
{unsigned char Count;
	for(Count=0;Count <RF_BUFFER_LENGHT;Count++)
	{	RF_BUFFER[Count] = 0x00;	}
	RF_State = RF_RESET;// reset state machine
	//----------------------------------------------------------
	OutHoldTime = 0x00;//输出保持时间
	//----------------------------------------------------------
	RF_KEY_Hold[0] = 0x00;
	RF_KEY_Hold[1] = 0x00;//重置
	//----------------------------------------------------------
	RF_LearnAddrssTime = 0x00;//学习地址码时间
}
//************************************************************************
void RF_SampleReceive(void)
{	if(RF_SampledPort != EQUAL_L)//获取端口状态，适应同向，和反向输出的接收模块
	{	RF_Bit.RF_Sampled = EQUAL_H;	}//解码程序默认为，收到的信号是反向的
	else
	{	RF_Bit.RF_Sampled = EQUAL_L;	}
	//------------------------------------------------------------
	if(RF_Bit.RF_BufferFull == EQUAL_L)//接收缓冲未满继续接收
	{	switch(RF_State)//state machine main switch
		{case RF_0_TO_1:
				if(RF_Bit.RF_Sampled == 0)// sampling RF pin
				{// falling edge detected  ____
				 //                            |
				 //                            |_____
					RF_State = RF_1_TO_0;
				}
				else//while high 
				{	if(--RF_Count < HIGH_TO)
					{	RF_State = RF_RESET; // reset if too long
					}
				}
			break;
		 case RF_1_TO_0:
				if(RF_Bit.RF_Sampled == 1)// sampling RF pin
				{// rising edge detected   _____
				 //                       |
				 //                   ____|
					RF_BUFFER[RF_Index] <<= 0x01;//Shift Bit
					//-------------------------------------------------------------
					if(RF_Count <= 0x00)
					{	RF_BUFFER[RF_Index] |=0x01;		}
					else
					{	RF_BUFFER[RF_Index] &=0xFE;		}
					RF_Count = 0x00;//Reset counter
					RF_State= RF_0_TO_1;
					//-------------------------------------------------------------
					if((++BitCount & 0x07)==0x00)
					{	RF_Index++;			}
					if(BitCount >= NBIT)
					{	RF_State = RF_RESET;//完成接收
						RF_Bit.RF_BufferFull = EQUAL_H;
					}
				}
				else// still low
				{	if(++RF_Count >= LOW_TO)	// too long low
					{	RF_State = RF_SYNCHRO;// fail back into RFSYNC state 
						BitCount = 0x00;
						RF_Index = 0x00; // reset pointers 
					}
				}
			break;
		case RF_SYNCHRO:
				if(RF_Bit.RF_Sampled == 1)// sampling RF pins
				{// rising edge detected  ___                  ___
				 //                          |   <-Header->   |
				 //                          |________________|
					if(RF_Count < SHORT_HEAD)// too short, no header
					{	RF_State = RF_RESET;
					}
					else
					{	RF_Count =0x00;     // restart counter
						RF_State= RF_0_TO_1;//Header
					}
				}
				else//Still low
				{	if(++RF_Count >= LONG_HEAD)// too long, no header
					{	RF_State = RF_RESET;
					}
				}
			break;
		default://State Initial
				RF_State = RF_SYNCHRO;// reset state machine
				RF_Count =0x00;
				BitCount = 0x00;
				RF_Index = 0x00;
			break;
		}// switch
	}
}
//************************************************************************
void WirelessDecode(void)
{
#ifdef  RF_OLD_CODE	//兼容原精一公司遥控编码
	if(RF_Bit.RF_BufferFull == EQUAL_H)
	{	if((RF_LearnAddrssTime > 0x00)//在5S学习时间内
		 &&(((RF_BUFFER[2] ==0x05)&&(RF_BUFFER[3] == 0x04))
		  ||((RF_BUFFER[2] ==0x11)&&(RF_BUFFER[3] == 0x58))))
		{	Config.Struct.RF_Address.Array[1] = RF_BUFFER[0];//地址高位
			Config.Struct.RF_Address.Array[2] = RF_BUFFER[1];//地址低位
			RF_LearnAddrssTime = 0x00;//完成学习后禁止
			DataChangeUpdataTime = 6;//设定参数回存时间
		}
		//-----------------------------------------------------------
		if((Config.Struct.RF_Address.Array[1] != RF_BUFFER[0])
		 ||(Config.Struct.RF_Address.Array[2] != RF_BUFFER[1]))
		{	goto DecodeEnd;//收到地址码错误，退出
		}//验证地址码
		if((RF_KEY_Hold[0] == RF_BUFFER[2])
		 &&(RF_KEY_Hold[1] == RF_BUFFER[3]))
		{	OutHoldTime = OUTACTIONHOLD;
			goto DecodeEnd ;//在规定时间内收到重复码
		}
		//-----------------------------------------------------------
		RF_KEY_Hold[0] = RF_BUFFER[2];
		RF_KEY_Hold[1] = RF_BUFFER[3];//保存上次键值
		BackLightTime = 500;//设置背光高亮时间
		//-----------------------------------------------------------
		if((RF_KEY_Hold[0] ==0x08)&&(RF_KEY_Hold[1] == 0x78))
		{	if(Config.Struct.EnergyLevel <9)//输出功率 0-9 共10级
			{   Config.Struct.EnergyLevel++;
				DataChangeUpdataTime = 6;//设定参数回存时间
			}
			DisplayStateIndex = 0x02;//显示设定温度
			ExitSetupTime = 3;//以秒为单位
			OutHoldTime = OUTACTIONHOLD;
		}
		if((RF_KEY_Hold[0] ==0x02)&&(RF_KEY_Hold[1] == 0x44))//下调
		{	if(Config.Struct.EnergyLevel >1)//输出功率 0-9 共10级
			{   Config.Struct.EnergyLevel--;
				DataChangeUpdataTime = 6;//设定参数回存时间
			}
			DisplayStateIndex = 0x02;//显示设定温度
			ExitSetupTime = 3;//以秒为单位
			OutHoldTime = OUTACTIONHOLD;
		}
		if((RF_KEY_Hold[0] ==0x05)&&(RF_KEY_Hold[1] == 0x04))
		{	SystemRunIndex = UART_PowerOff;
			OutHoldTime = OUTACTIONHOLD;
		}
		if((RF_KEY_Hold[0] ==0x11)&&(RF_KEY_Hold[1] == 0x58))
		{	if(((RunVoltage.Data >100)&&(RunVoltage.Data <170))
			 ||((RunVoltage.Data >190)&&(RunVoltage.Data <320)))//电压限制
			{	SystemRunIndex = URAT_PowerOn;
				KeyCount_Time = 0x00;
			}//启动
			OutHoldTime = OUTACTIONHOLD;
		}
		//-----------------------------------------------------------
	 DecodeEnd://收到重码退出
		RF_Bit.RF_BufferFull = EQUAL_L;
	}
#else
//	if(RF_Bit.RF_BufferFull == EQUAL_H)
//	{	if((RF_LearnAddrssTime > 0x00)//在5S学习时间内
//		 &&(((RF_BUFFER[2] ==0x05)&&(RF_BUFFER[3] == 0x04))
//		  ||((RF_BUFFER[2] ==0x11)&&(RF_BUFFER[3] == 0x58))))
//		{	Config.Struct.RF_Address.Array[1] = RF_BUFFER[0];//地址高位
//			Config.Struct.RF_Address.Array[2] = RF_BUFFER[1];//地址低位
//			RF_LearnAddrssTime = 0x00;//完成学习后禁止
//			DataChangeUpdataTime = 6;//设定参数回存时间
//		}
//		//-----------------------------------------------------------
//		if((Config.Struct.RF_Address.Array[1] != RF_BUFFER[0])
//		 ||(Config.Struct.RF_Address.Array[2] != RF_BUFFER[1]))
//		{	goto DecodeEnd;//收到地址码错误，退出
//		}//验证地址码
//		if((RF_KEY_Hold[0] == RF_BUFFER[2])
//		 &&(RF_KEY_Hold[1] == RF_BUFFER[3]))
//		{	OutHoldTime = OUTACTIONHOLD;
//			goto DecodeEnd ;//在规定时间内收到重复码
//		}
//		//-----------------------------------------------------------
//		RF_KEY_Hold[0] = RF_BUFFER[2];
//		RF_KEY_Hold[1] = RF_BUFFER[3];//保存上次键值
//		//-----------------------------------------------------------
//		if((RF_KEY_Hold[0] ==0x08)&&(RF_KEY_Hold[1] == 0x78))
//		{	if(Config.Struct.EnergyLevel <9)//输出功率 0-9 共10级
//			{   Config.Struct.EnergyLevel++;
//				DataChangeUpdataTime = 6;//设定参数回存时间
//			}
//			DisplayStateIndex = 0x02;//显示设定温度
//			ExitSetupTime = 3;//以秒为单位
//			OutHoldTime = OUTACTIONHOLD;
//		}
//		if((RF_KEY_Hold[0] ==0x02)&&(RF_KEY_Hold[1] == 0x44))//下调
//		{	if(Config.Struct.EnergyLevel >1)//输出功率 0-9 共10级
//			{   Config.Struct.EnergyLevel--;
//				DataChangeUpdataTime = 6;//设定参数回存时间
//			}
//			DisplayStateIndex = 0x02;//显示设定温度
//			ExitSetupTime = 3;//以秒为单位
//			OutHoldTime = OUTACTIONHOLD;
//		}
//		if((RF_KEY_Hold[0] ==0x05)&&(RF_KEY_Hold[1] == 0x04))
//		{	SystemRunIndex = UART_PowerOff;
//			OutHoldTime = OUTACTIONHOLD;
//		}
//		if((RF_KEY_Hold[0] ==0x11)&&(RF_KEY_Hold[1] == 0x58))
//		{	if(((RunVoltage.Data >100)&&(RunVoltage.Data <170))
//			 ||((RunVoltage.Data >190)&&(RunVoltage.Data <320)))//电压限制
//			{	SystemRunIndex = URAT_PowerOn;
//				KeyCount_Time = 0x00;
//			}//启动
//			OutHoldTime = OUTACTIONHOLD;
//		}
//		//-----------------------------------------------------------
//	 DecodeEnd://收到重码退出
//		RF_Bit.RF_BufferFull = EQUAL_L;
//	}
#endif
}


