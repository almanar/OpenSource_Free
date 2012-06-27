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
								  << Defines >>
******************************************************************************/
#ifndef _DATAFLASH_H
	#define _DATAFLASH_H
	
	#define DF_AT45DB161	FALSE
	#define SST25VF016B		TRUE
	
	#ifdef DF_AT45DB161
		#define DF_BYTESPERPAGE		528
		#define DF2_BYTESPERPAGE	4096
		#define DF_NOOFPAGES		4096
		
		#define DF_BUFFERSIZE		DF_BYTESPERPAGE
		#define DF_PAGEADDBITSHIFT	2
		
		//Read Commands
		#define MAIN_MEMORY_PAGE_READ						0xD2 
		#define CONTINUOUS_ARRAY_READ_LEGACY_COMMAND		0xE8 
		#define CONTINUOUS_ARRAY_READ_LOW_FREQUENCY			0x03 
		#define CONTINUOUS_ARRAY_READ_HIGH_FREQUENCY		0x0B 
		#define BUFFER_1_READ_LOW_FREQUENCY					0xD1 
		#define BUFFER_2_READ_LOW_FREQUENCY					0xD3 
		#define BUFFER_1_READ								0xD4
		#define BUFFER_2_READ								0xD6		
		//Program and Erase Commands
		#define BUFFER_1_WRITE								0x84 
		#define BUFFER_2_WRITE								0x87 
		#define BUFFER_1_TO_PAGE_PROGRAM_WITH_ERASE			0x83 
		#define BUFFER_2_TO_PAGE_PROGRAM_WITH_ERASE			0x86 
		#define BUFFER_1_TO_PAGE_PROGRAM_WITHOBUT_ERASE		0x88 
		#define BUFFER_2_TO_PAGE_PROGRAM_WITHOUT_ERASE		0x89 
		#define PAGE_ERASE									0x81 
		#define BLOCK_ERASE									0x50 
		#define SECTOR_ERASE								0x7C 
		#define MAIN_MEMORY_PAGE_PROGRAM_THROUGH_BUFFER_1	0x82 
		#define MAIN_MEMORY_PAGE_PROGRAM_THROUGH_BUFFER_2	0x85 														
		//Additional Commands								
		#define MAIN_MEMORY_PAGE_TO_BUFFER_1_TRANSFER		0x53 
		#define MAIN_MEMORY_PAGE_TO_BUFFER_2_TRANSFER		0x55 
		#define MAIN_MEMORY_PAGE_TO_BUFFER_1_COMPARE		0x60 
		#define MAIN_MEMORY_PAGE_TO_BUFFER_2_COMPARE		0x61 
		#define AUTO_PAGE_REWRITE_THROUGH_BUFFER_1			0x58 
		#define AUTO_PAGE_REWRITE_THROUGH_BUFFER_2			0x59 
		#define DEEP_POWER_DOWN								0xB9 
		#define RESUME_FROM_DEEP_POWER_DOWN					0xAB 
		#define STATUS_REGISTER_READ						0xD7 
		#define MANUFACTURER_AND_DEVICE_ID_READ				0x9F 								

	#endif //DF_AT45DB161
	
	#define DF_FLASH_BUSY		0x01
	#define DF_FLASH_READY		0x00

		#define DF_READ				( 0x03 )
		#define DF_HIGH_SPEED_READ 	( 0x0B )
		#define	DF_4KB_SECTOR_ERASE	( 0x20 )
		#define DF_32KB_BLOCK_ERASE	( 0x52 )
		#define DF_64KB_BLOCK_ERASE	( 0xD8 )
		#define DF_CHIP_ERASE		( 0x60 )
		#define DF_BYTE_PROGRAM 	( 0x02 )
		#define DF_AAI_WORD_PROGRAM	( 0xAD )
		#define DF_RDSR				( 0x05 )
		#define DF_EWSR				( 0x50 )
		#define DF_WRSR				( 0x01 )
		#define DF_WREN				( 0x06 )
		#define DF_WRDI				( 0x04 )
		#define	DF_RDID				( 0x90 )
		#define	DF_JEDEC_ID			( 0x9F )
		#define DF_EBSY				( 0x70 )
		#define DF_DBSY				( 0x80 )	
/******************************************************************************
 **                         << Function Declarations >>
 *****************************************************************************/                                                         
	uint32 DF_ReadPage( uint8 *Buffer, uint32 PageNo );  
	uint32 DF_ReadPageToBuffer( uint32 PageNo );                                                
	uint32 DF_ReadDataFromBuffer( uint8 *Buffer );
	uint32 DF_ReadDataBytesFromBuffer( uint8 *Buffer, uint32 Offset, uint32 NoOfBytes );                                            
	uint32 DF_ReadBytesFromPage( uint32 PageNo, uint8 *Buffer, uint32 ByteOffset, uint32 NoOfBytes );
	uint32 DF_WriteDataIntoBuffer( uint8 *Buffer );
	uint32 DF_WriteBufferToPage( uint32 PageNo );	
	uint32 DF_WriteDataBytesIntoBuffer( uint8 *Buffer, uint32 Offset, uint32 NoOfBytes );	
	uint32 DF_WriteBytesToPage( uint32 PageNo, uint8 *Buffer, uint32 ByteOffset, uint32 NoOfBytes ); 
	uint32 DF_CheckIfBusy( void ); 
	uint32 DF_ErasePage( uint32 PageNo );                                               
	uint32 DF_ChipErase( void );
	void DF_DisableBlkProt( void );
	uint8 Read_ID( void );
	void DF_ByteProgram( uint32 Address, uint8 Data );
#endif //_DATAFLASH_H
