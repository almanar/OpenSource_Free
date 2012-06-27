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
 **    File Name:     Lcd.c
 **
 **    Description:   This is the LCD module.
 **
 **    Project Name:  Arya MDT 
 **
 **    Platform:      ARM7TDMI LPC213x
 **
 **    Creation Date: 04/01/2008
 **
 **    Revision History: Please use VSS to get the revision history.
 *****************************************************************************/

/******************************************************************************
 **                           << Include Files >>
 *****************************************************************************/
#include "LPC214x.h"
#include "Types.h"
#include "Global.h"
#include "Timer.h"
#include "Lcd.h"
#include <string.h>
/******************************************************************************
 **                         << Local Definitions >>
 *****************************************************************************/
const uint8 RIGHT_ARROW[] = {
	0x0F,   // ---01111
	0x07,   // ---00111
	0x03,   // ---00011
	0x01,   // ---00001
	0x03,   // ---00011
	0x07,   // ---00111
	0x0F,   // ---01111
	0x1F    // ---11111
};

const uint8 LEFT_ARROW[] = {
	0x1E,   // ---11110
	0x1C,   // ---11100
	0x18,   // ---11000
	0x10,   // ---10000
	0x18,   // ---11000
	0x1C,   // ---11100
	0x1E,   // ---11110
	0x1F    // ---11111
};

const uint8 LCD_CHAR_BAR3[] = {
	0x1c,   // ---11100
	0x1c,   // ---11100
	0x1c,   // ---11100
	0x1c,   // ---11100
	0x1c,   // ---11100
	0x1c,   // ---11100
	0x1c,   // ---11100
	0x1c    // ---11100
};

const uint8 LCD_CHAR_BAR4[] = {
	0x1e,   // ---11110
	0x1e,   // ---11110
	0x1e,   // ---11110
	0x1e,   // ---11110
	0x1e,   // ---11110
	0x1e,   // ---11110
	0x1e,   // ---11110
	0x1e    // ---11110
};

const uint8 LCD_CHAR_BAR5[] = {
	0x1f,   // ---11111
	0x1f,   // ---11111
	0x1f,   // ---11111
	0x1f,   // ---11111
	0x1f,   // ---11111
	0x1f,   // ---11111
	0x1f,   // ---11111
	0x1f    // ---11111
};

const uint8 LCD_CHAR_UP_ARROW[] = {
	0x1f,		// ---11111
	0x1b,		// ---11011
	0x11,		// ---10001
	0x0a,		// ---01010
	0x1b,		// ---11011
	0x1b,		// ---11011
	0x1b,		// ---11011
	0x1f 		// ---11111
};

const uint8 LCD_CHAR_DOWN_ARROW[] = {
	0x1f,		// ---11111
	0x1b,		// ---11011
	0x1b,		// ---11011
	0x1b,		// ---11011
	0x0a,		// ---01010
	0x11,		// ---10001
	0x1b,		// ---11011
	0x1f 		// ---11111
};

const uint8 LCD_CHAR_TRADEMARK_T[] = {
	0x1f,		// ---11111
	0x04,		// ---00100
	0x04,		// ---00100
	0x04,		// ---00100
	0x00,		// ---00000
	0x00,		// ---00000
	0x00,		// ---00000
	0x00 		// ---00000
};

const uint8 LCD_CHAR_TRADEMARK_M[] = {
	0x11,		// ---10001
	0x1b,		// ---11011
	0x15,		// ---10101
	0x11,		// ---10001
	0x00,		// ---00000
	0x00,		// ---00000
	0x00,		// ---00000
	0x00 		// ---00000
};

 /******************************************************************************
 **                          << Global Variables >>
 *****************************************************************************/
/******************************************************************************
 **                          << Static Variables >>
 *****************************************************************************/
/******************************************************************************
 **                    << Private Function Declarations >>
 *****************************************************************************/                                                         
static void LCD_InitDriver(void);
static void LCD_WriteControl( uint8 line, uint8 data );
static void LCD_WriteData( uint8 line, uint8 data );

/******************************************************************************
 **                         << Function Definitions >>
 *****************************************************************************/
/**
 ******************************************************************************
 **
 **	Function Name		: LCD_Init
 **
 **	Description			: Initializes the LCD.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_Init(void)
{
	//Set LCD Port Pins as GPIO
	PINSEL0 &= ~(0x30000000);	//SET Port0 LCD Pins as GPIO
	PINSEL1 &= ~(0x00003FC0);	//SET Port0 LCD Pins as GPIO
	PINSEL2 &= ~(0x0000000C);	//SET Port Pins P1.16 - P1.31 as GPIO

	SET_PORT0_PIN_DIRN_OUT( LCD_RS_PIN | LCD_RW_PIN | LCD_EN1_PIN | LCD_EN2_PIN | LCD_CLK );
	SET_PORT1_PIN_DIRN_OUT( LCD_DATA | LCD_BKLIT | LCD_PWR | LCD_FS );
	LCD_POWER_ON;	
	LCD_BKLIT_ON;
	
	LCD_InitDriver();
	LCD_Clear();
	LCD_CursorOff();

	// load user-defined characters into LCD
	LCD_DefineChar(0, RIGHT_ARROW);
	LCD_DefineChar(1, LEFT_ARROW);
	LCD_DefineChar(2, LCD_CHAR_TRADEMARK_T);
	LCD_DefineChar(3, LCD_CHAR_TRADEMARK_M);
	LCD_DefineChar(4, RIGHT_ARROW);
	LCD_DefineChar(5, LEFT_ARROW);
	LCD_DefineChar(6, LCD_CHAR_BAR3);
	LCD_DefineChar(7, LCD_CHAR_BAR4);
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_Clear
 **
 **	Description			: Clear the LCD screen(also homes cursor).
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_Clear(void)
{
   LCD_WriteControl( 1, 0x01 );
   LCD_WriteControl( 2, 0x01 );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_Home
 **
 **	Description			: Position the LCD cursor at row 1, col 1.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_Home( void )
{
	LCD_WriteControl( 1, 0x02 );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_DisplayCharacter
 **
 **	Description			: Display a single character, 
 **						  at the current cursor location.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_DisplayCharacter( uint8 line, uint8 a_char )
{
	switch( line )
	{
		case 1:
		case 2:
			LCD_WriteData( 1, a_char );
		break;

		case 3:
		case 4:
			LCD_WriteData( 2, a_char );
		break;
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_DisplayString
 **
 **	Description			:  Display a string at the specified row and column.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_DisplayString( uint8 row, uint8 column, uint8 *string )
{
	uint8 Count = 0;
	LCD_Cursor( row, column );
	while( *string && Count<40 )
	{
		LCD_DisplayCharacter( row, *string++ );
		Count++;
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_DisplayStringCentered
 **
 **	Description			: Display a string centered on the specified row.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_DisplayStringCentered( uint8 row, uint8 *string )
{
	uint8 len = strlen( (char *)string );

	if( len <= 40 ) {
		// if the string is less than one line, center it ...
		uint8 i;
		LCD_Cursor( row, 1 );
		for( i=0; i<40; i++ )
			LCD_DisplayCharacter( row, ' ' );
		
		LCD_DisplayString( row,( ( 40 - len ) / 2 )+1,string );
	}
	else {
		// if the string is more than one line, display first 20 characters
		uint8 temp[41];
		strncpy((char *)temp, (char *)string, 40 );
		temp[40] = 0;
		LCD_DisplayString( row,1,temp );
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_Cursor
 **
 **	Description			: Position the LCD cursor at "row", "column".
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_Cursor( uint8 row, uint8 column )
{
	switch( row ) {
		case 1: LCD_WriteControl( 1, 0x80 + column - 1 ); break;
		case 2: LCD_WriteControl( 1, 0xc0 + column - 1 ); break;
		case 3: LCD_WriteControl( 2, 0x80 + column - 1 ); break;
		case 4: LCD_WriteControl( 2, 0xc0 + column - 1 ); break;
		default: break;
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_DisplayScreen
 **
 **	Description			: Display a string on the entire screen.
 **
 **	Passed Parameters	: ptr = pointer to a string containing the entire screen
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_DisplayScreen( uint8 *ptr )
{
	LCD_DisplayRow( 1,ptr+ 0 );
	LCD_DisplayRow( 2,ptr+40 );
	LCD_DisplayRow( 3,ptr+80 );
	LCD_DisplayRow( 4,ptr+120 );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_WipeOnLR
 **
 **	Description			: Display an entire screen by
 **			              wiping it on( left to right ).
 **
 **	Passed Parameters	: ptr = pointer to a string containing the entire screen
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_WipeOnLR( uint8 *ptr )
{
	// "wipe" on new screen
	uint8 i;
	for( i=0; i<40; i++ ) {
		LCD_Cursor( 1,i+1 );		LCD_DisplayCharacter( 1, *( ptr+ 0+i ) );
		LCD_Cursor( 2,i+1 );		LCD_DisplayCharacter( 1, *( ptr+40+i ) );
		LCD_Cursor( 3,i+1 );		LCD_DisplayCharacter( 2, *( ptr+80+i ) );
		LCD_Cursor( 4,i+1 );		LCD_DisplayCharacter( 2, *( ptr+120+i ) );
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_WipeOnLR
 **
 **	Description			: Display an entire screen by
 **			              wiping it on( right to left ).
 **
 **	Passed Parameters	: ptr = pointer to a string containing the entire screen
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_WipeOnRL( uint8 *ptr )
{
	// "wipe" on new screen
	uint8 i;
	for( i=20; i>0; i-- ) {
		LCD_Cursor( 1,i );		LCD_DisplayCharacter( 1, *( ptr+ 0+i-1 ) );
		LCD_Cursor( 2,i );		LCD_DisplayCharacter( 1, *( ptr+40+i-1 ) );
		LCD_Cursor( 3,i );		LCD_DisplayCharacter( 2, *( ptr+80+i-1 ) );
		LCD_Cursor( 4,i );		LCD_DisplayCharacter( 2, *( ptr+120+i-1 ) );
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_WipeOffLR
 **
 **	Description			: "Wipe" screen left-to-right.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_WipeOffLR( void )
{
	// "wipe" off old screen( left to right )
	uint8 i;
	for( i=1; i<41; i++ ) {
		#define BLOCK 0xff
		LCD_Cursor( 1,i );		LCD_DisplayCharacter( 1, BLOCK );
		LCD_Cursor( 2,i );		LCD_DisplayCharacter( 1, BLOCK );
		LCD_Cursor( 3,i );		LCD_DisplayCharacter( 2, BLOCK );
		LCD_Cursor( 4,i );		LCD_DisplayCharacter( 2, BLOCK );
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_WipeOffRL
 **
 **	Description			: "Wipe" screen lright-to-left.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_WipeOffRL( void )
{
	uint8 i;
	for( i=40; i>0; i-- ) {
		#define BLOCK 0xff
		LCD_Cursor( 1,i );		LCD_DisplayCharacter( 1, BLOCK );
		LCD_Cursor( 2,i );		LCD_DisplayCharacter( 1, BLOCK );
		LCD_Cursor( 3,i );		LCD_DisplayCharacter( 2, BLOCK );
		LCD_Cursor( 4,i );		LCD_DisplayCharacter( 2, BLOCK );
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_DisplayRow
 **
 **	Description			: Display a string at the specified row.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_DisplayRow( uint8 row, uint8 *string )
{
	uint8 i;
	LCD_Cursor( row, 1 );
	for( i=0; i<40; i++ )		
		if( *string != '\0')
			LCD_DisplayCharacter( row, *string++ );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_CursorLeft
 **
 **	Description			: Move the cursor left by one character.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_CursorLeft( void )
{
	LCD_WriteControl( 1, 0x10 );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_CursorRight
 **
 **	Description			: Move the cursor right by one character.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_CursorRight( void )
{
	LCD_WriteControl( 1, 0x14 );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_CursorOn
 **
 **	Description			: Turns the cursor on.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_CursorOn( void )
{
	LCD_WriteControl( 1, 0x0d );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_CursorOff
 **
 **	Description			: Turns the cursor off.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_CursorOff( void )
{
	LCD_WriteControl( 1, 0x0c );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_DisplayOff
 **
 **	Description			: Turn Off LCD.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_DisplayOff( void )
{
	LCD_WriteControl( 1, 0x08 );
	LCD_WriteControl( 2, 0x08 );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_DisplayOn
 **
 **	Description			: Turn On LCD.
 **
 **	Passed Parameters	: None
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_DisplayOn( void )
{
	LCD_WriteControl( 1, 0x0c );
	LCD_WriteControl( 2, 0x0c );
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_DefineCharacter
 **
 **	Description			: Define dot pattern for user-defined character.
 **
 **	Passed Parameters	: address = address of character( 0x00-0x07 )
 **						  pattern = pointer to 8-byte array containing 
 **                                 the dot pattern
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void LCD_DefineChar( uint8 address, const uint8 *pattern )
{
	uint8 i;

	LCD_WriteControl( 1, 0x40 +( address << 3 ) );
	LCD_WriteControl( 2, 0x40 +( address << 3 ) );
	for( i=0; i<8; i++ ) {
		LCD_WriteData( 1, *pattern++ );
		LCD_WriteData( 2, *pattern++ );
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_InitDriver
 **
 **	Description			: Initialize the LCD driver.
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
static void LCD_InitDriver( void )
{
	LCD_WriteControl( 1, 0x38 );	Delay(10);
	LCD_WriteControl( 1, 0x38 );	Delay(10);
	LCD_WriteControl( 1, 0x38 );	Delay(10);
	LCD_WriteControl( 1, 0x06 );	Delay(10);
	LCD_WriteControl( 1, 0x0c );	Delay(10);

	LCD_WriteControl( 2, 0x38 );	Delay(10);
	LCD_WriteControl( 2, 0x38 );	Delay(10);
	LCD_WriteControl( 2, 0x38 );	Delay(10);
	LCD_WriteControl( 2, 0x06 );	Delay(10);
	LCD_WriteControl( 2, 0x0c );	Delay(10);
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_WriteControl
 **
 **	Description			: Write a control instruction to the LCD
 **
 **	Passed Parameters	: data 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
static void LCD_WriteControl( uint8 line, uint8 data )
{
	uint8 localCount;
	uint8 com;

	LCD_BKLIT_ON;
	SETFLAG(LCD_BACKLIT_ON);
	BacklitTimeoutTimer = MS_TIMER + 30000;

	com = data;
	
	// RS=0, R/W=0, E=0
	LCD_RS_LOW;		LCD_RW_LOW;		
	
	if( line == 1 )	{		LCD_EN1_HIGH;		}
	else			{		LCD_EN2_HIGH;		}
	for( localCount=0; localCount<8; localCount++ )
	{
		if( com & 0x80 )
		{
			SET_PORT1_PIN_HIGH( LCD_DATA );
			SET_PORT1_PIN_HIGH( LCD_DATA );
			SET_PORT1_PIN_HIGH( LCD_DATA );
			SET_PORT1_PIN_HIGH( LCD_DATA );
		}
		else
		{
			SET_PORT1_PIN_LOW( LCD_DATA );
			SET_PORT1_PIN_LOW( LCD_DATA );
			SET_PORT1_PIN_LOW( LCD_DATA );
			SET_PORT1_PIN_LOW( LCD_DATA );
		}	
		
		LCD_CLK_LOW;	LCD_CLK_HIGH;
		com = com << 1;
	}
		
	// RS=0, R/W=0, E=1
	LCD_RS_LOW;	LCD_RW_LOW;
	if( line == 1 )	{		LCD_EN1_HIGH;		}
	else			{		LCD_EN2_HIGH;		}	
	Delay(5);
	// RS=0, R/W=0, E=0
	LCD_RS_LOW;	LCD_RW_LOW;
	if( line == 1 )	{		LCD_EN1_LOW;		}
	else			{		LCD_EN2_LOW;		}
	Delay(5);
	// RS=0, R/W=1, E=0*/
	LCD_RS_LOW;	LCD_RW_HIGH;
	if( line == 1)	{		LCD_EN1_LOW;		}
	else			{		LCD_EN2_LOW;		}
	Delay(5);
}

/**
 ******************************************************************************
 **
 **	Function Name		: LCD_WriteData
 **
 **	Description			: Write one byte of data to the LCD
 **
 **	Passed Parameters	: data 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
static void LCD_WriteData( uint8 line, uint8 data )
{
	uint8 localCount;
	uint8 lcddata;

	lcddata = data;
	// RS=1, R/W=0, E=0
	LCD_RS_HIGH;	LCD_RW_LOW;		
	if( line == 1 )	{		LCD_EN1_HIGH;		}
	else			{		LCD_EN2_HIGH;		}
	for( localCount=0; localCount<8; localCount++ )
	{
		if( lcddata & 0x80 )
		{
			SET_PORT1_PIN_HIGH( LCD_DATA );
		}
		else
		{
			SET_PORT1_PIN_LOW( LCD_DATA );
		}	
		LCD_CLK_HIGH;	LCD_CLK_LOW;	LCD_CLK_HIGH;
		lcddata = lcddata << 1;
	}

	// RS=1, R/W=0, E=1
	LCD_RS_HIGH;	LCD_RW_LOW;		
	if( line == 1 )	{		LCD_EN1_HIGH;		}
	else			{		LCD_EN2_HIGH;		}
	Delay(1);
	// RS=1, R/W=0, E=0	
	LCD_RS_HIGH;	LCD_RW_LOW;
	if( line == 1 )	{		LCD_EN1_LOW;		}
	else			{		LCD_EN2_LOW;		}
	Delay(1);
	// RS=1, R/W=1, E=0
	LCD_RS_HIGH;	LCD_RW_HIGH;
	if( line == 1 )	{		LCD_EN1_LOW;		}
	else			{		LCD_EN2_LOW;		}
	Delay(1);
}
