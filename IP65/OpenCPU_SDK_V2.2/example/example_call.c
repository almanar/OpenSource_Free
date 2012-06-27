#ifdef __EXAMPLE_CALL__
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_fcm.h"
#include "ql_trace.h"
#include "ql_call.h"
#include "ql_audio.h"
#include "ql_sms.h"
#include "ql_tts.h"
#include "ql_tcpip.h"
#include "ql_error.h"


QlEventBuffer g_event; // Keep this variable a global variable due to its big size
char notes[100];
char buffer[100];
u8   codec[200];
char testdata[1024] = {0x9};

extern void *memcpy(void *dest, const void *src, s32 count);
void Callback_Dial(s32 result);
void Callback_Ring(u8* coming_call_num);
void Callback_Hangup(void);
void Callback_TTS_Play(s32 result);
void Callback_RetrieveBSInfo(QL_STNodeB_Info* bsInfo);

Ql_STCall_Callback call_cb = 
{
    Callback_Dial,
    Callback_Ring,
    Callback_Hangup
};
void ql_entry(void)
{
    s32 ret;
    bool bRet;
    bool keepGoing = TRUE;
    QlTimer tm;
    u32 cnt = 0; 
    
    u32 t1, t2;
    char AtCmd[] = "AT+CPIN?\r";
    
    Ql_SetDebugMode(BASIC_MODE);    /* Only after the device reboots, 
                                     *  the set debug-mode takes effect. 
                                     */
    Ql_DebugTrace("OpenCPU: Call !\r\n\r\n");    /* Print out message through DEBUG port */
    Ql_OpenModemPort(ql_md_port1);  /* or ql_md_port2, two virtual modem ports are available.*/
#if 1
    //Ql_UartClrRxBuffer(ql_uart_port3);
    Ql_SetPortOwner(ql_uart_port3, ql_main_task);
    Ql_SetUartDCBConfig(ql_uart_port3, 9600, 8, 1, 0);
#endif
    // Start a timer
    //tm.timeoutPeriod = Ql_SecondToTicks(2); /* Set the interval of timer */
    //Ql_StartTimer(&tm);
    //Ql_DebugTrace("The timer starts.\r\n\r\n");

    Ql_Call_Initialize(&call_cb);

    // DEL: No TTS on this version
    //Ql_TTS_Initialize(&Callback_TTS_Play);

#if 0    
// AT+CPIN?
t1 = Ql_GetRelativeTime();
Ql_SendToModem(ql_md_port2, (u8 *)AtCmd, Ql_strlen(AtCmd));
t2 = Ql_GetRelativeTime();
Ql_DebugTrace("Time Consumption for Ql_SendToModem('AT+CPIN?'): %d.\r\n", t2 - t1);

// 1k data
t1 = Ql_GetRelativeTime();
Ql_SendToModem(ql_md_port2, (u8 *)testdata, 1024);
t2 = Ql_GetRelativeTime();
Ql_DebugTrace("Time Consumption for Ql_SendToModem(1k data): %d.\r\n", t2 - t1);
#endif

    while(keepGoing)
    {
        Ql_GetEvent(&g_event);
        switch(g_event.eventType)
        {
            case EVENT_INTR:
            {
                //TODO: add your codes for interruption handling here 
                Intr_Event* pIntrEvt = (Intr_Event *)&g_event.eventData.intr_evt;
                Ql_DebugTrace("pinName = %d, pinState = %d", pIntrEvt->pinName, (u8)pIntrEvt->pinState);
                break;
            }
            case EVENT_UARTDATA:
            {
                //TODO: receive and handle data from UART
                u8 temp;
                char* pChar;

                /* The following code segment is written to demonstrate the usage of 'AT+QLOCPU'.
                *   If send 'AT+QLOCPU=0' through serial port tool, and reboot the device, only Core 
                *   System software is booted (the application will not be booted).
                */
                PortData_Event* pDataEvt = (PortData_Event*)&g_event.eventData.uartdata_evt;
                if (ql_uart_port3 == pDataEvt->port)
                {// gps data
                    Ql_DebugTrace("UART3: %s", (char*)pDataEvt->data);
                    break;
                }
                
                Ql_sprintf(buffer, "AT+QLOCPU=0\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    Ql_SendToModem(ql_md_port1, (u8*)"AT+QLOCPU=0\n", Ql_strlen(buffer) + 1);
                    break;
                }
                
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "tst1\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret) {
                    Ql_DebugTrace("tst1111\r\n");
                    bRet = Ql_VoiceCallChangePath(QL_AUDIO_PATH_LOUDSPEAKER);
                    if (!bRet)
                    {
                        Ql_DebugTrace("Fail to Ql_VoiceCallChangePath()\r\n");
                        break;
                    }
                    //Ql_Sleep(30);
                    
                    ret = Ql_SetVolume(VOL_TYPE_SPH, 80);
                    if (ret != QL_RET_OK)
                    {
                        Ql_DebugTrace("Fail to Ql_SetVolume(): %d\r\n", ret);
                        break;
                    }
                    //Ql_Sleep(30);
                    
                    ret = Ql_SetMicGain(QL_AUDIO_PATH_LOUDSPEAKER, 8);
                    if (ret != QL_RET_OK)
                    {
                        Ql_DebugTrace("Fail to Ql_SetMicGain(): %d\r\n", ret);
                        break;
                    }
                    //Ql_Sleep(30);
                    
                    ret = Ql_CtrlEchoCancel(224, 2000, 30000, 374, QL_AUDIO_PATH_LOUDSPEAKER);
                    if (ret != QL_RET_OK)
                    {
                        Ql_DebugTrace("Fail to Ql_CtrlEchoCancel(): %d\r\n", ret);
                        break;
                    }
                    //Ql_Sleep(30);
                    
                    ret = Ql_SetSideToneGain(QL_AUDIO_PATH_LOUDSPEAKER, 8);
                    if (ret != QL_RET_OK)
                    {
                        Ql_DebugTrace("Fail to Ql_SetSideToneGain(): %d\r\n", ret);
                        break;
                    }
                    //Ql_Sleep(30);
                    
                    #if 0 // For test
                    #endif
                    break;
                }
                
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "tst2\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret) {
                    Ql_DebugTrace("tst2222\r\n");
                    
                    Ql_VoiceCallChangePath(QL_AUDIO_PATH_NORMAL);
                    Ql_DebugTrace("Ql_VoiceCallChangePath(QL_AUDIO_PATH_NORMAL);\r\n");
                    //Ql_Sleep(30);
                    
                    ret = Ql_SetVolume(VOL_TYPE_SPH, 80);
                    Ql_DebugTrace("Ql_SetVolume(VOL_TYPE_SPH, 80)=%d\r\n", ret);
                    //Ql_Sleep(30);

                    ret = Ql_SetMicGain(QL_AUDIO_PATH_NORMAL, 8);
                    Ql_DebugTrace("Ql_SetMicGain(QL_AUDIO_PATH_NORMAL, 8)=%d\r\n", ret);
                    //Ql_Sleep(30);
                    
                    ret = Ql_CtrlEchoCancel(224, 2000, 30000, 374, QL_AUDIO_PATH_NORMAL);
                    Ql_DebugTrace("Ql_CtrlEchoCancel(224, 2000, 30000, 374, QL_AUDIO_PATH_NORMAL)=%d\r\n", ret);
                    //Ql_Sleep(30);
                    
                    ret = Ql_SetSideToneGain(QL_AUDIO_PATH_NORMAL, 8);
                    Ql_DebugTrace("Ql_SetSideToneGain(QL_AUDIO_PATH_NORMAL, 8)=%d\r\n", ret);
                    //Ql_Sleep(30);
                    
                    #if 0 // For test
                    #endif
                    
                    break;
                }

                // Dial (ATD10086;\r\n)
                pChar = Ql_strstr((char*)pDataEvt->data, "ATD");
                if (pChar == (char*)pDataEvt->data)
                {
                #if 1
                    char* p1;
                    pChar = (char*)pDataEvt->data;
                    p1 = Ql_strstr((char*)pDataEvt->data, ";");
                    if (p1)
                    {
                        Ql_DebugTrace("p2 != NULL\r\n");
                        Ql_memset(buffer, 0x0, Ql_strlen(buffer));
                        memcpy(buffer, (char*)pDataEvt->data + 3, pDataEvt->len - 3 - 3); // 减去'ATD'及';\r\n'
                    }
                    else
                    {
                        Ql_DebugTrace("Incorrect input.\r\n");
                        break;
                    }
                    Ql_DebugTrace("Start to dial.\r\n");
                    Ql_Call_Dial(VOICE_CALL, buffer);
                    
                    Ql_VoiceCallChangePath(QL_AUDIO_PATH_HEADSET);
                    Ql_SetVolume(VOL_TYPE_SPH, 80);
                    Ql_SetMicGain(QL_AUDIO_PATH_HEADSET, 8);
                    Ql_CtrlEchoCancel(224, 2000, 30000, 374, QL_AUDIO_PATH_HEADSET);
                    Ql_SetSideToneGain(QL_AUDIO_PATH_HEADSET, 8);
                #else
                
                // ATD...
                t1 = Ql_GetRelativeTime();
                Ql_SendToModem(ql_md_port2, (u8*)pDataEvt->data, pDataEvt->len);
                t2 = Ql_GetRelativeTime();
                Ql_DebugTrace("Time Consumption for Ql_SendToModem('ATD...'): %d.\r\n", t2 - t1);

                #endif
                    break;
                }

                // Answer
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "answer\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                #if 1
                    Ql_Call_Answer();
                    Ql_DebugTrace(buffer);
                #else
                
                // ATA
                Ql_sprintf(AtCmd, "ATA\n");
                t1 = Ql_GetRelativeTime();
                Ql_SendToModem(ql_md_port2, (u8 *)AtCmd, Ql_strlen(AtCmd));
                t2 = Ql_GetRelativeTime();
                Ql_DebugTrace("Time Consumption for Ql_SendToModem('ATA'): %d.\r\n", t2 - t1);

                #endif
                    break;
                }

                // Hang up
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "hang up\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                #if 1
                    Ql_Call_Hangup();
                    Ql_DebugTrace(buffer);
                #else
                
                // ATH
                Ql_sprintf(AtCmd, "ATH\n");
                t1 = Ql_GetRelativeTime();
                Ql_SendToModem(ql_md_port2, (u8 *)AtCmd, Ql_strlen(AtCmd));
                t2 = Ql_GetRelativeTime();
                Ql_DebugTrace("Time Consumption for Ql_SendToModem('ATH'): %d.\r\n", t2 - t1);

                #endif
                    break;
                }

                // Get IMEI
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "imei?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    Ql_DebugTrace(buffer);
                    memset(buffer, 0x0, strlen(buffer));
                    Ql_DebugTrace("Start to get imei.\r\n");
                    if (Ql_GSM_GetIMEI((u8*)buffer, sizeof(buffer)) >= 0)
                    {
                        Ql_DebugTrace(buffer);
                    }
                    break;
                }

                // Get IMSI
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "imsi?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    Ql_DebugTrace(buffer);
                    memset(buffer, 0x0, strlen(buffer));
                    if (Ql_SIM_GetIMSI((u8*)buffer, sizeof(buffer)) >= 0)
                    {
                        Ql_DebugTrace(buffer);
                    }
                    break;
                }

                // Get version
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "ver?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    Ql_DebugTrace(buffer);
                    memset(buffer, 0x0, strlen(buffer));
                    if (Ql_GetCoreVer((u8*)buffer, sizeof(buffer)) >= 0)
                    {
                        Ql_DebugTrace(buffer);
                    }
                    break;
                }

                // Check SIM Card
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "sim card?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    Ql_DebugTrace(buffer);
                    memset(buffer, 0x0, strlen(buffer));
                    if (Ql_IsSIMInserted())
                    {
                        Ql_DebugTrace("SIM card is inserted.\r\n");
                    }
                    else
                    {
                        Ql_DebugTrace("SIM card is not inserted.\r\n");
                    }
                    break;
                }
                
                // Get Call Count (call count?)
                Ql_memset(buffer, 0x0, sizeof(buffer));
                //Ql_sprintf(buffer, "Ql_GetCallCntByType(1)\r\n");
                Ql_sprintf(buffer, "call count?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    temp = Ql_GetCallCntByType(VOICE_CALL);
                    Ql_DebugTrace("voice call count: %d\r\n", temp);
                    break;
                }

                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "Ql_GetCallCntByType(5)\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    temp = Ql_GetCallCntByType(CSD_CALL);
                    Ql_DebugTrace("csd call count: %d\r\n", temp);

                    Ql_SendToModem(ql_md_port1, (u8*)"ATH\r\n", 17);
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

                // Control echo cancellation
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "echo\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    Ql_sprintf(buffer, "AT+QECHO?\n");
                    Ql_SendToModem(ql_md_port1, (u8*)buffer, Ql_strlen(buffer));

                    ret = Ql_CtrlEchoCancel(224, 2000, 30000, 374, QL_AUDIO_PATH_LOUDSPEAKER);
                    Ql_DebugTrace("Ql_CtrlEchoCancel() = %d\r\n", ret);

                    Ql_sprintf(buffer, "AT+QECHO?\n");
                    Ql_SendToModem(ql_md_port1, (u8*)buffer, Ql_strlen(buffer));
                    break;
                }

                // Send DTMF
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "vts=");
                pChar = Ql_strstr((char*)pDataEvt->data, buffer);
                if (pChar)
                {
                    pChar = Ql_strstr((char*)pDataEvt->data, "=");
                    if (pChar)
                    {
                        char* p2;
                        u8 len;
                        p2 = Ql_strstr((char*)pDataEvt->data, "\r\n");
                        len = p2 - pChar -1;
                        Ql_memset(buffer, 0x0, sizeof(buffer));
                        Ql_memcpy(buffer, pChar + 1, len);
                        ret = Ql_VTS((u8*)buffer, len);
                        Ql_DebugTrace("Ql_VTS() = %d\r\n", ret);
                     }
                     break;
                 }

                #if 0
                #include "ql_utility.h"
                // GB2132 => UNICODE
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "convert=");
                pChar = Ql_strstr((char*)pDataEvt->data, buffer);
                if (pChar)
                {
                    pChar = Ql_strstr((char*)pDataEvt->data, "=");
                    if (pChar)
                    {
                        char* p2;
                        u8 len;
                        p2 = Ql_strstr((char*)pDataEvt->data, "\r\n");
                        len = p2 - pChar -1;
                        Ql_memset(buffer, 0x0, sizeof(buffer));
                        Ql_memcpy(buffer, pChar + 1, len);
                        Ql_DebugTrace("Ql_ConvertCodec(\"%s\")\r\n", buffer);
                        Ql_memset((char*)codec, 0x0, Ql_strlen((char*)codec));
                        //ret = Ql_ConvertCodec(QL_CODEC_GB2312, (u8*)buffer, QL_CODEC_UNICODE, codec);
                        ret = Ql_ConvertCodec(QL_CODEC_UNICODE, (u8*)buffer, QL_CODEC_GB2312, codec);
                        if (ret > 0)
                        {
                            codec[ret] = 0;
                            Ql_sprintf(buffer, "Ql_ConvertCodec(): %x\r\n", codec);
                            Ql_DebugTrace(buffer);
                        }
                        else
                        {
                            Ql_DebugTrace("Fail to convert codec, error code: %d\r\n", ret);
                        }
                        Ql_DebugTrace("Ql_ConvertCodec() = %d : %x\r\n", ret, codec);
                     }
                     break;
                 }
                 #endif

                #if 1 // DEL: No TTS on this version
                //  For 科大讯飞TTS
                Ql_sprintf(buffer, "qstk=");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    char* pCh = NULL;
                    pCh = Ql_strstr((char*)pDataEvt->data, "=");
                    if (pCh != NULL)
                    {
                        Ql_memset(buffer, 0x0, sizeof(buffer));
                        Ql_strncpy(buffer, pCh + 1, pDataEvt->len - (pCh - (char*)pDataEvt->data + 1) - 2);
                        Ql_DebugTrace("Ql_TTS_Play(), len=%d\r\n", Ql_strlen(buffer));
                        ret = Ql_TTS_Play((u8*)buffer, sizeof(buffer));
                        if (ret < 0)
                        {
                            Ql_DebugTrace("Fail to play TTS.\r\n");
                            break;
                        }
                        else
                        {
                            Ql_DebugTrace("Playing finished.\r\n");
                        }
                    }
                    break;
                }
                #endif

                // Get Operator (operator?)
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "operator?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    char oper[20] = { 0x0 };
                    Ql_DebugTrace(buffer);
                    memset(buffer, 0x0, strlen(buffer));
                    ret = Ql_GetOperator((u8*)oper, sizeof(oper));
                    Ql_DebugTrace("Ql_GetOperator()=%d: %s.\r\n", ret, oper);
                    break;
                }

                // Get device state (device state?)
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "device state?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    //u8 ps_status;
                    s32 simcard;
                    s32 creg;
                    s32 cgreg;
                    u8 rssi;
                    u8 ber;
                    /*the Ql_GetDeviceCurrentRunState function can replace these AT Command:  AT+CPIN? AT+CREG? AT+CGREG? AT+CSQ*/
                    Ql_GetDeviceCurrentRunState(&simcard, &creg, &cgreg, &rssi, &ber);
                    Ql_DebugTrace("Ql_GetDeviceCurrentRunState(): simcard=%d, creg=%d, cgreg=%d, rssi=%d, ber=%d\r\n", simcard, creg, cgreg, rssi, ber);
                    break;
                }

                // Get BS Information (bs info?)
                Ql_memset(buffer, 0x0, sizeof(buffer));
                Ql_sprintf(buffer, "bs info?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    ret = Ql_RetrieveNodeBInfo(Callback_RetrieveBSInfo);
                    Ql_DebugTrace("Ql_RetrieveNodeBInfo()=%d\r\n", ret);
                    break;
                }

                /*cmd:  Play=xxx.mp3*/
                /*cmd:  Play=xxx.wav*/
                /*cmd:  Play=xxx.amr*/
                /* Without '\r\n' */
                pChar = Ql_strstr((char*)pDataEvt->data, "Play=");
                if (pChar)
                {
                    s32 ret;
                    pChar += 5;
                    ret = Ql_StartPlayAudioFile((u8*)pChar ,0, 6, 1);
                    if(ret == QL_RET_OK)
                        ;//play ok
                    Ql_sprintf(buffer, "Ql_StartPlayAudioFile(%s)=%d\r\n", pChar, ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));    
                    break;
                }

                /*cmd:  Stop*/
                pChar = Ql_strstr((char*)pDataEvt->data,"Stop");
                if (pChar)
                {
                    s32 ret;
                    ret = Ql_StopPlayAudioFile();
                    if(ret == QL_RET_OK)
                        ;//stop ok
                    Ql_sprintf(buffer,"Ql_StopPlayAudioFile()=%d\r\n", ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));    
                    break;
                }    
                
                /*cmd:  play 8k audio*/
                pChar = Ql_strstr((char*)pDataEvt->data,"play 8k");
                if (pChar)
                {
                    s32 ret;
                    ret = Ql_PlayAudioFile_8k((u8*)"wav8k.wav", 0, QL_AUDIO_VOLUME_LEVEL6, QL_AUDIO_PATH_HEADSET, QL_AUDIO_VOLUME_MUTE);
                    Ql_sprintf(buffer,"Ql_PlayAudioFile_8k()=%d\r\n", ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));    
                    break;
                }
                
                pChar = Ql_strstr((char*)pDataEvt->data,"stop 8k");
                if (pChar)
                {
                    s32 ret;
                    ret = Ql_StopAudioFile_8k();
                    Ql_sprintf(buffer,"Ql_StopAudioFile_8k()=%d\r\n", ret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));    
                    break;
                }    

                /*cmd:  tone detect*/
                pChar = Ql_strstr((char*)pDataEvt->data,"tonedet");
                if (pChar)
                {
                    s32 ret;
                    Ql_sprintf(buffer,"AT+QTONEDET=1\r\n");
                    Ql_SendToModem(ql_md_port1, (u8*)buffer, Ql_strlen(buffer));    
                    break;
                }    

            }
            case EVENT_MODEMDATA:
            {
                //TODO: receive and hanle data from CORE through virtual modem port
                PortData_Event* pPortEvt = (PortData_Event*)&g_event.eventData.modemdata_evt;
                //Ql_DebugTrace("Modem Data at vPort [%d]: %s\r\n", pPortEvt->port, pPortEvt->data);
                Ql_DebugTrace("%s", pPortEvt->data);
                break;
            }
            case EVENT_TIMER:
            {
                //TODO: specify what you want to happen when the interval for timer elapes

                //Ql_DebugTrace("The timer raises for %d time(s).\r\n", ++cnt);
                memset(notes, 0x0, Ql_strlen(notes));
                sprintf(notes, "The timer raises for %d time(s).\r\n", ++cnt);
                Ql_DebugTrace(notes);

                // Start the timer again
                if (cnt < 10)
                {
                    Ql_StartTimer(&tm);
                }
                else
                {
                    Ql_DebugTrace("\r\nThe timer stops.\r\n");
                }
                break;
            }

            default:
                Ql_DebugTrace("\r\nUNKNOW EVENT(%x).\r\n", g_event.eventType);
                break;
        }
    }
}

void Callback_Dial(s32 result)
{
    Ql_DebugTrace("Callback_Dial(): result = %d\r\n", result);
}

void Callback_Ring(u8* coming_call_num)
{
    Ql_DebugTrace("Callback_Ring(): %s\r\n", (char*)coming_call_num);
}

void Callback_Hangup(void)
{
    Ql_DebugTrace("Callback_Hangup()\r\n");
}

void Callback_TTS_Play(s32 result)
{
    Ql_DebugTrace("Callback_TTS_Play(), result: %d\r\n", result);
}

void Callback_RetrieveBSInfo(QL_STNodeB_Info* bsInfo)
{
    if (bsInfo != NULL)
    {
        Ql_DebugTrace("BS Info: \r\n mcc=%d, mnc=%d, lac=%d, cellId=%d, bcch=%d, bsic=%d, dbm=%d, c1=%d, c2=%d, txp=%d, rla=%d\r\n", 
            bsInfo->mcc, bsInfo->mnc, bsInfo->lac, bsInfo->cellId, bsInfo->bcch, bsInfo->bsic, bsInfo->dbm, bsInfo->c1, bsInfo->c2, bsInfo->txp, bsInfo->rla);
    }
}
#endif // __EXAMPLE_CALL__

