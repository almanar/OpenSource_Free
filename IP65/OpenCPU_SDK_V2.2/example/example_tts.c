#ifdef __EXAMPLE_TTS__
 
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_fcm.h"
#include "ql_tts.h"

void SendAtCmd(void);

QlEventBuffer g_event; // Keep this variable a global variable due to its big size
QlPort g_mdPort = ql_md_port1;
ascii g_buffer[1024];
u16 g_cmd_idx;

void Callback_TTS_Play(s32 result)
{
    Ql_DebugTrace("Callback_TTS_Play(), result: %d\r\n", result);
}

void ql_entry(void)
{
    s32 ret;
    bool keepGoing = TRUE;
    bool isContinued = FALSE;
    u32 cnt = 0; 
    
    Ql_SetDebugMode(BASIC_MODE);    /* Only after the device reboots, 
                                                                         *  the set debug-mode takes effect. 
                                                                         */
    Ql_DebugTrace("OpenCPU: Hello TTS !\r\n\r\n");    /* Print out message through DEBUG port */
    Ql_OpenModemPort(g_mdPort);  /* or ql_md_port2, two virtual modem ports are available.*/

    // Start and initialize TTS Function
    //g_cmd_idx = 1;
    //isContinued = TRUE;
    //SendAtCmd();
    Ql_TTS_Initialize(&Callback_TTS_Play);
    
    while(keepGoing)
    {
        Ql_GetEvent(&g_event);
        switch(g_event.eventType)
        {
            case EVENT_UARTDATA:
            {
                // TODO: receive and handle data from UART
                ascii* pChar;
                PortData_Event* pDataEvt = (PortData_Event*)&g_event.eventData.uartdata_evt;

                // Start tts speech ("tts=text")
                //  Note: text should be UCS2 code. 
                //  For 捷通TTS
                Ql_sprintf(g_buffer, "tts=");
                ret = Ql_strncmp((char*)pDataEvt->data, g_buffer, Ql_strlen(g_buffer));
                if (0 == ret)
                {
                    Ql_memset(g_buffer, 0x0, sizeof(g_buffer));
                    Ql_sprintf((char *)g_buffer, "AT+QTTS=2,\"");
                    Ql_strncpy(g_buffer + Ql_strlen(g_buffer), (char*)pDataEvt->data + 4, pDataEvt->len -4 - 2);  // 4 for "tts=", 2 for "\r\n"
                    g_buffer[Ql_strlen(g_buffer)] = '\"';
                    g_buffer[Ql_strlen(g_buffer)] = '\n';
                    isContinued = FALSE;
                    Ql_SendToUart(ql_uart_port1, (u8*)g_buffer, Ql_strlen(g_buffer));
                    Ql_SendToModem(g_mdPort, (u8*)g_buffer, Ql_strlen(g_buffer));
                    break;
                }
                
                //  For 科大讯飞TTS
                Ql_sprintf(g_buffer, "qstk=");
                ret = Ql_strncmp((char*)pDataEvt->data, g_buffer, Ql_strlen(g_buffer));
                if (0 == ret)
                {
                    char* pCh = NULL;
                    pCh = Ql_strstr((char*)pDataEvt->data, "=");
                    if (pCh != NULL)
                    {
                        Ql_memset(g_buffer, 0x0, sizeof(g_buffer));
                        Ql_strncpy(g_buffer, pCh + 1, pDataEvt->len - (pCh - (char*)pDataEvt->data + 1) - 2);
                        Ql_DebugTrace("Ql_TTS_Play(), len=%d\r\n", Ql_strlen(g_buffer));
                        ret = Ql_TTS_Play((u8*)g_buffer, sizeof(g_buffer));
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
                
                // Stop TTS (For 科大讯飞TTS)
                Ql_memset(g_buffer, 0x0, Ql_strlen(g_buffer));
                Ql_sprintf(g_buffer, "stop tts\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, g_buffer, Ql_strlen(g_buffer));
                if (0 == ret)
                {
                    Ql_DebugTrace(g_buffer);
                    Ql_TTS_Stop();
                    break;
                }
                
                // Query TTS' status (For 科大讯飞TTS)
                Ql_memset(g_buffer, 0x0, Ql_strlen(g_buffer));
                Ql_sprintf(g_buffer, "tts?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, g_buffer, Ql_strlen(g_buffer));
                if (0 == ret)
                {
                    Ql_DebugTrace(g_buffer);
                    ret = Ql_TTS_Query();
                    switch (ret)
                    {
                    	case TTS_STATUS_IDLE:            /* 无效 */
                    	    Ql_DebugTrace("TTS: idle\r\n");
                    	    break;
                    	#if 0
                    	case TTS_STATUS_INITIALIZED:     /* 初始化完毕 */
                    	    Ql_DebugTrace("TTS: initializd\r\n");
                    	    break;
                    	case TTS_STATUS_PAUSE:           /* 暂停 */
                    	    Ql_DebugTrace("TTS: pause\r\n");
                    	    break;
                    	case TTS_STATUS_SYNTHESIZING:    /* 正在合成 */
                    	    Ql_DebugTrace("TTS: synthesizing\r\n");
                    	    break;
                    	case TTS_STATUS_SYNTHESIZED:     /* 合成完毕 */
                    	    Ql_DebugTrace("TTS: synthesized\r\n");
                    	    break;
                    	case TTS_STATUS_PALYEND:         /* 播音完毕 */
                    	    Ql_DebugTrace("TTS: played\r\n");
                    	    break;
                    	#endif
                    	case TTS_STATUS_PLAYING:         /* 正在播音 */
                    	    Ql_DebugTrace("TTS: playing\r\n");
                    	    break;
                    	default:
                    	    Ql_DebugTrace("Fail to query TTS' status. Error code: %d\r\n", ret);
                    	    break;
                    }
                    break;
                }

                // Disable OpenCPU ("AT+QLOCPU=0\r\n")
                Ql_sprintf(g_buffer, "AT+QLOCPU=0\r\n");
                ret = Ql_strncmp((ascii*)pDataEvt->data, g_buffer, Ql_strlen(g_buffer));
                if (0 == ret)
                {
                    g_cmd_idx = 0;
                    isContinued = FALSE;
                    SendAtCmd();
                    break;
                }
                break;
            }
            case EVENT_MODEMDATA:
            {
                // TODO: receive and hanle data from CORE through virtual modem port
                PortData_Event* pPortEvt = (PortData_Event*)&g_event.eventData.modemdata_evt;
                Ql_DebugTrace("Modem Data at vPort [%d]: %s\r\n", pPortEvt->port, pPortEvt->data);
                if (DATA_AT == pPortEvt->type)
                {
                    if ((Ql_strstr((char*)pPortEvt->data, "\r\nOK") != NULL || 
                                   Ql_strstr((char*)pPortEvt->data, "OK\r\n") != NULL ||
                                   Ql_strstr((char*)pPortEvt->data, "ERROR") != NULL)
                                   && isContinued != FALSE)
                    {
                        g_cmd_idx++;
                        SendAtCmd();
                    }
                    break;
                }
                break;
            }
            case EVENT_TIMER:
            {
                // TODO: specify what you want to happen when the interval for timer elapes
                Timer_Event* pTimerEvt = (Timer_Event *)&g_event.eventData.timer_evt;
                Ql_DebugTrace("The timer [id: %d] raises.\r\n", pTimerEvt->timer_id);
                break;
            }
            
            default:
                break;
        }
    }
}

void SendAtCmd(void)
{
    bool exec = TRUE;
    switch (g_cmd_idx)
    {
    case 0:// Disable OpenCPU
        Ql_sprintf((char *)g_buffer, "AT+QLOCPU=0\n");
        break;
    
    case 1:// TTS function
        Ql_sprintf((char *)g_buffer, "AT+QTTS=1\n");
        break;
        
    case 2:// Set audio channel
        Ql_sprintf((char *)g_buffer, "AT+QAUDCH=2\n");
        break;
    
    default:
        Ql_DebugTrace("at commands finished.\r\n");
        exec = FALSE;
        break;
    }
    if (exec)
    {
      Ql_DebugTrace((char *)g_buffer);
      Ql_SendToModem(g_mdPort, (u8*)g_buffer, Ql_strlen(g_buffer));
    }
}

#endif // __EXAMPLE_TTS__

