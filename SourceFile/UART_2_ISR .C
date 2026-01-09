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

#include "KeyEvent.H"
#include "UART2_ISR.h"
#include "HP_03N_DRV.H"
#include "DisplayUpdata.H"

//==================================================================================
//说明：列举所需的 RAM 资源

//功能============长度=============说明====================
//起始字符		 1字节 0x76		(旋钮下发的数据起始字符为 0x78	)
//长度			 1字节 22(1字节   1 -- 250)
//运行状态		 1字节 0x05(关机)，0xA0(开机)
//实测环境温度	 1字节 -40 - 50度
//设定控制温度	 1字节 8 -- 35度
//油泵起始频率	 1字节 0.8Hz -- 2.5Hz
//油泵终止频率	 1字节 3Hz --   8Hz
//启动转速值	 2字节 1450RPM -- 2600RPM
//最大转速值	 2字节 3000RPM -- 5000RPM
//工作电压选择	 1字节 12V / 24V
//风扇磁极个数	 1字节 1 - 2
//控温方式选择	 1字节 0x32 =自动控温，0xCD = 手动控温
//控制温度下限	 1字节 跟随面板数据(默认为  8度)
//控制温度上限	 1字节 跟随面板数据(默认为 35度)
//油量告警值	 1字节 1 - 5%
//手动泵油开关	 1字节 0x00 = 关闭手动泵油， 0x5A开启手动泵油
//壳体温度保护值 2字节 200 -- 300摄氏度，最低过热保护值200度，最高300度
//海拔高度		 2字节 0 -- 6000米， 数据从手机里的传感器上获得


//功能============长度=============说明====================
//起始字符		 1字节 0x76	(统一返回这个值)
//长度			 1字节 22(1字节   1 -- 250)
//运行状态		 1字节 0 = 关机，1-7代表的工作状态
//告警状态		 1字节 0 =没有告警，1 -8 代表的告警
//供电电压		 2字节 0 -- 36.5V 电压
//电机转速		 2字节 0 -- 5000转，电机转速
//电机电压		 2字节 0 -- 18.5V 电机电压
//机器壳体温度	 2字节 -40 -- +300摄氏度
//点火塞电压	 2字节 点火塞电压18.2V
//点火塞电流	 2字节 0A --10.05A
//当前泵油频率	 1字节 0.8Hz -- 8Hz
//历史故障码	 1字节 1 - 8
//使用次数		 1字节 0 - 100
//设定泵油频率	 1字节 0.8Hz -- 8Hz
//剩余油量		 1字节 0-100 %
//保留			 1字节 
//==================================================================================
xdata unsigned char UART2_SendHoldTime;

xdata unsigned char ReceiveIndex;
xdata unsigned char ReceiveCount;

xdata unsigned char UART2_OverTime;//接收超时
xdata unsigned char TakeRunStateTime;

xdata unsigned char UART_ERROR_Time;
//---------------------------------------------------------
xdata unsigned char UART2_TxCount;//发送次数计数
//---------------------------------------------------------
xdata unsigned char UART_2_RXD[RXD_LENGTH];
xdata unsigned char UART_2_TXD[TXD_LENGTH];
xdata union{
	unsigned char StateData;
	struct {
		unsigned char BIT_1:1;//接收完成
		unsigned char BIT_2:1;//发送命令
		////////
		unsigned char BIT_3:1;// 发送完成
	}BIT;
}UART_2;
//==================================================================================
xdata union{//Array[1]=MSB,Array[0]=LSB, STM8的编译器中 Array[0]=MSB,Array[1]=LSB
    unsigned char CRC_Array[2];
    unsigned int CRC_REG;//CRC值
 }CRC_DATA;//----存储CRC 计算结果
//==================================================================================
//ANSI CRC-16的生成多项式为：x16 + x15 + x2 + 1 , 先传输 LSB 方式
void CRC16_CHECK(unsigned char *Message, unsigned int Length)
{unsigned int LoopCount;unsigned char ShiftCount, CarryFlag;
//--------------------------------------------------------------
    CRC_DATA.CRC_REG = 0xFFFF;//CRC 初值
//--------------------------------------------------------------
    for(LoopCount=0x00;LoopCount<Length;LoopCount++)
    {   CRC_DATA.CRC_Array[1] = (CRC_DATA.CRC_Array[1]^*Message);
        for(ShiftCount=0x00;ShiftCount<0x08;ShiftCount++)
        {   CarryFlag = (CRC_DATA.CRC_Array[1] & 0x01);
            CRC_DATA.CRC_REG >>= 0x01;//CRC值右移一位
            if(CarryFlag == 0x01)//进位为1 与多项式值 异或
            {   CRC_DATA.CRC_REG = (CRC_DATA.CRC_REG ^ 0xA001);
            }
        }
        Message++;//下一地址
    }//不同编译器下，需注意union 结构高字节，低字节存储方式
}
//-----------------------------------------------------------------------------
void Uart2_Initial(unsigned long BaudRate)
{union{
	unsigned char Array[2];
	unsigned int Data;
 }Baud;
 unsigned char Count;
	//-------------------------------------------------
	for(Count=0x00;Count<RXD_LENGTH;Count++)
	{	UART_2_RXD[Count] = 0x00;	}//初始化接收缓冲区
	for(Count=0x00;Count<TXD_LENGTH;Count++)
	{	UART_2_TXD[Count] = 0x00;	}//初始化发送缓冲区
	//-------------------------------------------------
	GPIO_Init(P60F,P60_UART2_RX_SETTING |PU_EN);
	GPIO_Init(P61F,P61_UART2_TX_SETTING);
// 	P60C &= 0x9F;//关闭施密特触发功能，提高低电平输入门限电压至 0.5 *VDD //上拉电阻选择为弱上拉
// 	P60C &= 0xDF;//上拉电阻选择为弱上拉
// 	P60C &= 0xEF;//下拉电阻选择为弱下拉

	//-------------------------------------------------
	Baud.Data = 0x400 - FOSC/(BaudRate*32);
	S2RELH   = 	Baud.Array[0];
	S2RELL   = 	Baud.Array[1];
	S2CON = 0xD0;
	INT3EN = 1;
	//-------------------------------------------------
	ReceiveIndex = 0x00;
	UART2_OverTime = 0x00;//接收超时
	UART_2.StateData = 0x00;//初始化为未收到数据
	UART2_SendHoldTime = 10;//保持25mS
	//-------------------------------------------------
	TakeRunStateTime = 0x00;
}

//-----------------------------------------------------------------------------
void UART2_ISR (void) interrupt 8
{unsigned char ReceiveData;
	if((S2CON & 0x01) !=0x00)//接收中断
	{	S2CON = (S2CON &0xFE) |0x01;//清除中断标志
		ReceiveData = S2BUF;//读取串口数据
	//-------------------------------------------------------------
		UART2_OverTime = 12;//30mS超时时间
		if(UART_2.BIT.BIT_1 == EQUAL_H){ return; }//未处理完，丢弃后来数据，防止覆盖
		UART_2_RXD[ReceiveIndex] = ReceiveData;//存储数据
	//-------------------------------------------------------------
		switch(ReceiveIndex)
		{case 0x00://起始字符
				if((ReceiveData == 0x6A)//主板ID
				 ||(ReceiveData == 0xFF))//管理地址，全部接收
				{	ReceiveIndex++;		}//正确则继续
			break;
		 case 0x01://长度
				ReceiveCount = ReceiveData;//接收数据长度
				ReceiveIndex++;//开始正式接收，数据体
			break;
		 default:///数据体
				ReceiveIndex++;//开始正式接收，数据体
			//-------------------------------------------------
				if(--ReceiveCount < 0x01)
				{	ReceiveIndex = 0x00;//接收成功等待下一次
					UART_2.BIT.BIT_1 = EQUAL_H;//一次接收成功
				}//长度不为零，则一直接收
			break;
		}
		//-----------------------------------------------------
		if(ReceiveIndex >= RXD_LENGTH){	ReceiveIndex = 0x00;	}//防止缓冲区溢出
	}
	if((S2CON & 0x02) !=0x00)//发送中断
	{	
		S2CON = (S2CON &0xFD) |0x02;//清除中断标志
		UART_2.BIT.BIT_3 = EQUAL_L;
	}
}
//-----------------------------------------------------------------------------
void UART2_Transmit(unsigned char TXD_DATA)
{unsigned int TimeOverCount = 2000;
	do{	if(UART_2.BIT.BIT_3 ==EQUAL_L)
		{	TimeOverCount = 0;		}
	}while(TimeOverCount--);//消除死循环
	UART_2.BIT.BIT_3 = EQUAL_H;//发送完成，产生中断清除
	S2BUF = TXD_DATA;//装入数据
}

//==================================================================================
//  0 //0x6A		起始字符
//  1 // 24			长度
//  2 //0x00		机器运行状态	0待机，0xA3开机，0x5C关机，0xAC手动泵油
//  3 //0x01		产品类型 ID (1风暖 2水暖)
//  4 //0x01		燃油类型	1柴油，2汽油，3甲醛，其他类型--后面补充
//  5 // + 8		温度下限
//  6 // +35		温度上限
//  7 //1.3Hz		最小泵油量
//  8 //5.5Hz		最大泵油量
//  9
// 10 //1350 RPM	最小风速
// 11
// 12 //4500 RPM	最大风速
// 13 //12--24		工作电压
// 14 //1 - 6		点火塞功率
// 15 //3000 米
// 16 //=========== 海拔高度
// 17 // 20			空气含氧量
// 18 //1-2			控温类型	1自动控温，2手动控温（默认手动）
// 19 //8-35		面板设定温度
// 20 //-40  +50	面板环境温度
// 21 //0x00		开关量1
// 22 //0x00		开关量2
// 23 // xx 		发送次数计数值
// 24 // xx
// 25 // xx			校验码
void ControlDataInitial(void)
{union{
	unsigned int Data;
	unsigned char Array[2];
}Temp;
	UART2_TxCount++;//随机值 递增
//-----------------------------------------------------
	UART_2_TXD[ 0] = UART_Synchrodata;
	UART_2_TXD[ 1] = 24;//长度
//-----------------------------------------------------
	UART_2_TXD[ 2] = Config.Struct.ProductType;//产品类型 ID (1风暖 2水暖)
//-----------------------------------------------------
	UART_2_TXD[ 3] = SystemRunIndex;////运行状态
//-----------------------------------------------------
	UART_2_TXD[ 4] = Config.Struct.SetupFuelType;//燃油类型	1柴油，2汽油，3甲醛
	UART_2_TXD[ 5] = Config.Struct.SetupTemper_L_Limit;//温度下限
	UART_2_TXD[ 6] = Config.Struct.SetupTemper_H_Limit;//温度上限
//-----------------------------------------------------
	UART_2_TXD[ 7] = Config.Struct.MIN_OilPumpCycle;//最小泵油量
	UART_2_TXD[ 8] = Config.Struct.MAX_OilPumpCycle;//最大泵油量
	UART_2_TXD[ 9] = Config.Struct.MIN_FanSpeed.Array[0];
	UART_2_TXD[10] = Config.Struct.MIN_FanSpeed.Array[1];//最小风速
	UART_2_TXD[11] = Config.Struct.MAX_FanSpeed.Array[0];
	UART_2_TXD[12] = Config.Struct.MAX_FanSpeed.Array[1];//最大风速
//-----------------------------------------------------
	UART_2_TXD[13] = Config.Struct.SystemPowerVolatge;//工作电压
	UART_2_TXD[14] = Config.Struct.Ignition_Power;//点火塞功率
	//-------------------------------------------------
	UART_2_TXD[15] = Config.Struct.TemperatureType;//控温类型选择	(1自动控温，2手动控温  默认手动)
	UART_2_TXD[16] = Config.Struct.EnergyLevel;//输出功率级别
	UART_2_TXD[17] = WorkingTime;//工作时间
	//-------------------------------------------------
	Temp.Data = AvgAltitude.Data /100;//去除小数
	UART_2_TXD[18] = Temp.Array[0];
	UART_2_TXD[19] = Temp.Array[1];//实际海拔高度
	UART_2_TXD[20] = Config.Struct.RunStateType;//加热状态类型 == 1 单次加热，== 2 循环加热
//-----------------------------------------------------
	UART_2_TXD[21] = 0x00;
	UART_2_TXD[22] = 0x00;
	UART_2_TXD[23] = UART2_TxCount;//随机码
}
//==================================================================================
void UART2_DataTransmit (void)
{unsigned char *TxIndex; unsigned char Tx_Count,Lenght;
//---------------------------------------------------------
	if(TakeRunStateTime >120)//获取主机运行状态间隔300mS
	{	TakeRunStateTime = 0x00;//准备下一次
		UART_2.BIT.BIT_2 = EQUAL_H;//启动发送
	///////////////////////////////////////////////////////
		ControlDataInitial();//更新配置数据的状态
	}
    //-----------------------------------------------------
	if(UART_2.BIT.BIT_2 == EQUAL_H)//发送数据
	{	UART2_Direction_SET;///////启动发送

		RunData_Copy(&UART_2_TXD[2],&UART_2_TXD[2],UART_2_TXD[ 1],UART2_TxCount,(UART_2_TXD[1] -3));//加密发送的数据
	    //-----------------------------------------------------
		Lenght = UART_2_TXD[1];//获取长度
		CRC16_CHECK(&UART_2_TXD[0],Lenght);//生成校验码
		UART_2_TXD[Lenght++] = CRC_DATA.CRC_Array[0];
		UART_2_TXD[Lenght  ] = CRC_DATA.CRC_Array[1];//存储校验
		//-------------------------------------------------
		TxIndex = &UART_2_TXD[0];
		UART2_Transmit(*TxIndex++);//起始码
		Tx_Count = *TxIndex++;
		UART2_Transmit(Tx_Count);//发送长度
		//-------------------------------------------------
		while(Tx_Count-- > 0x00)//发送数据体
		{	UART2_Transmit(*TxIndex++);		}
		//-------------------------------------------------
		UART_2.BIT.BIT_2 = EQUAL_L;//执行后清除
		UART2_SendHoldTime = 3;//启动发送 到开始发送间隔 30mS
	}
}

//-----------------------------------------------------------------------------
//  0 //起始字符
//  1 //长度
//  2 //产品类型 ID
//  3 //运行状态
//  4 //历史故障码
//  5 //
//  6 //供电电压
//  7 //
//  8 //电机转速
//  9 //
// 10 //电机电压
// 11 //
// 12 //点火塞电压
// 13 //
// 14 //点火塞电流
// 15 //设定泵油频率
// 16 //当前泵油频率
// 17 //剩余油量
// 18 //
// 19 //机器壳体温度
// 20 //
// 21 //排气口温度
// 22 //
// 23 //水泵工作电压
// 24 //进水口温度
// 25 //出水口温度
// 26 //进风口温度
// 27 //
// 28 //开关量状态
// 29 //
// 30 //累计开机运行时间
// 31 //发送次数计数值
// 32 //
// 33 //校验码
//==================================================================================
unsigned char CommandSendCount;//命令发送计数
//==================================================================================
void UART2_DataProcess(void)
{unsigned char Lenght;
//-------------------------------------------------------------------
 	if(UART_2.BIT.BIT_1 == EQUAL_H)
	{	Lenght = UART_2_RXD[1];//获取长度
		CRC16_CHECK(&UART_2_RXD[0],Lenght);//生成校验码
		//-----------------------------------------------------------
		if((UART_2_RXD[  Lenght] == CRC_DATA.CRC_Array[0])
		 &&(UART_2_RXD[++Lenght] == CRC_DATA.CRC_Array[1]))
		{	switch(UART_2_RXD[0])
			{case 0x6A://收到正常数据
					RunData_Copy(&UART_2_RXD[2],&UART_2_RXD[2],UART_2_RXD[ 0],UART_2_RXD[UART_2_RXD[1] -1],(UART_2_RXD[1] -3));//还原收到的数据
				//---------------------------------------------------
					if(Config.Struct.ProductType != UART_2_RXD[2]){	break;	}//产品类型 ID 发送和收到的 ID 不符，直接退出
				//---------------------------------------------------
					if(SystemRunIndex != 0x00)//自动关闭
					{	if(++CommandSendCount >3)
						{	SystemRunIndex = UART_Null;	}//按键执行，清除状态
					}
					else
					{	CommandSendCount = 0x00;	}
				//-------------------------------------------------
					SystemRunState = UART_2_RXD[3];//运行状态
					ErrorIndex = UART_2_RXD[4];//历史故障码
				//---------------------------------------------------
					RunVoltage.Array[0] = UART_2_RXD[5];//工作电压
					RunVoltage.Array[1] = UART_2_RXD[6];
				//---------------------------------------------------
					RotateSpeed.Array[0] = UART_2_RXD[7];//转速数据
					RotateSpeed.Array[1] = UART_2_RXD[8];
				//---------------------------------------------------
					Fan_Vot.Array[0] = UART_2_RXD[9];
					Fan_Vot.Array[1] = UART_2_RXD[10];//电机电压
				//---------------------------------------------------
					Ignition_Vot.Data[0] = UART_2_RXD[11];
					Ignition_Vot.Data[1] = UART_2_RXD[12];//点火塞电压
					Ignition_CUR.Data[0] = UART_2_RXD[13];
					Ignition_CUR.Data[1] = UART_2_RXD[14];//点火塞电流
				//---------------------------------------------------
					OilpumpSetFrequ = UART_2_RXD[15];//油泵设定频率
					OilPumpFrequency = UART_2_RXD[16];//油泵实际工作频率
				//---------------------------------------------------
				//	UART_2_RXD[29];//剩余油量数据
				//---------------------------------------------------
					SW.TempArray[0] = UART_2_RXD[18];
					SW.TempArray[1] = UART_2_RXD[19];//壳体温度值
				//---------------------------------------------------
					HY.TempArray[0] = UART_2_RXD[20];
					HY.TempArray[1] = UART_2_RXD[21];//排气口温度
				//---------------------------------------------------
					WaterPump_CUR.Array[0] = UART_2_RXD[22];
					WaterPump_CUR.Array[1] = UART_2_RXD[23];//水泵工作电流
				//---------------------------------------------------
				//	UART_2_RXD[24];//进水口温度
				//	UART_2_RXD[25];//出水口温度
				//---------------------------------------------------
					RemainingWorkingTime = UART_2_RXD[26];//剩余工作时间
				//---------------------------------------------------
				//	UART_2_RXD[27];
				//	UART_2_RXD[28];//开关量状态
				//---------------------------------------------------
				//	UART_2_RXD[29];
				//	UART_2_RXD[30];
				//	UART_2_RXD[31];
				//	UART_2_RXD[32];//累计开机运行时间
				//---------------------------------------------------
				//---------------------------------------------------
				break;
			 case 0xFF://广播模式，必须响应
				break;
			}
			UART_ERROR_Time = 100;//通信错误故障
		}
		//-------------------------------------------------
		UART_2.BIT.BIT_1 = EQUAL_L;
	}
}

