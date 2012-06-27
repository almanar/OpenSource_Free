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
 **    File Name:     Timer.c
 **
 **    Description:   This is the Timer module.
 **
 **    Project Name:  Arya MDT 
 **
 **    Platform:      ARM7TDMI LPC213x
 **
 **    Creation Date: 04/01/2008
 **
 **    Revision History: Please use VSS to get the revision history.
 *****************************************************************************/
/* LPC21XX Peripheral Registers	*/
#include "LPC214x.h"
#include "Types.h"
#include "Target.h"
#include "Irq.h"
#include "Uart.h"
#include "LCD.h"
#include "Global.h"
#include "WDT.h"
#include "MiscFunctions.h"
#include "Timer.h"
#include "Meter.h"
extern uint8 Status;
uint64 livepulsetimer = 0;
uint64 MS_TIMER = 0;
uint32 power_count=0;
uint32 noise=0;
uint8  dummy_status=0;
/**
 ******************************************************************************
 **
 **	Function Name		: Timer0Handler
 **
 **	Description			: None 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void Timer0Handler( void ) __irq 
{  
    
	uint8 localCount;
	uint8 checksum = 0;
	uint32 localCharCount;
    static uint8 wdtfeed = 0;
	static uint8 pulseflag = 0;

	T0IR = 1;			/* clear interrupt flag */
    IENABLE;			/* handles nested interrupt */

	MS_TIMER++;
	
	if(wdtfeed>100)
	{		
		//WDTFeed();
		if(!CHECKFLAG( STOP_PULSES ))
		{
			//WDTFeed();
			wdtfeed = 0;
			if(pulseflag == 0)
			{
				SET_PORT0_PIN_LOW((1<<23));
				SET_PORT1_PIN_LOW((1<<20));
				pulseflag = 1;
			}
			else
			{
				SET_PORT0_PIN_HIGH((1<<23));		
				SET_PORT1_PIN_HIGH((1<<20));			
				pulseflag = 0;
			}
		}		
	}
	wdtfeed++;

	//LCD Timeout Check
	if(CHECKFLAG1(MONITORSTATUS))
	{
		power_count++;

	    if(power_count<8000)
		{
			if( IOPIN1 & MB_FAIL )
			dummy_status=1;
			else
			dummy_status=0;
			if(dummy_status!=Status)
			noise++;
		}
		else
		{
			if(noise==0)
			SETFLAG1(CURRENTSTATUS);
			
			
			power_count=0;
			noise=0;
			RESETFLAG1(MONITORSTATUS);
			SETFLAG1(POWERSTATUS);
		}
		

	}
	
	if(CHECKFLAG(LCD_ON))
	{
		if(PowerTimeoutTimer<MS_TIMER)
		{
			RESETFLAG(LCD_ON);
			LCD_POWER_OFF;
		}
	}
	
	//Backlit Timeout Check
	
	if(CHECKFLAG(LCD_BACKLIT_ON))
	{
		LCD_BKLIT_ON;		
		if(BacklitTimeoutTimer<MS_TIMER)
		{
			RESETFLAG(LCD_BACKLIT_ON);
			LCD_BKLIT_OFF;			
			LCD_BKLIT_OFF;			
		}
	}

	//UART0 Timeout
	if(CHECKFLAG(START_SERIAL_TIMEOUT))
	{
		if( MS_TIMER > SerialTimeoutTimer )
		{
			RESETFLAG( START_SERIAL_TIMEOUT | SERIAL0_BUSY );
			SETFLAG( SERIAL_TIMEOUT_OVER );
		}
	}

	//UART1 Timeout
	if(CHECKFLAG(START_METER_TIMEOUT))
	{
		if( MS_TIMER > MeterTimeoutTimer )
		{
			if(( UART1Buffer[ Uart1BufferIndex ][ 0 ] == 0xFE ) &&
			   	(( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0x99 ) || 
				 ( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0xEE ) ||
				 ( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0x66 ) || 
				 ( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0x88 )))
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
				else if( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0x66 )
				{
					SendACKToMeter( );
				}
				else if( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0x88 )
				{
					SendACKToMeter( );				
				}
				
				RESETFLAG( METER_HDR_RECEIVED | START_METER_TIMEOUT | SERIAL1_BUSY );
				UART1Buffer[ Uart1BufferIndex ][ 0 ] = '\0';
				UART1Buffer[ Uart1BufferIndex ][ 3 ] = '\0';
				Uart1Count = 0;				  
			}
			else
			{				
				checksum = UART1Buffer[ Uart1BufferIndex ][ 1 ] + UART1Buffer[ Uart1BufferIndex ][ 2 ];
				
				CurrentCheckSum = 0;
				for(localCount=0; localCount<UART1Buffer[ Uart1BufferIndex ][ 2 ]; localCount++)
				{
					checksum += UART1Buffer[ Uart1BufferIndex ][localCount + 3];
					CurrentCheckSum += UART1Buffer[ Uart1BufferIndex ][localCount + 3];
				}								
				
				if( checksum == UART1Buffer[ Uart1BufferIndex ][2 + 1 + localCount])					
				{										
					if( CurrentCheckSum != LastCheckSum )
					{
						LastCheckSum = CurrentCheckSum;
						if( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0xAB )
						{
							MeterHired = TRUE;
							MeterAddkeys[ 0 ] = 1;								
						}
						else if( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0xBA ||UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0x1D )
						{
							if( UART1Buffer[ Uart1BufferIndex ][ 3 ] == 0xBA )
							{
								FARE = 0x00000000;
								FARE = 	( uint32 )( UART1Buffer[ Uart1BufferIndex ][ 29 ] << 16	) |
								 		( uint32 )( UART1Buffer[ Uart1BufferIndex ][ 30 ] << 8	) |	
										( uint32 )( UART1Buffer[ Uart1BufferIndex ][ 31 ] << 0	);
							}
							MeterAddkeys[ 0 ] = 1;								
							MeterHired = FALSE;	
							KerbDuty = FALSE;
							CtrlDuty = FALSE;
						}
	
						SETFLAG( SEND_ACK_TO_METER );
						Uart1BufferIndex++;

						//COPY SECOND TIME & INCREMENT UART INDEX
						for(localCharCount=0; localCharCount<UART_BUFSIZE; localCharCount++)
						{						
							UART1Buffer[Uart1BufferIndex][localCharCount] = UART1Buffer[Uart1BufferIndex-1][localCharCount];
						}
						Uart1BufferIndex++;
						
						
						if( Uart1BufferIndex > ( UART_BUF_INDEX-2 ) )
		    			{
		    				for(localCount=0; localCount<(UART_BUF_INDEX - 2); localCount++)
							{
								for(localCharCount=0; localCharCount<UART_BUFSIZE; localCharCount++)
								{						
									UART1Buffer[localCount][localCharCount] = UART1Buffer[localCount+1][localCharCount];
								}
							}
							UART1Buffer[localCount][0] = '\0';
							Uart1BufferIndex = Uart1BufferIndex - 1;
						}
					}
					else
					{
						SETFLAG( SEND_ACK_TO_METER );
						UART1Buffer[Uart1BufferIndex][0] = '\0';
					}
				}
				else
				{
					SETFLAG( SEND_NACK_TO_METER );
					UART1Buffer[Uart1BufferIndex][0] = '\0';
				}
				
				RESETFLAG( METER_HDR_RECEIVED | START_METER_TIMEOUT | SERIAL1_BUSY );
				Uart1Count = 0;
			}							
		}	
	}
	
    IDISABLE;
    VICVectAddr = 0;    /* Acknowledge Interrupt */
}

/**
 ******************************************************************************
 **
 **	Function Name		: Timer1Handler
 **
 **	Description			: None 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void Timer1Handler( void ) __irq 
{  
    T1IR = 1;			/* clear interrupt flag */
    IENABLE;			/* handles nested interrupt */

    IDISABLE;
    VICVectAddr = 0;		/* Acknowledge Interrupt */
}

/**
 ******************************************************************************
 **
 **	Function Name		: enable_timer
 **
 **	Description			: None 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void enable_timer( uint8 timer_num )
{
    if ( timer_num == 0 )
    {
		T0TCR = 1;
    }
    else
    {
		T1TCR = 1;
    }
    return;
}

/**
 ******************************************************************************
 **
 **	Function Name		: disable_timer
 **
 **	Description			: None 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void disable_timer( uint8 timer_num )
{
    if ( timer_num == 0 )
    {
		T0TCR = 0;
    }
    else
    {
		T1TCR = 0;
    }
    return;
}

/**
 ******************************************************************************
 **
 **	Function Name		: reset_timer
 **
 **	Description			: None 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void reset_timer( uint8 timer_num )
{
    uint64 regVal;

    if ( timer_num == 0 )
    {
		regVal = T0TCR;
		regVal |= 0x02;
		T0TCR = regVal;
    }
    else
    {
		regVal = T1TCR;
		regVal |= 0x02;
		T1TCR = regVal;
    }
    return;
}

/**
 ******************************************************************************
 **
 **	Function Name		: init_timer0
 **
 **	Description			: None 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint8 init_timer0( void ) 
{
    T0MR0 = INTERVAL_1MS;	/* 10mSec = 150.000-1 counts */
    T0MCR = 3;			/* Interrupt and Reset on MR0 */
    if ( install_irq( TIMER0_INT, (void *)Timer0Handler ) == FALSE )
    {
		return (FALSE);
    }
    else
    {
		return (TRUE);
    }
}

/**
 ******************************************************************************
 **
 **	Function Name		: init_timer1
 **
 **	Description			: None 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint8 init_timer1( void ) 
{
    T1MR0 = INTERVAL_1MS;	/* 10mSec = 150.000-1 counts */
    T1MCR = 3;			/* Interrupt and Reset on MR0 */
    if ( install_irq( TIMER1_INT, (void *)Timer1Handler ) == FALSE )
    {
		return (FALSE);
    }
    else
    {
		return (TRUE);
    }
}

/**
 ******************************************************************************
 **
 **	Function Name		: Delay
 **
 **	Description			: None 
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void Delay( uint64 Delaytime )
{
	uint64 Delay;
	
	Delay = (uint64)(Delaytime + MS_TIMER);
	while( MS_TIMER < Delay )
	{
		;
	}
	
	return;
}
