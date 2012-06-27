#ifdef __EXAMPLE_LED__
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_pin.h"
#include "ql_fcm.h"


void Callback_Read_ADC(QlADCPin adc_pin, u8 status, u16 adc_val);

QlEventBuffer  g_event;
void ql_entry()
{
    bool keepGoing = TRUE;
    char buffer[100];
    char *pData, *p;
    s32 ret;

    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("OpenCPU: LED\r\n");
    Ql_OpenModemPort(ql_md_port1);       
     
    while(keepGoing)
    {    
        Ql_GetEvent(&g_event);
        switch(g_event.eventType)
        {
            case EVENT_UARTDATA:
            {
                PortData_Event* pPortEvt = (PortData_Event*)&g_event.eventData.uartdata_evt;
                QlPinName pin_name = QL_PINNAME_MAX;
                s32 iret;
                
                pData = (char*)pPortEvt->data;
                if (pPortEvt->len <= 0)
                {
                    Ql_DebugTrace("pPortEvt->len <= 0\r\n");
                    break;
                }

                // Subscribe (feed content: led_r/b/vib/mos)
                p = Ql_strstr(pData, "led_");
                if (p)
                {
                    Ql_DebugTrace(pData);
                    if (Ql_strcmp(pData, "led_r\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_M30_LED_R;
                    }
                    else if (Ql_strcmp(pData, "led_g\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_M30_LED_G;
                    }
                    else if (Ql_strcmp(pData, "led_b\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_M30_LED_B;
                    }
                    else if (Ql_strcmp(pData, "led_vib\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_M30_VIB;
                    }
                    else if (Ql_strcmp(pData, "led_mos\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_LIGHT_MOS;
                    }
                    else
                    {
                        Ql_DebugTrace("No pin name matches!\r\n");
                        break;
                    }
                    iret = Ql_pinSubscribe(pin_name, QL_PINMODE_1, NULL);    
                    Ql_sprintf(buffer, "Ql_pinSubscribe(pin=%d,mod=%d)=%d\r\n", pin_name, QL_PINMODE_1, iret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));  
                    break;
                }
                
                // Unsubscribe (feed content: unsub=r/b/vib/mos)
                p = Ql_strstr(pData, "unsub=");
                if (p)
                {
                    Ql_DebugTrace(pData);
                    p = Ql_strstr(pData, "=");
                    Ql_memset(buffer, 0x0, Ql_strlen(buffer));
                    Ql_strcpy(buffer, p + 1);
                    if (Ql_strcmp(buffer, "r\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_M30_LED_R;
                    }
                    else if (Ql_strcmp(buffer, "g\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_M30_LED_G;
                    }
                    else if (Ql_strcmp(buffer, "b\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_M30_LED_B;
                    }
                    else if (Ql_strcmp(buffer, "vib\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_M30_VIB;
                    }
                    else if (Ql_strcmp(buffer, "mos\r\n") == 0)
                    {
                        pin_name = QL_PINNAME_LIGHT_MOS;
                    }
                    else
                    {
                        Ql_DebugTrace("No pin name matches!\r\n");
                        break;
                    }
                    iret = Ql_pinUnSubscribe(pin_name);
                    Ql_sprintf(buffer, "Ql_pinUnSubscribe(pin=%d)=%d\r\n", iret, pin_name);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));  
                    break;
                }
                
                // Control (feed content: led=r/b/vib/mos,false/true)
                p = Ql_strstr(pData, "led=");
                if (p)
                {
                    char* p2 = NULL;
                    u8 ctrl = END_OF_QL_PINCONTROL;

                    Ql_DebugTrace(pData);

                    // Get pin
                    p = Ql_strstr(pData, "=");
                    p2 = Ql_strstr(pData, ",");
                    Ql_memset(buffer, 0x0, Ql_strlen(buffer));
                    Ql_strncpy(buffer, p + 1, p2 - p - 1);
                    if (Ql_strcmp(buffer, "r") == 0)
                    {
                        pin_name = QL_PINNAME_M30_LED_R;
                    }
                    else if (Ql_strcmp(buffer, "g") == 0)
                    {
                        pin_name = QL_PINNAME_M30_LED_G;
                    }
                    else if (Ql_strcmp(buffer, "b") == 0)
                    {
                        pin_name = QL_PINNAME_M30_LED_B;
                    }
                    else if (Ql_strcmp(buffer, "vib") == 0)
                    {
                        pin_name = QL_PINNAME_M30_VIB;
                    }
                    else if (Ql_strcmp(buffer, "mos") == 0)
                    {
                        pin_name = QL_PINNAME_LIGHT_MOS;
                    }
                    else
                    {
                        Ql_DebugTrace("No pin name matches!\r\n");
                        break;
                    }

                    // Get enable
                    //p = p2;
                    //p2 = Ql_strstr(pData, "\r\n");
                    Ql_memset(buffer, 0x0, Ql_strlen(buffer));
                    Ql_strcpy(buffer, p2 + 1);
                    if (Ql_strcmp(buffer, "true\r\n") == 0)
                    {
                        ctrl = QL_PINCONTROL_START;
                    }
                    else if (Ql_strcmp(buffer, "false\r\n") == 0)
                    {
                        ctrl = QL_PINCONTROL_STOP;
                    }
                    else
                    {
                        Ql_DebugTrace("No control matches!\r\n");
                        break;
                    }
                    iret = Ql_pinControl(pin_name, ctrl);
                    Ql_sprintf(buffer, "Ql_pinControl(pin=%d, ctrl=%d)=%d\r\n", pin_name, ctrl, iret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));  
                    break;
                }

                // Read ADC (feed content: adc<0/1>?)
                p = Ql_strstr(pData, "adc<");
                if (p)
                {
                    QlADCPin adcPin;
                    Ql_DebugTrace(pData);
                    if (Ql_strcmp(pData, "adc<0>?\r\n") == 0)
                    {
                        adcPin = QL_PIN_ADC0;
                    }
                    else if (Ql_strcmp(pData, "adc<1>?\r\n") == 0)
                    {
                        adcPin = QL_PIN_ADC1;
                    }
                    else
                    {
                        Ql_DebugTrace("No adc pin matches!\r\n");
                        break;
                    }
                    ret = Ql_ReadADC(adcPin, Callback_Read_ADC);
                    Ql_DebugTrace("Ql_ReadADC(adcpin=%d,...) = %d\r\n", (u8)adcPin, ret);
                    break;
                }
            }
                
            default:
                break;
        }
    }
}

void Callback_Read_ADC(QlADCPin adc_pin, u8 status, u16 adc_val)
{
    Ql_DebugTrace("adc: %d, status: %d, adcValue: %d\r\n", adc_pin, status, adc_val);
}

#endif // __EXAMPLE_LED__

