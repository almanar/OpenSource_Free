#ifdef __EXAMPLE_LCD__
  
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_pin.h"
#include "ql_bus.h"
#include "ql_fcm.h"


QL_BUS_HANDLE busLcd = (QL_BUS_HANDLE)-1;

void ql_entry()
{
    bool           keepGoing = TRUE;
    QlEventBuffer  flSignalBuffer;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("lcd: ql_entry\r\n");
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
                    s32 pinpullenable = 0;
                    s32 pindirection = 0;
                    s32 pinlevel = 0;
                    s32 iret;

                    pData = (char*)flSignalBuffer.eventData.uartdata_evt.data;
                    p = Ql_strstr(pData,"s");
                    if (p)
                    {
                        QlBusParameter busparameter;
                        busparameter.busconfigversion = QL_BUS_VERSION;
                        busparameter.busparameterunion.lcdparameter.brequire_cs = 1;
                        busparameter.busparameterunion.lcdparameter.brequire_resetpin = 1;
                        /*following selcet 3.25M clock, cs low valid, clock rising edge  sampling  data*/
                        busparameter.busparameterunion.lcdparameter.lcd_serial_config = 
                            QL_LCD_SERIAL_CONFIG_13MHZ_CLK | QL_LCD_SERIAL_CONFIG_CLOCK_DIVIDE_3
                            /*| QL_LCD_SERIAL_CONFIG_CS_POLARITY*/
                            | QL_LCD_SERIAL_CONFIG_8_BIT_MODE;
                            /*QL_LCD_SERIAL_CONFIG_CLOCK_PHASE | QL_LCD_SERIAL_CONFIG_CLOCK_POLARITY;*/

                        if(busLcd > 0)                                    
                        {
                            Ql_sprintf(buffer, "\r\nAlready Subscribe LCD\r\n",iret);
                            Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                            break;
                        }
                        
                        busLcd = Ql_busSubscribe(QL_BUSTYPE_LCD, &busparameter);
                        if(busLcd < 0)                                    
                        {
                            Ql_sprintf(buffer, "\r\nSubscribe LCD Failed=%d\r\n",busLcd);
                            Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                            break;
                        }
                
                        Ql_sprintf(buffer, "\r\nSubscribe LCD succcess busLcd =%x\r\n", busLcd );
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }

                    p = Ql_strstr(pData,"u");
                    if (p)
                    {
                        iret = Ql_busUnSubscribe(busLcd);
                        busLcd = (QL_BUS_HANDLE)-1;
                        Ql_sprintf(buffer, "\r\nUnSubscribe(%d)\r\n",iret);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }                        
                    p = Ql_strstr(pData,"c");
                    if (p)
                    {
                        QlBusAccess access;
                        u8 data[3] = {0x11,0x22,0xaa};
                        access.opcode = QL_BUSACCESSOPCODE_LCD_CMDWRITE;
                        iret = Ql_busWrite(busLcd, &access, data, 3);
                        Ql_sprintf(buffer, "\r\nLcd Write Cmd, ret=%d,busLcd=%x\r\n",iret,busLcd);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }  
                    p = Ql_strstr(pData,"d");
                    if (p)
                    {
                        QlBusAccess access;
                        u8 data[3] = {0x11,0x22,0xaa};
                        access.opcode = QL_BUSACCESSOPCODE_LCD_DATAWRITE;
                        iret = Ql_busWrite(busLcd, &access, data, 3);
                        Ql_sprintf(buffer, "\r\nLcd Write Data, ret=%d,busLcd=%x\r\n",iret,busLcd);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }  
                    p = Ql_strstr(pData,"0");
                    if (p)
                    {
                        QlBusAccess access;
                        u8 data[3] = {0x11,0x22,0xaa};
                        access.opcode = QL_BUSACCESSOPCODE_LCD_CLEAR_RESET;
                        iret = Ql_busWrite(busLcd, &access, NULL, 0);
                        Ql_sprintf(buffer, "\r\nLcd Clean Reset, ret=%d,busLcd=%x\r\n",iret,busLcd);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }  
                    p = Ql_strstr(pData,"1");
                    if (p)
                    {
                        QlBusAccess access;
                        u8 data[3] = {0x11,0x22,0xaa};
                        access.opcode = QL_BUSACCESSOPCODE_LCD_SET_RESET;
                        iret = Ql_busWrite(busLcd, &access, NULL, 0);
                        Ql_sprintf(buffer, "\r\nLcd Set Reset, ret=%d,busLcd=%x\r\n",iret,busLcd);
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

#endif // __EXAMPLE_LCD__

