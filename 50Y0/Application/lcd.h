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
								  << Defines >>
******************************************************************************/
#ifndef _LCD_H_
#define _LCD_H_

	// LCD Defines
    #define LCD_RS_PIN		( 1 << 22 )
	#define LCD_RW_PIN		( 1 << 21 )
	#define LCD_EN1_PIN		( 1 << 14 )
	#define LCD_EN2_PIN		( 1 << 19 )
	#define LCD_DATA		( 1 << 29 )
	#define LCD_CLK			( 1 << 20 )
	#define LCD_BKLIT		( 1 << 18 )
	#define LCD_PWR			( 1 << 19 )
	#define LCD_FS			( 1 << 27 ) 

	#define LCD_RS_LOW  	SET_PORT0_PIN_HIGH( LCD_RS_PIN )
	#define LCD_RS_HIGH  	SET_PORT0_PIN_LOW( LCD_RS_PIN )	
	#define LCD_RW_LOW		SET_PORT0_PIN_HIGH( LCD_RW_PIN )
	#define LCD_RW_HIGH		SET_PORT0_PIN_LOW( LCD_RW_PIN )
	#define LCD_EN1_LOW		SET_PORT0_PIN_HIGH( LCD_EN1_PIN )
	#define LCD_EN1_HIGH	SET_PORT0_PIN_LOW( LCD_EN1_PIN )
	#define LCD_EN2_LOW		SET_PORT0_PIN_HIGH( LCD_EN2_PIN )
	#define LCD_EN2_HIGH	SET_PORT0_PIN_LOW( LCD_EN2_PIN )
	#define LCD_CLK_LOW		SET_PORT0_PIN_LOW( LCD_CLK )
	#define LCD_CLK_HIGH	SET_PORT0_PIN_HIGH( LCD_CLK )
	#define LCD_POWER_ON	SET_PORT1_PIN_HIGH( LCD_PWR )
	#define LCD_POWER_OFF	SET_PORT1_PIN_LOW( LCD_PWR )	
	#define LCD_BKLIT_ON	SET_PORT1_PIN_HIGH( LCD_BKLIT )
	#define LCD_BKLIT_OFF	SET_PORT1_PIN_LOW( LCD_BKLIT )


/******************************************************************************
 **                         << Function Declarations >>
 *****************************************************************************/
	void LCD_Init( void);
	void LCD_DefineChar( uint8 address, const uint8 *pattern );
	void LCD_DisplayOff( void );
	void LCD_DisplayOn( void );
	void LCD_Clear( void );
	void LCD_Home( void );
	void LCD_Cursor( uint8 row, uint8 column );
	void LCD_CursorLeft( void );
	void LCD_CursorRight( void );
	void LCD_CursorOn( void );
	void LCD_CursorOff( void );
	void LCD_DisplayCharacter( uint8 line, uint8 a_char );
	void LCD_DisplayString( uint8 row, uint8 column, uint8 *string );
	void LCD_DisplayStringCentered( uint8 row, uint8 *string );
	void LCD_DisplayScreen( uint8 *ptr );
	void LCD_DisplayRow( uint8 row, uint8 *string );
	void LCD_WipeOnLR( uint8 *ptr );
	void LCD_WipeOnRL( uint8 *ptr );
	void LCD_WipeOffLR( void );
	void LCD_WipeOffRL( void );
#endif
