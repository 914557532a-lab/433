/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, CMOSTEK SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * Copyright (C) CMOSTEK SZ.
 */

/*!
 * @file    cmt2300a.h
 * @brief   CMT2300A transceiver RF chip driver
 *
 * @version 1.3
 * @date    Jul 17 2017
 * @author  CMOSTEK R@D
 */

#ifndef __CMT2300A_H
#define __CMT2300A_H

#include "CMT2300_DRV.H"

#ifdef __cplusplus 
extern "C" { 
#endif

/* ************************************************************************
   The following are for chip status controls.
*  ************************************************************************ */
void CMT2300A_SoftReset(void);
unsigned char CMT2300A_GetChipStatus(void);
void CMT2300A_AutoSwitchStatus(unsigned char nGoCmd);
void CMT2300A_GoSleep(void);
void CMT2300A_GoStby(void);
void CMT2300A_GoTFS(void);
void CMT2300A_GoRFS(void);
void CMT2300A_GoTx(void);
void CMT2300A_GoRx(void);


/* ************************************************************************
*  The following are for chip interrupts, GPIO, FIFO operations.
*  ************************************************************************ */
void CMT2300A_ConfigGpio(unsigned char nGpioSel);
void CMT2300A_ConfigInterrupt(unsigned char nInt1Sel, unsigned char nInt2Sel);
void CMT2300A_SetInterruptPolar(unsigned char bActiveHigh);
void CMT2300A_SetFifoThreshold(unsigned char nFifoThreshold);
void CMT2300A_EnableAntennaSwitch(unsigned char nMode);
void CMT2300A_EnableInterrupt(unsigned char nEnable);
void CMT2300A_EnableRxFifoAutoClear(unsigned char bEnable);
void CMT2300A_EnableFifoMerge(unsigned char bEnable);
void CMT2300A_EnableReadFifo(void);
void CMT2300A_EnableWriteFifo(void);
void CMT2300A_RestoreFifo(void);
unsigned char CMT2300A_ClearTxFifo(void);
unsigned char CMT2300A_ClearRxFifo(void);
unsigned char CMT2300A_ClearInterruptFlags(void);


/* ************************************************************************
*  The following are for Tx DIN operations in direct mode.
*  ************************************************************************ */
void CMT2300A_ConfigTxDin(unsigned char nDinSel);
void CMT2300A_EnableTxDin(unsigned char bEnable);
void CMT2300A_EnableTxDinInvert(unsigned char bEnable);


/* ************************************************************************
*  The following are general operations.
*  ************************************************************************ */
unsigned char CMT2300A_IsExist(void);
unsigned char CMT2300A_GetRssiCode(void);
int CMT2300A_GetRssiDBm(void);
void CMT2300A_SetFrequencyChannel(unsigned char nChann);
void CMT2300A_SetFrequencyStep(unsigned char nOffset);
void CMT2300A_SetPayloadLength(unsigned int nLength);
void CMT2300A_EnableLfosc(unsigned char bEnable);
void CMT2300A_EnableLfoscOutput(unsigned char bEnable);
void CMT2300A_EnableAfc(unsigned char bEnable);
void CMT2300A_SetAfcOvfTh(unsigned char afcOvfTh);


/* ************************************************************************
*  The following are for chip initializes.
*  ************************************************************************ */
void CMT2300A_Init_Reg(void);
void CMT2300A_ConfigRegBank(unsigned char code *base_addr, unsigned char len);

#ifdef __cplusplus
} 
#endif

#endif
