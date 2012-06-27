#ifdef __EXAMPLE_FOTA__
 
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

char * pData=NULL;
char buffer[100];
#define UA_DATA_BUfFER_LEN 128
u8 deltabuf[UA_DATA_BUfFER_LEN];

u16 idxCmd;
void SendAtCmd();
ascii coreBinFile[100];
ascii appBinFile[100];

void start_fota(u8 mode);

QlEventBuffer    flSignalBuffer;
void ql_entry()
{
    bool keepGoing = TRUE;
    s32 period = 10;
    QlSysTimer t;
    u32 ret,ret1;
    bool initialized = FALSE;
    bool bgnCheck = FALSE;
    idxCmd = 0;
    
    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("\r\nfota: ql_entry (app 1)\r\n");

    // Open Virtual Modem Serial Port1
    Ql_OpenModemPort(ql_md_port1);
                    
    //Ql_sprintf(coreBinFile,"core-delta.bin");
    Ql_sprintf(coreBinFile,"core-delta.bin");
    Ql_sprintf(appBinFile,"app-delta.bin");
  
    // 1. Retieve Some Settings
    bgnCheck = TRUE;
    idxCmd = 1;
    SendAtCmd();

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer);
        switch(flSignalBuffer.eventType)
        {
            case EVENT_MODEMDATA:
            {                
                PortData_Event* pDataEvt = (PortData_Event*)&flSignalBuffer.eventData.modemdata_evt;
                Ql_DebugTrace("\r\nEVENT_MODEMDATA type=%d \r\n", pDataEvt->type);
                
                if (pDataEvt->type==DATA_AT)
                {
                    Ql_DebugTrace("\r\nModem data =%s\r\n", pDataEvt->data);
                    if ((Ql_strstr((char*)pDataEvt->data, "\r\nOK") != NULL || 
                          Ql_strstr((char*)pDataEvt->data, "OK\r\n") != NULL ||
                          Ql_strstr((char*)pDataEvt->data, "ERROR") != NULL)
                        && bgnCheck != FALSE)
                    {
                        idxCmd++;
                        SendAtCmd();
                    }
                }
                break;
            }
            
            case EVENT_UARTDATA:
            {
                PortData_Event* pDataEvt = (PortData_Event*)&flSignalBuffer.eventData.uartdata_evt;
                Ql_DebugTrace("\r\nEVENT_UARTDATA PORT=%d\r\n", pDataEvt->port);
                Ql_DebugTrace("\r\nuart data =%s\r\n", pDataEvt->data);        
                
                Ql_sprintf(buffer, "AT+QLOCPU=0\r\n");
                ret = Ql_strncmp((char*)pDataEvt->data, buffer, Ql_strlen(buffer));
                if (0 == ret)
                {
                    Ql_SendToModem(ql_md_port1, (u8*)buffer, Ql_strlen(buffer));
					break;
                }

				// start fota update (start fota=1/2)
				Ql_memset(buffer, 0x0, sizeof(buffer));
				Ql_sprintf(buffer, "start fota=");
				pData = Ql_strstr((char*)pDataEvt->data, buffer);
				if (pData)
				{
					u8 appUpdateMode;
					char* p1;
					char* p2;
					char appMode[10] = {'\0'};
					Ql_DebugTrace((char*)pDataEvt->data);
					p1 = Ql_strstr((char*)pDataEvt->data, "=");
					p2 = Ql_strstr((char*)pDataEvt->data, "\r\n");
					Ql_memset(appMode, 0x0, sizeof(appMode));
					memcpy(appMode, p1 + 1, p2 - p1 -1);
					appUpdateMode = atoi(appMode);
					start_fota(appUpdateMode);
					break;
				}
			}
            
            case EVENT_SERIALSTATUS:
            {
                bool val = flSignalBuffer.eventData.portstatus_evt.val;
                u8 port = flSignalBuffer.eventData.portstatus_evt.port;
                u8 type = flSignalBuffer.eventData.portstatus_evt.type;

                Ql_DebugTrace("EVENT_SERIALSTATUS port=%d type=%d val=%d\r\n",port,type,val);
                break;
            }
            
            default:
                break;
        }
    }
}

void start_fota(u8 mode)
{
	u32 filesize,fd_file;
	u8 *file_buffer=NULL;
	u16 off=0;
	s32 ret2;
	u32 realLen;
	u8 updateItem = 0;

	if (mode != FOTA_APP_DELTA && mode != FOTA_APP_COVER)
	{
        Ql_DebugTrace("\r\n Input error parameter\n");
	    return;
	}

	//1. Update Core System
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
            if (filesize <= UA_DATA_BUfFER_LEN)
            {
                Ql_FileRead(fd_file, file_buffer, filesize, &realLen);
                Ql_Fota_Core_Write_Data(filesize,(s8*)file_buffer);
                filesize=0;
            }else
            {
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

    //2. Update Embedded App.
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
        Ql_Fota_App_Init(mode);            

        
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
    /*
    // Delete Upgrade File Bins.
    if ((updateItem & FOTA_UPDATE_FLAG_CORE) == FOTA_UPDATE_FLAG_CORE)
    {// Delete Fota Core Bin
        Ql_DebugTrace("\r\n Delete Fota Core Bin\n");
        Ql_FileDelete(coreBinFile);
    }
    if ((updateItem & FOTA_UPDATE_FLAG_APP) == FOTA_UPDATE_FLAG_APP)
    {// Delete Fota App Bin
        Ql_DebugTrace("\r\n Delete Fota App Bin\n");
        Ql_FileDelete(appBinFile);
    }
    */
    Ql_DebugTrace("\r\n\r\n");
    Ql_Fota_Update(updateItem);
}


void SendAtCmd()
{
  bool exec = 1;
  switch (idxCmd)
  {
    //1. Get...
        case 1:// ATI
        Ql_sprintf((char *)buffer,"ATI\r\n");
        break;
    case 2:// Ver
        Ql_sprintf((char *)buffer,"AT+CSUB\r\n");
        break;
/*
    case 2:// UFS
        Ql_sprintf((char *)buffer,"AT+QFLDS=\"UFS\"\r\n");
        break;
    case 2:// CMGF
        Ql_sprintf((char *)buffer,"AT+CMGF?\r\n");
        break;
    case 3:// CLIP
        Ql_sprintf((char *)buffer,"AT+CLIP?\r\n");
        break;
    case 4:// CMEE
        Ql_sprintf((char *)buffer,"AT+CMEE?\r\n");
        break;
    case 5:// QIFGCNT
        Ql_sprintf((char *)buffer,"AT+QIFGCNT?\r\n");
        break;
    case 6:// QISCON
        Ql_sprintf((char *)buffer,"AT+QISCON?\r\n");
        break;

    //2.    Set...
    case 101:// CMGF
        Ql_sprintf((char *)buffer,"AT+CMGF=1&W\r\n");
        break;
    case 102:// CLIP
        Ql_sprintf((char *)buffer,"AT+CLIP=1&W\r\n");
        break;
    case 103:// CMEE
        Ql_sprintf((char *)buffer,"AT+CMEE=2&W\r\n");
        break;
    case 104:// QIFGCNT
        Ql_sprintf((char *)buffer,"AT+QIFGCNT=0\r\n");
        break;
    case 105:// QISCON
        Ql_sprintf((char *)buffer,"AT+QICSGP=0,\"17201\",\"172\",\"172\",2\r\n");
        break;
    case 106:// QISCON - Save
        Ql_sprintf((char *)buffer,"AT+QISCON\r\n");
        break;
    */    
    default:
        exec = 0;
        break;
  }
  if (exec != 0)
  {
      Ql_DebugTrace((char *)buffer);
      Ql_SendToModem(ql_md_port1, (u8*)buffer, Ql_strlen(buffer));
  }
}

#endif // __EXAMPLE_FOTA__

