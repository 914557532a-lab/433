#ifndef DEFINE_H
#define DEFINE_H

typedef union 
{	
	unsigned long int 	dwVal;
	unsigned int 		wVal[2];
	unsigned char 		bVal[4];
}
DWORD_UNION;

typedef union 
{	
	unsigned int 	wVal;
	unsigned char 	bVal[2];
}
WORD_UNION;


#define BIT0        0x01
#define BIT1        0x02
#define BIT2        0x04
#define BIT3        0x08
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80
#define BIT8        0x100
#define BIT9        0x200
#define BIT10       0x400
#define BIT11       0x800
#define BIT12       0x1000
#define BIT13       0x2000
#define BIT14       0x4000
#define BIT15       0x8000


#define __NOP1__  	_nop_()
#define __NOP2__ 	__NOP1__; __NOP1__
#define __NOP4__ 	__NOP2__; __NOP2__
#define __NOP8__ 	__NOP4__; __NOP4__
#define __NOP16__ 	__NOP8__; __NOP8__
#define __NOP32__ 	__NOP16__; __NOP16__
#define __NOP64__ 	__NOP32__; __NOP32__
#define __NOP128__ 	__NOP64__; __NOP64__


#define DelayNop(a)             		\
    if ((a)&(0x01))    {__NOP1__;}      \
    if ((a)&(0x02))    {__NOP2__;}      \
    if ((a)&(0x04))    {__NOP4__;}      \
    if ((a)&(0x08))    {__NOP8__;}      \
    if ((a)&(0x10))    {__NOP16__;}     \
    if ((a)&(0x20))    {__NOP32__;}     \
    if ((a)&(0x40))    {__NOP64__;}     \
    if ((a)&(0x80))    {__NOP128__;}

#endif
//////////////////////////////////////////////////////
#define EQUAL_L		0
#define EQUAL_H		1

#define Enable_CMT2300	//打开双向遥控

#define Closevoice		0	//关闭语音
#define Chinese			1
#define English			2
#define Multilingual	3	//多种语言
	
#define LanguageType	Closevoice//定义语言的类型
	
	
#define Select_OneWay_RF 1
#define Select_TwoWay_RF 2

//////////////////////////////////////////////////////

void Delay_100us(unsigned int Loop);

