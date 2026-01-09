//-----------------------------------------------------------------------------
//功能说明：读取海拔高度传感器HP203N，高度，温度数据
//特点说明：供电电压：1.8V~3.6V		压力量程：300mbar~1200mbar	直接读数，补偿
//			高分辨率：10cm		待机电功耗： < 0.1uA	工作温度： -40~+85℃
//功能码：0x0D	 INT_SRC	(内部状态寄存器初值 0x00) 
//      7        6         5        4         3         2         1         0
//   TH_ERR   DEV_RDY   PA_RDY    T_RDY    PA_TRAV   T_TRAV    PA_WIN     T_WIN
//-----------------------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

#include "HP_03N_DRV.H"
//-----------------------------------------------------------------------------
#define NOP _nop_	//重定义空操作，适应不同的编译器
//-----------------------------------------------------------------------------
#define IIC_SCL_CLR		P3 &= 0xEF
#define IIC_SCL_SET		P3 |= 0x10

#define IIC_SDA_CLR		P3 &= 0xDF
#define IIC_SDA_SET		P3 |= 0x20

#define IIC_SDA_I	GPIO_Init(P35F,INPUT |OP_EN)//改数据方向为输入
#define IIC_SDA_O	GPIO_Init(P35F,OUTPUT |OP_EN)//改数据方向为输出

#define Read_IIC_SDA_pin	(P35)
//-----------------------------------------------------------------------------
xdata unsigned char HP20X_ReadTime;

xdata long	Temperature;

#define PressureBufLength	8
xdata unsigned char PressureIndex;
xdata long	PressureBuf[PressureBufLength];
xdata long	Pressure;
xdata long	AvgPressure;

#define AltitudeBufLength	8
xdata unsigned char AltitudeIndex;
xdata long	AltitudeBuf[AltitudeBufLength];
xdata long	Altitude;
xdata union{
	unsigned char Array[4];
	signed long	Data;
}AvgAltitude;
//-----------------------------------------------------------------------------
void Delay_10us(unsigned int Time)
{	do{	 NOP();	 NOP();
	}while(Time--);//11.0592MHz主频==3.5uS
}
//-----------------------------------------------------------------------------
void IIC_Start(void)
{	IIC_SDA_SET;
	Delay_10us(1);
	IIC_SCL_SET;
  	Delay_10us(1);
	IIC_SDA_CLR;
	Delay_10us(1);
	IIC_SCL_CLR;
  	Delay_10us(1);
}
//-----------------------------------------------------------------------------
void IIC_Stop(void)
{	IIC_SCL_CLR;
	Delay_10us(1);
	IIC_SDA_CLR;
	Delay_10us(1);
	IIC_SCL_SET;
	Delay_10us(1);
	IIC_SDA_SET;
	Delay_10us(20);
}
//-----------------------------------------------------------------------------
void IIC_W_ACK(void)
{	IIC_SDA_I;//有管脚方向的单片机
	IIC_SDA_SET;
	IIC_SCL_SET;//第9 个时钟,应答时钟
  	Delay_10us(3);
	IIC_SCL_CLR;
	IIC_SDA_O;//有管脚方向的单片机
}
void IIC_R_ACK(void)
{	IIC_SDA_O;//有管脚方向的单片机
	IIC_SDA_CLR;
  	Delay_10us(1);
	IIC_SCL_SET;
  	Delay_10us(3);
	IIC_SCL_CLR;
  	Delay_10us(1);
	IIC_SDA_SET;
}
//-----------------------------------------------------------------------------
void IIC_NoAck(void)
{	IIC_SCL_CLR;
	IIC_SDA_O;//有管脚方向的单片机
	Delay_10us(1);
	IIC_SDA_SET;
	Delay_10us(3);
	IIC_SCL_SET;
	Delay_10us(1);
	IIC_SCL_CLR;
}
//-----------------------------------------------------------------------------
void IIC_WriteByte (unsigned char DATA)
{unsigned char Count;
//------------------------------------------
	IIC_SDA_O;//有管脚方向的单片机
	//-------------------------------------------
	for(Count=0x00;Count<0x08;Count++)
	{	if((DATA & 0x80) == 0x80)
		{	IIC_SDA_SET;	}
		else
		{	IIC_SDA_CLR;	}
		DATA <<= 0x01;
		Delay_10us(1);
		IIC_SCL_SET;
		Delay_10us(1);
		IIC_SCL_CLR;
	}
}

//-----------------------------------------------------------------------------
unsigned char IIC_ReadByte(void)
{unsigned char Count,DATA=0x00;
//------------------------------------------
	IIC_SDA_I;//有管脚方向的单片机
	//-------------------------------------------
	for(Count=0x00;Count<0x08;Count++)
	{	IIC_SCL_SET;
		Delay_10us(1);
		DATA <<= 0x01;
		if(Read_IIC_SDA_pin != 0)
		{	DATA = (DATA | 0x01);	}
		else
		{	DATA = (DATA & 0xFE);	}
		IIC_SCL_CLR;
		Delay_10us(1);
	}
	//-------------------------------------------
	IIC_SDA_O;//有管脚方向的单片机
	return (DATA);
}
//===================================================================
void HP20X_IIC_WriteCmd(unsigned char Cmd)
{	IIC_Start();
	IIC_WriteByte(HP20X_I2C_WR_ID);
	IIC_W_ACK();//等待应答信号
	IIC_WriteByte(Cmd);
	IIC_W_ACK();//等待应答信号
	IIC_Stop();
}
//===================================================================
unsigned long HP20X_IIC_Read_3_Byte(void)
{union{
	unsigned char Array[4];
	unsigned long Data;
}Long;
	Long.Data = 0x00000000;//初值
	Long.Array[1] = IIC_ReadByte();
	IIC_R_ACK();//写入应答信号
	Long.Array[2] = IIC_ReadByte();
	IIC_R_ACK();//写入应答信号
	Long.Array[3] = IIC_ReadByte();
	if(Long.Data & 0x00800000)//负值，补码
	{	Long.Data |= 0xFF000000;	}
	return Long.Data;
}

//===================================================================
unsigned char HP20X_IIC_ReadIntReg(unsigned char Reg_ID)//读取芯片状态
{unsigned char ChipState;
	HP20X_IIC_WriteCmd(HP20X_R_INT_REG |Reg_ID);
	IIC_Start();
	IIC_WriteByte(HP20X_I2C_RD_ID);
	IIC_W_ACK();//等待应答信号
	ChipState = IIC_ReadByte();
	IIC_NoAck();
	IIC_Stop();
	return ChipState;
}
//===================================================================                 
void HP20X_Initial(void)
{	HP20X_IIC_WriteCmd(HP20X_SOFT_RST);
	//-------------------------------------------
	AltitudeIndex = 0x00;
	AvgAltitude.Data = 0x00;
	Temperature = 0x00;
}
//=================================================================== 
void HP20X_IIC_ReadPressure(void)
{	if(HP20X_IIC_ReadIntReg(INT_SRC_REG) == 0x40)
	{	HP20X_IIC_WriteCmd(HP20X_READ_P);
		IIC_Start();
		IIC_WriteByte(HP20X_I2C_RD_ID);
		IIC_W_ACK();//等待应答信号
		Pressure = HP20X_IIC_Read_3_Byte();
		IIC_NoAck();
		IIC_Stop();
		//---------------计算大气压力平均值
		if(PressureIndex <PressureBufLength)
		{	PressureBuf[PressureIndex++] = Pressure;	}
		else
		{	AvgPressure = 0x00000000;
			for(PressureIndex =0;PressureIndex <PressureBufLength;)
			{	AvgPressure += PressureBuf[PressureIndex++];	}//准备计算均值
			AvgPressure /= PressureBufLength;
			//-----------------------------------
			PressureIndex =0;
		}
	}
}
//===================================================================                          
void HP20X_ReadAltitudeTemperature(void)
{	if(HP20X_IIC_ReadIntReg(INT_SRC_REG) == 0x40)
	{	HP20X_IIC_WriteCmd(HP20X_READ_AT);
		IIC_Start();
		IIC_WriteByte(HP20X_I2C_RD_ID);
		IIC_W_ACK();//等待应答信号
		Temperature = HP20X_IIC_Read_3_Byte();
		IIC_R_ACK();
		Altitude = HP20X_IIC_Read_3_Byte();
		IIC_NoAck();
		IIC_Stop();
		//---------------计算海拔高度平均值
		if(AltitudeIndex <AltitudeBufLength)
		{	AltitudeBuf[AltitudeIndex++] = Altitude;	}
		else
		{	AvgAltitude.Data = 0x00000000;
			for(AltitudeIndex =0;AltitudeIndex <AltitudeBufLength;)
			{	AvgAltitude.Data += AltitudeBuf[AltitudeIndex++];	}//准备计算均值
			AvgAltitude.Data /= AltitudeBufLength;
			//-----------------------------------
			if((AvgAltitude.Data & 0x80000000)== 0x80000000)//负值，补码转换
			{	AvgAltitude.Data = (~AvgAltitude.Data +1) &0x00FFFFFF;	}
			//-----------------------------------
			AltitudeIndex =0;
		}
	}
}
//===================================================================       
void HP20X_DataProcess(void)
{	if(HP20X_ReadTime == 1)
	{	HP20X_ReadTime++;
		HP20X_IIC_WriteCmd(HP20X_ANA_CAL);//执行内部校准
	}
	if(HP20X_ReadTime == 5)//启动转换
	{	HP20X_ReadTime++;
		HP20X_IIC_WriteCmd(HP20X_CONVER_CMD |OSR_CFG);
	}
	if(HP20X_ReadTime == 8)//等待压力值转换完成
	{	HP20X_ReadTime++;
		HP20X_IIC_ReadPressure();
	}
	if(HP20X_ReadTime == 9)//等待高度温度转换完成
	{	HP20X_ReadTime++;
		HP20X_ReadAltitudeTemperature();
	}
}


