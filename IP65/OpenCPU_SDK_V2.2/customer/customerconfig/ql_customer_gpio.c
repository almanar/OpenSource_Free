/***************************************************************************
 *
 *            Quectel Open CPU
 *
 *           Copyright (c) 2009 The Quectel Ltd.
 *
 ***************************************************************************
 *
 *  Designed by    :  Jay xin
 *  Coded by       :   Jay xin
 *  Tested by      :
 *
 ***************************************************************************
 *
 * File Description
 * ----------------
 *
 *
 ***************************************************************************
 *
 ***************************************************************************/


/****************************************************************************
 * Nested Include Files
 ***************************************************************************/
#include "Ql_type.h"
#include "ql_pin.h"
#include "ql_audio.h"

#ifndef __QUECTEL_PROJECT_NAME_M10__
#define __QUECTEL_PROJECT_NAME_M10__
#endif

/****************************************************************************
 * Constants
 ***************************************************************************/


/****************************************************************************
 * Type Definitions
 ***************************************************************************/


/****************************************************************************
 *  Macros
 ***************************************************************************/


/****************************************************************************
 *  Function Prototypes
 ***************************************************************************/
const QlCustomerConfig Customer_user_qlconfig =
{
    /******************************************************************
    * handfree config
    * if user want to control PA with GPIO, user must config gpio mode and output direction in Customer_QlPinConfigTable
    * example , use QL_PINNAME_GPIO1 to control PA, user must config below in Customer_QlPinConfigTable
    *{ {QL_PINNAME_GPIO1, QL_PINSUBSCRIBE_UNSUB, QL_PINMODE_1, QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW,0} 
    *******************************************************************/
    QL_PINNAME_MAX, // this GPIO control PA when Handfree
                                // if config QL_PINNAME_MAX, indicate do not control PA
    QL_MIC_MIC1, // QL_MIC_MIC2 indicate use MIC2 when handfree, QL_MIC_MIC1 indicate use MIC1 when handfree

    /******************************************************************
    *headset config
    * if user want to detect headset plugin or  plugout with GPIO, this GPIO must have eint function, 
    * user must config this GPIO to gpio mode and input direction in Customer_QlPinConfigTable
    * example , use QL_PINNAME_GPIO0 to detect headset , user must config below in Customer_QlPinConfigTable
    *{QL_PINNAME_GPIO0,QL_PINSUBSCRIBE_UNSUB,QL_PINMODE_1,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_IN,0,0} 
    *******************************************************************/
    QL_PINNAME_MAX, /* this GPIO must has input function, EINT function, to detect headset plugin or plugout
                     * if config QL_PINNAME_MAX, indicate do not detect headset, and QL_ADC_ADC0 config is invalide at below
                     */
    FALSE,                  // not capture headset ADC value
    100,                        // headset plugin or plugout eint debounce handle, unit is 10ms, so , here is 1S
    QL_ADC_MAX,     //QL_ADC_ADC0 indicate use adc0 to detect headset, 0 adc0,QL_ADC_TEMP_BAT indicate use adc0 to detect headset, 0 TEMP_BAT
    2800000,               //this is high level value,  level rang of high level value an when low level value  headset plugin
    500000,                 // this is low level value,  , level rang of high level value an when low level value  headset plugin
    300000,                 // max level value when headset plugin and press sendkey

    //<<< power config
    TRUE,       // FALSE indicate user to power on, TRUE indicate module core auto to power on
    TRUE,       // FALSE indicate user to power off, TRUE indicate module core auto to power off
    //end-of 'power config' >>>

    //<<< UART config
    TRUE,   /* bool uart3supportvfifo;// TRUE indicate VFIFO, to use vfifo functions, example  Ql_UartSetVfifoThreshold,Ql_UartMaxGetVfifoThresholdInfo, Ql_UartGetVfifoThresholdInfo
                     * FALSE indicate Generic uart driver and , to use Ql_UartSetGenericThreshold, Ql_UartGenericClearFEFlag, EVENT_UARTFE
                     */
    // end-of 'uart config' >>>
};

const QlPinConfigTable Customer_QlPinConfigTable =
{
    QL_PIN_VERSION,/*now this version is 0x100*/
    QL_OPENCPU_FLAG,
    {//pinconfigitem[END_OF_QL_PINNAME+1]
    {QL_PINNAME_NETLIGHT,               QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,     QL_PINPULLENABLE_ENABLE,0,0,0},
    //{QL_PINNAME_NETLIGHT,           QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_2,     QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_HIGH,0},/*config GPIO function*/
    //{QL_PINNAME_NETLIGHT,          QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_3,     QL_PINPULLENABLE_ENABLE,0,0,0},/*config PWM other parameter, please use Ql_pinSubscribe(...) function to config*/
    {QL_PINNAME_SIM_PRESENCE,   QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,     QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,0,0}, /*config GPIO function*/
    {QL_PINNAME_KBR0,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_KBR1,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_KBR2,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_KBR3,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_KBR4,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_KBC0,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_KBC1,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_KBC2,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_KBC3,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_KBC4,                       QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET, 0,0,0,0},
    {QL_PINNAME_BUZZER,                 QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,     QL_PINPULLENABLE_ENABLE,0,0,0},
    {QL_PINNAME_DSR,                        QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,     QL_PINPULLENABLE_ENABLE,0,0,0},
    {QL_PINNAME_RI,                             QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,     QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,0,0},
    {QL_PINNAME_CTS,                        QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,     QL_PINPULLENABLE_ENABLE,0,0,0},
    {QL_PINNAME_RTS,                        QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,     QL_PINPULLENABLE_DISABLE,0,0,0},/*config rts, must set QL_PINPULLENABLE_DISABLE*/
    {QL_PINNAME_DTR,                        QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,     QL_PINPULLENABLE_ENABLE,0,0,0},

    /***********************************************
    *  For M10
    ***********************************************/
#if (defined(__QUECTEL_PROJECT_NAME_M10__))
    {QL_PINNAME_M10_DISP_DATA,      QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET,  0,0,0,0},
    {QL_PINNAME_M10_DISP_CLK,          QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET,  0,0,0,0},
    {QL_PINNAME_M10_DISP_CS,            QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET,  0,0,0,0},
    {QL_PINNAME_M10_DISP_DC,            QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET,  0,0,0,0},
    {QL_PINNAME_M10_DISP_RST,           QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_UNSET,  0,0,0,0},
    {QL_PINNAME_M10_GPIO0,                  QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,      QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW,0},
    //{QL_PINNAME_M10_GPIO0,              QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_2,      QL_PINPULLENABLE_ENABLE,0,0,0},/*config EINT other parameter, please use Ql_pinSubscribe(...) function to config*/
    //{QL_PINNAME_M10_GPIO1,             QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_3,     QL_PINPULLENABLE_ENABLE,0,0,0},/*config CLOCK other parameter, please use Ql_pinSubscribe(...) function to config*/
    {QL_PINNAME_M10_GPIO1,                  QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,      QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW,0},
    {QL_PINNAME_M10_DCD,                    QL_PINSUBSCRIBE_UNSUB,    QL_PINMODE_1,      QL_PINPULLENABLE_ENABLE,0,0,0},

    /***********************************************
    *  For M30
    ***********************************************/
#elif (defined(__QUECTEL_PROJECT_NAME_M30__))
    {QL_PINNAME_M30_SPI_IO,         QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_OUT, QL_PINLEVEL_LOW,  0},
    {QL_PINNAME_M30_SPI_CLK,        QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_OUT, QL_PINLEVEL_LOW,  0},
    {QL_PINNAME_M30_SPI_CS,         QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_OUT, QL_PINLEVEL_HIGH, 0},
    {QL_PINNAME_M30_SPI_I,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_OUT, QL_PINLEVEL_LOW,  0},
    {QL_PINNAME_M30_SCL,            QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_HIGH, 0},
    {QL_PINNAME_M30_SDA,            QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_OUT, QL_PINLEVEL_HIGH, 0},
    {QL_PINNAME_M30_PCM_CLK,        QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_HIGH, 0},
    {QL_PINNAME_M30_PCM_IN,         QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_HIGH, 0},
    {QL_PINNAME_M30_PCM_SYNC,       QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_HIGH, 0},
    {QL_PINNAME_M30_PCM_OUT,        QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_LOW,  0},
    {QL_PINNAME_M30_GPIO0,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_LOW,  0},
    {QL_PINNAME_M30_STATUS,         QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_LOW,  0},
    {QL_PINNAME_M30_DCD,            QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_HIGH, 0},

    {QL_PINNAME_M30_GPIO2,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_HIGH, 0},
    {QL_PINNAME_M30_GPIO3,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_HIGH, 0},
    {QL_PINNAME_M30_EINT1,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   QL_PINPULLENABLE_ENABLE, QL_PINDIRECTION_IN,  QL_PINLEVEL_HIGH, 0},
    {QL_PINNAME_M30_EINT0,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   0,0,0,0},
    {QL_PINNAME_M30_LED_B,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   0,0,0,0},
    {QL_PINNAME_M30_LED_G,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   0,0,0,0},
    {QL_PINNAME_M30_LED_R,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   0,0,0,0},
    {QL_PINNAME_M30_VIB,            QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   0,0,0,0},
#endif // End-of '__QUECTEL_PROJECT_NAME_M30__'

    {QL_PINNAME_LIGHT_MOS,          QL_PINSUBSCRIBE_UNSUB,  QL_PINMODE_1,   0,0,0,0},
    {QL_PINNAME_MAX,                    QL_PINSUBSCRIBE_UNSUB,             QL_PINMODE_UNSET,                  0,0,0,0} /*must end item is QL_PINNAME_MAX*/
    }
};

/* END OF FILE */

