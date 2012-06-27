#ifdef __EXAMPLE_ALERT__
  
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
    Ql_DebugTrace("alert: ql_entry\r\n");
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

                    //command-->select QL_PINNAME_BUZZER pin function to mode2, it is gpio function
                    p = Ql_strstr(pData,"2");
                    if (p)
                    {
                        iret = Ql_pinSubscribe(QL_PINNAME_BUZZER, QL_PINMODE_2, NULL);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d\r\n",iret,QL_PINNAME_BUZZER,QL_PINMODE_2);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }
                    
                    //command-->select QL_PINNAME_BUZZER pin function to mode3, it is alert function
                    p = Ql_strstr(pData,"3");
                    if (p)
                    {
                        QlPinParameter pinparameter;
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.alertparameter.alertclock = QL_ALERTCLOCK_13DIV8M;
                        pinparameter.pinparameterunion.alertparameter.alertmode = QL_ALERTMODE_1;
                        pinparameter.pinparameterunion.alertparameter.alertcounter1 = 20;
                        pinparameter.pinparameterunion.alertparameter.alertcounter2 = 1400;
                        pinparameter.pinparameterunion.alertparameter.alertthreshold = 600;
                        iret = Ql_pinSubscribe(QL_PINNAME_BUZZER, QL_PINMODE_3, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d\r\n",iret,QL_PINNAME_BUZZER,QL_PINMODE_3);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }
                    
                    /* if (p)
                    {
                        QlPinParameter pinparameter;
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.alertparameter.alertclock = QL_ALERTCLOCK_13DIV8M;
                        pinparameter.pinparameterunion.alertparameter.alertmode = QL_ALERTMODE_1;
                        pinparameter.pinparameterunion.alertparameter.alertcounter1 = 600;
                        pinparameter.pinparameterunion.alertparameter.alertcounter2 = 1200;
                        pinparameter.pinparameterunion.alertparameter.alertthreshold = 400;
                        iret = Ql_pinSubscribe(QL_PINNAME_BUZZER, QL_PINMODE_3, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d\r\n",iret,QL_PINNAME_BUZZER,QL_PINMODE_3);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }*/
                    
                    //command-->unsubscribe
                    p = Ql_strstr(pData,"u");
                    if (p)
                    {
                        iret = Ql_pinUnSubscribe(QL_PINNAME_BUZZER);
                        Ql_sprintf(buffer, "\r\nUnSubscribe(%d),pin=%d\r\n",iret,QL_PINNAME_BUZZER);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }
                    
                    //command-->control start alert
                    p = Ql_strstr(pData,"s");
                    if (p)
                    {
                        iret = Ql_pinControl(QL_PINNAME_BUZZER, QL_PINCONTROL_START);
                        Ql_sprintf(buffer, "\r\nStart(%d),pin=%d\r\n",iret,QL_PINNAME_BUZZER);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }
                    
                    //command-->control stop alert
                    p = Ql_strstr(pData,"t");
                    if (p)
                    {
                        iret = Ql_pinControl(QL_PINNAME_BUZZER, QL_PINCONTROL_STOP);
                        Ql_sprintf(buffer, "\r\nStop(%d),pin=%d\r\n",iret,QL_PINNAME_BUZZER);
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

#endif // __EXAMPLE_ALERT__

