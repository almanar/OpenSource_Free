/*******************************************************
* Example: 
*   At command pipe
*
* Description:
*   This example demonstrates how to establish a pipe
*   between application and core. So, this example only
*   receives AT command from MAIN port and hands it to
*   CORE, or prints responses from CORE to DEBUG port.
*
* Usage:
*   Input AT command termined by '\n' or '\r\n' through
*   UART1(MAIN) port, and the response will be returned
*   through this port.
********************************************************/
#ifdef __ATC_PIPE__
#include "ql_type.h"
#include "ql_appinit.h"
#include "ql_trace.h"
#include "ql_interface.h"
#include "ql_fcm.h"
#include "ql_stdlib.h"

QlEventBuffer g_event;
void ql_entry(void)
{
    Ql_SetDebugMode(BASIC_MODE);
    Ql_DebugTrace("OpenCPU: AT Commands Pipe.\r\n");
    Ql_OpenModemPort(ql_md_port1);
    Ql_SetPortOwner(ql_md_port1, ql_main_task);
    while(TRUE)
    {
        Ql_GetEvent(&g_event);
        switch(g_event.eventType)
        {
            case EVENT_UARTDATA:
            {
                //TODO: receive and handle data from UART
                PortData_Event* pDataEvt = (PortData_Event*)&g_event.eventData.uartdata_evt;
                Ql_SendToModem(ql_md_port1, (u8*)pDataEvt->data, Ql_strlen((ascii*)pDataEvt->data));
                break;
            }
            case EVENT_MODEMDATA:
            {
                //TODO: receive and hanle data from CORE through virtual modem port
                PortData_Event* pPortEvt = (PortData_Event*)&g_event.eventData.modemdata_evt;
                //Ql_DebugTrace("%s", pPortEvt->data);
                Ql_SendToUart(ql_uart_port1, (u8*)pPortEvt->data, pPortEvt->len);
                break;
            }
            
            default:
                break;
        }
    }
}

#endif // __ATC_PIPE__

