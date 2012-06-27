#ifdef __EXAMPLE_FILESYSTEM__

#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_filesystem.h"
#include "ql_error.h"
#include "ql_fcm.h"

/*  Test Code */
char textBuf[100];
u8 strBuf[100];
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
    Ql_DebugTrace("filesystem: ql_entry\r\n");
    Ql_OpenModemPort(ql_md_port1);       

    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer);
        switch(flSignalBuffer.eventType)
        {
            case EVENT_MODEMDATA:
            {
                PortData_Event* pPortEvt = (PortData_Event*)&flSignalBuffer.eventData.modemdata_evt;
                Ql_DebugTrace("%s\r\n", pPortEvt->data);
                //Ql_DebugTrace("\r\nComing vPort data type = %d \r\n", pPortEvt->type);
                break;
            }
            case EVENT_UARTDATA:
            {                
                if (flSignalBuffer.eventData.uartdata_evt.len>0)
                {
                     pData = (char*)flSignalBuffer.eventData.uartdata_evt.data;

                    /*set last tail data eq 0*/
                    pData[flSignalBuffer.eventData.uartdata_evt.len+1] = '\0';

                    /*cmd:  Ql_GetLocalTime*/
                    p = Ql_strstr(pData,"Ql_GetLocalTime");
                    if (p)
                    {
                        bool ret;
                        QlSysTimer datetime;
                        ret=Ql_GetLocalTime(&datetime);
                        Ql_sprintf(textBuf,"Ql_GetLocalTime:%d/%d/%d %d:%d:%d\r\n",datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_SetLocalTime*/
                    p = Ql_strstr(pData,"Ql_SetLocalTime=");
                    if (p)
                    {
                        bool ret;
                        QlSysTimer datetime;
                        extern int atoi(const char*);

                        ret=Ql_GetLocalTime(&datetime);
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        datetime.year = (u16)atoi(p);
                        ret=Ql_SetLocalTime(&datetime);
                        Ql_sprintf(textBuf,"Ql_SetLocalTime()=%d\r\n",ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileGetFreeSize*/
                    p = Ql_strstr(pData,"Ql_FileGetFreeSize");
                    if (p)
                    {
                        s32 size;
                        size=Ql_FileGetFreeSize();
                        Ql_sprintf(textBuf,"Ql_FileGetFreeSize()=%d\r\n",size);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileOpen=cDir\File1.txt*/
                    /*Create file*/
                    p = Ql_strstr(pData,"Ql_FileOpen=c");
                    if (p)
                    {
                        s32 ret;
                        p = Ql_strstr(pData,"=c");
                        p += 2;
                        ret = Ql_FileOpen((u8*)p ,TRUE, FALSE);
                        if(ret >= QL_RET_OK)
                            filehandle = ret;
                        Ql_sprintf(textBuf,"Ql_FileOpen Create (%s)=%d\r\n", p, ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }        

                    /*cmd:  Ql_FileOpen=rDir\File1.txt*/
                    /*Open only read file*/
                    p = Ql_strstr(pData,"Ql_FileOpen=r");
                    if (p)
                    {
                        s32 ret;
                        p = Ql_strstr(pData,"=r");
                        p += 2;
                        ret = Ql_FileOpen((u8*)p ,FALSE, TRUE);
                        if(ret >= QL_RET_OK)
                            filehandle = ret;
                        Ql_sprintf(textBuf,"Ql_FileOpen OpenRead (%s)=%d\r\n", p, ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileOpenEx=oFlag=fDir\File1.txt*/
                    /*Open file*/
                    p = Ql_strstr(pData,"Ql_FileOpenEx=o");
                    if (p)
                    {
                        s32 ret;
                        char *pfile;
                        p = Ql_strstr(pData,"=o");
                        p += 2;
                        pfile = Ql_strstr(p ,"=f");
                        pfile[0] = '\0';
                        pfile += 2;
                        ret = Ql_FileOpenEx((u8*)pfile , atoi(p));
                        if(ret >= QL_RET_OK)
                            filehandle = ret;
                        Ql_sprintf(textBuf,"Ql_FileOpenEx Create (%s,%08x)=%d\r\n", pfile, atoi(p),ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    p = Ql_strstr(pData,"Ql_FileRead");
                    if (p)
                    {
                        s32 ret;
                        u32 readedlen;
                        Ql_memset(strBuf,0,100);
                        Ql_memset(textBuf,0,100);
                        ret = Ql_FileRead(filehandle, strBuf, 100, &readedlen);
                        Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileWrite=xxxxxxxxxxx*/
                    p = Ql_strstr(pData,"Ql_FileWrite=");
                    if (p)
                    {
                        s32 ret;
                        u32 writeedlen;
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        Ql_memset(textBuf,0,100);
                        ret = Ql_FileWrite(filehandle, (u8*)p, Ql_strlen((char*)p), &writeedlen);
                        Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileSeek=xxx                 xxx is pos*/
                    p = Ql_strstr(pData,"Ql_FileSeek=");
                    if (p)
                    {
                        s32 ret;
                        u32 writeedlen;
                        extern int atoi(const char*);
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        Ql_memset(textBuf,0,100);
                        ret = Ql_FileSeek(filehandle, atoi(p) , QL_FS_FILE_BEGIN);
                        Ql_sprintf(textBuf,"Ql_FileSeek()=%d: offset=%d\r\n",ret, atoi(p));
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    p = Ql_strstr(pData,"Ql_FileGetFilePosition");
                    if (p)
                    {
                        s32 ret;
                        u32 Position;
                        ret = Ql_FileGetFilePosition(filehandle, &Position);
                        Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    p = Ql_strstr(pData,"Ql_FileClose");
                    if (p)
                    {
                        Ql_FileClose(filehandle);
                        filehandle = -1;
                        Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileGetSize=Dir\File1.txt*/
                    p = Ql_strstr(pData,"Ql_FileGetSize");
                    if (p)
                    {
                        s32 ret;
                        u32 filesize;
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        ret = Ql_FileGetSize((u8*)p ,&filesize);
                        Ql_sprintf(textBuf,(char*)("Ql_FileGetSize(%s)=%d, filesize=%d\r\n"), p, ret, filesize);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileDelete=Dir\File1.txt*/
                    p = Ql_strstr(pData,"Ql_FileDelete");
                    if (p)
                    {
                        s32 ret;
                        s32 filesize;
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        ret = Ql_FileDelete((u8*)p );
                        Ql_sprintf(textBuf,"Ql_FileDelete(%s)=%d\r\n", p, ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd: Ql_FileRename=oldname,newname*/
                    p = Ql_strstr(pData,"Ql_FileRename=");
                    if (p)
                    {
                        char oldName[255] = {'\0'};
                        char newName[255] = {'\0'};
                        s32 ret;
                        char* p2;
                        p2 = Ql_strstr(pData, ",");
                        if (NULL == p2)
                        {
                            Ql_DebugTrace("Input error parameter\r\n");
                            break;
                        }
                        p = Ql_strstr(pData, "=");
                        Ql_strncpy(oldName, p + 1, p2 - p - 1);
                        p = p2;
                        p2 = Ql_strstr(pData, "\r\n");
                        Ql_strncpy(newName, p + 1, p2 - p - 1);
                        ret = Ql_FileRename((u8*)oldName, (u8*)newName);
                        Ql_sprintf(textBuf, "Ql_FileRename(\"%s\",\"%s\")=%d\r\n", oldName, newName, ret);
                        Ql_SendToUart(ql_uart_port1, (u8*)textBuf, Ql_strlen(textBuf));
                        Ql_SendToModem(ql_md_port1, (u8 *)"AT+QFLST\n", 9);
                        break;
                    }

                    /*cmd:  Ql_FileCheck=Dir\File1.txt*/
                    p = Ql_strstr(pData,"Ql_FileCheck=");
                    if (p)
                    {
                        s32 ret;
                        s32 filesize;
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        ret = Ql_FileCheck((u8*)p );
                        Ql_sprintf(textBuf,"Ql_FileCheck(%s)=%d\r\n", p, ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileCreateDir=Dir1\Dir2*/
                    p = Ql_strstr(pData,"Ql_FileCreateDir=");
                    if (p)
                    {
                        s32 ret;
                        s32 filesize;
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        ret = Ql_FileCreateDir((u8*)p );
                        Ql_sprintf(textBuf,"Ql_FileCreateDir(%s)=%d\r\n", p, ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileRemoveDir=Dir1\Dir2*/
                    p = Ql_strstr(pData,"Ql_FileRemoveDir=");
                    if (p)
                    {
                        s32 ret;
                        s32 filesize;
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        ret = Ql_FileRemoveDir((u8*)p );
                        Ql_sprintf(textBuf,"Ql_FileRemoveDir(%s)=%d\r\n", p, ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileCheckDir=Dir1\Dir2*/
                    p = Ql_strstr(pData,"Ql_FileCheckDir=");
                    if (p)
                    {
                        s32 ret;
                        s32 filesize;
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        ret = Ql_FileCheckDir((u8*)p );
                        Ql_sprintf(textBuf,"Ql_FileCheckDir(%s)=%d\r\n", p, ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileFindFirst=Dir1\Dir2\*.txt    */
                    /*cmd:  Ql_FileFindFirst=Dir1\Dir2\*    */
                    p = Ql_strstr(pData,"Ql_FileFindFirst=");
                    if (p)
                    {
                        s32 ret;
                        u32 filesize;
                        bool isdir;
                        s32 findfile;
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        findfile = Ql_FileFindFirst((u8*)p, (u8*)strBuf, 100, &filesize, &isdir);
                        if(findfile < 0)
                        {
                            Ql_sprintf(textBuf,"Failed Ql_FileFindFirst(%s)=%d\r\n", p, findfile);
                            Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                            break;
                        }
                        else
                        {
                            Ql_sprintf(textBuf,"Sueecss Ql_FileFindFirst(%s)\r\n", p);
                            Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        }
                        ret = findfile;
                        while(ret >= 0)
                        {
                            Ql_sprintf(textBuf,"filesize(%d),isdir(%d),Name(%s)\r\n", filesize, isdir, strBuf);
                            Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                            ret = Ql_FileFindNext(findfile, (u8*)strBuf, 100, &filesize, &isdir);
                            if(ret != QL_RET_OK)
                                break;
                        }
                        Ql_FileFindClose(findfile);
                        break;
                    }

                    /*cmd:  Ql_FileXDelete=Dir1\Dir2,flag*/
                    p = Ql_strstr(pData,"Ql_FileXDelete=");
                    if (p)
                    {
                        s32 ret;
                        char *pflag;
                        extern int atoi(const char*);
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        pflag = Ql_strstr(p,",");
                        pflag[0] = '\0';
                        pflag += 1;
                        ret = Ql_FileXDelete((u8*)p ,(u32)atoi(pflag));
                        Ql_sprintf(textBuf,"Ql_FileXDelete(%s,%x)=%d\r\n", p, atoi(pflag), ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }

                    /*cmd:  Ql_FileXMove=Dir1\Dir2,Dir1\Dir2,flag*/
                    p = Ql_strstr(pData,"Ql_FileXMove=");
                    if (p)
                    {
                        s32 ret;
                        char *p2;
                        char *pflag;
                        extern int atoi(const char*);
                        p = Ql_strstr(pData,"=");
                        p += 1;
                        p2 = Ql_strstr(p ,",");
                        p2[0] = '\0';
                        p2 += 1;
                        pflag = Ql_strstr(p2,",");
                        pflag[0] = '\0';
                        pflag += 1;
                        ret = Ql_FileXMove((u8*)p,(u8*)p2, (u32)(atoi(pflag)));
                        Ql_sprintf(textBuf,"Ql_FileXMove(%s.%s,%x)=%d\r\n", p, p2, atoi(pflag), ret);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                        break;
                    }                        
                }
                break;            
            }
            
            default:
                break;
        }
    }
}

#endif // __EXAMPLE_FILESYSTEM__

