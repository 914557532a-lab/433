//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#ifndef _FLASH_C_
#define _FLASH_C_
//-----------------------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

#include "flash.h"

#ifdef Enable_CMT2300	//打开双向遥控
	#include "Radio.H"
#endif

#include "UART2_ISR.h"
#include "DisplayUpdata.H"
//-------------------------------------------------------------------
xdata tpdef_struct_SN Config;//所有数据交换 

//-----------------------------------------------------------------------------
void Data_Area_Sector_Set(void)
{	PADRD = DATA_FLASH_START;//设置数据区位置
}
//-----------------------------------------------------------------------------
//函 数 名：Data_Area_Erase_Sector
//功能描述：擦除数据区空间的一个扇区
//输    入：unsigned char SectorNumber	扇区号，范围 0-255
//-----------------------------------------------------------------------------
void Data_Area_Sector_Erase(unsigned char SectorNumber)
{union{
	unsigned char Array[2];
	unsigned int Address;
}Sector;
	EA = 0;//关总中断
	Sector.Address = (unsigned int)SectorNumber *0x80;
	FSCMD 	= 	0;
	LOCK  = CMD_DATA_AREA_UNLOCK;//数据区解锁
	PTSH = Sector.Array[0];//填写扇区地址
	PTSL = Sector.Array[1];//填写扇区地址	
	FSCMD = CMD_DATA_AREA_ERASE_SECTOR;//执行擦除扇区操作
	LOCK  = CMD_FLASH_LOCK;//对FLASH加锁
	EA = 1;//开总中断
} 
//-----------------------------------------------------------------------------
//函 数 名：Data_Area_Read_Byte
//功能描述：从FLASH数据区读出一字节数据
//输    入：unsigned int Address	数据区空间读地址
//返    回：读取的一字节数据
//-----------------------------------------------------------------------------
unsigned char Data_Area_Read_Byte(unsigned int Address)
{unsigned char Data_Temp;
	EA = 0;//关总中断
	FSCMD 	= 	0;
	LOCK  = CMD_DATA_AREA_UNLOCK;//数据区解锁
	PTSH = (unsigned char)(Address>>8);//填写高位地址
	PTSL = (unsigned char)Address;//填写低位地址
	FSCMD = CMD_DATA_AREA_READ;//执行读操作
	Data_Temp = FSDAT;
	LOCK  = CMD_FLASH_LOCK;//对FLASH加锁
	EA = 1;//开总中断
	return Data_Temp;
}
//-----------------------------------------------------------------------------
//函 数 名：Data_Area_Mass_Read
//功能描述：从FLASH数据区批量读出数据
//输    入：unsigned int Address	数据区空间读起始地址
//		  unsigned char *pData	数据指针，指向读出数据缓存数组
//		  unsigned char Length	读数据长度
//-----------------------------------------------------------------------------
void Data_Area_Mass_Read(unsigned int Address,unsigned char *pData,unsigned char Length)
{unsigned int Count;
	EA = 0;//关总中断
	FSCMD 	= 	0;
	LOCK  = CMD_DATA_AREA_UNLOCK;//数据区解锁
	PTSH = (unsigned char)(Address>>8);//填写高位地址
	PTSL = (unsigned char)Address;//填写低位地址
	FSCMD = CMD_DATA_AREA_READ;//执行读操作
	for(Count = 0;Count <Length; Count++)
	{	*pData++ = FSDAT;		}
	LOCK  = CMD_FLASH_LOCK;//对FLASH加锁
	EA = 1;//开总中断
}
//-----------------------------------------------------------------------------
//函 数 名：Data_Area_Write_Byte
//功能描述：向FLASH数据区写入一个字节数据
//输    入：unsigned int Address	数据区空间写入地址
//			unsigned char Data		写入数据
//-----------------------------------------------------------------------------
void Data_Area_Write_Byte(unsigned int Address,unsigned char Data)
{	EA = 0;//关总中断
	FSCMD 	= 	0;
	LOCK  = CMD_DATA_AREA_UNLOCK;//数据区解锁
	PTSH = (unsigned char)(Address>>8);//填写高位地址
	PTSL = (unsigned char)Address;//填写低位地址
	FSCMD = CMD_DATA_AREA_WIRTE;//执行写操作
	FSDAT = Data;//装载数据
	LOCK  = CMD_FLASH_LOCK;//对FLASH加锁
	EA = 1;//开总中断
}/////////////////////////////////////////////////////////////////////////////////////////
#pragma optimize(0)
void Data_Area_ReadWrite(unsigned char Type,unsigned char Sector,unsigned char *DataIndex,unsigned char DataLenght)
{signed int DATA_ADDRESS = (SECTOR_SIZE *Sector) +ROM_ADDR_OFFSET;//设置EEPROM 存储区,起始地址
 signed int ROM_ADDR_INC = DATA_ADDRESS;//备份起始地址
 signed int ROM_ORG_ADDRES = DATA_ADDRESS;//备份起始地址
 signed int ROM_END_ADDRES = (ROM_ORG_ADDRES +SECTOR_SIZE);//计算结束地址
 unsigned char Count; unsigned char StateIndex ; unsigned char *OriginDataIndex = DataIndex;
 //-------------------------------------------------------------
	Data_Area_Sector_Set();//设置数据区位置
 //-------------------------------------------------------------
	for(StateIndex=0x00;StateIndex<0x07;)
	{	switch(StateIndex)
		{case 0x06://读取ROM中的数据
				DataIndex = OriginDataIndex;//恢复数据区地址
				Data_Area_Mass_Read(DATA_ADDRESS,DataIndex,DataLenght);
				//----------------------------------------------
				StateIndex = 0x07;	//读完成 退出
			break;
		 case 0x05://写数据时地址溢出,执行擦除操作
		 ADDRES_OVER_PROCESS://地址溢出
				Data_Area_Sector_Erase(0);//擦除一个扇区，128字节
				Data_Area_Sector_Erase(1);//擦除一个扇区，128字节
				DATA_ADDRESS = ROM_ORG_ADDRES;//指向EEPROM 起始地址
				DataIndex = OriginDataIndex;//恢复数据区地址
		 case 0x04://写数据到ROM存储区
				for(Count=0x00;Count<DataLenght;Count++)//闪存数据拷贝
				{	Data_Area_Write_Byte(DATA_ADDRESS++,*DataIndex++);
					if(DATA_ADDRESS >= ROM_END_ADDRES)//地址溢出
					{	goto ADDRES_OVER_PROCESS;	}////擦除从写整个区
				}
				//----------------------------------------------
				DATA_ADDRESS -= DataLenght;//写后重新读出，后退一个字段
				StateIndex = 0x06;//初始化写入后，要再次读出初始值
			break;
		 case 0x03://数据定位有效
				if(Type == READ_DATA)//若是读取数据,则需后退整个数据区长度
				{	if(DATA_ADDRESS >= ROM_END_ADDRES)//地址溢出
					{	if((DATA_ADDRESS -DataLenght) >(ROM_END_ADDRES -DataLenght))
						{	DATA_ADDRESS -= DataLenght;		}//读取过程会溢出，后退2个连续区域
					}
					DATA_ADDRESS -= DataLenght;//不会溢出后退 1个连续区域
					StateIndex = 0x06;//转入 读取 ROM 中的用户数据
				}
				else///////////////////若是写入数据，则需要擦除后再写
				{	if(DATA_ADDRESS >= ROM_END_ADDRES)//地址溢出
					{	StateIndex = 0x05;		}//执行擦除操作
					else
					{	StateIndex = 0x04;		}//转入 执行用户参数写入
				}
				//--------------------------------------------------------
				DATA_ADDRESS -= 2;//后退查找0xFF，引起地址偏移
				if(DATA_ADDRESS < ROM_ORG_ADDRES){	DATA_ADDRESS = ROM_ORG_ADDRES;	}//防止溢出
			break;
		 case 0x02://第二个 0xFF
		 case 0x01://第一个 0xFF
				if(DATA_ADDRESS >= ROM_END_ADDRES)/////没有剩余空间了
				{	StateIndex = 0x03;//读操作，则结束定位查找数据过程
					DATA_ADDRESS += 1;//地址偏移补偿
					if(Type == WRITE_DATA)//写操作，擦除选定的Flash块
					{	StateIndex = 0x05;	}
				}
				else
				{	if(Data_Area_Read_Byte(DATA_ADDRESS++) == 0xFF)
					{	StateIndex++;	}//2次检测到 0xFF，当前区域为空，可以正常操作
					else
					{	StateIndex = 0x00;//否则检索不成功，重来
						ROM_ADDR_INC += DataLenght;//指向下一的数据区
						DATA_ADDRESS = ROM_ADDR_INC;//给出访问的数据地址
					}
				}
			break;
		 default:
				if(DATA_ADDRESS >= ROM_END_ADDRES)/////没有剩余空间了
				{	StateIndex = 0x03;//读操作，则结束定位查找数据过程
					DATA_ADDRESS += 2;//地址偏移补偿
					if(Type == WRITE_DATA)//写操作，擦除选定的Flash块
					{	StateIndex = 0x05;	}
				}
				else
				{	if(Data_Area_Read_Byte(DATA_ADDRESS++) == 0xFF)//则当前地址没数据
					{	StateIndex = 0x02;		}//寻找第二个 0xFF
					else//当前闪存地址 !=0xFF,有存储过数据，指向下一区域
					{	ROM_ADDR_INC += DataLenght;//指向下一的数据区
						DATA_ADDRESS = ROM_ADDR_INC;//给出访问的数据地址
					}
				}
			break;
		}
	}
}
//-----------------------------------------------------------------------------
//函 数 名：	GetChipID
//功能描述：	从芯片读出芯片识别码(每个芯片都有唯一的识别码）
//-----------------------------------------------------------------------------
unsigned long GetChipID(void)
{union{
	unsigned char Array[4];
	unsigned long ID;
}Chip;
//------------------------------------------
	EA = 0;//关总中断
	LOCK  = 0x2B;
	FSCMD = 0x80;
	PTSH = 0x01;
	PTSL = 0x00;
	FSCMD = 0x81;
	Chip.Array[0] = FSDAT;
	Chip.Array[1] = FSDAT;
	Chip.Array[2] = FSDAT;
	Chip.Array[3] = FSDAT;
	FSCMD = 0;
	LOCK = 0xAA;
	EA = 1;//开总中断
	return Chip.ID;
}

#endif 
//-----------------------------------------------------------------------------
#define Config_5KW	//按照5KW 参数运行
//-----------------------------------------------------------------------------
unsigned char code RunDataConfig[CONFIGDATA_LENGHT] =
{//	0x00,0x00,0x00,0x00,//检查初始化的标志
 //--------------------------------------------------
	0x16,0x88,	//管理密码 --5
	//-----------------------------------------------
	0x00,0x00,0x00,0x00,//备用(遥控码地址) --9
	//-----------------------------------------------
	0x00,0x76,	//制造商 ID 默认 0 == 兼容原公司产品
	0x02,		//产品类型 ID == 风暖1 水暖2 其他待定
	0x01,		//产品语音，语言播报类型
	//-----------------------------------------------
	0x01,		//燃油类型 1柴油，2汽油，3甲醛
	0x2D,		//加热重启温度 45 ℃
	0x4B,		//加热停止温度 75 ℃  --13
	//-----------------------------------------------
	0x12,		//最小泵油量 1.0Hz -- 6.5Hz (默认 1.8Hz)
	0x38,		//最大泵油量 4.0Hz -- 9.0Hz (默认 5.6Hz) --15
	0x0B,0xB8,	//最小风速	3000 r/min(起始转速) --17
	0x21,0xCA,	//最大风速	8650 r/min(结束转速) --19
	0x78,		//工作电压 (默认24V)  , 0x78 == 120, 0xF0 == 240
	0x05,		//点火塞功率	点火塞默认功率 85W  误差 2% --21
	//-----------------------------------------------
	0x01,		//1 =档位控温方式，2 = 时间控温方式 ，3 保留
	0x06,		//控温分9档，每次加 1 档
	//-----------------------------------------------
	0x02		//加热状态类型 == 1 单次加热，== 2 循环加热
};
//==================================================================================
void RunData_Copy(unsigned char *Purpose,unsigned char *Source,unsigned char Seed,unsigned char Proof,unsigned char Lenght)
{unsigned char Count;unsigned char SeedData = Seed ^Proof;//生产种子码
	//----------------------------------------------------------
	if((SeedData ==0x00)||(SeedData == 0xFF))
	{	SeedData = (SeedData |0x88) >>0x01;		}//规避 0 和 FF 值
	//----------------------------------------------------------
	for(Count = 0x00;Count < Lenght;Count++)//还原加密数据
	{	*Purpose++ = *Source++ ^SeedData;		}
}
//-----------------------------------------------------------------------------
void RunDataUpdate(void)
{	CRC16_CHECK(&Config.Data[CHIP_ID_LENGHT],CONFIGDATA_LENGHT);//生成校验码
	if(Config.Struct.CRC_DATA.DATA != CRC_DATA.CRC_REG)
	{	Config.Struct.CRC_DATA.DATA = CRC_DATA.CRC_REG;//存储校验
		Data_Area_ReadWrite(WRITE_DATA,0,&Config.Data[0],ROM_DATA_1_LENGHT);//设定初值
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void RunData_Initial(void)
{union{
	unsigned char Array[4];
	unsigned long Data;
}Chip_ID;
unsigned char Loop;
//-------------------------------------------------------------------
	Chip_ID.Data = GetChipID();//获取校验ID数据
	if((Chip_ID.Data == 0x00000000)||(Chip_ID.Data == 0xFFFFFFFF))
	{	Chip_ID.Data = 0x20220919;		}//没有ID 写入时标信息
//-------------------------------------------------------------------
ReCheckInitData://初始化后重新检查
	Data_Area_ReadWrite(READ_DATA,0,&Config.Data[0],ROM_DATA_1_LENGHT);
//-------------------------------------------------------------------
	if((Chip_ID.Array[0] != Config.Struct.Chip_ID_0)||(Chip_ID.Array[1] != Config.Struct.Chip_ID_1)\
	 ||(Chip_ID.Array[2] != Config.Struct.Chip_ID_2))//校验数据是否被初始化过
	{	Config.Struct.Chip_ID_0 = Chip_ID.Array[0]; Config.Struct.Chip_ID_1 = Chip_ID.Array[1];
		Config.Struct.Chip_ID_2 = Chip_ID.Array[2];//设置ID号
		//-----------------------------------------------------------
		for(Loop=0;Loop<CONFIGDATA_LENGHT;Loop++)		//调试需关闭
		{	Config.Data[CHIP_ID_LENGHT +Loop] = RunDataConfig[Loop];		}//重置运行参数
		//-----------------------------------------------------------
		RunDataUpdate();//计算校验码，并存储结果
		//-----------------------------------------------------------
		goto ReCheckInitData;
	}
	//----------------------------------------------------------
	SystemRunIndex = 0x00;//运行状态
	SystemStateHold = SystemRunState;
	SystemStateTime = 0x00;
	UART_ERROR_Time = 30;//通信错误故障
	//----------------------------------------------------------
	//Config.Struct.Password.DATA;//读取初始密码
	//----------------------------------------------------------
	ErrorIndex = 0x00;

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
	RemainingWorkingTime = 0x00;//剩余运行时间

//----------------------------------------------------------
#ifdef Enable_CMT2300	//打开双向遥控
	CMT2300_PairingTime = 0x00;//遥控地址配对时间
	CMT2300_T_Array[0] = 0xAA;//标识
	CMT2300_T_Array[2] = 0x00;//功能码
#endif
}

