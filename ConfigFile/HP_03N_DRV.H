//-----------------------------------------------------------------------------
//功能说明：HP203N 温度和高度值读取
//-----------------------------------------------------------------------------
//HP20X INCLUDED:HP203B,HP206C,HP209F

#define HP20X_I2C_WR_ID		0xEC//CSB PIN is VDD
#define HP20X_I2C_RD_ID		0xED

//#define HP20X_I2C_WR_ID	0xEE//CSB PIN is GND
//#define HP20X_I2C_RD_ID	0xEF

//HP20X REGISTER AND COMMAND
#define HP20X_SOFT_RST		0x06
#define HP20X_CONVER_CMD	0x40

#define HP20X_ANA_CAL	0x28  //内部校准
#define HP20X_READ_P	0x30  //读压力值
#define HP20X_READ_A	0x31  //读高度值
#define HP20X_READ_T	0x32  //读温度值
#define HP20X_READ_PT	0x10  //读压力温度值
#define HP20X_READ_AT	0x11  //读高度温度值

#define HP20X_R_INT_REG   0x80//读内部寄存器

#define INT_SRC_REG		0x0D//读状态寄存器
#define INT_PARA_REG	0x0F//读补偿状态

#define OSR_CFG		0x08//1024 采样率，同时转换2个通道
//-----------------------------------------------------------------------------
extern xdata unsigned char HP20X_ReadTime;
extern xdata union{
	unsigned char Array[4];
	signed long	Data;
}AvgAltitude;
extern xdata long Temperature;

#define HJ_Temperature		((Temperature %10000) /100)//环境温度

void HP20X_Initial(void);
void HP20X_DataProcess(void);

