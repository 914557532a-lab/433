//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#define KEY_HOLDTIME	5//
#define KeyCount_HOLD	400

extern xdata unsigned char ExitSetupTime;//退出设置计时

extern xdata unsigned char KeyState;
extern xdata unsigned char KeyEvent;
extern xdata unsigned int KeyCount_Time;//长按计时
extern xdata unsigned char KeyTimeLimit;
extern xdata unsigned char KeyEventCount;
//--------------------------------------------------------------
extern xdata union {
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

extern xdata unsigned char DataChangeUpdataTime;
extern xdata unsigned char OilpumpEnableState;//允许泵油条件
//-----------------------------------------------------------------------------
enum {
    TASK_IdleMode		= 0x00,	//待机
    TASK_Self_Test		= 0x01,	//自检(准备工作前，自检所有驱动部件)
    TASK_Ignition		= 0x02,	//点火启动
    TASK_Ignition_Fail	= 0x03,	//点火失败 (进行2次重启)
    TASK_RapidWarming	= 0x04,	//快速升温
    TASK_Normal_Run		= 0x05,	//正常运行
    TASK_Turn_Off		= 0x06,	//准备关机
    TASK_Wait_Off		= 0x07,	//等待关机
    TASK_Shutdown		= 0x08,	//关机结束
	TASK_WaterPump		= 0xA3,	//手动运行水泵
	TASK_Pump_Oil		= 0xAC,	//手动泵油
};

enum {
	UART_Extensive   =0xFF,	//广播读取
	UART_Synchrodata =0xA6, //根据面板数据运行
	UART_FixedData   =0xA5,	//根据主板数据运行
	//-------------------------------------------
	UART_Null = 0x00,//无效状态
	UART_PumpOil = 0x53,//手动泵油
	UART_ExitPumpOil = 0x55,//退出泵油
	UART_RunWaterPump = 0x57,//手动启动水泵
	UART_StopWaterPump = 0x58,//关闭水泵运行
	URAT_PowerOn = 0x5A,//开机
	UART_PowerOff = 0x5C,//关机
};
//-----------------------------------------------------------------------------
void KeyStateScan(void);
void KeyEventProcess(void);


