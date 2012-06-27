/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2010 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |      Definitions for some system APIs.
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by     :   Jay XIN
 |  Coded    by     :   Jay XIN
 |  Tested   by     :   Stanley YONG
 |
 \=========================================================================*/

#ifndef __QL_INTERFACE_H__
#define __QL_INTERFACE_H__
#include "ql_appinit.h"
#include "ql_type.h"
#include "ql_memory.h"


/*****************************************************************
* Function:     Ql_Sleep 
* 
* Description:
*               Suspends the execution of the current task 
*               until the time-out interval elapses.
* Parameters:
*               msec: 
*                   The time interval for which execution is to
*                   be suspended, in milliseconds.
* Return:        
*               None
*****************************************************************/
void Ql_Sleep(u32 msec);


/*****************************************************************
* Function:     Ql_osGetCurrentTaskPriority 
* 
* Description:
*               This function gets system EVENTs from Core System. 
*               When there is no event in customer task's event queue, 
*               the task will go into waiting state.
* Parameters:
*               signal_p: 
*                   Pointer to QlEventBuffer structure, which contains
*                   Event Type and Event Data.
* Return:        
*               None
*****************************************************************/
void Ql_GetEvent(QlEventBuffer  *signal_p);


/*****************************************************************
* Function:     Ql_Reset 
* 
* Description:
*               This function can be the alias of some functions
*               by different parameters.
* Parameters:
*               resettype: 
*                   Function types of this function.
*                    0 = WDT_RESET()
*                    1 = DRV_ABN_RESET()
*                    2 = DRV_RESET()
*                    3 = ASSERT()
* Return:        
*               None
*****************************************************************/
void Ql_Reset(u8 resettype);


/*****************************************************************
* Function:     Ql_PowerDown 
* 
* Description:
*               Calling this function can switch off
*               the power supply to the module.
* Parameters:
*               powertype: 
*                   Action types of this function.
*                    0 = Urgently power off
*                    1 = Normal power off
* Return:        
*               None
*****************************************************************/
void Ql_PowerDown(u8 powertype);


/*****************************************************************
* Function:     Ql_PowerOnAck 
* 
* Description:
*               
* Parameters:
*               None
* Return:        
*               None
*****************************************************************/
void Ql_PowerOnAck(void);


/*****************************************************************
* Function:     Ql_StartWatchdog 
* 
* Description:
*               This function starts watch-dog service.
*               If not call Ql_FeedWatchdog within 
*               90 times*(200*10ms)=180000ms=3 minutes,
*               the module will automatically power down.
* Parameters:
*               tick10ms: 
*                   Counter which counts by step 1 every 10ms.
*
*               overfeedcount:
*                   Counter threshold.
*                   The module will restart when the counter's value
*                   is over this threshold.
*
*               resettype:
*                   0 = WDT_RESET, 1 = powerdown, 2 = ASSERT
*
*               Example: 
*                   Ql_StartWatchdog(200,  90, 1);
* Return:        
*               TRUE indicates suceess in starting watch dog service.
*               FALSE indicates failure.
*****************************************************************/
bool Ql_StartWatchdog(u16 tick10ms, u32 overfeedcount, u16 resettype);


/*****************************************************************
* Function:     Ql_FeedWatchdog 
* 
* Description:
*               Calling this function will reset watch-dog.
*               If not call Ql_FeedWatchdog within 
*               90 times*(200*10ms)=180000ms=3 minutes,
*               the module will automatically power down.
* Parameters:
*               None
* Return:        
*               None
*****************************************************************/
void Ql_FeedWatchdog(void);


/*****************************************************************
* Function:     Ql_StopWatchdog 
* 
* Description:
*               Stop the watch-dog, which was started previously.
*
* Parameters:
*               None
* Return:        
*               None
*****************************************************************/
void Ql_StopWatchdog(void);


/*****************************************************************
* Function:     Ql_ReadModuleUID 
* 
* Description:
*               Get the flash UID.
*
* Parameters:
*               uid_buf:
*                   Pointer to a unsigned integer, which is 
*                   the UID. 
* Return:        
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
*****************************************************************/
s32  Ql_ReadModuleUID(u16* uid_buf);


/*****************************************************************
* Function:     Ql_GetCoreVer 
* 
* Description:
*               Get the version ID of the core.
*
* Parameters:
*               ptr_ver:
*                   [out] Pointer to a unsigned char, which is 
*                           the the version ID of the core.
*               len:
*                   [in] It must be equal or greater than the length of version ID.
*                         Otherwise error code will be returned.
* Return:        
*               The length of version ID indicates success.
*               Negative indicates failure. please see Error Code Definition.
*****************************************************************/
s32  Ql_GetCoreVer(u8* ptr_ver, u16 len);


/*****************************************************************
* Function:     Ql_GetSDKVer
* 
* Description:
*               Get the version ID of the SDK.
*
* Parameters:
*               ptr_ver:
*                   [out] Pointer to a unsigned char, which is 
*                           the the version ID of the SDK.
*               len:
*                   [in] A number will be compare with the length of version ID.
*                         
* Return:        
*               The smaller between len and the length of version ID.
*****************************************************************/
s32  Ql_GetSDKVer(u8* ptr_ver, u16 len);


/*****************************************************************
* Function:     Ql_GetPowerOnReason 
* 
* Description:
*               Get Power on Reason.
*
* Parameters:
*               cause point to the data which indicate the reason of the Power on
*       	        PWRKEYPWRON = 0,
*	                CHRPWRON	= 1,
*	                RTCPWRON = 2,
*	                CHRPWROFF = 3,
*	                WDTRESET = 4,
*	                ABNRESET = 5, 
*	                USBPWRON = 6,  
*	                USBPWRON_WDT = 7,
*	                PRECHRPWRON = 8,
*	                HWSYSRST = 9,
*	                UNKNOWN_PWRON = 0xF9	
*
* Return:        
*               None
*****************************************************************/
void Ql_GetPowerOnReason(u8* cause);
#endif  // End-of __QL_INTERFACE_H__

