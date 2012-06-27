/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2010 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |      Definitions for Call
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by   :    Stanley YONG
 |  Coded by        :    Stanley YONG
 |  Tested by        :    Stanley YONG
 |
 \=========================================================================*/

#ifndef __QL_VOICE_H__
#define __QL_VOICE_H__

#include "ql_type.h"
#include "ql_appinit.h"

typedef enum tagQl_CallType
{
   VOICE_CALL,                                  /* Voice Call */
   CSD_CALL = VOICE_CALL + 4,   /* Circuit Switched Data */
   INVALID_CALL_TYPE = 255
} Ql_CallType;
typedef enum tagQl_SIMCard_Status
{
    SIMCARD_ALREADY = 0,
    SIMCARD_NOT_INSERTED = 1,
    SIMCARD_PIN_LOCKED = 2,
    SIMCARD_PUK_LOCKED = 3
}Ql_SIMCard_Status;

typedef enum tagCallResponse
{
  CALL_RESPONSE_OK,
  CALL_RESPONSE_CONNECT,
  CALL_RESPONSE_RING,
  CALL_RESPONSE_NO_CARRIER,
  CALL_RESPONSE_ERROR,
  CALL_RESPONSE_INV,
  CALL_RESPONSE_NO_DIALTONE,
  CALL_RESPONSE_BUSY,
  CALL_RESPONSE_NO_ANSWER,
  CALL_RESPONSE_PROCEEDING,
  CALL_RESPONSE_FCERROR,
  CALL_RESPONSE_INTERMEDIATE,
  RESULT_CODE_INVALID_COMMAND,
}Ql_CallResponse;

typedef struct
{
    u16 mcc;
    u16 mnc;
	u16 lac;
	u16 cellId;
	u16 bcch;
	u8  bsic;
	s16 dbm;
	s8  c1;
	s8  c2;
	s16 txp;
	s16 rla;
}QL_STNodeB_Info;

typedef struct
{
    void (*OCPU_CB_DIAL)(s32 result);
    void (*OCPU_CB_RING)(u8* coming_num);
    void (*OCPU_CB_HANGUP)(void);
}Ql_STCall_Callback;

typedef void (*OCPU_CB_QENG)(QL_STNodeB_Info* bsInfo);

s32 Ql_Call_Initialize(Ql_STCall_Callback* call_cbFunc);

/******************************************************************************
* Function:     Ql_GetCallCntByType
*  
* Description:
*               Get the current call count by call type.
*
* Parameters:    
*               call_type:
*                       Call Type, which is one value of 'Ql_CallType'.
*
* Return:  
*               A nonnegative number as the current call number.
******************************************************************************/
u8 Ql_GetCallCntByType(Ql_CallType call_type);


/******************************************************************************
* Function:     Ql_Call_Dial
*  
* Description:
*               Dial a number.
*               This function will check the status of SIM card.
*
* Parameters:    
*               call_type:
*                       Call Type, which is one value of 'Ql_CallType'.
*
*               callNo:
*                       Call number.
* Return:  
*               1, SIM card is not inserted.
*               2, SIM card pin is locked.
*               3, SIM card PUK is locked.
*               A nonnegative number indicates other error.
******************************************************************************/
s32 Ql_Call_Dial(Ql_CallType call_type, char* callNo);

/******************************************************************************
* Function:     Ql_Call_Answer
*  
* Description:
*               This function answer a coming call.
*
* Parameters:    
*               None.
*
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32 Ql_Call_Answer(void);


/******************************************************************************
* Function:     Ql_Call_Hangup
*  
* Description:
*               This function hang up a call.
*
* Parameters:    
*               None.
*
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32 Ql_Call_Hangup(void);


/******************************************************************************
* Function:     Ql_GSM_GetIMEI
*  
* Description:
*               This function gets the IMEI(International Mobile Equipment Identity) of GSM.
*
* Parameters:    
*               ptr_imei:
*                   [out] A pointer to the IMEI buffer.
*               len:
*                   [in] It must be equal or greater than the length of version ID.
*                         Otherwise error code will be returned. 
*
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32 Ql_GSM_GetIMEI(u8* ptr_imei, u16 len);


/******************************************************************************
* Function:     Ql_SIM_GetIMSI
*  
* Description:
*               This function gets the IMSI(International Mobile Subscriber Identity) of SIM Card.
*
* Parameters:    
*               ptr_imei:
*                   [out] A pointer to the IMSI buffer.
*               len:
*                   [in] It must be equal or greater than the length of version ID.
*                         Otherwise error code will be returned. 
*
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32 Ql_SIM_GetIMSI(u8* ptr_imsi, u16 len);


/******************************************************************************
* Function:     Ql_IsSIMInserted
*  
* Description:
*               This function detects if SIM Card is inserted or not.
*
* Parameters:    
*               None.
*
* Return:  
*               0 indicate SIM Card is inserted and 1 is not.
******************************************************************************/
bool Ql_IsSIMInserted(void);


/******************************************************************************
* Function:     Ql_GetOperator
*  
* Description:
*               Get the current operator name.
*
* Parameters:    
*               opr_str:
*                  [out] A pointer to the operator names buffer.
*               len:
*                  [in] The lenth of the operator names buffer.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32 Ql_GetOperator(u8* opr_str, u8 len);


/******************************************************************************
* Function:     Ql_RetrieveNodeBInfo
*  
* Description:
*               Get base station Info.
*
* Parameters:    
*               callback_bsInfo:
*                     A pointer to callback function.               
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see Error Code Definition.
******************************************************************************/
s32 Ql_RetrieveNodeBInfo(OCPU_CB_QENG callback_bsInfo);

#endif  // End-of __QL_VOICE_H__

