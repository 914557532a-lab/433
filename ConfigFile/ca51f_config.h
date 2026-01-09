#ifndef CA51F_CONFIG_H
#define CA51F_CONFIG_H
//=============================================================================
//CKCON寄存器定义
#define ILCKE		(1<<7)
#define IHCKE		(1<<6)
#define TFCKE		(1<<5)
#define XHCS		(1<<4)
#define XLCKE		(1<<3)
#define XLSTA		(1<<2)
#define XHCKE		(1<<1)
#define XHSTA		(1<<0)

//CKSEL寄存器定义
#define RTCKS(N)		(N<<7)

#define CK_IRCH		0
#define CK_IRCL		1
#define CK_XOSCH	2
#define CK_XOSCL	3
#define CK_PLL		4
#define CK_TFRC		5

//PLLCON寄存器定义
#define PLLON(N)		(N<<7)
#define MULFT(N)		(N<<3)	//N=0-8


#define PLSTA		(1<<0)

//寄存器RCCON定义	
#define MODE(N)		(N<<6)	//N=0~3
#define MSEX(N)		(N<<5)	//N=0~1

#define HMSK		(1<<4)

#define CKSS(N)		N		//N=0~11
///////
//#define SYSCLK_SRC	CK_IRCH	//系统内部时钟
//#define FOSC	11059200	//定义时钟频率，用来计算波特率

#define SYSCLK_SRC	CK_PLL	//芯片PLL时钟选择
#define FOSC	12000000	//定义时钟频率，用来计算波特率

//=============================================================================
//=============================================================================
//=============================================================================

//=============I2C功能定义===================================================
#define I2C_ADDR		0xCA

#define MASTER_WRITE	0
#define MASTER_READ		1

#define MASTER_MODE		MASTER_WRITE


#endif
