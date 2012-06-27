/***************************************************************************************************
*   Example:
*       
*           Keyboard Routine
*
*   Description:
*
*           This example gives an example for handling key event.
*           While recevied key event from core, this example will print the key information through uart port.
*
*   Usage:
*
*           Precondition:
*
*                   1. Use "make/make128 keyboard" to compile, and download bin image to module.
*                   2. Turn off D/L after the app starts up.
*
*           Press a key on board(if M1O board, the pins related to keybad are pin28~37, among them, pin28~32  are KEYR related, 
*           pin 33~37 are KEYC related. Make a pin from KEYR connected to a pin from KEYC, that will produce a key event), the 
*           information of the key will be printed through uart port.
*
****************************************************************************************************/
#ifdef __EXAMPLE_KEYBOARD__                                            //if used "make keyboard" to compile, this macro will be defined
  
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_pin.h"
#include "ql_fcm.h"

typedef struct
{
    char pinName[50];
    u16  pinNo;
}ST_Pin;

void queryPinStatus();

void ql_entry()
{
    bool                    keepGoing = TRUE;
    QlEventBuffer   flSignalBuffer;
    s32 period = 10;
    char buffer[100];
    char *pData, *p;

    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("keyboard: ql_entry\r\n");
    Ql_OpenModemPort(ql_md_port1);	   

    queryPinStatus();
    
    while(keepGoing)
    {
        Ql_GetEvent(&flSignalBuffer);
        switch(flSignalBuffer.eventType)
        {
            case EVENT_KEY:
            {
                Key_Event* pKeyEvt = &flSignalBuffer.eventData.key_evt;
                Ql_sprintf(buffer, "\r\nEVENT_KEY: key_val(%x), isPressed=%d\r\n", pKeyEvt->key_val, pKeyEvt->isPressed);
                Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));  
                break;
            }

            case EVENT_POWERKEY:
            {
                Powerkey_Event* pPwrkeyEvt = &flSignalBuffer.eventData.powerkey_evt;
                char* pwrKeyOn = "POWERKEY_ON";
                char* pwrKeyOff = "POWERKEY_OFF";
                if (POWERKEY_ON == pPwrkeyEvt->powerkey)
                {
                    Ql_sprintf(buffer, "\r\nEVENT_KEY: POWERKEY_ON");
                }
                else
                {
                    Ql_sprintf(buffer, "\r\nEVENT_KEY: POWERKEY_OFF");
                }
                if (pPwrkeyEvt->isPressed)
                {
                    Ql_sprintf(buffer + Ql_strlen(buffer), ", Press Key Down\r\n");
                }
                else
                {
                    Ql_sprintf(buffer + Ql_strlen(buffer), ", Release Key\r\n");
                }
                Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));  
                break;
            }

            default:
                break;
        }
    }
}

void queryPinStatus()
{
    QlPinSubscribe subscribe;
    QlPinMode wrkMode;
    QlPinParameter pinParam;
    ST_Pin pins[10];
    char notes[100];
    int i;

    // Fill in pins
    Ql_sprintf(pins[0].pinName, "QL_PINNAME_KBR0");
    pins[0].pinNo= QL_PINNAME_KBR0;
    Ql_sprintf(pins[1].pinName, "QL_PINNAME_KBR1");
    pins[1].pinNo= QL_PINNAME_KBR1;
    Ql_sprintf(pins[2].pinName, "QL_PINNAME_KBR2");
    pins[2].pinNo= QL_PINNAME_KBR2;
    Ql_sprintf(pins[3].pinName, "QL_PINNAME_KBR3");
    pins[3].pinNo= QL_PINNAME_KBR3;
    Ql_sprintf(pins[4].pinName, "QL_PINNAME_KBR4");
    pins[4].pinNo= QL_PINNAME_KBR4;
    Ql_sprintf(pins[5].pinName, "QL_PINNAME_KBC0");
    pins[5].pinNo= QL_PINNAME_KBC0;
    Ql_sprintf(pins[6].pinName, "QL_PINNAME_KBC1");
    pins[6].pinNo= QL_PINNAME_KBC1;
    Ql_sprintf(pins[7].pinName, "QL_PINNAME_KBC2");
    pins[7].pinNo= QL_PINNAME_KBC2;
    Ql_sprintf(pins[8].pinName, "QL_PINNAME_KBC3");
    pins[8].pinNo= QL_PINNAME_KBC3;
    Ql_sprintf(pins[9].pinName, "QL_PINNAME_KBC4");
    pins[9].pinNo= QL_PINNAME_KBC4;

    // Query the status of all multifunction pins
    for (i = 0; i <  (sizeof(pins) / sizeof(pins[0])); i++)
    {
        Ql_pinQueryMode(pins[i].pinNo, &subscribe, &wrkMode, &pinParam);
        Ql_sprintf(notes, "%s: subscribe:%d, pinMode:%d\r\n", pins[i].pinName, subscribe, wrkMode);
        Ql_SendToUart(ql_uart_port1, (u8*)notes, Ql_strlen(notes));  
    }
}

#endif //__EXAMPLE_KEYBOARD__

