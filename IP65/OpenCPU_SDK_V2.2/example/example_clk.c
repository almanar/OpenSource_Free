#ifdef __EXAMPLE_CLK__
  
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_pin.h"
#include "ql_fcm.h"

void ql_entry()
{
    bool           keepGoing = TRUE;
    QlEventBuffer  flSignalBuffer;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("clk: ql_entry\r\n");
    Ql_OpenModemPort(ql_md_port1);       

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer);
        switch(flSignalBuffer.eventType)
        {
            case EVENT_UARTDATA:
            {
                if (flSignalBuffer.eventData.uartdata_evt.len>0)
                {
                    s32 pin;
                    s32 mod = 0;
                    s32 iret;

                    pData = (char*)flSignalBuffer.eventData.uartdata_evt.data;

                    //command-->select QL_PINNAME_GPIO1 pin function to mode1, it is gpio function
                    p = Ql_strstr(pData,"1");
                    if (p)
                    {
                        iret = Ql_pinSubscribe(QL_PINNAME_M10_GPIO1, QL_PINMODE_1, NULL);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d\r\n",iret,QL_PINNAME_M10_GPIO1,QL_PINMODE_1);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }

                    //command-->select QL_PINNAME_GPIO1 pin function to mode3, it is clk function
                    p = Ql_strstr(pData,"3");
                    if (p)
                    {
                        QlPinParameter pinparameter;
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.clockparameter.clocksource = QL_CLOCKSOURCE_32K;
                        iret = Ql_pinSubscribe(QL_PINNAME_M10_GPIO1, QL_PINMODE_3, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d\r\n",iret,QL_PINNAME_M10_GPIO1,QL_PINMODE_3);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }                        

                    //command-->unsubscribe
                    p = Ql_strstr(pData,"u");
                    if (p)
                    {
                        iret = Ql_pinUnSubscribe(QL_PINNAME_M10_GPIO1);
                        Ql_sprintf(buffer, "\r\nUnSubscribe(%d),pin=%d\r\n",iret,QL_PINNAME_M10_GPIO1);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }  

                    //command-->control start clk
                    p = Ql_strstr(pData,"s");
                    if (p)
                    {
                        iret = Ql_pinControl(QL_PINNAME_M10_GPIO1, QL_PINCONTROL_START);
                        Ql_sprintf(buffer, "\r\nStart(%d),pin=%d\r\n",iret,QL_PINNAME_M10_GPIO1);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }  

                    //command-->control stop clk
                    p = Ql_strstr(pData,"t");
                    if (p)
                    {
                        iret = Ql_pinControl(QL_PINNAME_M10_GPIO1, QL_PINCONTROL_STOP);
                        Ql_sprintf(buffer, "\r\nStop(%d),pin=%d\r\n",iret,QL_PINNAME_M10_GPIO1);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }  
                }
                break;            
            }

            default:
                break;
        }
    }
}

#endif // __EXAMPLE_CLK__

