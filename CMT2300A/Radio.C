/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, CMOSTEK SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * Copyright (C) CMOSTEK SZ.
 */

//--------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"
//--------------------------------------------------------------
#ifdef Enable_CMT2300	//打开双向遥控
	#include "Radio.H"

#include "CMT2300_DRV.H"
#include "CMT2300_HAL.H"
#include "CMT2300A_defs.H"
#include "CMT2300A_params.H"

#include "flash.h"
#include "UART2_ISR.h"
#include "HP_03N_DRV.H"
#include "KeyEvent.H"
#include "DisplayUpdata.H"
#include "WT588Fxx.h"
//==============================================================
//配置接收发送缓冲区，TX FIFO 32字节，RX FIFO 32字节
#define CMT2300_T_Lenght	32
#define CMT2300_R_Lenght	32

#define g_nRxLength 15
#define g_nTxLength 23//数据长度

//==============================================================
EnumRFStatus g_nNextRFState = RF_STATE_SLEEP;

xdata unsigned char g_nRxEventCount;
xdata unsigned char g_nTxEventCount;

xdata unsigned int g_nTx_RxTimeout;//收发超时时间

xdata unsigned char CMT2300_PairingTime;
xdata unsigned char TX_SynchroCount;//检测相同的帧数据
xdata unsigned char RX_SynchroCount;//接收同步

xdata unsigned char CMT2300_T_Array[CMT2300_T_Lenght];
xdata unsigned char CMT2300_R_Array[CMT2300_R_Lenght];

//==============================================================
void CMT2300_SetInitial(void)
{unsigned char tmp;

	CMT2300A_InitGpio();
	CMT2300A_Init_Reg();
	//------------------------------------------------
	if(Config.Struct.RF_ModeSelect == Select_OneWay_RF)
	{	CMT2300A_ConfigRegBank(&CMT2300A_D_Data[0] , (sizeof CMT2300A_D_Data) /2);

		tmp = (CMT2300A_ReadReg(CMT2300A_CUS_CMT10) & 0xF8);// xosc_aac_code[2:0] = 2
		CMT2300A_WriteReg(CMT2300A_CUS_CMT10, tmp|0x02);

		//------------------------------------------------
		CMT2300A_ConfigGpio(// Config GPIOs
		//	CMT2300A_GPIO1_SEL_INT1 | // INT1 > GPIO1
		//	CMT2300A_GPIO2_SEL_INT2 | // INT2 > GPIO2
		//	CMT2300A_GPIO3_SEL_INT2

			CMT2300A_GPIO3_SEL_DOUT
		);
	}//单收--直通
	else///双向遥控初化
	{	CMT2300A_ConfigRegBank(&CMT2300A_S_Data[0] , (sizeof CMT2300A_S_Data) /2);

		tmp = (CMT2300A_ReadReg(CMT2300A_CUS_CMT10) & 0xF8);// xosc_aac_code[2:0] = 2
		CMT2300A_WriteReg(CMT2300A_CUS_CMT10, tmp|0x02);

		//------------------------------------------------
		CMT2300A_ConfigGpio(// Config GPIOs
		//	CMT2300A_GPIO1_SEL_INT1 | // INT1 > GPIO1
		//	CMT2300A_GPIO2_SEL_INT2 | // INT2 > GPIO2
		//	CMT2300A_GPIO3_SEL_DOUT

			CMT2300A_GPIO3_SEL_INT2
		);
	}//双向--收发

	CMT2300A_EnableLfosc(0);// Disable low frequency OSC calibration
	//----------------------------------------------------------
	g_nTxEventCount = 0x00;//数据 待发送
	g_nRxEventCount = 0x02;//数据 待接收
	g_nTx_RxTimeout = 0x02;//等待200mS
	g_nNextRFState = RF_STATE_SLEEP;//默认为休眠状态
}

//==============================================================
void CMT2300A_GpioInterruptConfig(EnumRFResult InterruptType)
{
	switch(InterruptType)
	{case RF_TX_INT_COFIG://配置发送中断管脚
			CMT2300A_ConfigInterrupt(// Config interrupt
			//	CMT2300A_INT_SEL_TX_DONE,	// Config INT1
			//	CMT2300A_INT_SEL_PKT_OK		// Config INT2

				CMT2300A_INT_SEL_PKT_OK,	// Config INT1
				CMT2300A_INT_SEL_TX_DONE	// Config INT2
			);

			CMT2300A_EnableInterrupt(// Enable interrupt
				CMT2300A_MASK_PKT_DONE_EN	|
				CMT2300A_MASK_TX_DONE_EN
			);
		break;
	 case RF_RX_INT_COFIG://配置接收中断管脚
			CMT2300A_ConfigInterrupt(// Config interrupt
			//	CMT2300A_INT_SEL_TX_DONE,	// Config INT1
			//	CMT2300A_INT_SEL_PKT_OK		// Config INT2

				CMT2300A_INT_SEL_PKT_OK,	// Config INT1
				CMT2300A_INT_SEL_CRC_OK		// Config INT2
			);

			CMT2300A_EnableInterrupt(// Enable interrupt
				CMT2300A_MASK_PKT_DONE_EN	|
				CMT2300A_MASK_CRC_OK_EN
			);
		break;
	 case RF_IDLE://无效
		break;
	}

}
//==============================================================
void CMT2300A_RF_Process(void)
{	switch(g_nNextRFState)
	{case RF_STATE_RX_START:
		
			CMT2300A_GoStby();		//新
			CMT2300A_ClearInterruptFlags();
			CMT2300A_GpioInterruptConfig(RF_RX_INT_COFIG);//配置接收
			//--------------------------------------------------
			// Must clear FIFO after enable SPI to read or write the FIFO
			CMT2300A_EnableReadFifo();
			CMT2300A_ClearRxFifo();
			//--------------------------------------------------
			CMT2300A_GoRFS();
			CMT2300A_GoRx();
			//--------------------------------------------------
			if(Config.Struct.RF_ModeSelect != Select_OneWay_RF)
			{	g_nNextRFState = RF_STATE_RX_WAIT;
				g_nTx_RxTimeout = 60;//设置接收超时时间，单位mS
			}
			else
			{	g_nNextRFState = RF_STATE_RX_ONLY;	}
        break;
     case RF_STATE_RX_WAIT:
			if(CMT2300A_ReadGpio3() != 0)// Read INT2, PKT_OK
			{	CMT2300A_GoStby();
				g_nNextRFState = RF_STATE_RX_DONE;
			}
			if(g_nTx_RxTimeout == 0)//倒计时归零，就超时了
			{	g_nNextRFState = RF_STATE_SLEEP;	}
        break;
     case RF_STATE_RX_DONE:
			// The length need be smaller than 32
			CMT2300A_ReadFifo(&CMT2300_R_Array[1], g_nRxLength);//读取接收到的数据
			CMT2300_R_Array[0] = 0x55;//接收数据被更新，等待处理
			g_nNextRFState = RF_STATE_SLEEP;
        break;
//===================================================================
	  case RF_STATE_TX_START://数据发送准备
			CMT2300A_ClearInterruptFlags();
			CMT2300A_GpioInterruptConfig(RF_TX_INT_COFIG);//配置发送	//新
			//--------------------------------------------------
			// Must clear FIFO after enable SPI to read or write the FIFO
			CMT2300A_ClearTxFifo();
			CMT2300A_SetPayloadLength(CMT2300_T_Array[1]);
			//--------------------------------------------------
			CMT2300A_EnableWriteFifo();
			// The length need be smaller than 32
			CMT2300A_WriteFifo(&CMT2300_T_Array[2],CMT2300_T_Array[1] -1);//写入待发送的数据
			//--------------------------------------------------
			if(0 ==(CMT2300A_MASK_TX_FIFO_NMTY_FLG & CMT2300A_ReadReg(CMT2300A_CUS_FIFO_FLAG)) )
			{	g_nNextRFState = RF_STATE_ERROR;	}
			//--------------------------------------------------
			CMT2300A_GoTFS();
			CMT2300A_GoTx();
			//--------------------------------------------------
			g_nNextRFState = RF_STATE_TX_WAIT;//发送数据设置成功，转入等待发送结束
			g_nTx_RxTimeout = 12;//设置发送超时时间，单位mS
		break;
    case RF_STATE_TX_WAIT:
			if(CMT2300A_ReadGpio3() !=0)// Read INT1, TX_DONE
			{	g_nNextRFState = RF_STATE_SLEEP;
			}
			if(g_nTx_RxTimeout == 0)//倒计时归零，就超时了
			{	g_nNextRFState = RF_STATE_ERROR;	}//发送失败，重新初始化
        break;
	case RF_STATE_ERROR:
			CMT2300_SetInitial();
	case RF_STATE_SLEEP:
			CMT2300A_GoSleep();
			if((g_nTxEventCount !=0)||(g_nRxEventCount != 0))
			{	g_nNextRFState = RF_STATE_IDLE;		}//全部执行完毕
		break;
	case RF_STATE_IDLE://当前执行动作全部结束，空等待
			CMT2300A_GoStby();
			CMT2300A_ClearInterruptFlags();
			//--------------------------------------------------
			if(g_nTxEventCount != 0x00)//数据 待发送
			{	
//				CMT2300A_GpioInterruptConfig(RF_TX_INT_COFIG);//配置发送
				g_nNextRFState = RF_STATE_TX_START;
				//--------------------------------------------------
				g_nTxEventCount-- ;
				g_nRxEventCount = 5;//设定接收次数
//				if((--g_nTxEventCount)==0)
//				{	g_nTxEventCount = 0x05;		}//持续发送
			}
			else if(g_nRxEventCount != 0x00)
			{	
//				CMT2300A_GpioInterruptConfig(RF_RX_INT_COFIG);//配置接收
				g_nNextRFState = RF_STATE_RX_START;
				//--------------------------------------------------
				if(--g_nRxEventCount ==0)//接收周期结束，发送新数据
				{	if(CMT2300_PairingTime > 0x00)
					{	g_nRxEventCount = 0x01;//数据 待发送
						CMT2300_T_Array[0] = 0xAA;
						CMT2300_T_Array[2] = 0x00;
					}//在学习状态，重启发送
					else
					{	g_nRxEventCount = 0x02;		}
				}
			}
        break;
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void RunStateControl(void)
{	if(CMT2300_T_Array[0] ==0xAA)//准备发送数据
	{	CMT2300_T_Array[0] = 0x00;
		//----------------------------------------------
		CMT2300_T_Array[ 1] = g_nTxLength;//设置数据长度
		//CMT2300_T_Array[ 2] = 00 = 正常数据，0xAA = 遥控对码
		//-------------------------------------------
		CMT2300_T_Array[ 3] = Config.Struct.RF_Address.Array[0];
		CMT2300_T_Array[ 4] = Config.Struct.RF_Address.Array[1];
		CMT2300_T_Array[ 5] = Config.Struct.RF_Address.Array[2];
		CMT2300_T_Array[ 6] = Config.Struct.RF_Address.Array[3];//写入ID号
		//-------------------------------------------
		CMT2300_T_Array[ 7] = SystemRunState;
		CMT2300_T_Array[ 8] = ErrorIndex;
		CMT2300_T_Array[ 9] = RunVoltage.Array[0];
		CMT2300_T_Array[10] = RunVoltage.Array[1];
		CMT2300_T_Array[11] = HJ_Temperature;///实际温度
		CMT2300_T_Array[12] = SW.TempArray[0];
		CMT2300_T_Array[13] = SW.TempArray[1];//燃烧温度
		//-------------------------------------------
		CMT2300_T_Array[14] = Config.Struct.EnergyLevel;//设置温度
		CMT2300_T_Array[15] = Config.Struct.TemperatureType;//控温方式选择
		CMT2300_T_Array[16] = UART_2_RXD[19];//主板返回的泵油设定频率
		//-------------------------------------------
		CMT2300_T_Array[17] = AvgAltitude.Array[0];
		CMT2300_T_Array[18] = AvgAltitude.Array[1];
		CMT2300_T_Array[19] = AvgAltitude.Array[2];
		CMT2300_T_Array[20] = AvgAltitude.Array[3];//海拔高度
		//-------------------------------------------
		CMT2300_T_Array[21] = TX_SynchroCount++;//发送数据同步计数
		//-------------------------------------------
		CRC16_CHECK(&CMT2300_T_Array[1],CMT2300_T_Array[ 1] -2);//生成校验码
		//-------------------------------------------
		CMT2300_T_Array[22] = CRC_DATA.CRC_Array[0];
		CMT2300_T_Array[23] = CRC_DATA.CRC_Array[1];//写入校验
		//-------------------------------------------
		g_nTxEventCount = 0x01;//数据 待发送
		g_nNextRFState = RF_STATE_SLEEP;//启动发送
	}
	//---------------------------------------------------------------
	if((CMT2300_R_Array[0] == 0x55)&&(CMT2300_R_Array[1] == 0x09)&&(RX_SynchroCount != CMT2300_R_Array[7]))//接收到有效长度 和 同步值
	{	CRC16_CHECK(&CMT2300_R_Array[1],CMT2300_R_Array[1] -2);//计算校验
		CMT2300_R_Array[0] = 0x00;//等待下一次有效数据
		//---------------------------------------
		if((CMT2300_R_Array[9] ==CRC_DATA.CRC_Array[1])&&(CMT2300_R_Array[ 8] ==CRC_DATA.CRC_Array[0]))//校验合格
		{	RX_SynchroCount = CMT2300_R_Array[7];//数据同步计数
			//---------------------------------------
			if((CMT2300_R_Array[2] == 0xAA)&&(CMT2300_PairingTime >0))//遥控对码
			{	Config.Struct.RF_Address.Array[0] = CMT2300_R_Array[3];
				Config.Struct.RF_Address.Array[1] = CMT2300_R_Array[4];
				Config.Struct.RF_Address.Array[2] = CMT2300_R_Array[5];
				Config.Struct.RF_Address.Array[3] = CMT2300_R_Array[6];//写入地址码
				DataChangeUpdataTime = 5;//250mS 后存储
				goto DATA_PROCESS;
			}
			else if((Config.Struct.RF_Address.Array[0] == CMT2300_R_Array[3])&&(Config.Struct.RF_Address.Array[1] == CMT2300_R_Array[4])
		          &&(Config.Struct.RF_Address.Array[2] == CMT2300_R_Array[5])&&(Config.Struct.RF_Address.Array[3] == CMT2300_R_Array[6]))//地址码相符
			{//允许配对成功后，立即返回运行参数
			DATA_PROCESS:
				CMT2300_T_Array[0] = 0xAA;//准备发送数据
				CMT2300_T_Array[ 2] = 0x00;//返回数据
				///-------------------------------------------------------
				switch(CMT2300_R_Array[2])
				{case 0x23://确认键
					break;
				 case 0x24://模式转换
						KeyEvent = 0xF5;//控温方式
						KeyCount_Time = 0x00;
					break;
				 case 0x2B://开关机
						if(SystemRunState != 0x00)//ON
						{	SystemRunIndex = UART_PowerOff;		}//关闭
						else
						{	if(((RunVoltage.Data >100)&&(RunVoltage.Data <170))
							 ||((RunVoltage.Data >190)&&(RunVoltage.Data <320)))//电压限制
							{	SystemRunIndex = URAT_PowerOn;		}//启动
						}
					break;
				 case 0x3C://上调键
						KeyEvent = 0xFD;//上调
						KeyCount_Time = 0x00;
					break;
				 case 0x3E://下调键
						KeyEvent = 0xFB;//下调
						KeyCount_Time = 0x00;
					break;
				}
				
				BackLightTime = 250;//设置背光高亮时间

				
				if(CMT2300_PairingTime != 0x00)
				{	CMT2300_PairingTime = 0x00;
					CMT2300_T_Array[ 2] = 0xAA;
				#if		(LanguageType == Multilingual)	//多种语言
					WT588F_Play_Index(0x11);//遥控配对成功
				#elif	(LanguageType == Chinese)	//单独中文语音
					WT588F_Play_Index(16);//遥控配对成功
				#endif
				}//退出学习//让遥控器取消配对状态
			}
		}
	}
}

#endif
