/***************************************************************************************************
*   Example:
*       
*           I2C Routine
*
*   Description:
*
*           This example gives an example for i2c operation.
*           Through Uart port, input the special command, there will be given the response about i2c operation.
*
*   Usage:
*
*           Precondition:
*
*                   Use "make/make128 i2c" to compile, and download bin image to module.
*           
*           Through Uart port:
*
*               If input "s", that will subscribe i2c.
*               If input "u", that will unsubscribe.
*               If input "w", that will write data.
*               If input "r", that will read data.
*           
****************************************************************************************************/
#ifdef __EXAMPLE_I2C__
  
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


QL_BUS_HANDLE busi2c = (QL_BUS_HANDLE) - 1;

void ql_entry()
{
    bool           keepGoing = TRUE;
    QlEventBuffer  flSignalBuffer;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("i2c: ql_entry\r\n");
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
                        busparameter.busparameterunion.i2cparameter.pin_i2cdata = QL_PINNAME_M10_DISP_DATA;
                        busparameter.busparameterunion.i2cparameter.pin_i2cclk = QL_PINNAME_M10_DISP_CLK;

                        if(busi2c > 0)                                    
                        {
                            Ql_sprintf(buffer, "\r\nAlready Subscribe I2C\r\n",iret);
                            Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                            break;
                        }
                        
                        busi2c= Ql_busSubscribe(QL_BUSTYPE_I2C, &busparameter);
                        if(busi2c < 0)                                    
                        {
                            Ql_sprintf(buffer, "\r\nSubscribe I2C Failed=%d\r\n",busi2c);
                            Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                            break;
                        }
                
                        Ql_sprintf(buffer, "\r\nSubscribe I2C succcess busi2c =%x\r\n", busi2c );
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }
                    p = Ql_strstr(pData,"u");
                    if (p)
                    {
                        iret = Ql_busUnSubscribe(busi2c);
                        busi2c = (QL_BUS_HANDLE)-1;
                        Ql_sprintf(buffer, "\r\nUnSubscribe(%d)\r\n",iret);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }                        
                    p = Ql_strstr(pData,"w");
                    if (p)
                    {
                        QlBusAccess access;
                        u8 data[3] = {0x11,0x22,0xaa};
                        access.Address = 0x7;
                        // note , do not send stopbit
                        access.opcode = QL_BUSACCESSOPCODE_I2C_NOTSTOPBIT;
                        iret = Ql_busWrite(busi2c, &access, data, 3);
                        Ql_sprintf(buffer, "\r\nI2c Write Data, ret=%d,busi2c=%x\r\n",iret,busi2c);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                        break;
                    }  
                    p = Ql_strstr(pData,"r");
                    if (p)
                    {
                        QlBusAccess access;
                        u8 data[3] = {0x0};
                        access.Address = 0x7;
                        // note , this will send stopbit
                        access.opcode = QL_BUSACCESSOPCODE_I2C_NOTHING;
                        iret = Ql_busRead(busi2c, &access, data, 3);
                        Ql_sprintf(buffer, "\r\nI2c Read Data, ret=%d,busi2c=%x\r\n",iret,busi2c);
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

#endif // __EXAMPLE_I2C__

