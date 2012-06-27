/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2010 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |      Definitions for SMS operations
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by    :    Willis YANG
 |  Coded by       :    Willis YANG
 |  Tested by      :    Stanley YONG
 |
 \=========================================================================*/

#ifndef __QL_SMS_H__
#define __QL_SMS_H__

#include "ql_type.h"
#include "ql_appinit.h"
#include "ql_api_type.h"

/********************************************************************
*   SMS Message Format
**********************************************************************/
typedef enum QlSMSFormatTag
{
    QL_SMS_FMT_PDU = 0,
    QL_SMS_FMT_TXT = 1
}QlSMSFormat;


/******************************************************************************
* Function:     Ql_SMSInitialize
*  
* Description:
*               Do some initial work before send or receive a message.
*
* Parameters:    
*               None.
*
* Return:  
*               None.
******************************************************************************/
void Ql_SMSInitialize(void);


/******************************************************************************
* Function:     Ql_SMSUnInitialize
*  
* Description:
*               Uninitial work after sending or receiving messages.
*
* Parameters:    
*               None.
*
* Return:  
*               None.
******************************************************************************/
void Ql_SMSUnInitialize(void);


/******************************************************************************
* Function:     Ql_SendTextSMS
*  
* Description:
*               Send a message with text format.
*
* Parameters:    
*               number:
*                       Phone number.
*
*               msg:
*                       Message content.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32  Ql_SendTextSMS(u8 * number, u8 * msg);


/******************************************************************************
* Function:     Ql_SendPDUSMS
*  
* Description:
*               Send a message with PDU format.
*
* Parameters:    
*               length:
*                       The length of message.
*
*               msg:
*                       Message content.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32  Ql_SendPDUSMS(u16 length, u8 * msg);


/******************************************************************************
* Function:     Ql_ReadSMS
*  
* Description:
*               Read a message with a specified index and message format.
*               This function doesn't return the message content, and it will 
*               be returned via the EVENT_MODEMDATA event later.
*
* Parameters:    
*               index:
*                       Message index to read in message list.
*                       if you want to read all messages in message list, you can 
*                       set 'index' to -1.
*
*               smsfmt:
*                       SMS format, which is one value of 'QlSMSFormat'.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
//extern s32  (*Ql_ReadSMS)(s8 index, u8 smsfmt);
s32  Ql_ReadSMS(u8 index, OCPU_CB_CMGR cb);


/******************************************************************************
* Function:     Ql_SetNewSMSCallBack
*  
* Description:
*               Set the callback function of new SMS.
*
* Parameters:    
*               cb:
*                    A pointer to the callback function.
* Return:  
*               None.
******************************************************************************/
void Ql_SetNewSMSCallBack(OCPU_CB_CMTI cb);


/******************************************************************************
* Function:     Ql_SetSMSFormat
*  
* Description:
*               Set SMS format.
*
* Parameters:    
*               format:
*                     One value of 'QlSMSFormat'
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32  Ql_SetSMSFormat(u8 format);


/******************************************************************************
* Function:     Ql_GetSMSFormat
*  
* Description:
*               Get SMS format.
*
* Parameters:    
*               None
* Return:  
*               one value of 'QlSMSFormat'.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s8   Ql_GetSMSFormat(void);


/******************************************************************************
* Function:     Ql_SetSMSStorage
*  
* Description:
*               Set SMS storages to be used for reading,writing,etc.
*
* Parameters:    
*               mem1: 
*                      0|1 (SIM|ME), for read and delete
*               mem2:
*                      0|1 (SIM|ME), for write and send
*               mem3:
*                      0|1 (SIM|ME), for receive
*               please refer to 'AT+CPMS' for extended informat in ATC document.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32  Ql_SetSMSStorage(u8 mem1,u8 mem2, u8 mem3);


/******************************************************************************
* Function:     Ql_DeleteSMS
*  
* Description:
*               Delete SMS messages.
*
* Parameters:    
*               index: 
*                      Location number.
*               flag:
*                      0 for single message, 1 for all messages.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32  Ql_DeleteSMS(u8 index,u8 flag);


/******************************************************************************
* Function:     Ql_ReadSMSList
*  
* Description:
*               List SMS messages.
*
* Parameters:    
*               cb: 
*                    A pointer to the callback function.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32  Ql_ReadSMSList(OCPU_CB_CMGL cb);


/******************************************************************************
* Function:     Ql_SetInfoCentreNum
*  
* Description:
*               Set SMS server center number.
*
* Parameters:    
*               num:
*                    Pointer to the number.
*               len:
*                    The length of the number.
*               set_csca: 
*                    A pointer to the callback function.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32  Ql_SetInfoCentreNum(u8* num, u8 len, OCPU_CB_SET_CSCA set_csca);


/******************************************************************************
* Function:     Ql_GetInfoCentreNum
*  
* Description:
*               Get SMS server center number.
*
* Parameters:    
*               set_csca: 
*                    A pointer to the callback function.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32  Ql_GetInfoCentreNum(OCPU_CB_GET_CSCA get_csca);

#endif  // End-of __QL_SMS_H__

