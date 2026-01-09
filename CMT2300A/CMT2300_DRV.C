//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include <intrins.h>
#include "ca51f_config.h"
#include "ca51f2sfr.h"
#include "ca51f2xsfr.h"
#include "gpiodef_f2.h"
#include "system.h"

//-----------------------------------------------------------------------------
#include "CMT2300_DRV.H"

//* *************************************************************************
void Cmt2300A_Delay_us(unsigned int Loop)
{	do{
		unsigned int n = 10;
		while(n--);
	}while(Loop--);
}

//* *************************************************************************
void cmt_SPI_send(unsigned char Data)
{ unsigned char i;
	cmt_sdio_Pin_out;//保证 数据口为输出
	for(i=0; i<8; i++)
    {	if((Data & 0x80) !=0)
        {	cmt_sdio_Pin_1;		}
        else            
		{	cmt_sdio_Pin_0;		}

        Data <<= 1;
        Cmt2300A_Delay_us(1);
        cmt_sclk_Pin_1;//Send bit on the rising edge of SCLK
		Cmt2300A_Delay_us(1);
        cmt_sclk_Pin_0;
	}
}

unsigned char cmt_SPI_recv(void)
{unsigned char i;	unsigned char Data;
	cmt_sdio_Pin_in;//防止硬件冲突
	for(i=0; i<8; i++)
	{	Cmt2300A_Delay_us(1);
		cmt_sclk_Pin_1;//Read bit on the rising edge of SCLK 
		Data <<= 1;
		if((cmt_sdio_Pin_read) != 0)
		{	Data |= 0x01;	}
        else
		{	Data &= 0xFE;	}

        Cmt2300A_Delay_us(1);
		cmt_sclk_Pin_0;
    }
    return (Data);
}
//* *************************************************************************
//* @name    CMT2300A_WriteReg
//* @desc    Write the CMT2300A register at the specified address.
//* @param   addr: register address
//*          dat: register value
//* *************************************************************************
void CMT2300A_WriteReg(unsigned char addr, unsigned char dat)
{	cmt_fcsb_Pin_1;
    cmt_csb_Pin_0;//选中访问寄存器
//--------------------------------
	Cmt2300A_Delay_us(2);//> 1 SCLK cycle

    cmt_SPI_send(addr&0x7F);// r/w = 0
    cmt_SPI_send(dat);

	Cmt2300A_Delay_us(2);//> 1 SCLK cycle

    cmt_csb_Pin_1;//关闭访问寄存器
}
//* *************************************************************************
//* @name    CMT2300A_ReadReg
//* @desc    Read the CMT2300A register at the specified address.
//* @param   addr: register address
//* @return  Register value
//* *************************************************************************
unsigned char CMT2300A_ReadReg(unsigned char addr)
{unsigned char Reg_dat;
//--------------------------------
	cmt_fcsb_Pin_1;
	cmt_csb_Pin_0;//选中访问寄存器
//--------------------------------
	Cmt2300A_Delay_us(2);//> 1 SCLK cycle

    cmt_SPI_send(addr|0x80);//r/w = 1
    Reg_dat = cmt_SPI_recv();

	Cmt2300A_Delay_us(2);//> 1 SCLK cycle

    cmt_csb_Pin_1;//关闭访问寄存器
	return (Reg_dat);
}
//* *************************************************************************
//* @name    CMT2300A_WriteFifo
//* @desc    Writes the buffer contents to the CMT2300A FIFO.
//* @param   buf: buffer containing data to be put on the FIFO
//*          len: number of bytes to be written to the FIFO
//* *************************************************************************
void CMT2300A_WriteFifo(const unsigned char * p_buf, unsigned int len)
{	unsigned int i;
//--------------------------------
    cmt_fcsb_Pin_1;
    cmt_csb_Pin_1;
//--------------------------------
    for(i=0; i<len; i++)
    {	cmt_fcsb_Pin_0;//开始访问 FIFO
        Cmt2300A_Delay_us(2);//> 1 SCLK cycle

        cmt_SPI_send(p_buf[i]);
        Cmt2300A_Delay_us(4);//> 2 us

        cmt_fcsb_Pin_1;//关闭 FIFO 访问
        Cmt2300A_Delay_us(6);//> 4 us
    }
}
//* *************************************************************************
//* @name    CMT2300A_ReadFifo
//* @desc    Reads the contents of the CMT2300A FIFO.
//* @param   buf: buffer where to copy the FIFO read data
//*          len: number of bytes to be read from the FIFO
//* *************************************************************************
void CMT2300A_ReadFifo(unsigned char * p_buf, unsigned int len)
{ unsigned int i;
//--------------------------------
    cmt_fcsb_Pin_1;
    cmt_csb_Pin_1;
//--------------------------------
    for(i=0; i<len; i++)
    {	cmt_fcsb_Pin_0;//开始访问 FIFO
        Cmt2300A_Delay_us(2);//> 1 SCLK cycle

        p_buf[i] = cmt_SPI_recv();
        Cmt2300A_Delay_us(4);//> 2 us

        cmt_fcsb_Pin_1;//关闭 FIFO 访问
        Cmt2300A_Delay_us(6);//> 4 us
    }
}

/*! ********************************************************
* @name    CMT2300A_InitGpio
* @desc    Initializes the CMT2300A interface GPIOs.
* *********************************************************/
void CMT2300A_InitGpio(void)
{
//--------------------------------------------------------------
	// PxM1.n		PxM2.n		I/O 类型
	//   0			  0			准双向
	//   0			  1			推挽输出
	//   1			  0			输入 (高阻)
	//   1			  1			开漏
//--------------------------------------------------------------
	cmt_sclk_Pin_0;// SCLK has an internal pull-down resistor
	cmt_sclk_Init;//cmt_sclk_Pin = P00

    cmt_sdio_Pin_1;
	cmt_sdio_Init;//cmt_sdio_Pin = P10

	cmt_csb_Pin_1;// CSB has an internal pull-up resistor
	cmt_csb_Init;//cmt_csb_Pin = P11

	cmt_fcsb_Pin_1;// FCSB has an internal pull-up resistor
	cmt_fcsb_Init;//cmt_fcsb_Pin = P12

	cmt_GPIO_Init;//SetGpio3In = P01

    Cmt2300A_Delay_us(10);
}

