#ifndef FLASH_H
#define FLASH_H
enum
{
	CMD_DATA_AREA_READ 	= 1,	
	CMD_DATA_AREA_WIRTE = 2,	
	CMD_DATA_AREA_ERASE_SECTOR = 3,
	CMD_CODE_AREA_READ = 5,	
	CMD_CODE_AREA_WRITE = 6,
	CMD_CODE_AREA_ERASE_SECTOR = 7,
};
enum
{
	CMD_CODE_AREA_UNLOCK = 0x29,
	CMD_DATA_AREA_UNLOCK = 0x2A,
	CMD_FLASH_LOCK = 0xAA,
};

 //寄存器FSCMD位定义
#define IFEN		(1<<7)

//-----------------------------------------------------------------------------
#define MAX_08K_FLASH	32
#define MAX_16K_FLASH	64
#define MAX_32K_FLASH	128//可用扇区数量

#define DATA_FLASH_USER		2//n *256的倍数划分 Falsh区，作为存储数据用
#define DATA_FLASH_START	(MAX_32K_FLASH -DATA_FLASH_USER)

#define SECTOR_SIZE			256
#define ROM_ADDR_OFFSET		0x0000//设置EEPROM 地址

#define	READ_DATA	0xAA//对ROM 操作的类型
#define WRITE_DATA	0x55
//-----------------------------------------------------------------------------
#define CHIP_ID_LENGHT	4
#define CONFIGDATA_LENGHT 26
#define CHECKSUM_LENGHT	2 //校验和

#define ROM_DATA_1_LENGHT (CHIP_ID_LENGHT + CONFIGDATA_LENGHT +CHECKSUM_LENGHT)//数据长度

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
typedef union{
	unsigned char Data[ROM_DATA_1_LENGHT];//配置参数
	struct{
		unsigned char Chip_ID_0;//芯片序列号 1
		unsigned char Chip_ID_1;//芯片序列号 2
		unsigned char Chip_ID_2;//芯片序列号 3
		unsigned char Chip_ID_3;//芯片序列号 4
	//-------------------------------------------------
		union{
			unsigned char Array[2];
			unsigned int DATA;
		}Password;//管理密码
		union{
			unsigned char Array[4];
			unsigned long DATA;
		}RF_Address;//遥控地址编号   ---10
	//-------------------------------------------------
		union{
			unsigned char Array[2];
			unsigned int DATA;
		}Manufacturer;//制造商 ID
		unsigned char ProductType;//产品类型
		unsigned char ProductLanguage;//产品语言类型
	//-------------------------------------------------
		unsigned char SetupFuelType;//设定的燃油类型
	//-------------------------------------------------
		unsigned char SetupTemper_L_Limit;//温度下限
		unsigned char SetupTemper_H_Limit;//温度上限
	//-------------------------------------------------
		unsigned char MIN_OilPumpCycle;//起始泵油量
		unsigned char MAX_OilPumpCycle;//最大泵油量
	//-------------------------------------------------
		union{
			unsigned char Array[2];
			unsigned int DATA;
		}MIN_FanSpeed;//设定的起始转速
		union{
			unsigned char Array[2];
			unsigned int DATA;
		}MAX_FanSpeed;//设定的最高转速
	//-------------------------------------------------
		unsigned char SystemPowerVolatge;//设定工作电压,面板默认为 12V = 120
		unsigned char Ignition_Power;//点火塞功率
	//-------------------------------------------------
		unsigned char TemperatureType;//温度类型，根据手动选择调节
		unsigned char EnergyLevel;//输出功率级别
		unsigned char RunStateType;//加热状态类型 == 1 单次加热，== 2 循环加热
	//-------------------------------------------------
		unsigned char RF_ModeSelect;//
		unsigned char temptemp;//
	//-------------------------------------------------
		union{
			unsigned char Array[2];
			unsigned int DATA;
		}CRC_DATA;//Config[0 - 11] 数据的校验结果
	}Struct;
}tpdef_struct_SN;

//-----------------------------------------------------------------------------
extern xdata tpdef_struct_SN Config;//所有数据交换 

void RunData_Initial(void);
void RunDataUpdate(void);
void RunData_Copy(unsigned char *Purpose,unsigned char *Source,unsigned char Seed,unsigned char Proof,unsigned char Lenght);
void Data_Area_ReadWrite(unsigned char Type,unsigned char Sector,unsigned char *DataIndex,volatile unsigned char DataLenght);
#endif

