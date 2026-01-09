//---------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

#include "Timer0_ISR.H"
#include "UART2_ISR.h"
#include "DisplayUpdata.H"
#include "KeyEvent.H"

#include "RF_Receiver.H"
//---------------------------------------------------------
//#define T0L_DATA	0xF8
//#define T0H_DATA	0xF6

//#define T0L_DATA	0x8F
//#define T0H_DATA	0xFF

#define T0L_DATA	0x82
#define T0H_DATA	0xFF
//---------------------------------------------------------
xdata unsigned char InterruptCount;

xdata unsigned char Interrupt_2_5mS;
xdata unsigned char OneSecondTime;//秒闪
//---------------------------------------------------------
void Timer0_Initial(void)//@ 11.0592MHz
{	TMOD &= 0xF0;
	TMOD |= 0x01;
	TL0 = T0L_DATA;//设置定时初值
	TH0 = T0H_DATA;//设置定时初值 2.5mS
	TF0 = 0;		//清除TF0标志
	ET0 = 1;		//使能 定时器0 中断
	TR0 = 1;		//定时器0开始计时
}
//---------------------------------------------------------
void Timer0_ISR(void) interrupt 1 using 1
{	TF0 = 0;		//清除TF0标志
	TL0 = T0L_DATA;//设置定时初值
	TH0 = T0H_DATA;//设置定时初值 2.5mS
	//------------------------------------------------------------
	RF_SampleReceive();
	//------------------------------------------------------------
//	P6 ^=0x02;//测试定时中断
	//------------------------------------------------------------
	if(++Interrupt_2_5mS <20){		return;		}
	Interrupt_2_5mS = 0x00;
	//------------------------------------------------------------
	InterruptCount++;// 2.5mS中断一次
	Fan_DisplayTime++;//控制扇叶动态显示
	TakeRunStateTime++;
	//------------------------------------------------------------
	KeyStateScan();//读取按键
	//------------------------------------------------------------
	if(UART2_OverTime > 0x00)//防止串口数据超时
	{	UART2_OverTime--;		}
	else
	{	ReceiveIndex = 0x00;	}
	//------------------------------------------------------------
	if(UART2_SendHoldTime >0x00)
	{	if(--UART2_SendHoldTime ==0)
		{	UART2_Direction_CLR;	}//设为接收
	}
}

