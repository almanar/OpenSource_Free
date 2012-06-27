#ifdef __EXAMPLE_SMS__
 #include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_sms.h"
#include "ql_fcm.h"
#include "ql_api_type.h"
#include "ql_tcpip.h"


void  Callback_SetCSCA(bool result);
void  Callback_GetCSCA(bool result, u8* srvNum, u8 len);

void Test_SendSMS(u8 smsType)
{
    s32 ret;
    u64 t1, t2;

    if (QL_SMS_FMT_TXT == smsType)
    {
        /*Send TEXT SMS*/
        t1 = Ql_GetRelativeTime();
        ret=Ql_SendTextSMS((u8*)"13636619201", (u8*)"This is a test from quectel (text)");
        t2 = Ql_GetRelativeTime();
        Ql_DebugTrace("Time Consumption for Ql_SendTextSMS(): %lld.\r\n", t2 - t1);
        if (ret==0)
        {
            Ql_DebugTrace("OCPU: Send text sms ok\n");
        }
        else
        {
            Ql_DebugTrace("OCPU: Send text sms fail. cause=%d\n",ret);
        }
    }
    else  if (QL_SMS_FMT_PDU == smsType)
    {
        /*Send PDU SMS*/                                           
        t1 = Ql_GetRelativeTime();
        ret=Ql_SendPDUSMS(29, (u8*)"0011000D91683118876788F30008010E0061006200630064006500660067");
        t2 = Ql_GetRelativeTime();
        Ql_DebugTrace("Time Consumption for Ql_SendPDUSMS(): %lld.\r\n", t2 - t1);
        if (ret==0)
        {
            Ql_DebugTrace("OCPU: Send pdu sms ok\n");
        }
        else
        {
            Ql_DebugTrace("OCPU: Send pdu sms fail. cause=%d\n",ret);
        }
    }
}

QlEventBuffer    g_event;
void read_sms(char* buffer, u16 length)
{
    if (length==0)
    {
	    Ql_DebugTrace("Callback:length==0\r\n%s\r\n", buffer);
        return;
	}
    
    if (buffer != NULL)
    {
	    Ql_DebugTrace("Callback:\r\n%s\r\n", buffer);
	}
	else
	{
	    Ql_DebugTrace("Callback: No content.\r\n");
	}
}

void new_sms_notification(char* buffer, u16 length)
{
	Ql_DebugTrace("Callback: %s\r\n",buffer);
}

void ql_entry(void)
{
    bool keepGoing = TRUE;
    s32 ret;

    Ql_DebugTrace("sms: ql_entry\r\n");
    Ql_SetDebugMode(BASIC_MODE);
    Ql_OpenModemPort(ql_md_port1);

    Ql_SMSInitialize();
    Ql_SetNewSMSCallBack(&new_sms_notification);
         
    while(keepGoing)
    {    
        Ql_GetEvent(&g_event);
        switch(g_event.eventType)
        {
            case EVENT_MODEMDATA:
            {
                char* pdata; 
                PortData_Event* pEvent = (PortData_Event*)&g_event.eventData.modemdata_evt;
                // Ql_DebugTrace("\r\nsms len=%d : %s\r\n", Ql_strlen((char*)pEvent->data),  pEvent->data);
                Ql_DebugTrace("%s\r\n", pEvent->data);
                if (g_event.eventData.modemdata_evt.type==DATA_AT)
                {
                    pdata = (char*)g_event.eventData.modemdata_evt.data;
                    if (Ql_strlen(pdata)>=10 && Ql_strstr((char *)pdata,"Call Ready")!=NULL)
                    {
                        /*The initilization of SIM card complete, call SMS example function*/
                        //Test_SendSMS(QL_SMS_FMT_TXT);
                    }
                }
                break;
            }
            
            case EVENT_UARTDATA:
            {
                u8 temp;
                char* pChar;
                char* pChar2;
                char notes[60];
                char param[10];
                u8 sms_idx;
                u8 sms_mode;
                extern int atoi(const char*);
                PortData_Event* pDataEvt = (PortData_Event*)&g_event.eventData.uartdata_evt;

                // List sms (list sms)
                Ql_memset(notes, 0x0, sizeof(notes));
                Ql_sprintf(notes, "list sms\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, notes, Ql_strlen(notes));
                if (0 == ret)
                {
                    Ql_memset(notes, 0x0, sizeof(notes));
                    Ql_sprintf(notes, "AT+QINISTAT\n");
                    Ql_SendToModem(ql_md_port1, (u8*)notes, Ql_strlen(notes));
                    
                    ret = Ql_SetSMSFormat(QL_SMS_FMT_TXT);
                    ret = Ql_ReadSMSList(&read_sms);
                    Ql_DebugTrace("Ql_ReadSMSList() = %d\r\n", ret);
                    break;
                }

                // Send sms (send sms)
                Ql_memset(notes, 0x0, sizeof(notes));
                Ql_sprintf(notes, "send sms\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, notes, Ql_strlen(notes));
                if (0 == ret)
                {
                    Test_SendSMS(QL_SMS_FMT_TXT);
                    Ql_DebugTrace("Test_SendSMS(QL_SMS_FMT_TXT)\r\n");                    break;
                }
                
                // Read sms (read sms=10,1)
                /* 
                *   You can input the characters 'read sms=' + msm index and sms format (0 or 1) through UART1 to read message.
                *    ex., 'read sms=3,1' means read the 3rd message with text format.
                *           'read sms=3,0' means read the 3rd message with PDU format.
                *    if want to read allmessages, please set index to -1.
                */
                Ql_sprintf(notes, "read sms=");
                pChar = Ql_strstr((char*)pDataEvt->data, notes);
                if (pChar)
                {
                    // Retrieve message index
                    Ql_sprintf(notes, "=");
                    pChar = Ql_strstr((char*)pDataEvt->data, notes);
                    Ql_sprintf(notes, ",");
                    pChar2 = Ql_strstr((char*)pDataEvt->data, notes);
                    if (pChar != NULL && pChar2 != NULL)
                    {
                        Ql_memset(param, 0x0, sizeof(param));
                        Ql_strncpy(param, pChar + 1, pChar2 - pChar -1);
                        sms_idx = atoi(param);
                    }
                    else
                    {
                        Ql_DebugTrace("Incorrect parameter\r\n");
                        break;
                    }

                    // Retrieve message format
                    pChar = pChar2;
                    Ql_sprintf(notes, "\r\n");
                    pChar2 = Ql_strstr((char*)pDataEvt->data, notes);
                    if (pChar2 != NULL)
                    {
                        Ql_memset(param, 0x0, sizeof(param));
                        Ql_strncpy(param, pChar + 1, pChar2 - pChar -1);
                        sms_mode = atoi(param);
                        ret = Ql_SetSMSFormat(sms_mode);
                        if ( ret != QL_RET_OK)
                        {
                            Ql_DebugTrace("Fail to set sms format! sms mode: %d, error code: %d", sms_mode, ret);
                            break;
                        }
                        #if 0
                        ret = Ql_ReadSMS(sms_idx, &read_sms);
                        if ( ret != QL_RET_OK)
                        {
                            Ql_DebugTrace("Fail to read SMS ! index: %d, error code: %d", sms_idx, ret);
                            break;
                        }
                        Ql_DebugTrace("\r\nsms len=%d : %s\r\n", Ql_strlen(pChar),  pChar);	
                        #else
                        Ql_memset(notes, 0x0, sizeof(notes));
                        Ql_sprintf(notes, "AT+CMGR=%d\n", sms_idx);
                        Ql_SendToModem(ql_md_port1, (u8*)notes, Ql_strlen(notes));
                        #endif
                    }
                    else
                    {
                        Ql_DebugTrace("Need a <CR><LF> terminated string.\r\n");
                        break;
                    }
                }

                // Set SMS Centre Number (csca=+8613800210500)
                Ql_memset(notes, 0x0, sizeof(notes));
                Ql_sprintf(notes, "csca=");
                pChar = Ql_strstr((char*)pDataEvt->data, notes);
                if (pChar)
                {
                    pChar = Ql_strstr((char*)pDataEvt->data, "=");
                    if (pChar)
                    {
                        char* p2;
                        u8 len;
                        p2 = Ql_strstr((char*)pDataEvt->data, "\r\n");
                        len = p2 - pChar -1;
                        Ql_memcpy(notes, pChar + 1, len);
                        ret = Ql_SetInfoCentreNum((u8*)notes, len, Callback_SetCSCA);
                        Ql_DebugTrace("Ql_SetInfoCentreNum() = %d, number: %s\r\n", ret, notes);
                        Ql_sprintf(notes, "AT+CSCA?\n");
                        Ql_SendToModem(ql_md_port1, (u8*)notes, Ql_strlen(notes));
                     }
                     break;
                 }
                 
                // Get SMS Centre Number (csca?)
                Ql_memset(notes, 0x0, sizeof(notes));
                Ql_sprintf(notes, "csca?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, notes, Ql_strlen(notes));
                if (0 == ret)
                {
                    ret = Ql_GetInfoCentreNum(Callback_GetCSCA);
                    Ql_DebugTrace("Ql_GetInfoCentreNum() = %d\r\n", ret);
                    break;
                }
                
                // Get device state (device state?)
                Ql_memset(notes, 0x0, sizeof(notes));
                Ql_sprintf(notes, "device state?\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, notes, Ql_strlen(notes));
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
            }
            
            default:
                break;
        }
    }
    Ql_SMSUnInitialize();
}

void  Callback_SetCSCA(bool result)
{
    Ql_DebugTrace("Callback_SetCSCA()\r\n");
}
void  Callback_GetCSCA(bool result, u8* srvNum, u8 len)
{
    Ql_DebugTrace("Callback_GetCSCA(): %s\r\n", srvNum);
}

#endif //__EXAMPLE_SMS__

