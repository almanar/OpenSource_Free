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
 **    Description:   This is the hardware init module.
 **
 **    Project Name:  Arya MDT 
 **
 **    Platform:      ARM7TDMI LPC213x
 **
 **    Creation Date: 04/01/2008
 **
 **    Revision History: Please use VSS to get the revision history.
 *****************************************************************************/
#include "LPC214x.h"
#include "Types.h"
#include "Target.h"
#include "Irq.h"
#include "Timer.h"
#include "Global.h"
#include "Uart.h"

//Uart Data Buffers
uint8 UART0Buffer[ UART_BUF_INDEX ][ UART_BUFSIZE ];
uint8 UART1Buffer[ UART_BUF_INDEX ][ UART_BUFSIZE ];
uint8 UART0TxBuffer[ UART_TX_BUF_INDEX ];
uint32 UARTTxBufferCount = 0;
uint8 Uart0BufferIndex;
uint8 Uart1BufferIndex;
uint32 Uart0Count;
uint32 Uart1Count;
 
/**
 ******************************************************************************
 **
 **	Function Name		: UART0Handler
 **
 **	Description			: UART0 interrupt handler 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void UART1Handler (void) __irq 
{
    uint8 IIRValue, LSRValue;
    volatile uint8 Dummy;

    IENABLE;					/* handles nested interrupt */	
    IIRValue = U1IIR;
    
    IIRValue >>= 1;				/* skip pending bit in IIR */
    IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */

	if ( IIRValue == IIR_RLS )	/* Receive Line Status */
    {
		LSRValue = U1LSR;
		
		/* Receive Line Status */
		if ( LSRValue & ( LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI ))
		{
		    /* There are errors or break interrupt */
		    /* Read LSR will clear the interrupt */
		    Dummy = LSRValue;
		    Dummy = U1RBR;		/* Dummy read on RX to clear 
						interrupt, then bail out */
		    IDISABLE;
		    VICVectAddr = 0;		/* Acknowledge Interrupt */
		    return;
		}
    }
    else if( IIRValue == IIR_RDA )	/* Receive Data Available */
    {
		/* Receive Data Available */
		//Receive uint8 in buffer
		UART0Buffer[Uart0BufferIndex][Uart0Count] = U1RBR;
		
		//Process Data Received
		if( CHECKFLAG( STX_RECEIVED ))
		{
			if( UART0Buffer[Uart0BufferIndex][Uart0Count] == ETX || 
				UART0Buffer[Uart0BufferIndex][Uart0Count] == '%' )
			{			
				SETFLAG( ETX_RECEIVED );
				RESETFLAG( STX_RECEIVED );
							
				UART0Buffer[Uart0BufferIndex][Uart0Count] = '\0';
				Uart0Count = 0;
				Uart0BufferIndex++;

				RESETFLAG(START_SERIAL_TIMEOUT | SERIAL0_BUSY);
				SETFLAG( SERIAL_TIMEOUT_OVER );				
			}
			else if( UART0Buffer[Uart0BufferIndex][Uart0Count] == STX || 
				 	 UART0Buffer[Uart0BufferIndex][Uart0Count] == '#' )
			{				
			    SETFLAG(SERIAL0_BUSY);
				SETFLAG( STX_RECEIVED | START_SERIAL_TIMEOUT );
				RESETFLAG( SERIAL_TIMEOUT_OVER | ETX_RECEIVED );
				SerialTimeoutTimer = MS_TIMER + SERIAL_TIMEOUT_TIME ;
				Uart0Count = 0;
				UART0Buffer[Uart0BufferIndex][Uart0Count] = '\0';
			}
			else
			{
				Uart0Count++;
				UART0Buffer[Uart0BufferIndex][Uart0Count] = '\0';		
				SerialTimeoutTimer = MS_TIMER + SERIAL_TIMEOUT_TIME ;
			}
		}
		else if( UART0Buffer[Uart0BufferIndex][Uart0Count] == STX ||
				 UART0Buffer[Uart0BufferIndex][Uart0Count] == '#' )
		{			
		    SETFLAG(SERIAL0_BUSY);
			SETFLAG( STX_RECEIVED | START_SERIAL_TIMEOUT );
			SerialTimeoutTimer = MS_TIMER + SERIAL_TIMEOUT_TIME ;
			RESETFLAG( SERIAL_TIMEOUT_OVER | ETX_RECEIVED );
			Uart0Count = 0;
			UART0Buffer[Uart0BufferIndex][Uart0Count] = '\0';
		}
		
		//Check for buffer overflow
		if( Uart0Count == UART_BUFSIZE )
		{
		    Uart0Count = 0;		/* buffer overflow */
			RESETFLAG( STX_RECEIVED | ETX_RECEIVED | START_SERIAL_TIMEOUT | SERIAL_TIMEOUT_OVER );
		}

		//CHeck for index overflow
		if( Uart0BufferIndex == UART_BUF_INDEX )
		{
			Uart0BufferIndex = 0;	
		}
    }   
    IDISABLE;
    VICVectAddr = 0;		/* Acknowledge Interrupt */	
}

/**
 ******************************************************************************
 **
 **	Function Name		: UART1Handler
 **
 **	Description			: UART0 interrupt handler 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void UART0Handler( void ) __irq 
{
    uint8 IIRValue, LSRValue;
	volatile uint8 Dummy;

    IENABLE;				/* handles nested interrupt */	
    IIRValue = U0IIR;
    
    IIRValue >>= 1;			/* skip pending bit in IIR */
    IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
    if( IIRValue == IIR_RLS )		/* Receive Line Status */
    {
		LSRValue = U0LSR;
		/* Receive Line Status */
		if( LSRValue &(LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
		{
		    /* There are errors or break interrupt */
		    /* Read LSR will clear the interrupt */
		    Dummy = LSRValue;
		    Dummy = U0RBR;		/* Dummy read on RX to clear 
						interrupt, then bail out */
		    IDISABLE;
		    VICVectAddr = 0;		/* Acknowledge Interrupt */
		    return;
		}
    }
    else if( IIRValue == IIR_RDA )	/* Receive Data Available */
    {		
//		MeterCmdCount = 0;		

		/* Receive Data Available */
		if( Uart1BufferIndex > UART_BUF_INDEX )
		{
			Uart1BufferIndex = 0;	
		}

	    UART1Buffer[Uart1BufferIndex][Uart1Count] = U0RBR;
	    
	    if( CHECKFLAG( METER_HDR_RECEIVED ) )
		{			
			//if( UART1Buffer[Uart1BufferIndex][Uart1Count] == METER_HEADER )
			//{				
				//UART1Buffer[Uart1BufferIndex][Uart1Count] = METER_ETX;
				//UART1Buffer[Uart1BufferIndex][Uart1Count + 1] = '\0';
				
				if(( UART1Buffer[ Uart1BufferIndex ][ 0 ] == 0xFE ) &&
				    (( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0x99 ) ||
					 ( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0xEE ) ))
				{
					if( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0x99 )					   
					{
						RESETFLAG( METER_NACK_RECEIVED );
						SETFLAG( METER_ACK_RECEIVED );						
					}
					else if( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0xEE )
					{
						RESETFLAG( METER_ACK_RECEIVED );
						SETFLAG( METER_NACK_RECEIVED );
					}
					else if( UART1Buffer[ Uart1BufferIndex ][ 0 ] == 0xFF )
					{
						RESETFLAG( METER_NACK_RECEIVED );
						SETFLAG( METER_ACK_RECEIVED );						
					}
					UART1Buffer[ Uart1BufferIndex ][ 0 ] = '\0';
					UART1Buffer[ Uart1BufferIndex ][ 3 ] = '\0';				  
					Uart1Count = 0;
				}
//				else
//				{
//					Uart1BufferIndex++;
//				}

//				Uart1Count = 0;
//				UART1Buffer[Uart1BufferIndex][Uart1Count] = METER_HEADER;
//				Uart1Count++;
//				UART1Buffer[Uart1BufferIndex][Uart1Count] = METER_ETX;
//				UART1Buffer[Uart1BufferIndex][Uart1Count + 1] = '\0';				
//				MeterTimeoutTimer = MS_TIMER + METER_TIMEOUT;
			//}
			//else
			//{
				Uart1Count++;
				UART1Buffer[Uart1BufferIndex][Uart1Count] = METER_ETX;
				UART1Buffer[Uart1BufferIndex][Uart1Count + 1] = '\0';
				MeterTimeoutTimer = MS_TIMER + METER_TIMEOUT;
			//}
		}
	    else if( UART1Buffer[Uart1BufferIndex][Uart1Count] == METER_HEADER )
		{
			RESETFLAG(METER_STATUS);
			SETFLAG(WELCOME_SCREEN);			
			Uart1Count = 0;
			UART1Buffer[Uart1BufferIndex][Uart1Count] = METER_HEADER;
			Uart1Count++;
			UART1Buffer[Uart1BufferIndex][Uart1Count] = METER_ETX;
			UART1Buffer[Uart1BufferIndex][Uart1Count + 1] = '\0';				
			MeterTimeoutTimer = MS_TIMER + METER_TIMEOUT;
			MeterCheckTimer = MS_TIMER + 600000;
			SETFLAG( METER_HDR_RECEIVED | START_METER_TIMEOUT | SERIAL1_BUSY );				
		}
    	    			
	    if( Uart1Count == UART_BUFSIZE )
	    {
			Uart1Count = 0;		/* buffer overflow */
	    }	
    }
    
    IDISABLE;
    VICVectAddr = 0;		/* Acknowledge Interrupt */
}

/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART0 port, setup pin select,
**				clock, parity, stop bits, FIFO, etc.
**
** parameters:			UART baudrate
** Returned value:		true or false, return false only if the 
**				interrupt handler can't be installed to the 
**				VIC table
** 
*****************************************************************************/
uint64 UART0Init( uint64 baudrate )
{
    uint64 Fdiv;

    PINSEL0 |= 0x00000005;       		/* Enable RxD1 and TxD1, RxD0 and TxD0 	*/

    U0LCR = 0x83;               		/* 8 bits, no Parity, 1 Stop bit    	*/
    Fdiv =( Fpclk / 16 ) / baudrate ;	/*baud rate 							*/
    U0DLM = Fdiv / 256;							
    U0DLL = Fdiv % 256;	
    U0LCR = 0x03;               		/* DLAB = 0                         	*/
    U0FCR = 0x07;						/* Enable and reset TX and RX FIFO. 	*/

    if( install_irq( UART0_INT,(void *)UART0Handler ) == FALSE )
    {
		return(FALSE);
    }
   
    //Initialize Index & Count
    Uart0BufferIndex = 0;
	Uart0Count = 0;
    
    U0IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART0 interrupt */
    return(TRUE);
}

/*****************************************************************************
** Function name:		UART1Init
**
** Descriptions:		Initialize UART0 port, setup pin select,
**				clock, parity, stop bits, FIFO, etc.
**
** parameters:			UART baudrate
** Returned value:		true or false, return false only if the 
**				interrupt handler can't be installed to the 
**				VIC table
** 
*****************************************************************************/
uint64 UART1Init( uint64 baudrate )
{
    uint64 Fdiv;

    PINSEL0 |= 0x00050000;       		/* Enable RxD1 and TxD1, RxD0 and TxD0 	*/

    U1LCR = 0x83;               		/* 8 bits, no Parity, 1 Stop bit    	*/
    Fdiv =( Fpclk / 16 ) / baudrate ;	/*baud rate 							*/
    U1DLM = Fdiv / 256;							
    U1DLL = Fdiv % 256;	
    U1LCR = 0x03;               		/* DLAB = 0                         	*/
    U1FCR = 0x07;						/* Enable and reset TX and RX FIFO. 	*/

    if( install_irq( UART1_INT,(void *)UART1Handler ) == FALSE )
    {
		return(FALSE);
    }

    //Initialize Index & Count
    Uart1BufferIndex = 0;
	Uart1Count = 0;
   
    U1IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART0 interrupt */
    return(TRUE);
}

/*****************************************************************************
** Function name:		UART0SendNow
**
** Descriptions:		Send a block of data to the UART 0 port based
**				on the data length
**
** parameters:			buffer pointer, and data length
** Returned value:		None
** 
*****************************************************************************/
void UART0Send(uint8 *BufferPtr, uint64 Length )
{
    U1IER = IER_THRE | IER_RLS;				/* Disable RBR */
	while( Length != 0 )
    {
		while( !(U1LSR & LSR_THRE) );	/* THRE status, contain valid data */
		U1THR = *BufferPtr;
		BufferPtr++;
		Length--;
    }	
	U1IER = IER_THRE | IER_RLS | IER_RBR;	/* Re-enable RBR */
    return;
}

/*****************************************************************************
** Function name:		UART0SendBuffer
**
** Descriptions:		Send a block of data to the UART 0 port based
**				on the data length
**
** parameters:			buffer pointer, and data length
** Returned value:		None
** 
*****************************************************************************/
void UART0SendBuffer(uint8 *BufferPtr, uint64 Length )
{
	while ( Length != 0 )
    {
		UART0TxBuffer[ UARTTxBufferCount ] = *BufferPtr;
		UARTTxBufferCount++;
		if( UARTTxBufferCount > UART_TX_BUF_INDEX )
		{
			UARTTxBufferCount = 0;
		}
		BufferPtr++;
		Length--;
    }	
    return;
}

/*****************************************************************************
** Function name:		UART1Send
**
** Descriptions:		Send a block of data to the UART 0 port based
**				on the data length
**
** parameters:			buffer pointer, and data length
** Returned value:		None
** 
*****************************************************************************/
void UART1Send(uint8 *BufferPtr, uint64 Length )
{
    U0IER = IER_THRE | IER_RLS;				/* Disable RBR */
	while( Length != 0 )
    {
		while( !(U0LSR & LSR_THRE) );	/* THRE status, contain valid data */		
		U0THR = *BufferPtr;
		BufferPtr++;
		Length--;
    }	
	U0IER = IER_THRE | IER_RLS | IER_RBR;	/* Re-enable RBR */
    return;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
