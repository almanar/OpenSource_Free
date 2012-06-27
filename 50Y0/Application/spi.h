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
 **    File Name:     Spi.h
 **
 **    Description:   This is the Spi comm. header module.
 **
 **    Project Name:  Arya MDT 
 **
 **    Platform:      ARM7TDMI LPC213x
 **
 **    Creation Date: 04/01/2008
 **
 **    Revision History: Please use VSS to get the revision history.
 *****************************************************************************/

#ifndef _SPI_H_
	#define _SPI_H_
	
	// SPI Defines
	#define SPI_CLK		( 1 << 4 ) 
	#define SPI_MISO	( 1 << 5 )
	#define SPI_MOSI 	( 1 << 6 )
	#define SPI_CS 		( 1 << 28 )
	#define SPI_CS2		( 1<< 7 )

	#define SPI_CLK_LOW	 	{	SET_PORT0_PIN_LOW( SPI_CLK );	}
	#define SPI_CLK_HIGH 	{	SET_PORT0_PIN_HIGH( SPI_CLK );	}
	#define SPI_CS_LOW	 	{	SET_PORT1_PIN_LOW( SPI_CS );	}
	#define SPI_CS_HIGH 	{	SET_PORT1_PIN_HIGH( SPI_CS );	}
	#define SPI_CS2_LOW	 	{	SET_PORT0_PIN_LOW( SPI_CS2 );	}
	#define SPI_CS2_HIGH 	{	SET_PORT0_PIN_HIGH( SPI_CS2 );	}

/******************************************************************************
 **                         << Function Declarations >>
 *****************************************************************************/
	void SPIInit( void );
	void SPISend( uint8 *buf, uint64 Length );
	void SPIReceive( uint8 *buf, uint64 Length );
#endif	 
