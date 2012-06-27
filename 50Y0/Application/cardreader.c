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
#include "Types.h"
#include "Timer.h"
#include "Global.h"
#include "Lcd.h"
#include "MiscFunctions.h"
#include "Uart.h"
#include "Extint.h"
#include "CardReader.h"

/**
 ******************************************************************************
 **
 **	Function Name		: Process_Card
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
void Process_Card( void )
{
	uint8 temp;
	uint32 localCount;
	static uint32 localCardTimer = 0;

	//Check if interrupt received
	if(CHECKFLAG( SENTINEL_START ))
	{
		if(!CHECKFLAG( START_EXT1_TIMER ))
		{
			SETFLAG( START_EXT1_TIMER );
			localCardTimer = MS_TIMER + 1000;
		}
	}

	//If interrupt received & timeout occured
	if( CHECKFLAG( START_EXT1_TIMER ) && ( localCardTimer < MS_TIMER ))
	{		
		//Check if data received
		if( eint1_buffer[0] == ';' )
		{
			for( localCount=0; localCount<128; localCount++ )
			{
				temp = eint1_buffer[localCount];
				if( temp == '?')	{	break;	}				
			}

			if( localCount>26 && localCount<80 )
			{
				RESETFLAG(SMS_RECEIVED);
				RESETFLAG(MENU_LEVEL_ZERO);
				GetCreditCardAmount( eint1_buffer, localCount+1 );
				SETFLAG(MENU_LEVEL_ZERO);	
			}
			else
			{	
				LCD_Clear();
				LCD_DisplayStringCentered(2,"CARD SWIPE ERROR, PLS TRY AGAIN\0");
				Delay(1000);
				RESETFLAG(SMS_RECEIVED);
				DisplayWelcomeScreen();
			}
		}
		else
		{
			LCD_Clear();
			LCD_DisplayStringCentered(2,"CARD SWIPE ERROR, PLS TRY AGAIN\0");
			Delay(1000);
			RESETFLAG(SMS_RECEIVED);
			DisplayWelcomeScreen();
		}

		for( localCount=0; localCount<128; localCount++ )
		{
			eint1_buffer[localCount] = 0x00;	
		}
		eint1_byte_counter = 0;
		RESETFLAG(SENTINEL_START);
		RESETFLAG(START_EXT1_TIMER);
	}
	return;
}
