#ifdef __EXAMPLE_MULTITASK__

#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_pin.h"
#include "Ql_multitask.h"
#include "Ql_tcpip.h"


/**************************************************************
* main task
***************************************************************/
 
QlEventBuffer    flSignalBuffer; // Set flSignalBuffer to global variables  may as well, otherwise it will occupy stack space
void ql_entry()
{
    bool keepGoing = TRUE;
    s32 sendsubtask = 1;
    s32 ret;

    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("multitask: ql_entry\r\n");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer);
        switch(flSignalBuffer.eventType)
        {
            case EVENT_UARTDATA:
            {            
                Ql_DebugTrace("\r\nEVENT_UARTDATA PORT=%d\r\n",flSignalBuffer.eventData.uartdata_evt.port);
                Ql_DebugTrace("\r\nuart data =%s\r\n",flSignalBuffer.eventData.uartdata_evt.data);    
                ret = Ql_osSendEvent(sendsubtask, flSignalBuffer.eventData.uartdata_evt.len, 0);
                sendsubtask++;
                if(sendsubtask > 10)
                sendsubtask = 1;
                break;            
            }
            
            default:
                break;
        }
    }
}


/**************************************************************
* the 1st sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask1;  //  Set flSignalBuffer_subtask1 to global variables  may as well, otherwise it will occupy stack space
void example_subtask1_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task1_entry\r\n");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer_subtask1);
        switch(flSignalBuffer_subtask1.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask1_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask1.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask1.eventData.msg_evt.data1,
                    flSignalBuffer_subtask1.eventData.msg_evt.data2);
                break;
            }

            default:
                break;
        }
    }    
}


/**************************************************************
* the 2nd sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask2;  //  Set flSignalBuffer_subtask2 to global variables  may as well, otherwise it will occupy stack space
void example_subtask2_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task2_entry\r\n");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer_subtask2);
        switch(flSignalBuffer_subtask2.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask2_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask2.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask2.eventData.msg_evt.data1,
                    flSignalBuffer_subtask2.eventData.msg_evt.data2);
                break;
            }

            default:
                break;
        }
    }
}


/**************************************************************
* the 3rd sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask3;  //  Set flSignalBuffer_subtask3 to global variables  may as well, otherwise it will occupy stack space
void example_subtask3_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task3_entry\r\n");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer_subtask3);
        switch(flSignalBuffer_subtask3.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask3_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask3.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask3.eventData.msg_evt.data1,
                    flSignalBuffer_subtask3.eventData.msg_evt.data2);
            break;
            }

            default:
                break;
        }
    }
}


/**************************************************************
* the 4th sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask4;  //  Set flSignalBuffer_subtask4 to global variables  may as well, otherwise it will occupy stack space
void example_subtask4_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task4_entry\r\n");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer_subtask4);
        switch(flSignalBuffer_subtask4.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask4_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask4.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask4.eventData.msg_evt.data1,
                    flSignalBuffer_subtask4.eventData.msg_evt.data2);
                break;
            }

            default:
                break;
        }
    }
}


/**************************************************************
* the 5th sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask5;  //  Set flSignalBuffer_subtask5 to global variables  may as well, otherwise it will occupy stack space
void example_subtask5_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task5_entry\r\n");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer_subtask5);
        switch(flSignalBuffer_subtask5.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask5_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask5.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask5.eventData.msg_evt.data1,
                    flSignalBuffer_subtask5.eventData.msg_evt.data2);
                break;
             }

            default:
                break;
        }
    }
}


/**************************************************************
* the 6th sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask6;  //  Set flSignalBuffer_subtask6 to global variables  may as well, otherwise it will occupy stack space
void example_subtask6_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task6_entry\r\n");

    while(keepGoing)
    {
        Ql_GetEvent(&flSignalBuffer_subtask6);
        switch(flSignalBuffer_subtask6.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask6_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask6.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask6.eventData.msg_evt.data1,
                    flSignalBuffer_subtask6.eventData.msg_evt.data2);
                break;
            }

            default:
                break;
        }
    }
}


/**************************************************************
* the 7th sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask7;  //  Set flSignalBuffer_subtask7 to global variables  may as well, otherwise it will occupy stack space
void example_subtask7_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task7_entry\r\n ");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer_subtask7);
        switch(flSignalBuffer_subtask7.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask7_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask7.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask7.eventData.msg_evt.data1,
                    flSignalBuffer_subtask7.eventData.msg_evt.data2);
                break;
            }

            default:
                break;
        }
    }
}


/**************************************************************
* the 8th sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask8;  //  Set flSignalBuffer_subtask8 to global variables  may as well, otherwise it will occupy stack space
void example_subtask8_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task8_entry\r\n");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer_subtask8);
        switch(flSignalBuffer_subtask8.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask8_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask8.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask8.eventData.msg_evt.data1,
                    flSignalBuffer_subtask8.eventData.msg_evt.data2);
                break;
            }

            default:
                break;
        }
    }
}


/**************************************************************
* the 9th sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask9;  //  Set flSignalBuffer_subtask9 to global variables  may as well, otherwise it will occupy stack space
void example_subtask9_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task9_entry\r\n");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer_subtask9);
        switch(flSignalBuffer_subtask9.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask9_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask9.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask9.eventData.msg_evt.data1,
                    flSignalBuffer_subtask9.eventData.msg_evt.data2);
            }
                break;

            default:
                break;
        }
    }
}


/**************************************************************
* the 10th sub task
***************************************************************/
QlEventBuffer    flSignalBuffer_subtask10;  //  Set flSignalBuffer_subtask10 to global variables  may as well, otherwise it will occupy stack space
void example_subtask10_entry(s32 TaskId)
{
    bool keepGoing = TRUE;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_DebugTrace("multitask: example_task10_entry\r\n");

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer_subtask10);
        switch(flSignalBuffer_subtask10.eventType)
        {
            case EVENT_MSG:
            {
                Ql_DebugTrace("example_subtask10_entry: EVENT_MSG:%d,%d,%d", flSignalBuffer_subtask10.eventData.msg_evt.src_taskid, 
                    flSignalBuffer_subtask10.eventData.msg_evt.data1,
                    flSignalBuffer_subtask10.eventData.msg_evt.data2);
                break;
            }

            default:
                break;
        }
    }
}

#endif // __EXAMPLE_MULTITASK__

