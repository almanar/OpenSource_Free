/******************************************************************************
 **    Pinnacle Solutions.
 **    COPYRIGHT 2007 Pinnacle Solutions, ALL RIGHTS RESERVED
 **    This program may not be reproduced, in whole or in part in any form
 **    or by any means whatsoever without the written permission of
 **        Pinnacle Solutions
 **        Plot No: 20A, S/No:257
 **        Khese Park, Lohegaon,
 **		   Pune: 411032
 **
 **    File Name:     target.h
 **
 **    Description:   This is the uart header module.
 **
 **    Project Name:  Arya MDT 
 **
 **    Platform:      ARM7TDMI LPC213x
 **
 **    Creation Date: 04/01/2008
 **
 **    Revision History: Please use VSS to get the revision history.
 *****************************************************************************/
#ifndef _UART_H 
#define _UART_H

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

#define UART_BUF_INDEX 		14
#define UART_TX_BUF_INDEX 	1024

#define UART_BUFSIZE		512

extern uint8 UART0TxBuffer[ UART_TX_BUF_INDEX ];
extern uint32 UARTTxBufferCount;

extern uint8 UART0Buffer[ UART_BUF_INDEX ][ UART_BUFSIZE ];
extern uint8 Uart0BufferIndex;
extern uint32 Uart0Count;

extern uint8 UART1Buffer[ UART_BUF_INDEX ][ UART_BUFSIZE ];
extern uint8 Uart1BufferIndex;
extern uint32 Uart1Count;

uint64 UART0Init( uint64 Baudrate );
uint64 UART1Init( uint64 Baudrate );

void UART0Handler( void ) __irq;
void UART1Handler( void ) __irq;

void UART0Send( uint8 *BufferPtr, uint64 Length );
void UART0SendBuffer(uint8 *BufferPtr, uint64 Length );
void UART1Send( uint8 *BufferPtr, uint64 Length );
#endif /* end _UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
