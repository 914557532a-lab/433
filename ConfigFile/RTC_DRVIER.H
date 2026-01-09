//---------------------------------------------------------
//功能说明：定义芯片内部 实时时钟寄存器值
//---------------------------------------------------------
//RTCON定义
#define RTCE(N)		(N<<7) 
#define MSE(N)		(N<<6) 
#define HSE(N)		(N<<5)
#define SCE(N)		(N<<4)
#define MCE(N)		(N<<3)
#define HCE(N)		(N<<2)
#define RTCWE(N)	(N<<1)



//RTCIF定义
#define RTC_MF		(1<<2)
#define RTC_HF		(1<<1)
#define RTC_AF		(1<<0)

typedef enum {
	RTCE_Off = 0,
	RTCE_On = 1,
} RTCE_TypeDef;

typedef enum {
	MSE_Off = 0,
	MSE_On = 1,
} MSE_TypeDef;

typedef enum {
	HSE_Off = 0,
	HSE_On = 1,
} HSE_TypeDef;

typedef enum {
	SCE_Off = 0,
	SCE_On = 1,
} SCE_TypeDef;

typedef enum {
	MCE_Off = 0,
	MCE_On = 1,
} MCE_TypeDef;

typedef enum {
	HCE_Off = 0,
	HCE_On = 1,
} HCE_TypeDef;

//---------------------------------------------------------
extern xdata unsigned char TimeBuffer[3];//时，分，秒
extern xdata unsigned char AlaramTime[6];

void RTC_InitialSet(void);
void RTC_TimeRead(void);
void RTC_TimeWrite(unsigned char hour,unsigned char minute,unsigned char second);
