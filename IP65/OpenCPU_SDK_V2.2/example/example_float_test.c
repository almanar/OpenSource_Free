#ifdef __EXAMPLE_FLOAT_TEST__
 
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_fcm.h"
#include "ql_memory.h"

QlEventBuffer g_event; // Keep this variable a global variable due to its big size
char notes[100];

void test_float(void);

void ql_entry(void)
{
    s32 ret;
    bool keepGoing = TRUE;
    //QlTimer tm;
    u32 cnt = 0; 

    double Value;
    char Buff[10]="0.345";
    char* pStr;

    Ql_SetDebugMode(BASIC_MODE);    /* Only after the device reboots, 
                                                                         *  the set debug-mode takes effect. 
                                                                         */
    Ql_DebugTrace("OpenCPU: float test !\r\n\r\n");    /* Print out message through DEBUG port */
    Ql_OpenModemPort(ql_md_port1);  /* or ql_md_port2, two virtual modem ports are available.*/

    pStr = Ql_GetMemory(20);
    if ( pStr != NULL)
    {
        Ql_sprintf(notes, "&pStr = %x\r\n", &pStr);
        Ql_DebugTrace(notes);
        Ql_memcpy(pStr, Buff, sizeof(Buff));
    }
    else
    {
        Ql_DebugTrace("Fail to get memory for 'pStr'\r\n");
    }
    
    // Start a timer
    //tm.timeoutPeriod = Ql_SecondToTicks(2); /* Set the interval of timer */
    //Ql_StartTimer(&tm);
    //Ql_DebugTrace("The timer starts.\r\n\r\n");

    while(keepGoing)
    {
        Ql_GetEvent(&g_event);
        switch(g_event.eventType)
        {
            case EVENT_UARTDATA:
            {
                //TODO: receive and handle data from UART

                /* The following code segment is written to demonstrate the usage of 'AT+QLOCPU'.
                *   If send 'AT+QLOCPU=0' through serial port tool, and reboot the device, only Core 
                *   System software is booted (the application will not be booted).
                */
                PortData_Event* pDataEvt = (PortData_Event*)&g_event.eventData.uartdata_evt;
                Ql_sprintf(notes, "AT+QLOCPU=0\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, notes, Ql_strlen(notes));
                if (0 == ret)
                {
                    Ql_SendToModem(ql_md_port1, (u8*)"AT+QLOCPU=0\n", Ql_strlen(notes) + 1);
                }
                else if (Ql_strstr((char *)pDataEvt->data, "atof(") != NULL)
                {
                    char * pos1 = Ql_strstr((char *)pDataEvt->data, "(");
                    char * pos2 = Ql_strstr((char *)pDataEvt->data, ")");
                    if ( pos1 != NULL && pos2 != NULL)
                    {
                        double fTemp = 0.0;
                        char a[20] = { 0x0 };
                        Ql_memcpy(a, pos1 + 1, pos2 - pos1 - 1);
                        fTemp = Ql_atof(a);
                        Ql_sprintf(notes, "atof(%s) = %f\r\n", a, fTemp);
                        Ql_DebugTrace(notes);
                    }
                    else
                    {
                        extern double atof(const char*);
                        char chTest[20] = { 0x0 };
                        Ql_memcpy(chTest, "0.12345", 7);
                        Value=atof(chTest);
                        Ql_DebugTrace("\r\nBuffer is %s.\r\n", chTest);
                        Ql_sprintf(notes, "\r\nFloat value is %f.\r\n", Value);
                        Ql_DebugTrace(notes);
                    }
                }
                break;
            }
            case EVENT_MODEMDATA:
            {
                //TODO: receive and hanle data from CORE through virtual modem port
                break;
            }
            case EVENT_TIMER:
            {
                //TODO: specify what you want to happen when the interval for timer elapes

                //Ql_DebugTrace("The timer raises for %d time(s).\r\n", ++cnt);
                //Ql_memset(notes, 0x0, Ql_strlen(notes));
                //Ql_sprintf(notes, "The timer raises for %d time(s).\r\n", ++cnt);
                //Ql_DebugTrace(notes);

                char chTest[20] = { 0x0 };
                Ql_memcpy(chTest, "0.12345", 7);
                Value=Ql_atof(chTest);
                Ql_DebugTrace("\r\nBuffer is %s.\r\n", chTest);
                Ql_DebugTrace("\r\nFloat value is %f.\r\n", Value);
                
                // Start the timer again
                if (cnt++ < 3)
                {
                    //Ql_StartTimer(&tm);
                }
                else
                {
                    Ql_DebugTrace("\r\nThe timer stops.\r\n");
                }
                break;
            }
            case EVENT_MSG:
            {
                //TODO: specify what you want to happen when the message from other task arrives
                break;
            }
            
            default:
                break;
        }
    }
    Ql_FreeMemory(pStr);
}

#endif // __EXAMPLE_FLOAT_TEST__

