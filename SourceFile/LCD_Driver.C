//-----------------------------------------------------------------------------
//功能说明：液晶驱动寄存器配置程序
//创建日期：
//-----------------------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

//-----------------------------------------------------------------------------
#include "LCD_DRIVER.H"

//-----------------------------------------------------------------------------
//函数名：LCD_init
//功能说明：LCD寄存器初始化
//输入参数：len		LCD时钟源选择
//			dmod	驱动电流选择
//			bias	偏压设置
//			ldrv	驱动电压等级设置
//			lxdiv	LCD时钟分频系数
//-----------------------------------------------------------------------------
void LCD_init(LEN_TypeDef Clk,DMOD_TypeDef dmod,BIAS_TypeDef bias,LDRV_TypeDef ldrv,unsigned int lxdiv)
{
	LXDIVH = (unsigned char)(lxdiv>>8);
	LXDIVL = (unsigned char)(lxdiv);
	LXCFG =	 DMOD(dmod) | BIAS(bias) | LDRV(ldrv);
	LXCON =  LEN(Clk) | LMOD(LMOD_lcd);
}

//-----------------------------------------------------------------------------
//函数名：	LCD_WriteLram
//功能说明：写入数据到显示缓存
//输入参数：laddr	显示缓存地址
//			ldata	显示缓存数据
//-----------------------------------------------------------------------------
void LCD_WriteLram(unsigned char laddr, unsigned char ldata)
{
	INDEX = laddr;
	LXDAT = ldata;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void LCD_Chip_RAM_Clear(void)
{unsigned char Count;
	for(Count=0;Count<LCD_RAM_SIZE;Count++)
	{	LCD_WriteLram(Count,0x00);		}
}
//-----------------------------------------------------------------------------
void LCD_PORT_Initial(void)
{	GPIO_Init(P00F,P00_COM0_SETTING);
	GPIO_Init(P01F,P01_COM1_SETTING);
	GPIO_Init(P02F,P02_COM2_SETTING);
	GPIO_Init(P03F,P03_COM3_SETTING);//COM 端口
//===============================================
 	GPIO_Init(P20F,P20_SEG31_SETTING);
	GPIO_Init(P21F,P21_SEG30_SETTING);
 	GPIO_Init(P22F,P22_SEG29_SETTING);
 	GPIO_Init(P23F,P23_SEG28_SETTING);
 	GPIO_Init(P24F,P24_SEG27_SETTING);
 	GPIO_Init(P25F,P25_SEG26_SETTING);
 	GPIO_Init(P26F,P26_SEG25_SETTING);
 	GPIO_Init(P27F,P27_SEG24_SETTING);
////////
 	GPIO_Init(P17F,P17_SEG23_SETTING);
 	GPIO_Init(P16F,P16_SEG22_SETTING);
 	GPIO_Init(P15F,P15_SEG21_SETTING);
 	GPIO_Init(P14F,P14_SEG20_SETTING);
 	GPIO_Init(P13F,P13_SEG19_SETTING);
 	GPIO_Init(P12F,P12_SEG18_SETTING);
 	GPIO_Init(P11F,P11_SEG17_SETTING);
	GPIO_Init(P10F,P10_SEG16_SETTING);
////////
 	GPIO_Init(P65F,P65_SEG15_SETTING);
	GPIO_Init(P64F,P64_SEG14_SETTING);
	GPIO_Init(P63F,P63_SEG13_SETTING);//位码驱动
//===============================================
#if		(SYSCLK_SRC	== CK_IRCH)//IRCH
	LCD_init(LEN_IRCH,DMOD_130_uA,BIAS_1_3,LDRV_7,100);
#else
	LCD_init(LEN_PLL,DMOD_130_uA,BIAS_1_3,LDRV_6,100);
	//7- 1/4	√-×
	//6- 1/2	√
	//6- 1/3	√
	//6- 1/4	√
	//5- 1/2	√
	//5- 1/3	×
	
#endif
//===============================================
	LCD_Chip_RAM_Clear();//清除显示数据
}
