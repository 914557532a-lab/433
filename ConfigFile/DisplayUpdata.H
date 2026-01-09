//-----------------------------------------------------------------------------
//功能说明：风暖面板，显示数据更新
//创建日期：
//-----------------------------------------------------------------------------
extern xdata unsigned int BackLightTime;
extern xdata unsigned char DisplaySetIndex;
extern xdata unsigned char DisplayStateIndex;
//////
extern xdata unsigned char FlashDisplayTime;
extern xdata unsigned char Fan_DisplayTime;

extern xdata unsigned char SystemRunIndex;
//////
extern xdata unsigned char SystemRunState;
extern xdata unsigned char SystemStateHold;
extern xdata unsigned char SystemStateTime;
//////
extern xdata unsigned int InputPassword;

///////////////////////////////////////////////////////////
extern xdata union{
	unsigned char TempArray[2];
	signed int Temperature;
}SW;
extern xdata union{
	unsigned char TempArray[2];
	signed int Temperature;
}HY;
extern xdata union{
	unsigned char Array[2];
	unsigned int Data;
}RunVoltage;
extern xdata union{
	unsigned char Array[2];
	unsigned int Data;
}RotateSpeed;
extern xdata union{
    unsigned char Array[2];
	unsigned int Data;
}Fan_Vot;
extern xdata union{
    unsigned int DutyCycle;
    unsigned char Data[2];
}Ignition_Vot;
extern xdata union{
    unsigned int Current;
    unsigned char Data[2];
}Ignition_CUR;
extern xdata union{
    unsigned char Array[2];
	unsigned int Data;
}WaterPump_CUR;

extern xdata unsigned char ErrorIndex;

extern xdata unsigned char OilpumpSetFrequ;//油泵设定频率
extern xdata unsigned char OilPumpFrequency;
extern xdata unsigned char WorkingTime;//剩余工作时间
extern xdata unsigned char RemainingWorkingTime;//剩余工作时间
///////////////////////////////////////////////////////////
void DisplayDataInitial(void);
void DisplayDataProcess(void);
void LCD_DisplayUpdate(void);
void Display_Test(void);
