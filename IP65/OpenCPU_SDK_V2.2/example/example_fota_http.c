#ifdef __EXAMPLE_FOTA_HTTP__
 
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_fcm.h"
#include "ql_filesystem.h"
#include "ql_fota.h"

typedef enum tagATCmdType
{
    AT_General,
    AT_QISTAT,
    AT_QHTTPURL,
    AT_QHTTPREAD,
    AT_QIDEACT
}ATCmdType;

u8 g_app_mode = FOTA_MODE_NONE;
void DoFOTAUpgrade();

#define DELTA_BIN_URL   "http://queclink.3322.org:8000/deltabin/download/"
ascii appBin_fName_svr[100] = { 0x0 };

QlTimer  timer;
char * pData=NULL;
char buffer[100];
#define UA_DATA_BUfFER_LEN 128
u8 deltabuf[UA_DATA_BUfFER_LEN];

u8   g_cmd_type;
u16 g_cmd_idx;
void SendAtCmd();
ascii coreBinFile[100] = { 0x0 };
ascii appBinFile[100] = { 0x0 };

QlEventBuffer    g_event;
void ql_entry()
{
    s32 ret;
    bool keepGoing = TRUE;
    bool bgnCheck = FALSE;
    QlSysTimer t;
    s32 hdlFile = -1;
    u32 fileSize = 0;
    
    g_cmd_idx = 0;
    
    Ql_sprintf(coreBinFile,"core-delta.bin");
    Ql_sprintf(appBinFile,"app-delta_U.bin");
      
    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("fota_http: ql_entry\r\n");
    Ql_DebugTrace("I am fota 1.\r\n");
    Ql_DebugTrace("I am fota 1.\r\n");
    Ql_OpenModemPort(ql_md_port1);
                    
    // Retieve Some Settings
    bgnCheck = TRUE;

    while(keepGoing)
    {    
        Ql_GetEvent(&g_event);
        switch(g_event.eventType)
        {
            case EVENT_TIMER:
                break;
                
            case EVENT_MODEMDATA:
            {
                PortData_Event* pPortEvt = (PortData_Event*)&g_event.eventData.modemdata_evt;
                //Ql_DebugTrace("\r\nComing vPort data type = %d \r\n", pPortEvt->type);
                if (DATA_AT == pPortEvt->type)
                {
                    //Ql_DebugTrace("Modem data =%s\r\n", pPortEvt->data);
                    Ql_DebugTrace("%s\r\n", pPortEvt->data);
                    if (Ql_strstr((char *)pPortEvt->data, "Call Ready") != NULL)
                    {
                        // Wait 2s for the stable signal quality
                        //timer.timeoutPeriod = Ql_SecondToTicks(2);
                        //Ql_StartTimer(&timer);
                    }
                    else if (g_cmd_type == AT_QHTTPURL && Ql_strstr((char*)pPortEvt->data, "CONNECT\r\n") != NULL)
                    {
                        g_cmd_idx++;
                        SendAtCmd();
                    }
                    else if (g_cmd_type == AT_QHTTPREAD && Ql_strstr((char*)pPortEvt->data, "CONNECT\r\n") != NULL)
                    {
                        // Create/Open the delta bin file to accept data from http server
                        hdlFile = Ql_FileOpen((u8*)appBinFile ,TRUE, FALSE);
                        if (hdlFile < QL_RET_OK)
                        {
                            Ql_DebugTrace("Fail to create or open file '%s' !Error code: %d.\r\n", appBinFile, hdlFile);
                        }
                    }
                    else if  (g_cmd_type == AT_QHTTPREAD && Ql_strstr((char*)pPortEvt->data, "OK\r\n") != NULL)
                    {
                        // Close file handle
                        Ql_FileClose(hdlFile);
                        Ql_DebugTrace("File Total Size: %d\r\n", fileSize);
                        
                        // Deactivate gprs PDP context
                        g_cmd_idx++;
                        SendAtCmd();
                        
                        // Pick up the data between 'CONNECT\r\n' and 'OK\r\n', and store to "app-delta.bin"
                    }
                    else if ((Ql_strstr((char*)pPortEvt->data, "\r\nOK") != NULL || 
                                   Ql_strstr((char*)pPortEvt->data, "OK\r\n") != NULL ||
                                   Ql_strstr((char*)pPortEvt->data, "ERROR") != NULL)
                                   && bgnCheck != FALSE)
                    {
                        g_cmd_idx++;
                        SendAtCmd();
                    }
                }
                else if (DATA_TCP_T == pPortEvt->type)
                {
                    // Write data to "app-delta.bin"
                    u32 writtenLen;
                    //Ql_DebugTrace("DATA_TCP_T, len:%d\r\n", pPortEvt->len);
                    Ql_SendToUart(ql_uart_port2, (u8*)pPortEvt->data, pPortEvt->len);
                    ret = Ql_FileWrite(hdlFile, (u8*)pPortEvt->data, pPortEvt->len, &writtenLen);
                    fileSize += writtenLen;
                }
                break;
            }
            
            case EVENT_UARTDATA:
            {
                char* pos;
                PortData_Event* pDataEvt = (PortData_Event*)&g_event.eventData.uartdata_evt;

                // AT+QLOCPU=0
                Ql_sprintf(buffer, "AT+QLOCPU=0\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    Ql_SendToModem(ql_md_port1, (u8*)"AT+QLOCPU=0\n", Ql_strlen(buffer) + 1);
                    break;            
                }

                // start fota (start fota=1/2,app-delta.bin)
                Ql_sprintf(buffer, "start fota=");
                pos = Ql_strstr((char*)pDataEvt->data, buffer);
                if (pos != NULL)
                {
                    char appMode[2] = {'\0'};
                    Ql_strncpy(appMode, pos + Ql_strlen(buffer), 1);
                    g_app_mode = atoi(appMode);
                    if (g_app_mode != FOTA_APP_DELTA && g_app_mode != FOTA_APP_COVER)
                    {
                        Ql_DebugTrace("\r\n Input error parameter\n");
                    	break;
                    }
                    pos = Ql_strstr((char*)pDataEvt->data, ",");
                    if (NULL == pos)
                    {
                        Ql_DebugTrace("\r\n Please input right parameters\n");
                    	break;
                    }
                    Ql_strncpy(appBin_fName_svr, pos + 1, pDataEvt->len - Ql_strlen(buffer) - 2 - 2);   // 2 for '\r\n'
                    Ql_sprintf(buffer, "url for delta bin: %s%s\r\n", DELTA_BIN_URL, appBin_fName_svr);
                    Ql_SendToUart(ql_uart_port1, (u8 *)buffer, Ql_strlen(buffer));

                    // Start to work...
                    g_cmd_idx = 1;
                    SendAtCmd();
                    break;
                }
                break;            
            }
            
            case EVENT_SERIALSTATUS:
            {
                PortStatus_Event* pPortStatus = (PortStatus_Event*)&g_event.eventData.portstatus_evt;
                bool val = pPortStatus->val;
                u8 port = pPortStatus->port;
                u8 type = pPortStatus->type;

                Ql_DebugTrace("EVENT_SERIALSTATUS port=%d type=%d val=%d\r\n",port,type,val);
                break;
            }
            
            default:
                break;
        }
    }
}

void SendAtCmd()
{
    bool exec = TRUE;
    switch (g_cmd_idx)
    {
    case 1:// Echo mode off
        Ql_sprintf((char *)buffer, "ATE0\n");
        g_cmd_type = AT_General;
        break;
        
    case 2:// Select a foreground context
        Ql_sprintf((char *)buffer, "AT+QIFGCNT=1\n");
        g_cmd_type = AT_General;
        break;
        
    case 3:// Select a bearer (0=CSD, 1=GPRS), if 'GPRS', set APN
        Ql_sprintf((char *)buffer, "AT+QICSGP=1,\"CMNET\"\n");
        g_cmd_type = AT_General;
        break;
        
    case 4:// Set HTTP server URL
                // after 'CONNECT' appears, input URL address string
    {
        u16 url_len;
        url_len = Ql_strlen(DELTA_BIN_URL) + Ql_strlen(appBin_fName_svr);
        Ql_sprintf((char *)buffer, "AT+QHTTPURL=%d,%d\n", url_len, 60/*60s timeout*/);
        g_cmd_type = AT_QHTTPURL;
        break;
    }

    case 5:// Input URL address string
        Ql_sprintf((char *)buffer, "%s%s\n", DELTA_BIN_URL, appBin_fName_svr);
        g_cmd_type = AT_General;
        break;
        
    case 6:// Send HTTP get request
        Ql_sprintf((char *)buffer, "AT+QHTTPGET=%d\n", 120/*2min timeout*/);
        g_cmd_type = AT_General;
        break;
        
    case 7:// Read data
        Ql_sprintf((char *)buffer, "AT+QHTTPREAD=%d\n", 120/*2min timeout*/);
        g_cmd_type = AT_QHTTPREAD;
        break;
        
    case 8:// List files
        Ql_sprintf((char *)buffer, "AT+QFLST\n");
        g_cmd_type = AT_General;
        break;
        
    case 9:// Deactivate CSD or GPRS PDP context
        Ql_sprintf((char *)buffer, "AT+QIDEACT\n");
        g_cmd_type = AT_General;
        break;

    default:
        Ql_DebugTrace("at commands finished.\r\n");
        g_cmd_type = AT_General;
        exec = FALSE;

        // Start to fota upgrade
        DoFOTAUpgrade();
        break;
    }
    if (exec)
    {
      Ql_DebugTrace((char *)buffer);
      Ql_SendToModem(ql_md_port1, (u8*)buffer, Ql_strlen(buffer));
    }
}

void DoFOTAUpgrade()
{
    u32 filesize,fd_file;
    u8 *file_buffer=NULL;
    u16 off=0;
    s32 ret2;
    u32 realLen;
    u8 updateItem = 0;
    bool initialized = FALSE;

    //1. Copy Core Delta Data
    ret2 = Ql_FileCheck((u8*)coreBinFile);
    if ( QL_RET_OK == ret2)
    {
        /*Read Core Delta bin*/
        if(Ql_FileGetSize((u8*)coreBinFile, &filesize) < 0)
        {
            Ql_DebugTrace("\r\n Fail to get size (Core)\n");
            return;
        }
        else
        {
            Ql_DebugTrace("\r\n Get size successfully. Core bin size=%d\n",filesize);

        }

        fd_file = Ql_FileOpen((u8*)coreBinFile, 0,1);
        if(fd_file < 0)
        {
            Ql_DebugTrace("\r\n Fail to open (Core))\n");
            return ;
        }
        file_buffer = deltabuf;
        off=0;

        /*Write Core Delta bin to flash*/
        Ql_Fota_Core_Init();            
        
        while(filesize>0)
        {
            Ql_DebugTrace("\r\nfilesize=%d\n",filesize);    
            if (filesize<=UA_DATA_BUfFER_LEN)
            {
                Ql_FileRead(fd_file, file_buffer, filesize, &realLen);
                Ql_Fota_Core_Write_Data(filesize,(s8*)file_buffer);
                filesize=0;
            }else{
                Ql_FileRead(fd_file, file_buffer, UA_DATA_BUfFER_LEN, &realLen);
                Ql_Fota_Core_Write_Data(UA_DATA_BUfFER_LEN,(s8*)file_buffer);
                filesize -= UA_DATA_BUfFER_LEN;                    
            }
        }

        Ql_Fota_Core_Finish();
        
        Ql_FileClose(fd_file);

        updateItem = updateItem | FOTA_UPDATE_FLAG_CORE;
    }
    else if (QL_RET_ERR_FILENOTFOUND == ret2)
    {
        Ql_DebugTrace("\r\nCore Delta Bin does not exsit\n");
    }

    //2. Copy Embedded App Delta Data.
    /*Read App Delta bin*/
    ret2 = Ql_FileCheck((u8*)appBinFile);
    if ( QL_RET_OK == ret2)
    {
        if(Ql_FileGetSize((u8*)appBinFile, &filesize) < 0)
        {
            Ql_DebugTrace("\r\n Fail to get size (App)\n");
            return;
        }
        else
        {
            Ql_DebugTrace("\r\n Get size Successfully. App bin size=%d\n",filesize);

        }

        fd_file = Ql_FileOpen((u8*)appBinFile, 0,1);
        if(fd_file < 0)
        {
            Ql_DebugTrace("\r\n Fail to open (App)\n");
            return ;
        }
        file_buffer = deltabuf;
        off=0;

        /*Write App Delta bin to flash*/
        Ql_Fota_App_Init(g_app_mode);            

        
        while(filesize>0)
        {
            if (filesize <= UA_DATA_BUfFER_LEN)
            {
                Ql_FileRead(fd_file, file_buffer, filesize, &realLen);
                Ql_Fota_App_Write_Data(filesize,(s8*)file_buffer);
                filesize=0;
            }else
            {
                Ql_FileRead(fd_file, file_buffer, UA_DATA_BUfFER_LEN, &realLen);
                Ql_Fota_App_Write_Data(UA_DATA_BUfFER_LEN,(s8*)file_buffer);
                filesize -= UA_DATA_BUfFER_LEN;                    
            }
        }

        Ql_Fota_App_Finish();
        
        Ql_FileClose(fd_file);

        updateItem = updateItem | FOTA_UPDATE_FLAG_APP;
    }
    else if (QL_RET_ERR_FILENOTFOUND == ret2)
    {
        Ql_DebugTrace("\r\nApp Delta Bin does not exsit\n");
    }

    // 3. Delete Delta Bin Files.
    if ((updateItem & FOTA_UPDATE_FLAG_CORE) == FOTA_UPDATE_FLAG_CORE)
    {// Delete Fota Core Bin
        Ql_DebugTrace("\r\n Delete Fota Core Bin\n");
        Ql_FileDelete((u8*)coreBinFile);
    }
    if ((updateItem & FOTA_UPDATE_FLAG_APP) == FOTA_UPDATE_FLAG_APP)
    {// Delete Fota App Bin
        Ql_DebugTrace("\r\n Delete Fota App Bin\n");
        Ql_FileDelete((u8*)appBinFile);
    }
    Ql_DebugTrace("\r\n\r\n");

    // 4. Start to upgrade
    Ql_Fota_Update(updateItem);
}

#endif // __EXAMPLE_FOTA_HTTP__

