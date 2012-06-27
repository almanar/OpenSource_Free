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
 **    File Name:     Spi.c
 **
 **    Description:   This is the Spi comm. module.
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
#include "Target.h"
#include "Irq.h"
#include "Global.h"
#include "Spi.h"

/******************************************************************************
								  << Defines >>
******************************************************************************/
	
/******************************************************************************
 **                     << Private Function Declarations >>
 *****************************************************************************/
static uint8 SPIReceiveByte( void );

/******************************************************************************
 **                         << Function Declarations >>
 *****************************************************************************/
/**
 ******************************************************************************
 **
 **	Function Name		: SpiInit
 **
 **	Description			: Initialization routine for SPI 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: Pin directions Ipnut or Output
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
/*void SPIInit( void )
{
	//Set IO Direction to Output
	SET_PORT0_PIN_DIRN_OUT((SPI_CLK)|(SPI_MOSI)|( SPI_CS2 ));
	SET_PORT0_PIN_DIRN_IN(SPI_MISO);
	SET_PORT1_PIN_DIRN_OUT( SPI_CS );
	//Set all port pins high
	SET_PORT0_PIN_HIGH( SPI_CLK | SPI_MISO | SPI_MOSI | SPI_CS2 );
	SET_PORT1_PIN_HIGH( SPI_CS );
	//Toggle Clock
	SET_PORT0_PIN_HIGH( SPI_CLK );
	SET_PORT0_PIN_LOW( SPI_CLK );
	
	return;
}*/

void SPIInit( void )
{
	uint8 i,dummy;
	/*SSPCR1=0x00;         //SSP master off in normal mode
	PINSEL1=0x000000A8;	 //Configure MOSI,MISO,SCK
	IODIR0=SPI_SEL;		 //Configure P0.20 as SSEL1
	IOSET0=SPI_SEL;		 //Set SSEL1 
	SSPCR0=0x0707;
	SSPCPSR=0x02;
	SSPCR1=0x02; */

	PINSEL0 |= 0x00001500;
	 IODIR0 |= SPI_CS2;
	 IOSET0 |= SPI_CS2;
	 //S0SPCCR = 0;
	 //S0SPCCR = 1;
	 S0SPCCR = 8;
	 //S0SPCCR = 12;
	 //S0SPCCR = 15;
	// S0SPCCR = 30;	 //OK
	// S0SPCCR = 60;
	 S0SPCR  =0x0834;





	for(i=0;i<8;i++)
	{
		dummy=S0SPDR;
	}
	return;
}

/**
 ******************************************************************************
 **
 **	Function Name		: SPISend
 **
 **	Description			: Sends data on SPI. 
 **
 **	Passed Parameters	: *buf, Length 
 **
 **	Modified Data		: Count, Data, i 
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
/*void SPISend( uint8 *buf, uint64 Length )
{
	uint8 Count;
	uint8 Data;
	uint64 i;

	if ( Length == 0 )
		return;
	
	for ( i = 0; i < Length; i++ )
	{
 		Data = *buf;
 		
 		// Loads data MSB First, SPI device lachs data in when low - high transition		 
 		for (Count=0; Count<8; Count++)
		{							
			// Load MSB bit into MOSI GPIO
    		if( Data & 0x80 )	
			{  
				SET_PORT0_PIN_HIGH(SPI_MOSI);
			}
			else 			
			{  
				SET_PORT0_PIN_LOW(SPI_MOSI);	
			}
		
			//SET_PORT0_PIN_HIGH( SPI_CLK );				

			// shift left, so always send MSB
			Data = (uint8)(Data << 1); 
		
			// Makes the clock Pulse so the Device shift data in							
			SET_PORT0_PIN_LOW( SPI_CLK );	// here device latches data in
			SET_PORT0_PIN_HIGH( SPI_CLK );
			
		}
		buf++;
	}
	return;
}*/

void SPISend(uint8 *Buf,uint64 Length)
{
	uint8 Dummy;
	if(Length==0)
	return;
	
	while(Length!=0)
	{
		/*Check Tx is Empty or Not*/
		S0SPDR=*Buf;
		while(!(S0SPSR & 0x80));
	    Dummy = S0SPDR;
		/*{
		Dummy=S0SPSR;
		UART1Send(&Dummy,1);
		}*/
		
		/*Check Rx is Full or Not*/
		/*while(!(S0SPSR & 0x04))
		{
		Dummy=S0SPSR;
		UART1Send(&Dummy,1);
		} */
		/*Flush Rx FIFO*/
		//Dummy=S0SPDR;
		Buf++;		  //Increment Buffer pointer
		Length--;	  //Decrement Buffer Counter
	}
	return;
}

/**
 ******************************************************************************
 **
 **	Function Name		: SPIReceive
 **
 **	Description			: Receives bytes of data over SPI 
 **
 **	Passed Parameters	: *buf, Length 
 **
 **	Modified Data		: *buf, i
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
void SPIReceive( uint8 *buf, uint64 Length )
{
    uint64 i;
	
    for( i = 0; i < Length; i++ )
    {
		*buf = SPIReceiveByte();
		buf++;
    }
    return; 
}

/**
 ******************************************************************************
 **
 **	Function Name		: SPIReceiveByte
 **
 **	Description			: Receives a byte over the SPI
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: Count, Data
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
/*static uint8 SPIReceiveByte( void )
{
	uint8 Count;
 	uint8 Data;
	uint8 lclTemp;
 	
	Data = 0x00;
 	
	IOCLR0 |= SPI_MOSI; // dummy byte

 	for( Count=0; Count<8; Count++ ) 
  	{  		  										
		lclTemp = 0;
		Data = Data<<1;		
		SET_PORT0_PIN_HIGH( SPI_CLK );	
		SET_PORT0_PIN_LOW( SPI_CLK );
  		if( IOPIN0 & SPI_MISO )	{	lclTemp = 1;	}	  	
		

		if( lclTemp == 1 )
		{
			Data = Data | 0x01;	
		}
		
		//SET_PORT0_PIN_LOW( SPI_CLK );	  		
  	}	
  return Data;
}*/

static uint8 SPIReceiveByte(void)
{
	uint8 Byte;
	S0SPDR=0xff;
	
	while(!(S0SPSR & 0x80));

	/*{
	Byte=S0SPSR;
	UART1Send(&Byte,1);
	}*/
	Byte=S0SPDR;
	return Byte;
}

/******************************************************************************
 **                            << End of FILE >>
 *****************************************************************************/


