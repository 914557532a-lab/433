#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

//-----------------------------------------------------------------------------
//LXCON定义
#define LEN(N)		(N<<5) 	//N=0~6
#define LMOD(N)		(N<<4)	//N=0~1

//LXCFG定义
#define DMOD(N)		(N<<6)
#define BIAS(N)		(N<<4)

#define LDRV(N)		(N) //N=0-7

typedef enum {
  LEN_DISABLE	= 0,
  LEN_IRCL  	= 1,
  LEN_IRCH  	= 2,
  LEN_XOSCL  	= 3,
  LEN_ERC_XOH	= 4,
  LEN_PLL  		= 5,
  LEN_TF  		= 6,
} LEN_TypeDef;

typedef enum {
  LMOD_lcd	= 0,
  LMOD_led  = 1,
} LMOD_TypeDef;

typedef enum {
	DMOD_5_uA	= 0,
	DMOD_40_uA  = 1,
	DMOD_80_uA 	= 2,
	DMOD_130_uA = 3,
} DMOD_TypeDef;

typedef enum {
	BIAS_1_4	= 0,
	BIAS_1_2  	= 1,
	BIAS_1_3 	= 2,
} BIAS_TypeDef;

typedef enum {
	LDRV_0		= 0,
	LDRV_1		= 1,
	LDRV_2		= 2,
	LDRV_3		= 3,
	LDRV_4		= 4,
	LDRV_5		= 5,
	LDRV_6		= 6,
	LDRV_7		= 7,
} LDRV_TypeDef;

#endif
//-----------------------------------------------------------------------------
#define LCD_RAM_SIZE	18

//-----------------------------------------------------------------------------
void LCD_WriteLram(unsigned char laddr, unsigned char ldata);

void LCD_PORT_Initial(void);
