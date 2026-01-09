#ifndef __CMT2003_DRV_H
#define __CMT2003_DRV_H

//* *************************************************************************
//*  The following need to be modified by user
//*  ************************************************************************
#define cmt_fcsb_Init	GPIO_Init(P47F,OUTPUT |OP_EN)
#define cmt_fcsb_Pin_1       P47 = 1
#define cmt_fcsb_Pin_0       P47 = 0//FIFO  访问片选

//---------------------------------------------------------
#define cmt_csb_Init	GPIO_Init(P46F,OUTPUT |OP_EN)
#define cmt_csb_Pin_1        P46 = 1
#define cmt_csb_Pin_0        P46 = 0//寄存器访问片选

//---------------------------------------------------------
#define cmt_sdio_Init	GPIO_Init(P45F,OUTPUT )

#define cmt_sdio_Pin_out     GPIO_Init(P45F,OUTPUT )// I/O 切换为输出
#define cmt_sdio_Pin_in      GPIO_Init(P45F,INPUT )// I/O 切换为输入

#define cmt_sdio_Pin_1       P45 = 1
#define cmt_sdio_Pin_0       P45 = 0//数据 输入输出
#define cmt_sdio_Pin_read    P45//读取端口 电平值

//---------------------------------------------------------
#define cmt_sclk_Init	GPIO_Init(P44F,OUTPUT )
#define cmt_sclk_Pin_1       P44 = 1
#define cmt_sclk_Pin_0       P44 = 0//SPI 时钟

//---------------------------------------------------------
#define cmt_GPIO_Init	GPIO_Init(P43F,INPUT |OP_EN)
#define CMT2300A_ReadGpio1()	P43
#define CMT2300A_ReadGpio2()	P43
#define CMT2300A_ReadGpio3()	P43

/* ************************************************************************
*  The following need to be modified by user
*  ********************************************************************* */
void Cmt2300A_Delay_us(unsigned int Loop);

void CMT2300A_InitGpio(void);
void CMT2300A_WriteReg(unsigned char addr, unsigned char dat);
unsigned char CMT2300A_ReadReg(unsigned char addr);

void CMT2300A_ReadFifo(unsigned char* p_buf, unsigned int len);
void CMT2300A_WriteFifo(const unsigned char* p_buf, unsigned int len);

#endif

