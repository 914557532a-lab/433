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
 * @file    radio.h
 * @brief   Generic radio handlers
 *
 * @version 1.2
 * @date    Jul 17 2017
 * @author  CMOSTEK R@D
 */
 
#ifndef __RADIO_H
#define __RADIO_H

#include "CMT2300_HAL.H"

#ifdef __cplusplus 
extern "C" { 
#endif

/* RF state machine */
typedef enum {
    RF_STATE_IDLE = 0,
	RF_STATE_SLEEP,
    RF_STATE_ERROR,
    RF_STATE_TX_WAIT,
    RF_STATE_TX_START,
    RF_STATE_RX_DONE,
    RF_STATE_RX_WAIT,
    RF_STATE_RX_START,
    RF_STATE_RX_ONLY,
} EnumRFStatus;

/* RF process function results */
typedef enum {
    RF_IDLE = 0,
	RF_RX_INT_COFIG,
	RF_TX_INT_COFIG,
} EnumRFResult;
//--------------------------------------------------------------
extern xdata unsigned int	g_nTx_RxTimeout;//收发超时时间

extern xdata unsigned char	CMT2300_T_Array[];//CMT2003_T_Lenght];
extern xdata unsigned char	CMT2300_R_Array[];//CMT2003_R_Lenght];


#define CMT2300_ADDRLEARNTIME	20
extern xdata unsigned char	CMT2300_PairingTime;//学习地址编号时间
extern xdata unsigned char RemoteKey;
//--------------------------------------------------------------
void CMT2300_SetInitial(void);
void CMT2300A_RF_Process(void);
void RunStateControl(void);
	
#ifdef __cplusplus 
} 
#endif

#endif
