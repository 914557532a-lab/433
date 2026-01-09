//-----------------------------------------------------------------------------
//功能说明:	PWM功能驱动
//-----------------------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

#include "pwm.h"

//-----------------------------------------------------------------------------
xdata unsigned int PWM_DivDat;
xdata unsigned int PWM_DutyDat;

//-----------------------------------------------------------------------------
//函数名：	PWM_init
//功能描述：PWM功能初始化
//输入参数: pwm_ch		PWM通道选择
//			AlignMod	对齐模式设置
//			cks			PWM时钟源设置（只有PWM0,2,4,6才有此项设置）
//			toggle		PWM是否反相输出设置
//			Prescal		PWM时钟分频设置
//			Comp		PWM互补输出设置（只有PWM1,3,5,7才有此项设置）
//-----------------------------------------------------------------------------
void PWM_init(PWM_Channel_TypeDef pwm_ch,
			 PWM_AlignMod_TypeDef AlignMod,
				  PWM_CKS_TypeDef cks,
			   PWM_Toggle_TypeDef toggle,
					unsigned char Prescal,
		PWM_Complementary_TypeDef Comp)
{
	INDEX = pwm_ch;
	if(pwm_ch & 0x01)	//PWM1,PWM3,PWM5,PWM7
	{
		PWMCON = (PWMCON&0xF0) | MS(AlignMod) | MOD(Comp);
		PWMCFG = TOG(toggle) | Prescal;
	}
	else				//PWM0,PWM2,PWM4,PWM6
	{
		PWMCON = (PWMCON&0xF0) | MS(AlignMod) | CKS(cks);
		PWMCFG = TOG(toggle) | Prescal;
	}
}

//-----------------------------------------------------------------------------
//函数名：	PWM_CfgDivDuty
//功能描述：设置PWM周期和占空比
//输入参数: pwm_ch		PWM通道选择
//			DIV			PWM周期设置
//			DUT			PWM占空比设置
//-----------------------------------------------------------------------------
void PWM_CfgDivDuty(PWM_Channel_TypeDef pwm_ch,unsigned int DIV,unsigned int DUT)
{	INDEX = pwm_ch;
	PWMDIVH	= (unsigned char)(DIV>>8);
	PWMDIVL	= (unsigned char)(DIV);
	PWMDUTH	= (unsigned char)(DUT>>8);
	PWMDUTL	= (unsigned char)(DUT);	
}

//-----------------------------------------------------------------------------
//函数名：	PWM_Update
//功能描述：PWM寄存器更新
//输入参数: PWMCH_Bits	
//-----------------------------------------------------------------------------
void PWM_Update(unsigned char PWMCH_Bits)
{	PWMUPD = PWMCH_Bits;
}

//-----------------------------------------------------------------------------
//函数名：	WaitPWMUpdateComplete
//功能描述：等待PWM配置值更新到实际寄存器
//-----------------------------------------------------------------------------
void WaitPWMUpdateComplete(void)
{
	while(PWMUPD);
}

//-----------------------------------------------------------------------------
//函数名：	PWM_EnContrl
//功能描述：	PWM使能
//输入参数: 	PWMEn_Bits	，每一位对应PWM的一个通道
//-----------------------------------------------------------------------------
void PWM_EnContrl(unsigned char PWMEn_Bits)
{
	PWMEN = PWMEn_Bits;
}

//-----------------------------------------------------------------------------
//函数名：	PWM_InterruptCfg
//功能描述：设置PWM周期和占空比
//输入参数: pwm_ch	PWM通道选择
//			tie		PWM顶点中断设置
//			zie		PWM零点中断设置
//			pie		PWM上升沿中断
//-----------------------------------------------------------------------------
void PWM_InterruptCfg(PWM_Channel_TypeDef pwm_ch,
						  PWM_TIE_TypeDef tie,
						  PWM_ZIE_TypeDef zie,
						  PWM_PIE_TypeDef pie,
						  PWM_NIE_TypeDef nie,
						    unsigned char pwmcmax	)
{
	INDEX = pwm_ch;
	PWMCON = (PWMCON&0x0F) | TIE(tie) | ZIE(zie) | PIE(pie) | NIE(nie);
	PWMCMAX = pwmcmax;
}
