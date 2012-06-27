/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2010 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |      Debug interface definition
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by     :   Jay XIN
 |  Coded    by     :   Jay XIN
 |  Tested   by     :   Stanley YONG
 |--------------------------------------------------------------------------
 | Revision History
 | ----------------
 |  Sep. 14, 2010       Stanley Yong        Change the type of timerId to u32.
 |                                          Revise the way to generate timer Id.
 |  ------------------------------------------------------------------------
 |  Sep. 19, 2010       Jay XIN             Add APIs: 'Ql_StarLowLevelTimer'
 |                                          and 'Ql_StopLowLevelTimer'.
 |  ------------------------------------------------------------------------
 |  Sep. 26, 2010       Stanley Yong        Make the notes clear.
 |
 \=========================================================================*/

#ifndef __QL_TIMER_H__
#define __QL_TIMER_H__

#include "ql_type.h"

/* Timer */
typedef struct QlTimerTag
{
    ticks		timeoutPeriod;
    u32			timerId;
}
QlTimer;

typedef struct QlSysTimerTag
{
    unsigned short year;    /* A short format of year, 0-127 */
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
}
QlSysTimer;


/*****************************************************************
* Function:     Ql_StartTimer 
* 
* Description:
*               Start up a timer.
*
* Parameters:
*               timer_p:
*                       [in] the timer object to start
* Return:        
*               Timer Id (positive number) if succeed in starting the timer
*               Error codes (negative number) if fail to start the timer
*****************************************************************/
u32   Ql_StartTimer(QlTimer *timer_p);


/*****************************************************************
* Function:     Ql_StopTimer 
* 
* Description:
*               Stop a timer.
*
* Parameters:
*               timer_p:
*                       [in] The QlTimer object to stop
* Return:        
*               0 if succeed in stopping the timer
*               Error codes (negative number) if fail to stop the timer
*****************************************************************/
s16   Ql_StopTimer(QlTimer *timer_p);


/*****************************************************************
* Function:     Ql_GetLocalTime 
* 
* Description:
*               Retrieves the current local date and time.
*
* Parameters:
*               datetime:
*                       [out] Pointer to the QlSysTimer object
* Return:        
*               TRUE if succeed in retrieving the local date and time
*               , otherwise FALSE
*****************************************************************/
bool  Ql_GetLocalTime(QlSysTimer * datetime);


/*****************************************************************
* Function:     Ql_SetLocalTime 
* 
* Description:
*               Sets the current local date and time.
*
* Parameters:
*               datetime:
*                       [in] Pointer to the QlSysTimer object
* Return:        
*               TRUE if succeed in retrieving the local date and time
*               , otherwise FALSE
*****************************************************************/
bool  Ql_SetLocalTime(QlSysTimer * datetime);


/*****************************************************************
* Function:     Ql_GetRelativeTime 
* 
* Description:
*               This function returns the number of milliseconds 
*               since the device booted.
*
* Parameters:
*               None
* Return:        
*               Number of milliseconds
*****************************************************************/
u64   Ql_GetRelativeTime(void);


/*****************************************************************
* Function:     Ql_GetRelativeTime 
* 
* Description:
*               This function returns the number of MCU counters
*               since the device booted.
*
* Parameters:
*               None
* Return:        
*               Number of MCU counters
*****************************************************************/
u32   Ql_GetRelativeTime_Counter(void);


/*****************************************************************
* Function:     Ql_SecondToTicks 
* 
* Description:
*               Converts time from seconds to ticks.
*
* Parameters:
*               seconds:
*                       Seconds to convert
* Return:        
*               ticks equivalent to the seconds
*****************************************************************/
ticks Ql_SecondToTicks(u32 seconds);


/*****************************************************************
* Function:     Ql_MillisecondToTicks 
* 
* Description:
*               Converts time from milliseconds to ticks.
*
* Parameters:
*               seconds:
*                       Milliseconds to convert
* Return:        
*               ticks equivalent to the milliseconds
*****************************************************************/
ticks Ql_MillisecondToTicks(u32 milliseconds);


/*****************************************************************
* Function:     Ql_Mktime 
* 
* Description:
*               This function get total seconds elapsed 
*               since 1970.01.01 00:00:00.
*
* Parameters:
*               psystime:
*                       [in] Pointer to the QlSysTimer object
* Return:        
*               The total seconds
*---------------
* Usage:
*               QlSysTimer systime;
*               Ql_GetLocalTime(&systime);
*               systime->year += your base year(such as 1900, 2000)
*               seconds = Ql_Mktime(&systime);
*****************************************************************/
u32  Ql_Mktime(QlSysTimer *psystime);


/*********************************************************************
example

struct xx_type  userparameter;
//start 2*10 ms timer, when timeout, will call Callback_timerout_example
ret = Ql_StarLowLevelTimer(2,Callback_timerout_example,(void*)&userparameter);

void Callback_timerout_example(void *parameter)
{
    struct xx_type *userp = (struct xx_type *)parameter;
    Ql_StopLowLevelTimer();

    userp->xxx;
    do_something;
    !!!!!!!can not do task scheduler function;
    you can Ql_osSendEvent to task

   //continue start timer
    Ql_StarLowLevelTimer(2,Callback_timerout_example,(void*)userp);
}

//stop timer
Ql_StopLowLevelTimer()

***************************************************************************/
/*********************************************************************
only support one lowlevel timer
tick10ms    unit is 10 ms
***************************************************************************/
bool Ql_StarLowLevelTimer(u32 tick10ms, void (*Callback_timerout)(void *), void *parameter);
void Ql_StopLowLevelTimer(void);

#endif  // End-of __QL_TIMER_H__

