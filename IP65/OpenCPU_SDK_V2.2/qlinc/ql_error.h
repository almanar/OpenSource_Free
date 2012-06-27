/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2010 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 |
 | File Description
 | ----------------
 |      Error Code Definition
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by    :    Jay XIN
 |  Coded by       :    Jay XIN
 |  Tested by      :    Stanley YONG
 |
 \=========================================================================*/
 
#ifndef __QL_ERROR_H__
#define __QL_ERROR_H__

/****************************************************************************
 * Error Code Definition
 ***************************************************************************/
enum {
    QL_RET_OK           = 0,
    QL_RET_ERR_PARAM = -1,
    QL_RET_ERR_PORT_NOT_OPEN = -2,
    QL_RET_ERR_TIMER_FULL =  -3,
    QL_RET_ERR_INVALID_TIMER = -4,
    QL_RET_ERR_FATAL = -5,
    QL_RET_ERR_INVALID_OP = -6,
    QL_RET_ERR_UART_BUSY = -7,    
    QL_RET_ERR_INVALID_PORT = -8,    
    QL_RET_ERR_NOMATCHVERSION        = -9,
    QL_RET_ERR_NOSUPPORTPIN      = -10,/*pin and bus error*/
    QL_RET_ERR_NOSUPPORTMODE      = -11,
    QL_RET_ERR_NOSUPPORTEINT      = -12,
    QL_RET_ERR_NOSUPPORTSET      = -13,
    QL_RET_ERR_NOSUPPORTGET      = -14,
    QL_RET_ERR_NOSUPPORTCONTROL      = -15,
    QL_RET_ERR_PINALREADYSUBCRIBE      = -16,
    QL_RET_ERR_BUSSUBBUSY      = -17,
    QL_RET_ERR_NOGPIOMODE      = -18,
    QL_RET_ERR_NORIGHTOPERATE      = -19,
    QL_RET_ERR_ALREADYUNSUBCRIBE      = -20,
    QL_RET_ERR_FULLI2CBUS      = -21,
    QL_RET_ERR_NOTSUPPORTBYHANDLE      = -22,
    QL_RET_ERR_INVALIDBUSHANDLE      = -23,
    QL_RET_ERR_NOEXISTOBJEXT      = -24,/*flash operate*/
    QL_RET_ERR_OPERATEOBJEXTFAILED      = -25,
    QL_RET_ERR_OPENOBJEXTFAILED      = -26,
    QL_RET_ERR_WRITEOBJEXTFAILED      = -27,
    QL_RET_ERR_READOBJEXTFAILED      = -28,
    QL_RET_ERR_FLASHFULLOVER     = -29,    /*flash full over*/
    QL_RET_ERR_FLASHSPACE              = -30,
    QL_RET_ERR_DRIVE                        = -31,
    QL_RET_ERR_DRIVEFULLOVER        = -32,
    QL_RET_ERR_INVALIDFLASHID      = -33,
    QL_RET_ERR_I2CHWFAILED      = -34,
    QL_RET_ERR_FILEFAILED      = -35,
    QL_RET_ERR_FILEOPENFAILED      = -36,
    QL_RET_ERR_FILENAMETOOLENGTH = -37,
    QL_RET_ERR_FILEREADFAILED = -38,
    QL_RET_ERR_FILEWRITEFAILED  = -39,
    QL_RET_ERR_FILESEEKFAILED  = -40,
    QL_RET_ERR_FILENOTFOUND  = -41,
    QL_RET_ERR_FILENOMORE  = -42,
    QL_RET_ERR_FILEDISKFULL = -43,
    QL_RET_ERR_INVALID_BAUDRATE = -44,
    QL_RET_ERR_API_NO_RESPONSE = -45,
    QL_RET_ERR_API_INVALID_RESPONSE = -46,
    QL_RET_ERR_SMS_EXCEED_LENGTH =-47,
    QL_RET_ERR_SMS_NOT_INIT = -48,
    QL_RET_ERR_INVALID_TASK_ID = -49,   
    QL_RET_ERR_NOT_IN_BASIC_MODE = -50,
    QL_RET_ERR_INVALID_PARAMETER = -51,
    QL_RET_ERR_PATHNOTFOUND = -52,
    QL_RET_ERR_GET_MEM = -53,
    QL_RET_ERR_GENERAL_FAILURE = -54,
    QL_RET_ERR_FILE_EXISTS = -55,
    QL_RET_ERR_SMS_INVALID_FORMAT = -56,    
    QL_RET_ERR_SMS_GET_FORMAT = -57,        
    QL_RET_ERR_SMS_INVALID_STORAGE = -58,        
    QL_RET_ERR_SMS_SET_STORAGE = -59,            
    QL_RET_ERR_SMS_SEND_AT_CMD = -60,
    QL_RET_ERR_API_CMD_BUSY = -61,    


    Ql_RET_ERR_UNKOWN = -9999,
    Ql_RET_NOT_SUPPORT = -10000,
};

#endif // End-of QL_ERROR_H 

