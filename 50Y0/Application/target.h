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
#ifndef _TARGET_H 
#define _TARGET_H

#ifdef __cplusplus
   extern "C" {
#endif

/* System configuration: Fosc, Fcclk, Fcco, Fpclk must be defined */
/* Crystal frequence,10MHz~25MHz should be the same as actual status. */
#define Fosc	12000000

/* System frequence,should be (1~32)multiples of Fosc,and should be equal or 
less than 60MHz. */
#define Fcclk	(Fosc * 5)

/* CCO frequence,should be 2/4/8/16 multiples of Fcclk, ranged from 156MHz to 
320MHz. */
#define Fcco	(Fcclk * 4)

/* VPB clock frequence , must be 1/2/4 multiples of (Fcclk / 4). */
#define Fpclk	((Fcclk / 4) * 1)

extern void TargetInit(void);

#ifdef __cplusplus
   }
#endif
 
#endif /* end _TARGET_H */
/******************************************************************************
**                            End Of File
******************************************************************************/
