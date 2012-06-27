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
 **    File Name:     Dataflash.c
 **
 **    Description:   This is the Dataflash memory module.
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
#include "Spi.h"
#include "Global.h"
#include "DataFlash.h"

static uint8 DFRxBuffer[ DF_BUFFERSIZE ];

uint8 DF2Buffer[ DF_BUFFERSIZE ];
uint8 DF2TempBuffer[ 4096 ];
volatile uint8 GBLDataflashType = SST25VF016B;

/******************************************************************************
 **                         << Function Declarations >>
 *****************************************************************************/
/**
 ******************************************************************************
 **
 **	Function Name		: DF_ReadPage
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */ 
uint32 DF_ReadPage( uint8 *Buffer, uint32 PageNo )
{

	if ( DF_ReadPageToBuffer( PageNo ) == SUCCESS )
	{
		if ( DF_ReadDataFromBuffer ( Buffer ) == SUCCESS )		
			return SUCCESS;
		else
			return FAILED;
	}
	else
	{
		return FAILED;
	}

}

/**
 ******************************************************************************
 **
 **	Function Name		: DF_ReadPageToBuffer
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_ReadPageToBuffer( uint32 PageNo )
{
	uint32 localPageNo;	
	uint8 localCmdBuffer[ 4 ];	
	uint32 localAddress;
		
	if( GBLDataflashType == DF_AT45DB161 )
	{
		localPageNo = PageNo << DF_PAGEADDBITSHIFT; 	//13 bit Page Address
		
		localCmdBuffer[ 0 ] = ( uint8 )( MAIN_MEMORY_PAGE_TO_BUFFER_1_TRANSFER );
		localCmdBuffer[ 1 ] = ( uint8 )( localPageNo >> 8 );	//Read Page into Buffer
		localCmdBuffer[ 2 ] = ( uint8 )( localPageNo >> 0 );
		localCmdBuffer[ 3 ] = ( uint8 )( 0x00 );
		
		SPI_CS_HIGH;	SPI_CS_LOW;
		SPISend( localCmdBuffer, 4 );
		SPI_CS_HIGH;
		while (DF_CheckIfBusy());
	}
	else
	{
		localAddress = PageNo * DF2_BYTESPERPAGE;
		IOCLR0 |= SPI_CS2;
		localCmdBuffer[ 0 ] = DF_READ;
		localCmdBuffer[ 1 ] = ( uint8 )(( localAddress & 0xFFFFFF ) >> 16 );
		localCmdBuffer[ 2 ] = ( uint8 )(( localAddress & 0xFFFF ) >> 8 );
		localCmdBuffer[ 3 ]	= ( uint8 )( localAddress & 0xFF );		
		SPISend( localCmdBuffer, 4 );
		SPIReceive( &DF2Buffer[ 0 ], DF_BYTESPERPAGE );							
		IOSET0 |= SPI_CS2;
		while (DF_CheckIfBusy());
	}				
	return SUCCESS; 
}

/**
 ******************************************************************************
 **
 **	Function Name		: DF_ReadDataFromBuffer
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_ReadDataFromBuffer( uint8 *Buffer )
{	
	uint8 localCmdBuffer[ 4 ];
	uint32 lclCount;
	
	if( GBLDataflashType == DF_AT45DB161 )
	{
		localCmdBuffer[ 0 ] = ( uint8 )( BUFFER_1_READ_LOW_FREQUENCY );		
		localCmdBuffer[ 1 ] = ( uint8 )( 0x00 );	
		localCmdBuffer[ 2 ] = ( uint8 )( 0x00 );
		localCmdBuffer[ 3 ] = ( uint8 )( 0x00 );	
		
		SPI_CS_HIGH;	SPI_CS_LOW;    
		SPISend( localCmdBuffer, 4 );
		SPIReceive( Buffer, DF_BUFFERSIZE );	
		SPI_CS_HIGH;
		
		while (DF_CheckIfBusy());
	}
	else
	{
		for( lclCount=0; lclCount<DF_BYTESPERPAGE; lclCount++ )
		{
			*( Buffer + lclCount ) = DF2Buffer[ lclCount ];
		} 
	}
	return SUCCESS;
}

/**
 ******************************************************************************
 **
 **	Function Name		: DF_ReadDataBytesFromBuffer
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_ReadDataBytesFromBuffer( uint8 *Buffer, uint32 Offset, uint32 NoOfBytes )
{                                                                         
	uint32 localOffset;                                                     
	uint8 localCmdBuffer[ 4 ];  
	uint32 lclCount;	

	localOffset = Offset;                                                 
    
	if( GBLDataflashType == DF_AT45DB161 )
	{                                                                      
		if ( (Offset + NoOfBytes )  <= DF_BUFFERSIZE )                                 
		{                                                                     
			localCmdBuffer[ 0 ] = ( uint8 )( BUFFER_1_READ_LOW_FREQUENCY );                     
	 		localCmdBuffer[ 1 ] = ( uint8 )( 0x00 );
			localCmdBuffer[ 2 ] = ( uint8 )( localOffset >> 8 );                       
			localCmdBuffer[ 3 ] = ( uint8 )( localOffset >> 0 );                      
		                                                                      
			SPI_CS_HIGH;	SPI_CS_LOW;                                             
	    	SPISend( localCmdBuffer, 4 );                                        
			SPIReceive( Buffer, NoOfBytes );                                 
			SPI_CS_HIGH;               
			                                         
			while (DF_CheckIfBusy());                                            
			return SUCCESS;                                                   
		}                                                                     
		else                                                                  
		{                                                                     
			return FAILED;                                                    
		}
	}
	else
	{
		if ((Offset + NoOfBytes )  <= DF_BUFFERSIZE ) 
		{
			for( lclCount=0; lclCount<NoOfBytes; lclCount++ )
			{
				*( Buffer + lclCount ) = DF2Buffer[ Offset + lclCount ];
			}
			return SUCCESS; 	
		}
		else
		{
			return FAILED;
		}
	}                                                                     
}                                                                         

/**
 ******************************************************************************
 **
 **	Function Name		: DF_ReadBytesFromPage
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_ReadBytesFromPage( uint32 PageNo, uint8 *Buffer, uint32 ByteOffset, uint32 NoOfBytes )
{                                                                                    
	uint32 localPageNo;                                                                
	uint8 localCmdBuffer[ 4 ];
	uint32 localAddress;
	
	if( GBLDataflashType == DF_AT45DB161 )
	{
		localPageNo = PageNo << DF_PAGEADDBITSHIFT; 	//12 bit Page Address                                
		                                                                                 
		localCmdBuffer[ 0 ] = ( uint8 )( CONTINUOUS_ARRAY_READ_LOW_FREQUENCY );                      
		localCmdBuffer[ 1 ] = ( uint8 )( localPageNo >> 8 );	//Read Page into Buffer              
		localCmdBuffer[ 2 ] = ( uint8 )( ( uint8 )( localPageNo >> 0 )|( uint8 )( ByteOffset >> 8 ));           
		localCmdBuffer[ 3 ] = ( uint8 )( ByteOffset >> 0 );                                          
	                                                                                     
		SPI_CS_HIGH;	SPI_CS_LOW;                                                            
		SPISend( localCmdBuffer, 4);                                                       
		SPIReceive( Buffer, NoOfBytes );                                                
		SPI_CS_HIGH;	                                                                     
		while (DF_CheckIfBusy());		
	}
	else
	{
		localAddress = ( PageNo * DF2_BYTESPERPAGE ) + ByteOffset;
		localCmdBuffer[ 0 ] = DF_READ;
		localCmdBuffer[ 1 ] = ( uint8 )(( localAddress & 0xFFFFFF ) >> 16 );
		localCmdBuffer[ 2 ] = ( uint8 )(( localAddress & 0xFFFF ) >> 8 );
		localCmdBuffer[ 3 ]	= ( uint8 )( localAddress & 0xFF );
		IOCLR0 |= SPI_CS2;
		SPISend( localCmdBuffer, 4 );
		SPIReceive( Buffer, NoOfBytes );	
		IOSET0 |= SPI_CS2;
		while (DF_CheckIfBusy());	
	}                                                          
                                                                                     
	return SUCCESS;                                                                  
}

/**
 ******************************************************************************
 **
 **	Function Name		: DF_WriteDataIntoBuffer
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_WriteDataIntoBuffer(  uint8 *Buffer )                                 
{	                                                                      	
	uint8 localCmdBuffer[ 4 ];
	uint32 lclCount; 
	
	if( GBLDataflashType == DF_AT45DB161 )
	{
		localCmdBuffer[ 0 ] = ( uint8 )( BUFFER_2_WRITE );                                      
		localCmdBuffer[ 1 ] = ( uint8 )( 0x00 );	                                              
		localCmdBuffer[ 2 ] = ( uint8 )( 0x00 );                                                
		localCmdBuffer[ 3 ] = ( uint8 )( 0x00 );	                                              
		                                                                      
		SPI_CS_HIGH;	SPI_CS_LOW;                                                 
		SPISend( localCmdBuffer, 4 );                                            
		SPISend( Buffer, DF_BUFFERSIZE );	                                          
		SPI_CS_HIGH;                                                           
		while (DF_CheckIfBusy());                                                
	}
	else
	{
		for( lclCount=0; lclCount<DF_BYTESPERPAGE; lclCount++ )	
		{			
			DF2Buffer[ lclCount ] = *( Buffer + lclCount );
		}
	}
	return SUCCESS;                                                       
}                                                                         

/**
 ******************************************************************************
 **
 **	Function Name		: DF_WriteBufferToPage
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_WriteBufferToPage( uint32 PageNo )
{
	uint32 localPageNo;
	uint8 localCmdBuffer[ 4 ];
	uint32 localAddress;
	uint32 lclCount;
	
	if( GBLDataflashType == DF_AT45DB161 )
	{
		localPageNo = PageNo << DF_PAGEADDBITSHIFT; 	//12 bit Page Address
		
		localCmdBuffer[ 0 ] = ( uint8 )( BUFFER_2_TO_PAGE_PROGRAM_WITH_ERASE );
		localCmdBuffer[ 1 ] = ( uint8 )( localPageNo >> 8 );	//Read Page into Buffer
		localCmdBuffer[ 2 ] = ( uint8 )( localPageNo >> 0 );
		localCmdBuffer[ 3 ] = ( uint8 )( 0x00 );
		
		SPI_CS_HIGH;	SPI_CS_LOW;
		SPISend( localCmdBuffer, 4 );	
		SPI_CS_HIGH;
		while (DF_CheckIfBusy());
	}
	else
	{
		localAddress = PageNo * DF2_BYTESPERPAGE;
		//DF_ErasePage( PageNo );
		for( lclCount=0; lclCount<DF_BYTESPERPAGE; lclCount++ )
		{
			DF_ByteProgram( localAddress + lclCount, DF2Buffer[ lclCount ]);	 
		}
	}	
	return SUCCESS; 
}

/**
 ******************************************************************************
 **
 **	Function Name		: DF_WriteDataBytesIntoBuffer
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_WriteDataBytesIntoBuffer( uint8 *Buffer, uint32 Offset, uint32 NoOfBytes )
{	                                                                       
	uint32 localOffset; 
	uint8 localCmdBuffer[ 4 ];
	uint32 lclCount;
                                                                           
	localOffset = Offset;                                                  
                                                                           
	if( GBLDataflashType == DF_AT45DB161 )
	{
		if ( (Offset + NoOfBytes )  <= DF_BUFFERSIZE )                                  
		{                                                                      
			localCmdBuffer[0] = ( uint8 )( BUFFER_2_WRITE );  
			localCmdBuffer[1] = ( uint8 )( 0x00 );	                                           
			localCmdBuffer[2] = ( uint8 )( localOffset >> 8 );                         
			localCmdBuffer[3] = ( uint8 )( localOffset >> 0 );                         
		                                                                       
			SPI_CS_HIGH;	SPI_CS_LOW;                                              
			SPISend( localCmdBuffer, 4);                                         
			SPISend( Buffer, NoOfBytes );	                                   
			SPI_CS_HIGH; 
			                                                        
			while (DF_CheckIfBusy());                                             
			return SUCCESS;
		}                                                                      
		else                                                                   
		{                                                                      
			return FAILED;                                                     
		}
	}
	else
	{
		if ( (Offset + NoOfBytes )  <= DF_BUFFERSIZE )
		{
			for( lclCount=0; lclCount<NoOfBytes; lclCount++ )
			{
				DF2Buffer[ Offset + lclCount ] = *( Buffer + lclCount );
			}
			return SUCCESS;
		}
		else
		{
			return FAILED;
		}		
	}                                                                      
}                                                                          

/**
 ******************************************************************************
 **
 **	Function Name		: DF_WriteBytesToPage
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_WriteBytesToPage( uint32 PageNo, uint8 *Buffer, uint32 ByteOffset, uint32 NoOfBytes )
{
	uint32 localCount;
	uint32 localAddress;
	uint32 lclCount;
	uint8 localCmdBuffer[ 4 ];

	if( GBLDataflashType == DF_AT45DB161 )
	{
		if ( DF_ReadPage( DFRxBuffer,PageNo ) == SUCCESS )
		{
			for (localCount=0; localCount<NoOfBytes; localCount++)
				DFRxBuffer[ByteOffset + localCount] = *(Buffer + localCount);
	
			if ( DF_WriteDataIntoBuffer ( DFRxBuffer ) == SUCCESS )
			{
				if ( DF_WriteBufferToPage( PageNo ) == SUCCESS )
					return SUCCESS;
				else
					return FAILED;
			}
			else
			{
				return FAILED;
			}
		}
		else
		{
			return FAILED;
		}
	}
	else
	{				
		localAddress = PageNo * DF2_BYTESPERPAGE;
		localCmdBuffer[ 0 ] = DF_READ;
		localCmdBuffer[ 1 ] = ( uint8 )(( localAddress & 0xFFFFFF ) >> 16 );
		localCmdBuffer[ 2 ] = ( uint8 )(( localAddress & 0xFFFF ) >> 8 );
		localCmdBuffer[ 3 ]	= ( uint8 )( localAddress & 0xFF );
		IOCLR0 |= SPI_CS2;
		SPISend( localCmdBuffer, 4 );
		SPIReceive( &DF2TempBuffer[ 0 ], DF_BYTESPERPAGE );	
		IOSET0 |= SPI_CS2;
		while (DF_CheckIfBusy());
		
		for( lclCount=0; lclCount<NoOfBytes; lclCount++ )
		{
			DF2TempBuffer[ ByteOffset + lclCount ] = *( Buffer + lclCount );	
		}
		
		DF_ErasePage( PageNo );		
		for( lclCount=0; lclCount<DF_BYTESPERPAGE; lclCount++ )
		{
			DF_ByteProgram( localAddress + lclCount, DF2TempBuffer[ lclCount ]);	 
		}

		return SUCCESS;
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: DF_CheckIfBusy
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_CheckIfBusy( void )
{
	uint8 Status;
	uint8 localCmdBuffer;
		
	if( GBLDataflashType == DF_AT45DB161 )
	{
		Status = 0x00;	
		localCmdBuffer = ( uint8 )STATUS_REGISTER_READ;
		
		SPI_CS_LOW;
		SPISend( &localCmdBuffer, 1);
		SPIReceive( &Status, 1 );	
		SPI_CS_HIGH;
		
		if ( Status & 0x80 )
		{				
			return DF_FLASH_READY;	
		}
		else
		{
			return DF_FLASH_BUSY;		
		}
	}
	else
	{
		Status = 0x01;
		localCmdBuffer = ( uint8 )DF_RDSR;
				
		IOCLR0 |= SPI_CS2;
		SPISend( &localCmdBuffer, 1);
		SPIReceive( &Status, 1 );	
		IOSET0 |= SPI_CS2;
		//UART1Send( &Status, 1 );
		if ( Status & 0x01 )
		{				
			return DF_FLASH_BUSY;	
		}
		else							  
		{
			return DF_FLASH_READY;		
		}	
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: DF_ErasePage
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_ErasePage( uint32 PageNo )
{
	uint32 localPageNo;
	uint8 localCmdBuffer[ 4 ];
	uint32 localAddress;

	if( GBLDataflashType == DF_AT45DB161 )
	{
		localPageNo = PageNo << DF_PAGEADDBITSHIFT; 	//12 bit Page Address
	
		//Read Page into Buffer
		localCmdBuffer[ 0 ] = ( uint8 )( PAGE_ERASE);
		localCmdBuffer[ 1 ] = ( uint8 )( localPageNo>>8 );
		localCmdBuffer[ 2 ] = ( uint8 )( localPageNo>>0 );
		localCmdBuffer[ 3 ] = ( uint8 )( 0x00 );
	
		SPI_CS_HIGH;	SPI_CS_LOW;
		SPISend( localCmdBuffer, 4);
		SPI_CS_HIGH;
	}
	else
	{
		localCmdBuffer[ 0 ] = DF_WREN;	
		IOCLR0 |= SPI_CS2;
		SPISend( localCmdBuffer, 1 );
		IOSET0 |= SPI_CS2;
		while (DF_CheckIfBusy());

		localAddress = PageNo * DF2_BYTESPERPAGE;
		localCmdBuffer[ 0 ] = DF_4KB_SECTOR_ERASE;
		localCmdBuffer[ 1 ] = ( uint8 )(( localAddress & 0xFFFFFF ) >> 16 );
		localCmdBuffer[ 2 ] = ( uint8 )(( localAddress & 0xFFFF ) >> 8 );
		localCmdBuffer[ 3 ]	= ( uint8 )( localAddress & 0xFF );
		IOCLR0 |= SPI_CS2;
		SPISend( localCmdBuffer, 4 );	
		IOSET0 |= SPI_CS2;			
	}
	while (DF_CheckIfBusy());
	
	return SUCCESS; 
}

/**
 ******************************************************************************
 **
 **	Function Name		: DF_ChipErase
 **
 **	Description			: 
 **
 **	Passed Parameters	: None 
 **
 **	Modified Data		: None
 **
 **	Return Value		: None
 **
 ******************************************************************************
 */
uint32 DF_ChipErase( void )
{
	uint32 ErasePageNo;
	uint8 localCmdBuffer;

	if( GBLDataflashType == DF_AT45DB161 )
	{
		for	( ErasePageNo=1; ErasePageNo<DF_NOOFPAGES; ErasePageNo++ )
		{
			DF_ErasePage( ErasePageNo );	
		}
	}
	else
	{
		DF_ReadPageToBuffer( 0 );
		
		localCmdBuffer = DF_WREN;	
		IOCLR0 |= SPI_CS2;
		SPISend( &localCmdBuffer, 1 );
		IOSET0 |= SPI_CS2;
		while (DF_CheckIfBusy());
		localCmdBuffer = DF_CHIP_ERASE;				
		IOCLR0 |= SPI_CS2;
		SPISend( &localCmdBuffer, 1 );		
		IOSET0 |= SPI_CS2;
		while (DF_CheckIfBusy());

		DF_WriteBufferToPage( 0 );	
		while (DF_CheckIfBusy());
	}
	return SUCCESS;
}

void DF_DisableBlkProt( void )
{
	uint8 localCmdBuffer[ 2 ];
	
	localCmdBuffer[ 0 ] = DF_WREN;	
	IOCLR0 |= SPI_CS2;
	SPISend( localCmdBuffer, 1 );
	IOSET0 |= SPI_CS2;

	localCmdBuffer[ 0 ] = DF_EWSR;	
	IOCLR0 |= SPI_CS2;
	SPISend( localCmdBuffer, 1 );
	IOSET0 |= SPI_CS2;

	localCmdBuffer[ 0 ] = DF_WRSR;
	localCmdBuffer[ 1 ] = ( 0<<4 ) |( 0<<3 ) |( 0<<2 );
	IOCLR0 |= SPI_CS2;
	SPISend( localCmdBuffer, 2 );
	IOSET0 |= SPI_CS2;			
}

uint8 Read_ID( void )
{
	uint8 byte;
	uint8 localCmdBuffer[ 4 ];

	IOCLR0 |= SPI_CS2;
	localCmdBuffer[ 0 ] = 0x90;
	localCmdBuffer[ 1 ] = 0x00;
	localCmdBuffer[ 2 ] = 0x00;
	localCmdBuffer[ 3 ]	= 0x01;
	
	SPISend( localCmdBuffer, 4 );
	SPIReceive( &byte, 1 );		/* receive byte */
	IOSET0 |= SPI_CS2;				/* disable device */
	return byte;
}

void DF_ByteProgram( uint32 Address, uint8 Data )
{
	uint8 localCmdBuffer[ 5 ];
	uint32 localAddress;

	localCmdBuffer[ 0 ] = DF_WREN;	
	IOCLR0 |= SPI_CS2;
	SPISend( localCmdBuffer, 1 );
	IOSET0 |= SPI_CS2;
	while (DF_CheckIfBusy());

	localAddress = Address;
	localCmdBuffer[ 0 ] = DF_BYTE_PROGRAM;
	localCmdBuffer[ 1 ] = ( uint8 )(( localAddress & 0xFFFFFF ) >> 16 );
	localCmdBuffer[ 2 ] = ( uint8 )(( localAddress & 0xFFFF ) >> 8 );
	localCmdBuffer[ 3 ]	= ( uint8 )( localAddress & 0xFF );
	localCmdBuffer[ 4 ] = Data;
	IOCLR0 |= SPI_CS2;
	SPISend( localCmdBuffer, 5 );	
	IOSET0 |= SPI_CS2;
	while (DF_CheckIfBusy());
}

/******************************************************************************
 **                            << End of FILE >>
 *****************************************************************************/
 
                                              
