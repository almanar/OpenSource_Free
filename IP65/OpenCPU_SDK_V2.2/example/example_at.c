#ifdef __EXAMPLE_AT__
 
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_fcm.h"


typedef enum AT_ResponseTag
{
    INVAT,/*invalid at*/
    CPIN,
    CREG,
    CSQ
}AT_Response;


char * pData=NULL;
u8 atbuffer[100];
char buffer[100];
QlTimer  timer;
QlTimer timer_p;

AT_Response parse_at_response(u8* offset )
{
    char *p;

    if (!pData)
        return INVAT;
    
    p = Ql_strstr(pData,"+CPIN:");
    if (p)
    {    
        *offset=p-pData+6;
        return CPIN;
    }

    p = Ql_strstr(pData,"+CREG:");
    if (p)
    {
        *offset=p-pData+6;
        return CREG;                
    }

    p = Ql_strstr(pData,"+CSQ:");
    if (p)
    {
        *offset=p-pData+5;
        return CSQ;                
    }

    *offset = 0;
    return INVAT;
}
    
void custom_at_handler(void)
{
    char *p=NULL;
    AT_Response response = INVAT;
    u8 off;

    if (!pData)
        return;

    p = pData;
    response = parse_at_response(&off);
    p+=off;

    switch(response)
    {
        case CPIN:
        {
            if (Ql_strncmp(p," READY",6)==0)
            {
                Ql_sprintf((char *)buffer,"\r\nCPIN is ready\r\n");
                Ql_SendToUart(ql_uart_port1, (u8*)buffer,Ql_strlen(buffer));
                Ql_sprintf((char *)buffer,"\r\nSet timer for 5 second\r\n");
                Ql_SendToUart(ql_uart_port1, (u8*)buffer,Ql_strlen(buffer));
                
                timer.timeoutPeriod = Ql_SecondToTicks(5);
                Ql_StartTimer(&timer);
                
            }else if (Ql_strncmp(p," SIM PIN",8)==0)
            {
                Ql_sprintf((char *)buffer,"\r\nSIM PIN is needed\r\n");
                Ql_SendToUart(ql_uart_port1, (u8*)buffer,Ql_strlen(buffer));
                
                Ql_memset(atbuffer,0,100);
                Ql_sprintf((char *)atbuffer,"AT+CPIN=1234\n");
                Ql_SendToModem(ql_md_port1,atbuffer,Ql_strlen((char *)atbuffer));    
            }
            break;
        }
        
        default:
            break;
    }
}

void ql_entry()
{
    bool           keepGoing = TRUE;
    QlEventBuffer  flSignalBuffer;
    s32 period = 10;

    Ql_DebugTrace("at: ql_entry\r\n");
    
    Ql_SetDebugMode(BASIC_MODE);   
    Ql_OpenModemPort(ql_md_port1);       

    Ql_memset(atbuffer,0,100);
    //Ql_sprintf((char *)atbuffer,"AT+QLOCPU=0\n");
    Ql_sprintf((char *)atbuffer,"AT+CPIN?\n");
    Ql_SendToModem(ql_md_port1,atbuffer,Ql_strlen((char*)atbuffer));    
         
    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer);
        switch(flSignalBuffer.eventType)
        {
            case EVENT_TIMER:
            {
                Ql_sprintf((char *)buffer,"\r\nTimer expired\r\n");
                Ql_SendToUart(ql_uart_port1, (u8*)buffer,Ql_strlen(buffer));

                Ql_memset(atbuffer,0,100);
                Ql_sprintf((char *)atbuffer,"AT+QAUDCH=1\n");
                Ql_SendToModem(ql_md_port1,atbuffer,Ql_strlen((char *)atbuffer));
                //Ql_PlayAudio(QL_AUDIO_EMS_TADA,0);    
                
                break;
            }
            
            case EVENT_MODEMDATA:
            {                
                Ql_DebugTrace("\r\nEVENT_MODEMDATA type=%d \r\n",flSignalBuffer.eventData.modemdata_evt.type);
                if (flSignalBuffer.eventData.modemdata_evt.type==DATA_AT)
                {
                    pData = (char*)flSignalBuffer.eventData.modemdata_evt.data;
                    custom_at_handler();            
                }
                break;
            }
            
            case EVENT_UARTDATA:
            {            
                Ql_DebugTrace("\r\nEVENT_UARTDATA PORT=%d\r\n",flSignalBuffer.eventData.uartdata_evt.port);
                Ql_DebugTrace("\r\nuart data =%s\r\n",flSignalBuffer.eventData.uartdata_evt.data);        
                break;            
            }
            
            case EVENT_SERIALSTATUS:
            {
                bool val = flSignalBuffer.eventData.portstatus_evt.val;
                u8 port = flSignalBuffer.eventData.portstatus_evt.port;
                u8 type = flSignalBuffer.eventData.portstatus_evt.type;

                Ql_DebugTrace("EVENT_SERIALSTATUS port=%d type=%d val=%d\r\n",port,type,val);
                break;
            }
            
            default:
                break;
        }
    }
}

#endif //__EXAMPLE_AT__

