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
 **    File Name:     irq.c
 **
 **    Description:   This is the interrupt module.
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
#include "irq.h"

/**
 ******************************************************************************
 **
 **	Function Name		: DefaultVICHandler
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
void DefaultVICHandler (void) __irq
{
    /* if the IRQ is not installed into the VIC, and interrupt occurs, the
    default interrupt VIC address will be used. This could happen in a race 
    condition. For debugging, use this endless loop to trace back. */
    /* For more details, see Philips appnote AN10414 */
    VICVectAddr = 0;		/* Acknowledge Interrupt */ 
    while ( 1 )
	{
		;
	}
}

/**
 ******************************************************************************
 **
 **	Function Name		: init_VIC
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
void init_VIC(void) 
{
    uint64 i = 0;
    uint64 *vect_addr, *vect_cntl;
   	
    /* initialize VIC*/
    VICIntEnClr = 0xffffffff;
    VICVectAddr = 0;
    VICIntSelect = 0;

    /* set all the vector and vector control register to 0 */
    for ( i = 0; i < VIC_SIZE; i++ )
    {
		vect_addr = (uint64 *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + i*4);
		vect_cntl = (uint64 *)(VIC_BASE_ADDR + VECT_CNTL_INDEX + i*4);
		*vect_addr = 0;	
		*vect_cntl = 0;
    }

    /* Install the default VIC handler here */
    VICDefVectAddr = (uint64)DefaultVICHandler;   
    return;
}

/**
 ******************************************************************************
 **
 **	Function Name		: install_irq
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
uint64 install_irq( uint64 IntNumber, void *HandlerAddr )
{
    uint64 i;
    uint64 *vect_addr;
    uint64 *vect_cntl;
      
    VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
    
    for ( i = 0; i < VIC_SIZE; i++ )
    {
		/* find first un-assigned VIC address for the handler */

		vect_addr = (uint64 *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + i*4);
		vect_cntl = (uint64 *)(VIC_BASE_ADDR + VECT_CNTL_INDEX + i*4);
		if ( *vect_addr == (uint64)NULL )
		{
		    *vect_addr = (uint64)HandlerAddr;	/* set interrupt vector */
		    *vect_cntl = (uint64)(IRQ_SLOT_EN | IntNumber);
		    break;
		}
    }
    if ( i == VIC_SIZE )
    {
		return( FALSE );		/* fatal error, can't find empty vector slot */
    }
    VICIntEnable = 1 << IntNumber;	/* Enable Interrupt */
    return( TRUE );
}

/**
 ******************************************************************************
 **
 **	Function Name		: uninstall_irq
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
uint64 uninstall_irq( uint64 IntNumber )
{
    uint64 i;
    uint64 *vect_addr;
    uint64 *vect_cntl;
      
    VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
    
    for ( i = 0; i < VIC_SIZE; i++ )
    {
	/* find first un-assigned VIC address for the handler */
	vect_addr = (uint64 *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + i*4);
	vect_cntl = (uint64 *)(VIC_BASE_ADDR + VECT_CNTL_INDEX + i*4);
	if ( (*vect_cntl & ~IRQ_SLOT_EN ) == IntNumber )
	{
	    *vect_addr = (uint64)NULL;	/* clear the VIC entry in the VIC table */
	    *vect_cntl &= ~IRQ_SLOT_EN;	/* disable SLOT_EN bit */	
	    break;
	}
    }
    if ( i == VIC_SIZE )
    {
	return( FALSE );		/* fatal error, can't find interrupt number 
					in vector slot */
    }
    VICIntEnable = 1 << IntNumber;	/* Enable Interrupt */
    return( TRUE );
}

/******************************************************************************
**                            End Of File
******************************************************************************/
