/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2010 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 |
 | File Description
 | ----------------
 |      Definitions for audio parameters & APIs
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by    :    Jay XIN
 |  Coded by       :    Jay XIN
 |  Tested by      :    Stanley YONG
 |
 \=========================================================================*/

#ifndef __QL_AUDIO_H__
#define __QL_AUDIO_H__

#include "Ql_type.h"


/****************************************************************************
 *  Built-in Audio Resource
 ***************************************************************************/
typedef enum QlAudioNameTag
{
    QL_AUDIO_EMS_CHIMES_HI = 1,
    QL_AUDIO_EMS_CHIMES_LO,
    QL_AUDIO_EMS_DING,
    QL_AUDIO_EMS_TADA,
    QL_AUDIO_EMS_NOTIFY,
    QL_AUDIO_EMS_DRUM,
    QL_AUDIO_EMS_CLAPS,
    QL_AUDIO_EMS_FANFARE,
    QL_AUDIO_EMS_CHORD_HI,
    QL_AUDIO_EMS_CHORD_LO,
    QL_AUDIO_1,
    QL_AUDIO_2,
    QL_AUDIO_3,
    QL_AUDIO_4,
    QL_AUDIO_5,
    QL_AUDIO_6,
    QL_AUDIO_7,
    QL_AUDIO_8,
    QL_AUDIO_9,
    QL_AUDIO_10,
    QL_AUDIO_11,
    QL_AUDIO_12,
    QL_AUDIO_13,
    QL_AUDIO_14,
    QL_AUDIO_15,
    QL_AUDIO_16,
    QL_AUDIO_17,
    QL_AUDIO_18,
    QL_AUDIO_19,
    QL_AUDIO_END
}
QlAudioName;


/****************************************************************************
 *  Audio Volume Level Definition
 ***************************************************************************/
typedef enum QlAudioVolumeLeveltag
{
    QL_AUDIO_VOLUME_LEVEL1 = 0,
    QL_AUDIO_VOLUME_LEVEL2,
    QL_AUDIO_VOLUME_LEVEL3,
    QL_AUDIO_VOLUME_LEVEL4,
    QL_AUDIO_VOLUME_LEVEL5,
    QL_AUDIO_VOLUME_LEVEL6,
    QL_AUDIO_VOLUME_LEVEL7,
    QL_AUDIO_VOLUME_MUTE,
    QL_AUDIO_VOLUME_LEVEL_END
}QlAudioVolumeLevel;


/****************************************************************************
 *  Audio Output Source
 ***************************************************************************/
typedef enum QlAudioPlayPathTag
{
    QL_AUDIO_PATH_NORMAL = 0,       /* speak1 */
    QL_AUDIO_PATH_HEADSET = 1,      /* earphone */
    QL_AUDIO_PATH_LOUDSPEAKER = 2,  /* speaker2, loudspeaker for free sound */
    QL_AUDIO_PATH_END
}QlAudioPlayPath;


/****************************************************************************
 *  Audio Format
 ***************************************************************************/
typedef enum QlAudioStreamFormattag
{
    QL_AUDIO_STREAMFORMAT_MP3 = 1,
    QL_AUDIO_STREAMFORMAT_AMR = 2,
    QL_AUDIO_STREAMFORMAT_WAV = 3,
    QL_AUDIO_STREAMFORMAT_END
}QlAudioStreamFormat;


typedef void (*OCPU_CB_GET_VOLUME)(s8 vol_level);

/* Define volume type.*/
typedef enum {
    VOL_TYPE_CTN  = 0 ,   /* MMI can apply to associate volume; call tone attribute */
    VOL_TYPE_KEY      ,   /* MMI can apply to associate volume; keypad tone attribute */
    VOL_TYPE_MIC      ,   /* microphone attribute */
    VOL_TYPE_GMI      ,   /* MMI can apply to associate volume; melody, imelody, midi attribute */
    VOL_TYPE_SPH      ,   /* MMI can apply to associate volume; speech sound attribute */
    VOL_TYPE_SID	  ,   /* side tone attribute */
    VOL_TYPE_MEDIA    ,   /* MMI can apply to associate volume; As MP3, Wave,... attribute */
    MAX_VOL_TYPE           
}volume_type_enum;


/*****************************************************************
* Function:     Ql_PlayAudio 
* 
* Description:
*               Plays the built-in audio resource.
*               When audio plays ended,
*               a EVENT_MEDIA_FINISH event will be received by embedded application.
* Parameters:
*               name:
*                       Audio resource built in Core System
*
*               repeat:
*                       TRUE or FALSE, indicates that Core System
*                       play the audio repeatedly or just once.
* Return:        
*               0 indicates the function suceeds
*               <0 indicates failure
*****************************************************************/
s32 Ql_PlayAudio(QlAudioName name, bool repeat);


/*****************************************************************
* Function:     Ql_StopAudio 
* 
* Description:
*               Stops playing the audio.
* Parameters:
*               name:
*                       Audio resource built in Core System
* Return:        
*               0 indicates the function suceeds
*               <0 indicates failure
*****************************************************************/
s32 Ql_StopAudio(QlAudioName name);


/*****************************************************************
* Function:     Ql_StartPlayAudioFile 
* 
* Description:
*               Plays an audio file, and sets the items as below:
*                   - Repeatedly or just Once
*                   - Volume level
*                   - Audio output source.
*               When audio plays ended,
*               a EVENT_MEDIA_FINISH event will be received by embedded application.
* Parameters:
*               asciifilename:
*                       Audio file name
*
*               repeat:
*                       TRUE or FALSE, indicates that Core System
*                       play the audio repeatedly or just once.
*
*               volumelevel:
*                       Volume level
*
*               audiopath:
*                       Audio output source
* Return:        
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32 Ql_StartPlayAudioFile(u8 * asciifilename, bool repeat, u8 volumelevel, u8 audiopath);


/*****************************************************************
* Function:     Ql_PlayAudioFile_8k 
* 
* Description:
*               Plays an audio file with 8 kHz sample.
*               When audio plays ended,
*               a EVENT_MEDIA_FINISH event will be received by embedded application.
* Parameters:
*               file_name:
*                       Audio file name
*
*               repeat:
*                       TRUE or FALSE, indicates that Core System
*                       play the audio repeatedly or just once.
*
*               volumelevel:
*                       Volume level
*
*               audiopath:
*                       Audio output source
* Return:        
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32 Ql_PlayAudioFile_8k(u8* file_name, bool repeat, u8 volume, u8 path, u8 dlvolume);

/*****************************************************************
* Function:     Ql_StopAudioFile_8k 
* 
* Description:
*               Stops playing the audio 8K file.
* Parameters:
*               None
* Return:        
*               0 indicates the function suceeds
*               <0 indicates failure
*****************************************************************/

s32 Ql_StopAudioFile_8k(void);

/*****************************************************************
* Function:     Ql_StopPlayAudioFile 
* 
* Description:
*               Stops playing the audio file.
* Parameters:
*               None
* Return:        
*               0 indicates the function suceeds
*               <0 indicates failure
*****************************************************************/
s32 Ql_StopPlayAudioFile(void);


/*****************************************************************
* Function:     Ql_StartPlayAudioStream 
* 
* Description:
*               Plays an audio stream with stream size and stream format,
*               and sets the items as below:
*                   - Repeatedly or just Once
*                   - Volume level
*                   - Audio output source.
*               When audio plays ended,
*               a EVENT_MEDIA_FINISH event will be received by embedded application.
* Parameters:
*               stream:
*                       Audio stream
*
*               streamsize:
*                       Audio stream size
*
*               streamformat:
*                       Audio stream format
*
*               repeat:
*                       TRUE or FALSE, indicates that Core System
*                       play the audio repeatedly or just once.
*
*               volumelevel:
*                       Volume level
*
*               audiopath:
*                       Audio output source
* Return:        
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32 Ql_StartPlayAudioStream(u8 * stream, u32 streamsize, s32 streamformat, bool repeat, u8 volume, u8 audiopath);


/*****************************************************************
* Function:     Ql_StopPlayAudioStream 
* 
* Description:
*               Stops playing the audio stream.
* Parameters:
*               None
* Return:        
*               0, indicates the function suceeds
*               <0, indicates failure
*****************************************************************/
s32 Ql_StopPlayAudioStream(void);


/*****************************************************************
* Function:     Ql_VoiceCallChangePath 
* 
* Description:
*               Switches voice output source.
* Parameters:
*               path:
*                   Voice output source
* Return:        
*               TRUE, indicates the function suceeds
*               FALSE, indicates failure
*****************************************************************/
bool Ql_VoiceCallChangePath(QlAudioPlayPath path);


/*****************************************************************
* Function:     Ql_VoiceCallGetCurrentPath 
* 
* Description:
*               Retrieves the current voice output source.
* Parameters:
*               None
* Return:        
*               The current voice output source
*****************************************************************/
QlAudioPlayPath Ql_VoiceCallGetCurrentPath(void);


/*****************************************************************
* Function:     Ql_SetVolume 
* 
* Description:
*               Set the volume during calling.
* Parameters:
*               vol_type:
*                        0 = VOL_TYPE_CTN ,     call tone attribute 
*                        1 = VOL_TYPE_KEY ,     keypad tone attribute 
*                        2 = VOL_TYPE_MIC ,     microphone attribute  
*                        3 = VOL_TYPE_GMI ,     melody, imelody, midi attribute 
*                        4 = VOL_TYPE_SPH ,     speech sound attribute  
*                        5 = VOL_TYPE_SID ,     side tone attribute  
*                        6 = VOL_TYPE_MEDIA ,   As MP3, Wave,... attribute  
*               vol_level:
*                        Range is 1-100.
* Return:              
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32  Ql_SetVolume(u8 vol_type/*volume_type_enum*/, u8 vol_level/*0~100*/);


/*****************************************************************
* Function:     Ql_SetVolume_Ex 
* 
* Description:
*               Set the volume of audio.
* Parameters:
*               vol_type:
*                        0 = VOL_TYPE_CTN ,     call tone attribute 
*                        1 = VOL_TYPE_KEY ,     keypad tone attribute 
*                        2 = VOL_TYPE_MIC ,     microphone attribute  
*                        3 = VOL_TYPE_GMI ,     melody, imelody, midi attribute 
*                        4 = VOL_TYPE_SPH ,     speech sound attribute  
*                        5 = VOL_TYPE_SID ,     side tone attribute  
*                        6 = VOL_TYPE_MEDIA ,   As MP3, Wave,... attribute  
*                vol_path:
*                        0 = AUD_MODE_NORMAL   ,   Normal Mode  
*                        1 = AUD_MODE_HEADSET  ,   HeadSet (Earphone) Mode 
*                        2 = AUD_MODE_LOUDSPK  ,   Loudspeaker Mode  
*                vol_level:
*                        Range is 1-100.
* Return:              
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32  Ql_SetVolume_Ex(u8 vol_type/*volume_type_enum*/, u8 vol_path/*QlAudioPlayPath*/, u8 vol_level);


/*****************************************************************
* Function:     GetVolume 
* 
* Description:
*               Get the volume level of audio.
* Parameters:
*               vol_type:
*                        0 = VOL_TYPE_CTN ,     call tone attribute 
*                        1 = VOL_TYPE_KEY ,     keypad tone attribute 
*                        2 = VOL_TYPE_MIC ,     microphone attribute  
*                        3 = VOL_TYPE_GMI ,     melody, imelody, midi attribute 
*                        4 = VOL_TYPE_SPH ,     speech sound attribute  
*                        5 = VOL_TYPE_SID ,     side tone attribute  
*                        6 = VOL_TYPE_MEDIA ,   As MP3, Wave,... attribute  
* Return:              
*               The volume level, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32  Ql_GetVolume(u8 vol_type/*volume_type_enum*/);


/*****************************************************************
* Function:     Ql_SetMicGain 
* 
* Description:
*               Change the micphone gain level.
* Parameters:
*                channel:
*                        0 = AUD_MODE_NORMAL   ,   Normal Mode  
*                        1 = AUD_MODE_HEADSET  ,   HeadSet (Earphone) Mode 
*                        2 = AUD_MODE_LOUDSPK  ,   Loudspeaker Mode  
*                gain_level:
*                        Range is 1-15.
* Return:              
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32  Ql_SetMicGain(u8 channel/*QlAudioPlayPath*/, u8 gain_level/*0~15*/);


/*****************************************************************
* Function:     Ql_GetMicGain 
* 
* Description:
*               Get the micphone gain level.
* Parameters:
*                channel:
*                        0 = AUD_MODE_NORMAL   ,   Normal Mode  
*                        1 = AUD_MODE_HEADSET  ,   HeadSet (Earphone) Mode 
*                        2 = AUD_MODE_LOUDSPK  ,   Loudspeaker Mode  
*                gain_level:
*                        Range is 1-15.
* Return:              
*               The gain level, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32  Ql_GetMicGain(u8 channel/*QlAudioPlayPath*/);


/*****************************************************************
* Function:     Ql_SetSideToneGain 
* 
* Description:
*               Change the sidetone gain level.
* Parameters:
*                channel:
*                        0 = AUD_MODE_NORMAL   ,   Normal Mode  
*                        1 = AUD_MODE_HEADSET  ,   HeadSet (Earphone) Mode 
*                        2 = AUD_MODE_LOUDSPK  ,   Loudspeaker Mode  
*                gain_level:
*                        Range is 1-15.
* Return:              
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32  Ql_SetSideToneGain(u8 channel/*QlAudioPlayPath*/, u8 gain_level/*0~15*/);


/*****************************************************************
* Function:     Ql_GetSideToneGain 
* 
* Description:
*               Get the sidetone gain level.
* Parameters:
*                channel:
*                        0 = AUD_MODE_NORMAL   ,   Normal Mode  
*                        1 = AUD_MODE_HEADSET  ,   HeadSet (Earphone) Mode 
*                        2 = AUD_MODE_LOUDSPK  ,   Loudspeaker Mode  
* Return:              
*               The sidetone gain level, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32  Ql_GetSideToneGain(u8 channel/*QlAudioPlayPath*/);


/*****************************************************************
* Function:     Ql_CtrlEchoCancel 
* 
* Description:
*               Echo cancellation control.
* Parameters:
*                ctrlWord:
*                        221 for handset 
*                        224 for handfree
*                        0 for disable  
*                nlp:
*                        Range is 0-2048.The greater the value, the more reduction of echo. 
*                        0 means disabling the NLP algorithm.
*                suppression:
*                        Range is 0-32767.The smaller the vallue, the more reduction of echo. 
*                        0 means disabling the echo suppression algorithm. 
*                        224 for handfree
*                        0 for disable  
*                nr:
*                        Noise reduction controller.Should NOT be set to 0. 
*                        849 suitable for handset and handset for applications. 
*                        374 suitable for handfree. 
*                channel:
*                        0 = AUD_MODE_NORMAL   ,   Normal Mode  
*                        1 = AUD_MODE_HEADSET  ,   HeadSet (Earphone) Mode 
*                        2 = AUD_MODE_LOUDSPK  ,   Loudspeaker Mode  
* Return:              
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32  Ql_CtrlEchoCancel(u16 ctrlWord,      /* 221 for handset, 224 for handfree, 0: disable*/
                                 u16 nlp,           /* 0-2048 */
                                 u16 suppression,   /* 0-32767 */
                                 u16 nr,            /* 849 for handset, 374 for handfree, */
                                 u8  channel        /* one value of 'volume_category_enum' */
                                 );


/*****************************************************************
* Function:     Ql_VTS
* 
* Description:
*                 The function allows the transmission of DTMF tones and arbitrary tones in voice mode.
*                 These tones may be used (for example) when annoncing the start of recording period.
* Parameters:
*                dtmf_str:
*                        The address of DTMG string. 
*                len:
*                        The length of DTMF string,max value is 20.
* Return:              
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
*****************************************************************/
s32 Ql_VTS(u8* dtmf_str, u8 len);

#endif // End-of __QL_AUDIO_H__

