#ifdef __EXAMPLE_AUDIO__

#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_filesystem.h"
#include "ql_error.h"
#include "resource_audio.h"
#include "ql_fcm.h"

char textBuf[100];
u8 strBuf[100];

char buffer[100];
char notes[100];
s32 filehandle = -1;


void ql_entry()
{
    bool           keepGoing = TRUE;
    QlEventBuffer  flSignalBuffer;
    s32 freesize;
    char *p=NULL;
    char *pData= NULL;
    s32 ret;

    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("audio: ql_entry\r\n");   
    Ql_OpenModemPort(ql_md_port1);       

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer);
        switch(flSignalBuffer.eventType)
        {
            case EVENT_UARTDATA:
            {
                char* pChar = NULL;
                PortData_Event* pDataEvt = NULL;
                if (flSignalBuffer.eventData.uartdata_evt.len <= 0)
                {
                    break;
                }
                pDataEvt = (PortData_Event*)&flSignalBuffer.eventData.uartdata_evt;
                pData = (char*)flSignalBuffer.eventData.uartdata_evt.data;

                /*set last tail data eq 0*/
                pData[flSignalBuffer.eventData.uartdata_evt.len+1] = '\0';

                /*cmd:  Play=xxx.mp3*/
                /*cmd:  Play=xxx.wav*/
                /*cmd:  Play=xxx.amr*/
                /* Without '\r\n' */
                p = Ql_strstr(pData,"Play=");
                if (p)
                {
                    s32 ret;
                    p += 5;
                    ret = Ql_StartPlayAudioFile((u8*)p ,0, 6, 1);
                    if(ret == QL_RET_OK)
                        ;//play ok
                    Ql_sprintf(textBuf,"Ql_StartPlayAudioFile(%s)=%d\r\n", p, ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)textBuf,Ql_strlen(textBuf));    
                    break;
                }

                /*cmd:  Stop*/
                p = Ql_strstr(pData,"Stop");
                if (p)
                {
                    s32 ret;
                    ret = Ql_StopPlayAudioFile();
                    if(ret == QL_RET_OK)
                        ;//stop ok
                    Ql_sprintf(textBuf,"Ql_StopPlayAudioFile()=%d\r\n", ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)textBuf,Ql_strlen(textBuf));    
                    break;
                }    

                /*cmd:  PlayS=mp3*/
                /*cmd:  PlayS=wav*/
                /*cmd:  PlayS=amr*/
                p = Ql_strstr(pData,"PlayS=mp3");
                if (p)
                {
                    s32 ret;
                    p += 6;
                    ret = Ql_StartPlayAudioStream((u8*)mp344k_mp3,sizeof(mp344k_mp3), 1, 1, 6, 1);
                    if(ret == QL_RET_OK)
                        ;//play ok
                    Ql_sprintf(textBuf,"Ql_StartPlayAudioStream(mp344k_mp3,sizeof=%d)=%d\r\n",sizeof(mp344k_mp3), ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)textBuf,Ql_strlen(textBuf));    
                    break;
                }

                /*cmd:  PlayS=wav*/
                p = Ql_strstr(pData,"PlayS=wav");
                if (p)
                {
                    s32 ret;
                    p += 6;
                    ret = Ql_StartPlayAudioStream((u8*)wav11k_wav, sizeof(wav11k_wav), 3, 1, 6, 1);
                    if(ret == QL_RET_OK)
                        ;//play ok
                    Ql_sprintf(textBuf,"Ql_StartPlayAudioStream(wav11k_wav,sizeof=%d)=%d\r\n",sizeof(wav11k_wav), ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)textBuf,Ql_strlen(textBuf));    
                    break;
                }

                /*cmd:  PlayS=amr*/
                p = Ql_strstr(pData,"PlayS=amr");
                if (p)
                {
                    s32 ret;
                    p += 6;
                    ret = Ql_StartPlayAudioStream((u8*)amr44k_amr, sizeof(amr44k_amr), 2, 1, 6, 1);
                    if(ret == QL_RET_OK)
                        ;//play ok
                    Ql_sprintf(textBuf,"Ql_StartPlayAudioStream(amr44k_amr,sizeof=%d)=%d\r\n",sizeof(amr44k_amr), ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)textBuf,Ql_strlen(textBuf));    
                    break;
                }

                /*cmd:  StopS*/
                p = Ql_strstr(pData,"StopS");
                if (p)
                {
                    s32 ret;
                    ret = Ql_StopPlayAudioStream();
                    if(ret == QL_RET_OK)
                        ;//stop ok
                    Ql_sprintf(textBuf,"Ql_StopPlayAudioStream()=%d\r\n", ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)textBuf,Ql_strlen(textBuf));    
                    break;
                }

                    
                // Change audio path
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "audio path=");
                pChar = Ql_strstr((char*)pDataEvt->data, buffer);
                if (pChar)
                {
                    u8 audPath;
                    char* p1;
                    char* p2;
                    Ql_DebugTrace((char*)pDataEvt->data);
                    p1 = Ql_strstr((char*)pDataEvt->data, "=");
                    p2 = Ql_strstr((char*)pDataEvt->data, "\r\n");
                    Ql_memset(notes, 0x0, sizeof(notes));
                    memcpy(notes, p1 + 1, p2 - p1 -1);
                    if (strcmp(notes, "1") == 0)
                    {
                        audPath = QL_AUDIO_PATH_HEADSET;
                    }
                    else if (strcmp(notes, "2") == 0)
                    {
                        audPath = QL_AUDIO_PATH_LOUDSPEAKER;
                    }
                    else
                    {
                        audPath = QL_AUDIO_PATH_NORMAL;
                    }
                    if (!Ql_VoiceCallChangePath(audPath))
                    {
                        Ql_DebugTrace("Fail to change audio path.\r\n");
                    }
                    else
                    {
                        Ql_DebugTrace("Change audio path to %d.\r\n", audPath);
                    }
                    break;
                }

                // Retrive current audio path
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "audio path?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    u8 aud_path;
                    Ql_DebugTrace(buffer);
                    memset(buffer, 0x0, sizeof(buffer));
                    aud_path = Ql_VoiceCallGetCurrentPath();
                    Ql_DebugTrace("Current audio path: %d.\r\n", aud_path);
                    break;
                }

                // Set volume
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "volume=");
                pChar = Ql_strstr((char*)pDataEvt->data, buffer);
                if (pChar)
                {
                    pChar = Ql_strstr((char*)pDataEvt->data, "=");
                    if (pChar)
                    {
                        int i;
                        char* p2;
                        u8 vol;
                        p2 = Ql_strstr((char*)pDataEvt->data, "\r\n");
                        Ql_memcpy(buffer, pChar + 1, p2 - pChar -1);
                        vol = atoi(buffer);
                        ret = Ql_SetVolume_Ex(VOL_TYPE_MEDIA, QL_AUDIO_PATH_HEADSET, vol);
                        Ql_DebugTrace("Ql_SetVolume_Ex(VOL_TYPE_MEDIA, %d) = %d;\r\n", vol, ret);
                        /*
                        ret = Ql_SetVolume(VOL_TYPE_SPH, vol);
                        Ql_DebugTrace("Ql_SetVolume(VOL_TYPE_SPH, %d) = %d;\r\n", vol, ret);
                        Ql_sprintf(buffer, "AT+CLVL?\n");
                        Ql_SendToModem(ql_md_port1, (u8*)buffer, Ql_strlen(buffer));
                        */
                    }
                    break;
                }

               // Get volume
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "volume?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    u8 i;
                    u8 vol_level;
                    Ql_DebugTrace(buffer);
                    memset(buffer, 0x0, strlen(buffer));
                    #if 0
                    vol_level = Ql_GetVolume(VOL_TYPE_MIC);
                    Ql_DebugTrace("Ql_GetVolume(VOL_TYPE_MIC) = %d.\r\n", vol_level);
                    vol_level = Ql_GetVolume(VOL_TYPE_SPH);
                    Ql_DebugTrace("Ql_GetVolume(VOL_TYPE_SPH) = %d.\r\n", vol_level);
                    #else
                    for (i = 0; i < MAX_VOL_TYPE; i++)
                    {
                        vol_level = Ql_GetVolume(i);
                        Ql_DebugTrace("Ql_GetVolume(%d) = %d.\r\n", i, vol_level);
                    }
                    #endif
                    
                    break;
                }
               
                // Set MIC gain (gain=channel,gain)
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "gain=");
                pChar = Ql_strstr((char*)pDataEvt->data, buffer);
                if (pChar)
                {
                    pChar = Ql_strstr((char*)pDataEvt->data, "=");
                    if (pChar)
                    {
                        char* p2;
                        u8 channel;
                        u8 micGain;
                        
                        p2 = Ql_strstr((char*)pDataEvt->data, ",");
                        Ql_memcpy(buffer, pChar + 1, p2 - pChar -1);
                        channel = atoi(buffer);
                        
                        pChar = p2;
                        p2 = Ql_strstr((char*)pDataEvt->data, "\r\n");
                        Ql_memcpy(buffer, pChar + 1, p2 - pChar -1);
                        micGain = atoi(buffer);
                        
                        Ql_SetMicGain(channel, micGain);
                        //Ql_SetSideToneGain(channel, micGain);
                        Ql_DebugTrace("Set mic gain at channel %d: %d.\r\n", channel, micGain);
                    }
                    break;
                }

                // Get MIC gain
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "mic gain?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    u8 micGain;
                    Ql_DebugTrace(buffer);
                    memset(buffer, 0x0, strlen(buffer));
                    #if 1
                    micGain = Ql_GetMicGain(QL_AUDIO_PATH_NORMAL);
                    Ql_DebugTrace("Mic gain at channel %d: %d.\r\n", QL_AUDIO_PATH_NORMAL, micGain);
                    micGain = Ql_GetMicGain(QL_AUDIO_PATH_HEADSET);
                    Ql_DebugTrace("Mic gain at channel %d: %d.\r\n", QL_AUDIO_PATH_HEADSET, micGain);
                    micGain = Ql_GetMicGain(QL_AUDIO_PATH_LOUDSPEAKER);
                    Ql_DebugTrace("Mic gain at channel %d: %d.\r\n", QL_AUDIO_PATH_LOUDSPEAKER, micGain);
                    #else
                    micGain = Ql_GetSideToneGain(QL_AUDIO_PATH_NORMAL);
                    Ql_DebugTrace("Mic gain at channel %d: %d.\r\n", QL_AUDIO_PATH_NORMAL, micGain);
                    micGain = Ql_GetSideToneGain(QL_AUDIO_PATH_HEADSET);
                    Ql_DebugTrace("Mic gain at channel %d: %d.\r\n", QL_AUDIO_PATH_HEADSET, micGain);
                    micGain = Ql_GetSideToneGain(QL_AUDIO_PATH_LOUDSPEAKER);
                    Ql_DebugTrace("Mic gain at channel %d: %d.\r\n", QL_AUDIO_PATH_LOUDSPEAKER, micGain);
                    #endif
                    break;
                }
                break;
            }
            
            default:
                break;
        }
    }
}

#endif //__EXAMPLE_AUDIO__

