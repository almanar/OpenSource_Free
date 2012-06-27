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
#ifndef _TIMER_H 
#define _TIMER_H

#define INTERVAL_1MS	( Fpclk/1000 - 1 )
#define INTERVAL_10MS	( Fpclk/100  - 1 )

extern uint64 MS_TIMER;
extern uint64 livepulsetimer;
uint8 init_timer0( void );
uint8 init_timer1( void );
void enable_timer( uint8 timer_num );
void disable_timer( uint8 timer_num );
void reset_timer( uint8 timer_num );
void Timer0Handler( void ) __irq; 
void Timer1Handler( void ) __irq;
void Delay( uint64 Delaytime ); 

#endif /* end _TIMER_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/

