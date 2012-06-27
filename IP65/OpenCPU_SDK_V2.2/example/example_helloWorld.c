#ifdef __EXAMPLE_HELLWORLD__
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_fcm.h"
#include "ql_trace.h"


QlEventBuffer g_event; // Keep this variable a global variable due to its big size
char notes[100];
void ql_entry(void)
{
    s32 ret;
    bool keepGoing = TRUE;
    QlTimer tm;
    u32 cnt = 0; 
    //s32*** triPointer;

    Ql_SetDebugMode(BASIC_MODE);    /* Only after the device reboots, 
                                                                         *  the set debug-mode takes effect. 
                                                                         */
    Ql_DebugTrace("OpenCPU: Hello World !\r\n\r\n");    /* Print out message through DEBUG port */
    Ql_OpenModemPort(ql_md_port1);  /* or ql_md_port2, two virtual modem ports are available.*/

    // Start a timer
    tm.timeoutPeriod = Ql_SecondToTicks(2); /* Set the interval of timer */
    Ql_StartTimer(&tm);
    Ql_DebugTrace("The timer starts.\r\n\r\n");
	
	#if 0
    triPointer = (s32***)Ql_GetMemory(sizeof(s32**));
    *triPointer = (s32**)Ql_GetMemory(sizeof(s32*));
    *(*triPointer) = (s32*)Ql_GetMemory(sizeof(s32));
    *(*(*triPointer)) = 20110124;
    Ql_DebugTrace("&triPointer: %x\r\n", &triPointer);
    Ql_DebugTrace("*triPointer: %x\r\n", *triPointer);
    Ql_DebugTrace("**triPointer: %x\r\n", *(*triPointer));
    Ql_DebugTrace("***triPointer: %d\r\n", *(*(*triPointer)));
    Ql_FreeMemory(**triPointer);
    Ql_FreeMemory(*triPointer);
    Ql_FreeMemory(triPointer);
	#endif
    
    while(keepGoing)
    {
        Ql_GetEvent(&g_event);
        switch(g_event.eventType)
        {
            case EVENT_INTR:
            {
                // TODO: add your codes for interruption handling here 
                Intr_Event* pIntrEvt = (Intr_Event *)&g_event.eventData.intr_evt;
                Ql_DebugTrace("pinName = %d, pinState = %d", pIntrEvt->pinName, (u8)pIntrEvt->pinState);
                break;
            }
            case EVENT_KEY:
            {
                // TODO: add your codes for pressing key event here
                break;
            }
            case EVENT_UARTREADY:
            {
                // TODO: continue sending data to UART
                break;
            }
            case EVENT_UARTDATA:
            {
                // TODO: receive and handle data from UART

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
                    break;
                }
                break;
            }
            case EVENT_MODEMDATA:
            {
                // TODO: receive and hanle data from CORE through virtual modem port
                PortData_Event* pPortEvt = (PortData_Event*)&g_event.eventData.modemdata_evt;
                Ql_DebugTrace("Modem Data at vPort [%d]: %s\r\n", pPortEvt->port, pPortEvt->data);
                break;
            }
            case EVENT_TIMER:
            {
                // TODO: specify what you want to happen when the interval for timer elapes

                //Ql_DebugTrace("The timer raises for %d time(s).\r\n", ++cnt);
                Ql_memset(notes, 0x0, Ql_strlen(notes));
                Ql_sprintf(notes, "The timer raises for %d time(s).\r\n", ++cnt);
                Ql_DebugTrace(notes);

                // Start the timer again
                if (cnt < 10)
                {
                    Ql_StartTimer(&tm);
                }
                else
                {
                    Ql_DebugTrace("\r\nThe timer stops.\r\n");
                }
                break;
            }
            case EVENT_SERIALSTATUS:
            {
                // TODO: specify what you want to happen when the status of CTS or DCD of serial port changes (now only for UART1)
                PortStatus_Event* pPortStatEvt = (PortStatus_Event *)&g_event.eventData.portstatus_evt;
                Ql_DebugTrace("Serial Status, port:%d, type:%d, value:%s.\r\n", pPortStatEvt->port, pPortStatEvt->type, pPortStatEvt->val);
                break;
            }
            case EVENT_MSG:
            {
                // TODO: specify what you want to happen when the message from other task arrives
                break;
            }
            case EVENT_POWERKEY:
            {
                // TODO: specify what you want to happen when the power key is pressed down
                break;
            }
            case EVENT_HEADSET:
            {
                // TODO: specify what you want to happen when earphone is plugged in device or plugged out from device
                break;
            }
            case EVENT_UARTESCAPE:
            {
                // TODO: specify what you want to happen when the Escape function happens
                break;
            }
            case EVENT_UARTFE:
            {
                // TODO: specify what you want to do when the EVENT_UARTFE event arrives
                break;
            }
            
            default:
                break;
        }
    }
}

#endif // __EXAMPLE_HELLWORLD__

