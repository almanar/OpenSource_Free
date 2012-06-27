/*****************************************************************************
 *   extint.c:  Target C file for Philips LPC214x Family Microprocessors
 *
 *   Copyright(C) 2006, Philips Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2005.10.01  ver 1.00    Prelimnary version, first Release
 *
*****************************************************************************/
#include "LPC214x.H"                        /* LPC21xx definitions */
#include "Types.h"
#include "Irq.h"
#include "Keypad.h"
#include "MiscFunctions.h"
#include "Global.h"
#include "Extint.h"


uint64 eint1_counter;
uint64 eint1_byte_counter;
uint8 eint1_buffer[128];

/*****************************************************************************
** Function name:		EINT1_Handler
**
** Descriptions:		external INT handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void EINT1_Handler (void) __irq 
{
    IENABLE;	
	EXTINT = EINT1;		/* clear interrupt */
	if( CHECKFLAG(MENU_LEVEL_ZERO) )
	{
		if(!(CHECKFLAG(SENTINEL_START)))
		{
			if(!(IOPIN1 & (0x02000000)))
			{
				SETFLAG(SENTINEL_START);				
				eint1_byte_counter = 0;
				eint1_counter = 0;
			}	
		}
		if( CHECKFLAG(SENTINEL_START))	
		{
			if(IOPIN1 & (0x02000000))
			{
				eint1_buffer[eint1_byte_counter] |= 0<<eint1_counter;
			}
			else
			{
				eint1_buffer[eint1_byte_counter] |= 1<<eint1_counter;	
			}
	
			eint1_counter++;
			if( eint1_counter > 4 ) 
			{
				eint1_counter = 0;
				eint1_buffer[eint1_byte_counter] &= 0x0F;
				eint1_buffer[eint1_byte_counter] += 0x30;
				eint1_byte_counter++;
			}
		}
	 }
	IDISABLE;
    VICVectAddr = 0;		/* Acknowledge Interrupt */
}

/*****************************************************************************
** Function name:		EINTInit
**
** Descriptions:		Initialize external interrupt pin and
**				install interrupt handler
**
** parameters:			None
** Returned value:		true or false, return false if the interrupt
**				handler can't be installed to the VIC table.
** 
*****************************************************************************/
uint64 EINTInit( void )
{
    PINSEL0 = 0x000000C0;		/* set P0.3 as EINT1 */
    SCS	= 0;				/* set GPIOx to use regular I/O */
   	IODIR1 &= ~(0x02000000);
	IOSET1 &= ~(0x02000000);
    EXTMODE = EINT1_EDGE;		/* INT1 edge trigger */
    EXTPOLAR = 0;	/* INT1 is falling edge by default */
     
    if ( install_irq( EINT1_INT, (void *)EINT1_Handler ) == FALSE )
    {
		return (FALSE);
    }
        
    return( TRUE );
}

/******************************************************************************
**                            End Of File
******************************************************************************/

