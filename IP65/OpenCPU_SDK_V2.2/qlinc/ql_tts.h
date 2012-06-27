/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2011 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |      Definitions for TTS
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by   :     Stanley YONG
 |  Coded by      :     Stanley YONG
 |  Tested by     :     Stanley YONG
 |
 \=========================================================================*/

#ifndef __QL_TTS_H__
#define __QL_TTS_H__

#include "ql_type.h"


/*TTS Player ÔËÐÐ×´Ì¬ */
typedef enum tagQl_TTSStatus
{
   TTS_STATUS_IDLE    = 0,
   TTS_STATUS_PLAYING = 1
}Ql_TTSStatus;

typedef void (*OCPU_CB_TTS_PLAY)(s32 res);


/*****************************************************************
* Function:     Ql_TTS_Initialize 
* 
* Description:
*               Initialize the TTS function.
* Parameters:
*               cb_play:
*                     A pointer to callback function.
* Return:        
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32 Ql_TTS_Initialize(OCPU_CB_TTS_PLAY cb_play);


/*****************************************************************
* Function:     Ql_TTS_Play 
* 
* Description:
*               Play TTS Text.
* Parameters:
*               content:
*                     pointer to text.
*               len:
*                     The lenth of text to speech.
* Return:        
*               ivTTS_ERR_OK, suceess
*               or other error codes, please see 'TTS Error Code Definition'
*****************************************************************/
s32 Ql_TTS_Play(u8* content, u8 len);


/*****************************************************************
* Function:     Ql_TTS_Stop 
* 
* Description:
*               Stop playing.
* Parameters:
*               None.
* Return:        
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32 Ql_TTS_Stop(void);


/*****************************************************************
* Function:     Ql_TTS_Query 
* 
* Description:
*               Check status of TTS.
* Parameters:
*               None.
*  return:
*	          0 = ivTTS_STATUS_IDLE,		 invalid 
*	          1 = ivTTS_STATUS_INITIALIZED,	 Initialized
*	          2 = ivTTS_STATUS_PAUSE,		 pause 
*	          3 = ivTTS_STATUS_SYNTHESIZING, synthesizing 
*	          4 = ivTTS_STATUS_PLAYING,		 playing 
*	          5 = ivTTS_STATUS_SYNTHESIZED,	 synthesizined
*	          6 = ivTTS_STATUS_PALYEND		 played
*               Negative indicates failed. Please see 'Error Code Definition'
***********************************/
s32 Ql_TTS_Query(void);

#endif  // End-of __QL_TTS_H__

