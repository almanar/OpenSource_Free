#ifdef __EXAMPLE_TCPLONG__

#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_pin.h"
#include "Ql_multitask.h"
#include "Ql_tcpip.h"
#include "Ql_fcm.h"


/*************************************************************
*create tcp connect, interval 5s send 100byte to server, and server response data
*when disconnect, program first query dns, then reconnect tcp server, resend
**************************************************************/


/*************************************************************
* config network param
**************************************************************/
#define HOST_NAME "quectel.3322.org"    // host name
//#define HOST_NAME "116.226.37.104"    //  host ip
u16 port = 7021; // ip port
#define APN_NAME  "CMNET\0"

#define NORMAL_TIMER  800 // 800ms
#define SENDDATA_TIMER  30000 // 20000ms

#define STATE_QUERY_DNS_MAX_NUMBER 10
#define QUERY_NDS_CALLBACK_TIME 60000


#define DATA_LEN 2048


/*************************************************************
* implement
**************************************************************/
#define OUT_DEBUG(x,...)  \
    Ql_memset((void*)(x),0,100);  \
    Ql_sprintf((char*)(x),__VA_ARGS__);   \
    Ql_SendToUart(ql_uart_port1,(u8*)(x),Ql_strlen((const char*)(x)));

void CallBack_GprsAPNSet(bool result, s32 error_code);
void CallBack_GprsAPNGet(u8 profileid, bool result, s32 error_code, u8 *apn, u8 *userId, u8 *password);
void CallBack_getipbyname(u8 contexid, bool result, s32 error, u8 num_entry, u8 *entry_address);
void CallBack_network_actived(u8 contexid);
void CallBack_network_deactived(u8 contexid,  s32 error_cause, s32 error);
void CallBack_socket_connect(u8 contexid, s8 sock, bool result, s32 error);
void CallBack_socket_close(u8 contexid, s8 sock, bool result, s32 error);   
void CallBack_socket_accept(u8 contexid, s8 sock, bool result, s32 error);
void CallBack_socket_read(u8 contexid, s8 sock, bool result, s32 error);
void CallBack_socket_write(u8 contexid, s8 sock, bool result, s32 error);

OpenCpuTcpIp_Callback_t     callback_func = 
{
    CallBack_network_actived,
    CallBack_network_deactived,
    CallBack_socket_connect,
    CallBack_socket_close,
    CallBack_socket_accept,
    CallBack_socket_read,
    CallBack_socket_write
};

typedef enum{
    STATE_QUERY_NETWORKSTATUS,
    STATE_GET_APN,
    STATE_GET_APN_ING,
    STATE_SET_APN,
    STATE_SET_APN_ING,
    STATE_ACTIVE_PDP,
    STATE_ACTIVE_PDP_ING,
    STATE_QUERY_DNS,
    STATE_QUERY_DNS_ING,
    STATE_CREATE_TCPCONNECT,
    STATE_CREATE_TCPCONNECT_ING,
    STATE_TRANSFERS_TCPDATA,
    STATE_TRANSFERS_TCPDATA_ING,
    STATE_TRANSFERS_TCPDATA_CHECKACK,
    STATE_DEACTIVE_PDP,
    STATE_TOTAL_NUM
}STATE_TCPLONG;


STATE_TCPLONG  nSTATE_TCPLONG = STATE_QUERY_NETWORKSTATUS;
u8 ipaddress[4];
u8 tcpsocket;

//////////////////////////////////////////////////////////////////////
//Debug
char textBuf[100];
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//send data
void *pMemSend = NULL;
void *pCurrentPos = NULL;
u32  remain_len;
u64 ackedNum = 0;

u32  serialnum = 0;


u32 query_dns_number = 0;
QlTimer timer_querydns;

/**************************************************************
* this is main task
***************************************************************/
 
QlEventBuffer    flSignalBuffer; // Set flSignalBuffer to global variables  may as well, otherwise it will occupy stack space
void ql_entry()
{
    bool keepGoing = TRUE;
    s32 ret;
    QlTimer timer;
    
    Ql_SetDebugMode(BASIC_MODE);   
    Ql_DebugTrace("tcplong: ql_entry\r\n");
    Ql_OpenModemPort(ql_md_port1);


    ret = Ql_GprsNetworkInitialize(0,  0, &callback_func);
    //if(ret != QL_SOC_SUCCESS) failed


    timer.timeoutPeriod = Ql_MillisecondToTicks(NORMAL_TIMER); //800 millisecond
    Ql_StartTimer(&timer);
    
    while(keepGoing)
    {    
        Ql_GetEvent(&flSignalBuffer);
        switch(flSignalBuffer.eventType)
        {
            case EVENT_TIMER:
            {
                
                if( timer_querydns.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
                {
                    nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
                    OUT_DEBUG(textBuf,"STATE_QUERY_DNS timerout, will Deactive\r\n");
                    Ql_GprsNetworkDeactive(0);
                    break;
                    
                }
                else if( timer.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
                {
                    switch(nSTATE_TCPLONG)
                    {
                        case  STATE_QUERY_NETWORKSTATUS:
                        {
                            u8 ps_status;
                            s32 simcard;
                            s32 creg;
                            s32 cgreg;
                            u8 rssi;
                            u8 ber;
                            /*the Ql_GetDeviceCurrentRunState function can replace these AT Command:  AT+CPIN? AT+CREG? AT+CGREG? AT+CSQ*/
                            Ql_GetDeviceCurrentRunState(&simcard, &creg, &cgreg, &rssi, &ber);
                            OUT_DEBUG(textBuf,"Ql_GetDeviceCurrentRunState(simcard=%d, creg=%d, cgreg=%d, rssi=%d, ber=%d)\r\n", simcard, creg, cgreg, rssi, ber);
                            if(simcard != 1)
                            {
                                OUT_DEBUG(textBuf,"Warning SIM card error, please check\r\n");
                                /*Warning,Warning,Warning,Warning,Warning!!!!!!!!!!!!!!!!!! Please check or reset your system when timeout*/
                            }
                            else if(creg != 1)
                            {
                                OUT_DEBUG(textBuf,"Warning GSM network is Registing, Please waiting\r\n");
                                /*Warning,Warning,Warning,Warning,Warning!!!!!!!!!!!!!!!!!! Please reset your system when timeout*/
                            }
                            else if(cgreg != 1)
                            {
                                OUT_DEBUG(textBuf,"Warning GPRS network is Registing, Please waiting\r\n");
                                /*Warning,Warning,Warning,Warning,Warning!!!!!!!!!!!!!!!!!! Please reset your system when timeout*/
                            }
                            else
                            {
                                ret = Ql_GprsNetworkGetState(0, NULL, &ps_status);
                                //if(QL_SOC_SUCCESS != ret) failed
                                OUT_DEBUG(textBuf,"Ql_GprsNetworkGetState(),ps_status=%d\r\n", ps_status);
                               if(ps_status == 1) //REG_STATE_REGISTERED
                                {
                                    nSTATE_TCPLONG = STATE_GET_APN; 
                                  OUT_DEBUG(textBuf,"GPRS Network Attach OK\r\n");
                                }
                                else
                                {
                                    //donot REG_STATE_REGISTERED, continue wait network gprs attch ok by timer
                                }
                            }
                            break;
                        }

                        case  STATE_GET_APN:
                        {
                            nSTATE_TCPLONG = STATE_GET_APN_ING;
                            ret = Ql_GprsAPNGet(0, CallBack_GprsAPNGet);
                            //if(QL_SOC_WOULDBLOCK != ret) failed
                            OUT_DEBUG(textBuf,"Ql_GprsAPNGet(%d)=%d\r\n",0, ret);
                            //will waiting CallBack_GprsAPNGet be called
                            break;
                        }

                        case  STATE_SET_APN:
                        {
                            nSTATE_TCPLONG = STATE_SET_APN_ING;
                            ret = Ql_GprsAPNSet(0, (u8*)APN_NAME, (u8*)"", (u8*)"", CallBack_GprsAPNSet);
                            //if(QL_SOC_WOULDBLOCK != ret) failed
                            OUT_DEBUG(textBuf,"Ql_GprsAPNSet()=%d\r\n",ret);
                            //will waiting CallBack_GprsAPNSet be called
                            break;
                        }

                        case  STATE_ACTIVE_PDP:
                        {
                            nSTATE_TCPLONG = STATE_ACTIVE_PDP_ING;
                            ret = Ql_GprsNetworkActive(0); // to active contxtid=0 pdp
                            //if((QL_SOC_SUCCESS != ret) && (QL_SOC_WOULDBLOCK != ret)) failed
                            OUT_DEBUG(textBuf,"Ql_GprsNetworkActive(contxtid=%d)=%d\r\n",0,ret);
                            //if QL_SOC_WOULDBLOCK will waiting CallBack_network_actived or CallBack_network_deactived
                            if(QL_SOC_SUCCESS == ret)
                                nSTATE_TCPLONG = STATE_QUERY_DNS; // will to Ql_GetHostIpbyName
                            break; 
                        }
                        
                        case  STATE_QUERY_DNS:
                        {
                            query_dns_number++;
                            //when network signal is bad, csq only 3-7, but you query dns will failed
                            if(query_dns_number > STATE_QUERY_DNS_MAX_NUMBER)
                            {
                                query_dns_number = 0;
                                nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
                                OUT_DEBUG(textBuf,"STATE_QUERY_DNS %d >  STATE_QUERY_DNS_MAX_NUMBER(%d)\r\n",query_dns_number,STATE_QUERY_DNS_MAX_NUMBER);
                                Ql_GprsNetworkDeactive(0);
                                break;
                            }
                            nSTATE_TCPLONG = STATE_QUERY_DNS_ING;
                            ret = Ql_SocketCheckIp((u8*)HOST_NAME, (u32*)ipaddress);
                            if(ret == QL_SOC_SUCCESS) // is ip address, xxx.xxx.xxx.xxx
                            {
                                nSTATE_TCPLONG = STATE_CREATE_TCPCONNECT; // to connect tcp server
                                query_dns_number = 0;
                            }
                            else if(ret == QL_SOC_ERROR) // is host name
                            {
                                u8 addr[4*5];
                                u8 *entryaddr;
                                u8 addr_len = 0;
                                u8 out_entry_num,i;
                                ret = Ql_GetHostIpbyName(0, (u8*)HOST_NAME, addr, &addr_len, 5, &out_entry_num, CallBack_getipbyname);
                                OUT_DEBUG(textBuf,"Ql_GetHostIpbyName()=%d\r\n",ret);
                                if(ret == QL_SOC_SUCCESS)
                                {
                                    query_dns_number = 0;
                                    Ql_memcpy((void*)ipaddress, (void*)addr, 4);
                                    nSTATE_TCPLONG = STATE_CREATE_TCPCONNECT; // to connect tcp server
                                    entryaddr = (u8*)addr;
                                    for(i=0;i<out_entry_num;i++)
                                    {
                                        entryaddr += (i*4);
                                        OUT_DEBUG(textBuf,"entry=%d, ip=%d.%d.%d.%d\r\n",i,entryaddr[0],entryaddr[1],entryaddr[2],entryaddr[3]);
                                    }
                                }
                                else if(ret == QL_SOC_WOULDBLOCK)
                                {
                                    //waiting CallBack_getipbyname be called
                                    OUT_DEBUG(textBuf,"please wait CallBack_getipbyname\r\n");

                                    //start timer to wait callback
                                    timer_querydns.timeoutPeriod = Ql_MillisecondToTicks(QUERY_NDS_CALLBACK_TIME); //800 millisecond
                                    Ql_StartTimer(&timer_querydns);
                                }
                                else
                                {
                                    OUT_DEBUG(textBuf,"Ql_GetHostIpbyName run error\r\n");
                                    //ASSERT(0);
                                    if(ret == QL_SOC_BEARER_FAIL)  
                                    {
                                        query_dns_number = 0;
                                        nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
                                         OUT_DEBUG(textBuf,"Ql_GetHostIpbyName error QL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
                                        Ql_GprsNetworkDeactive(0);
                                    }
                                    else
                                    {
                                        //re query dns
                                        nSTATE_TCPLONG = STATE_QUERY_DNS;
                                    }
                                }
                            }
                            else 
                            {
                                query_dns_number = 0;
                                //ASSERT(0);
                                //invalid argument
                            }
                            break;
                        }

                        case  STATE_CREATE_TCPCONNECT:
                        {
                            nSTATE_TCPLONG = STATE_CREATE_TCPCONNECT_ING;

                            serialnum = 0;
                            tcpsocket = Ql_SocketCreate(0, 0);
                            //if(tcpsocket < 0) failed
                            OUT_DEBUG(textBuf,"Ql_SocketCreate()=%d\r\n",tcpsocket);

                            ret = Ql_SocketConnect(tcpsocket, ipaddress, port);
                            OUT_DEBUG(textBuf,"Ql_SocketConnect(ip=%d,%d,%d,%d,port=%d)=%d\r\n",ipaddress[0],ipaddress[1],ipaddress[2],ipaddress[3],port,ret);
                            if(ret == QL_SOC_SUCCESS)
                            {
                                nSTATE_TCPLONG = STATE_TRANSFERS_TCPDATA;
                                OUT_DEBUG(textBuf,"Ql_SocketConnect connect successed\r\n");
                            }
                            else if(ret == QL_SOC_WOULDBLOCK)
                            {
                                OUT_DEBUG(textBuf,"please wait CallBack_socket_connect\r\n");
                                //to waiting CallBack_socket_connect
                            }
                            else
                            {
                                OUT_DEBUG(textBuf,"Ql_SocketConnect run error\r\n");

                                Ql_SocketClose(tcpsocket);
                                tcpsocket = 0xFF;
                                
                                if(ret == QL_SOC_BEARER_FAIL)  
                                {
                                    nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
                                     OUT_DEBUG(textBuf,"Ql_SocketConnect error QL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
                                    Ql_GprsNetworkDeactive(0);
                                }
                                else
                                {
                                    nSTATE_TCPLONG = STATE_QUERY_DNS;
                                }
                            }
                            break;
                        }
                        
                        case  STATE_TRANSFERS_TCPDATA:
                        {
                            ///////////////////////////////////////////////////////////////////////////////
                            //alloc data begin
                            pMemSend = Ql_GetMemory(DATA_LEN);
                            remain_len = DATA_LEN;
                            pCurrentPos = pMemSend;
                            OUT_DEBUG(textBuf,"Ql_GetMemory(%d)=%x\r\n",DATA_LEN,pMemSend);
                            if(!pMemSend)
                                break;
                            Ql_sprintf((char*)pMemSend,"%d",serialnum);
                            Ql_memset((char*)pMemSend+Ql_strlen((char*)pMemSend), 'A', DATA_LEN-Ql_strlen((char*)pMemSend));
                            serialnum++;
                            //alloc data end
                            ///////////////////////////////////////////////////////////////////////////////
                            
                            ret = Ql_SocketTcpAckNumber(tcpsocket, &ackedNum);
                            //if(QL_SOC_SUCCESS != ret) failed
                            OUT_DEBUG(textBuf,"Ql_SocketTcpAckNumber(socket=%d,ackedNum=%llu)=%d,DATA_LEN=%d\r\n",tcpsocket,ackedNum,ret,DATA_LEN);
                            do
                            {
                                ret = Ql_SocketSend(tcpsocket , (u8*)pCurrentPos , remain_len );
                                OUT_DEBUG(textBuf,"Ql_SocketSend(socket=%d,remain_len=%d)=%d,serialnum=%d\r\n",tcpsocket ,remain_len ,ret,serialnum);
                                if(ret == remain_len)
                                {
                                    //send compelete
                                    remain_len = 0;
                                    Ql_FreeMemory(pMemSend);
                                    pMemSend = NULL;
                                    pCurrentPos = NULL;
                                    nSTATE_TCPLONG = STATE_TRANSFERS_TCPDATA_CHECKACK;
                                    break;
                                }
                                else if((ret < 0) && (ret == QL_SOC_WOULDBLOCK)) 
                                {
                                    //you must wait CallBack_socket_write, then send data;     
                                    nSTATE_TCPLONG = STATE_TRANSFERS_TCPDATA_ING;
                                    break;
                                }
                                else if(ret < 0)
                                {
                                    //error , Ql_SocketClose
                                    Ql_SocketClose(tcpsocket);
                                    tcpsocket = 0xFF;

                                    remain_len = 0;
                                    Ql_FreeMemory(pMemSend);
                                    pMemSend = NULL;
                                    pCurrentPos = NULL; 
                                    

                                    if(ret == QL_SOC_BEARER_FAIL)  
                                    {
                                        nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
                                     OUT_DEBUG(textBuf,"Ql_SocketSend error QL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
                                        Ql_GprsNetworkDeactive(0);
                                    }
                                    else
                                    {
                                        nSTATE_TCPLONG = STATE_QUERY_DNS; // will to Ql_GetHostIpbyName
                                    }
                                    
                                    break;
                                }
                                else if(ret < remain_len)
                                {
                                    remain_len -= ret;
                                    (char*)pCurrentPos += ret;
                                     //continue send, do not send all data
                                }
                            }while(1);
                            break;
                        }

                        case  STATE_TRANSFERS_TCPDATA_CHECKACK:
                        {
                            u64 ackedNumCurr;
                            //check peer received all data
                            ret = Ql_SocketTcpAckNumber(tcpsocket, &ackedNumCurr);
                            //if(QL_SOC_SUCCESS != ret) failed
                            if((ackedNum + DATA_LEN) == ackedNumCurr)
                            {
                                nSTATE_TCPLONG = STATE_TRANSFERS_TCPDATA;
                            }
                            OUT_DEBUG(textBuf,"Ql_SocketTcpAckNumber(socket=%d,ackedNumCurr=%llu)=%d\r\n",tcpsocket,ackedNumCurr,ret);
                            break;
                         }
                    }

                    if(STATE_TRANSFERS_TCPDATA == nSTATE_TCPLONG)
                        timer.timeoutPeriod = Ql_MillisecondToTicks(SENDDATA_TIMER); 
                    else
                        timer.timeoutPeriod = Ql_MillisecondToTicks(NORMAL_TIMER); 
                    Ql_StartTimer(&timer); 
                }
                break;
            }

            case EVENT_MODEMDATA:
            {
                // BB AT Command return data, send to UART1
                Ql_SendToUart(ql_uart_port1, (u8*)flSignalBuffer.eventData.uartdata_evt.data, flSignalBuffer.eventData.uartdata_evt.len);        
                break;
            }

            case EVENT_UARTDATA:
            {
                //AT command send to BB, goto run 
                Ql_SendToModem(ql_md_port1, (u8*)flSignalBuffer.eventData.modemdata_evt.data, flSignalBuffer.eventData.modemdata_evt.len);                            
                break;
            }
            
            default:
                Ql_DebugTrace("eventType=%d",flSignalBuffer.eventType);
                break;
        }
    }
    
    Ql_GprsNetworkUnInitialize(0);
}

void CallBack_GprsAPNGet(u8 profileid, bool result, s32 error_code, u8 *apn, u8 *userId, u8 *password)
{
    if(result)
    {
        nSTATE_TCPLONG = STATE_SET_APN;  // will to Ql_GprsAPNSet
    }
    else
    {
        nSTATE_TCPLONG = STATE_GET_APN;  // will to Ql_GprsAPNSet
    }
    OUT_DEBUG(textBuf,"CallBack_GprsAPNGet(profileid=%d,result=%d,error_code=%d,apn=%s,userId=%s,password=%s)\r\n",profileid,result,error_code,apn,userId,password);
}

void CallBack_GprsAPNSet(bool result, s32 error_code)
{
    if(result)
    {
        nSTATE_TCPLONG = STATE_ACTIVE_PDP; // will to Ql_GprsNetworkActive
    }
    else
    {
        nSTATE_TCPLONG = STATE_SET_APN;  // will to Ql_GprsAPNSet
    }
   OUT_DEBUG(textBuf,"CallBack_GprsAPNSet(result=%d,error_code=%d)\r\n",result,error_code);
}



void CallBack_network_actived(u8 contexid)
{
    s8 ret;
    u8 ip_addr[4];

    OUT_DEBUG(textBuf,"CallBack_network_actived(contexid=%d)\r\n",contexid);
    ret = Ql_GetLocalIpAddress(contexid, ip_addr);
    OUT_DEBUG(textBuf,"Ql_GetLocalIpAddress(contexid=%d)=%d, ip=%d.%d.%d.%d\r\n",contexid, ret,ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
    nSTATE_TCPLONG = STATE_QUERY_DNS; // will to Ql_GetHostIpbyName
}

void CallBack_network_deactived(u8 contexid, s32 error_cause, s32 error)
{
    nSTATE_TCPLONG = STATE_QUERY_NETWORKSTATUS; // will to Ql_GprsNetworkGetState
    OUT_DEBUG(textBuf,"CallBack_network_deactived(contexid=%d,error_cause=%d, error=%d)\r\n",contexid,error_cause,error);
}


void CallBack_getipbyname(u8 contexid, bool result, s32 error, u8 num_entry, u8 *entry_address)
{
    u8 i;

    OUT_DEBUG(textBuf,"CallBack_getipbyname(contexid=%d, result=%d,error=%d,num_entry=%d)\r\n",contexid, result,error,num_entry);

    Ql_StopTimer(&timer_querydns);

    if(result)
    {
        Ql_memcpy((void*)ipaddress, (void*)entry_address, 4);
        nSTATE_TCPLONG = STATE_CREATE_TCPCONNECT; // to connect tcp server
        for(i=0;i<num_entry;i++)
        {
            entry_address += (i*4);
            OUT_DEBUG(textBuf,"entry=%d, ip=%d.%d.%d.%d\r\n",i,entry_address[0],entry_address[1],entry_address[2],entry_address[3]);
        }
    }
    else
    {
        if((error == QL_SOC_BEARER_FAIL)  || (error >=0)) 
        {
            query_dns_number = 0;
            nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
             OUT_DEBUG(textBuf,"CallBack_getipbyname error QL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
            Ql_GprsNetworkDeactive(0);
        }
        else
        {
            nSTATE_TCPLONG = STATE_QUERY_DNS;// will to Ql_GetHostIpbyName
        }
    }
}


void CallBack_socket_connect(u8 contexid, s8 sock, bool result, s32 error)
{
    if(result)
    {
        OUT_DEBUG(textBuf,"CallBack_socket_connect(contexid=%d,sock=%d ) successed \r\n",contexid,sock);
         nSTATE_TCPLONG = STATE_TRANSFERS_TCPDATA;
    }
    else
    {
        OUT_DEBUG(textBuf,"CallBack_socket_connect(contexid=%d,sock=%d,result=%d,error=%d) failed\r\n",contexid,sock,result,error);
        Ql_SocketClose(tcpsocket);
        tcpsocket = 0xFF;

        if((error == QL_SOC_BEARER_FAIL)  || (error >=0)) 
        {
            nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
             OUT_DEBUG(textBuf,"CallBack_socket_connect error  QL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
            Ql_GprsNetworkDeactive(0);
        }
        else
        {
            nSTATE_TCPLONG = STATE_QUERY_DNS;
        }
     }
}

void CallBack_socket_close(u8 contexid, s8 sock, bool result, s32 error)
{
    OUT_DEBUG(textBuf,"CallBack_socket_close(contexid=%d,sock=%d,result=%d,error=%d)\r\n",contexid,sock,result,error);

    remain_len = 0;
    if(pMemSend)
        Ql_FreeMemory(pMemSend);
    pMemSend = NULL;
    pCurrentPos = NULL;

    Ql_SocketClose(tcpsocket);
    tcpsocket = 0xFF;

    if((error == QL_SOC_BEARER_FAIL)  || (error >=0)) 
    {
        nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
             OUT_DEBUG(textBuf,"CallBack_socket_close error  QL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
        Ql_GprsNetworkDeactive(0);
    }
    else
    {
        nSTATE_TCPLONG = STATE_QUERY_DNS;
    }

    
}

void CallBack_socket_accept(u8 contexid, s8 sock, bool result, s32 error)
{
    if(!result)
    {
        //if((error == QL_SOC_BEARER_FAIL)  || (error >=0)) Ql_GprsNetworkDeactive(0);
    }
}

void CallBack_socket_read(u8 contexid, s8 sock, bool result, s32 error)
{
    s32 ret;
    u8 data_p[31];
    s32 dataLen;
    u8 address_peer[4];
    u16 port_peer;
    
    OUT_DEBUG(textBuf,"CallBack_socket_read(contexid=%d,sock=%d,result=%d,error=%d)\r\n",contexid,sock,result,error);

    if(!result)
    {
        OUT_DEBUG(textBuf,"CallBack_socket_read() failed\r\n");
        Ql_SocketClose(tcpsocket);
        tcpsocket = 0xFF;

        remain_len = 0;
        if(pMemSend)
            Ql_FreeMemory(pMemSend);
        pMemSend = NULL;
        pCurrentPos = NULL;

        if((error == QL_SOC_BEARER_FAIL)  || (error >=0)) 
        {
            nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
             OUT_DEBUG(textBuf,"CallBack_socket_read error  QL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
            Ql_GprsNetworkDeactive(0);
        }
        else
        {
            nSTATE_TCPLONG = STATE_QUERY_DNS;
        }
        
        return;
    }

    do
    {
        ret = Ql_SocketRecv(sock, data_p, 30 );

        if(ret == 0)
        {
            //peer close socket
            nSTATE_TCPLONG = STATE_QUERY_DNS;
            OUT_DEBUG(textBuf,"Ql_SocketRecv()=%d, peer close socket\r\n",ret );

            remain_len = 0;
            if(pMemSend)
                Ql_FreeMemory(pMemSend);
            pMemSend = NULL;
            pCurrentPos = NULL;

            Ql_SocketClose(tcpsocket);
            tcpsocket = 0xFF;
            
        }
        else if((ret < 0) && (ret == QL_SOC_WOULDBLOCK)) 
        {
            //you shoud wait next CallBack_socket_read
            break;
        }
        else if(ret < 0) 
        {
            OUT_DEBUG(textBuf,"Ql_SocketRecv()=%d, error exit\r\n",ret );

            remain_len = 0;
            if(pMemSend)
                Ql_FreeMemory(pMemSend);
            pMemSend = NULL;
            pCurrentPos = NULL;

            Ql_SocketClose(tcpsocket);
            tcpsocket = 0xFF;

            if(ret == QL_SOC_BEARER_FAIL) 
            {
                nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
             OUT_DEBUG(textBuf,"Ql_SocketRecv error  in CallBack_socket_readQL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
                Ql_GprsNetworkDeactive(0);
            }
            else
            {
                nSTATE_TCPLONG = STATE_QUERY_DNS;
            }
            
            break;
        }
        else 
        {
             OUT_DEBUG(textBuf,"Ql_SocketRecv: sock(%d)len(%d) \r\n",sock,ret );
             // continue to recv data
        }
    }
    while(1);

}
void CallBack_socket_write(u8 contexid, s8 sock, bool result, s32 error)
{
    s32 ret;
    OUT_DEBUG(textBuf,"CallBack_socket_write(contexid=%d,sock=%d,result=%d,error=%d)\r\n",contexid,sock,result,error);
    //now , here , you can continue use Ql_SocketSend to send data 
    while(result)
    {
        ret = Ql_SocketSend(tcpsocket , (u8*)pCurrentPos , remain_len );
        OUT_DEBUG(textBuf,"Ql_SocketSend(socket=%d,remain_len=%d)=%d\r\n",tcpsocket ,remain_len ,ret);
        if(ret == remain_len)
        {
            //send compelete
            remain_len = 0;
            Ql_FreeMemory(pMemSend);
            pMemSend = NULL;
            pCurrentPos = NULL;
            nSTATE_TCPLONG = STATE_TRANSFERS_TCPDATA_CHECKACK;
            break;
        }
        else if((ret < 0) && (ret == QL_SOC_WOULDBLOCK)) 
        {
            //you must next wait CallBack_socket_write, then send data;     
            nSTATE_TCPLONG = STATE_TRANSFERS_TCPDATA_ING;
            break;
        }
        else if(ret < 0)
        {
            //error , Ql_SocketClose
            Ql_SocketClose(tcpsocket);
            tcpsocket = 0xFF;

            remain_len = 0;
            Ql_FreeMemory(pMemSend);
            pMemSend = NULL;
            pCurrentPos = NULL;


            if(ret == QL_SOC_BEARER_FAIL) 
            {
                nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
             OUT_DEBUG(textBuf,"CallBack_socket_write errro QL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
                Ql_GprsNetworkDeactive(0);
            }
            else
            {
                nSTATE_TCPLONG = STATE_QUERY_DNS; // will to Ql_GetHostIpbyName
            }
            
            break;
        }
        else if(ret < remain_len)
        {
            remain_len -= ret;
            ((char*)pCurrentPos) += ret;
             //continue send, do not send all data
        }
    }

    if(result)
    {

        OUT_DEBUG(textBuf,"CallBack_socket_write() failed\r\n");

        //error , Ql_SocketClose
        Ql_SocketClose(tcpsocket);
        tcpsocket = 0xFF;

        remain_len = 0;
        if(pMemSend)
            Ql_FreeMemory(pMemSend);
        pMemSend = NULL;
        pCurrentPos = NULL;


        if((error == QL_SOC_BEARER_FAIL)  || (error >=0)) 
        {
            nSTATE_TCPLONG = STATE_DEACTIVE_PDP;
             OUT_DEBUG(textBuf,"Ql_SocketSend error  in CallBack_socket_write QL_SOC_BEARER_FAIL to Ql_GprsNetworkDeactive\r\n");
            Ql_GprsNetworkDeactive(0);
        }
        else
        {
            nSTATE_TCPLONG = STATE_QUERY_DNS; // will to Ql_GetHostIpbyName
        }
        
    }
}

#if 0

当调用这些函数Ql_SocketCreate,Ql_SocketClose,Ql_SocketConnect,Ql_SocketSend,Ql_SocketRecv,Ql_SocketSendTo,Ql_SocketRecvFrom,Ql_SocketListen,Ql_SocketAccept,Ql_GetHostIpbyName
返回< 0的错误号ql_soc_error_enum中的值(等同于soc_error_enum)，
当返回值是SOC_BEARER_FAIL值，那么除了关闭socket，
    你还需要Ql_GprsNetworkDeactive关闭当前的PDP连接，然后重新Ql_GprsNetworkActive 这个激活这个PDP
当返回值是SOC_WOULDBLOCK，不表示错误，仅仅是异步等待提示值
其它错误返回值，那么，你仅仅需要关闭socket  ，然后去重新连接socket就可以了


当回调函数被调用，CallBack_getipbyname,CallBack_socket_connect,CallBack_socket_close,CallBack_socket_accept,CallBack_socket_read,CallBack_socket_write 
当result == FALSE，表示失败, error 具体表示失败号
当error是ql_soc_error_enum中的值(等同于soc_error_enum)，除了SOC_BEARER_FAIL，那么处理方式是关闭当前的socket，然后去连接就可以了;
当error是SOC_BEARER_FAIL值，或者是>= 0的值(就是ps_cause_enum中的值)，那么除了关闭socket，
    你还需要Ql_GprsNetworkDeactive关闭当前的PDP连接，然后重新Ql_GprsNetworkActive 这个激活这个PDP

typedef struct
{
   kal_uint8       ref_count;
   kal_uint16      msg_len;    
   kal_int8        socket_id;    /* socket ID */
   soc_event_enum  event_type;   /* soc_event_enum */
   kal_bool        result;
   soc_error_enum  error_cause;  /* used only when EVENT is close/connect */
   kal_int32       detail_cause; /* refer to ps_cause_enum if error_cause is
                                  * SOC_BEARER_FAIL */
} app_soc_notify_ind_struct;

/* Socket return codes, negative values stand for errors */
typedef enum
{
    SOC_SUCCESS           = 0,
    SOC_ERROR             = -1,
    SOC_WOULDBLOCK        = -2,
    SOC_LIMIT_RESOURCE    = -3,    /* limited resource */
    SOC_INVALID_SOCKET    = -4,    /* invalid socket */
    SOC_INVALID_ACCOUNT   = -5,    /* invalid account id */
    SOC_NAMETOOLONG       = -6,    /* address too long */
    SOC_ALREADY           = -7,    /* operation already in progress */
    SOC_OPNOTSUPP         = -8,    /* operation not support */
    SOC_CONNABORTED       = -9,    /* Software caused connection abort */
    SOC_INVAL             = -10,   /* invalid argument */
    SOC_PIPE              = -11,   /* broken pipe */
    SOC_NOTCONN           = -12,   /* socket is not connected */
    SOC_MSGSIZE           = -13,   /* msg is too long */
    SOC_BEARER_FAIL       = -14,   /* bearer is broken */
    SOC_CONNRESET         = -15,   /* TCP half-write close, i.e., FINED */
    SOC_DHCP_ERROR        = -16,
    SOC_IP_CHANGED        = -17,
    SOC_ADDRINUSE         = -18,
    SOC_CANCEL_ACT_BEARER = -19    /* cancel the activation of bearer */
} soc_error_enum;

typedef enum {
/* ============================== CM cause ============================== */    
   CM_RADIO_LINK_FAIL = 0, /* Ripple: add for SAT3 use. sec 11.3.2, spec.. 11.14 */
// 3GPP 24.008 clause 10.5.4.11(Annex H) CC+SS cause value
// 3GPP 24.011 clause 8.2.5.4 SMS RP cause value
// 3GPP 24.011 clause 8.1.4.2 SMS CP cause value
// Class(000): 0-15 -> normal event             CC+SS    SMS
   CM_UNASSIGNED_NUM                     = 1,  // X       X
   CM_NO_ROUTE_TO_DESTINATION            = 3,  // X
   CM_CHANNEL_UN_ACCP                    = 6,  // X
   CM_OPR_DTR_BARRING                    = 8,  // X       X
   CM_CALL_BARRED                        = 10, //         X
   CM_RESERVED                           = 11, //         X
// Class(001): 16-31 -> normal event
   CM_NORMAL_CALL_CLR                    = 16, // X
   CM_USER_BUSY                          = 17, // X       X! (SMS: nwk failure)
   CM_NO_USER_RESPONDING                 = 18, // X
   CM_NO_ANSWER_ON_ALERT                 = 19, // X
   CM_CALL_REJECTED                      = 21, // X       X
   CM_NUMBER_CHANGED                     = 22, // X       X! (SMS: congestion)
   CM_PRE_EMPTION                        = 25, // X
   CM_NON_SEL_USER_CLEAR                 = 26, // X
   CM_DEST_OUT_OF_ORDER                  = 27, // X       X
   CM_INVALID_NUMBER_FORMAT              = 28, // X       X
   CM_FACILITY_REJECT                    = 29, // X       X
   CM_RES_STATUS_ENQ                     = 30, // X       X! (SMS:Unknown_Sub)
   CM_NORMAL_UNSPECIFIED                 = 31, // X
// Class(010): 32-48 -> resource unavailable
   CM_NO_CIRCUIT_CHANNEL_AVAIL           = 34, // X
   CM_NETWORK_OUT_OF_ORDER               = 38, // X       X
   CM_TEMPORARY_FAILURE                  = 41, // X       X
   CM_SWITCH_EQUIPMENT_CONGESTION        = 42, // X       X
   CM_ACCESS_INFO_DISCARDED              = 43, // X
   CM_REQUESTED_CKT_CHANEL_NOT_AVIL      = 44, // X
   CM_RESOURCE_UNAVAIL_UNSPECIFIED       = 47, // X       X
// Class(011): 49-64 -> service or option not available
   CM_QOS_UNAVAIL                        = 49, // X
   CM_REQ_FAC_NOT_SUBS                   = 50, // X       X
   CM_IC_BAR_CUG                         = 55, // X
   CM_BEARER_CAP_NOT_AUTHORISED          = 57, // X
   CM_BEARER_CAP_NOT_AVAIL               = 58, // X
   CM_SER_UNAVAILABLE                    = 63, // X
// Class(100): 65-80 -> service or option not implementated
   CM_BEARER_SER_UNIMPL                  = 65, // X
   CM_ACM_EXCEEDED                       = 68, // X
   CM_REQ_FACILITY_UNAVAIL               = 69, // X       X
   CM_RESTR_DIGITAL_INFO                 = 70, // X
   CM_SER_OPT_UNIMPL                     = 79, // X
// Class(101): 81-95 -> invalid message
   CM_INVALID_TI_VALUE                   = 81, // X       X
   CM_USER_NOT_IN_CUG                    = 87, // X
   CM_INCOMPATIBLE_DEST                  = 88, // X
   CM_INVALID_TRANSIT_NW_SEL             = 91, // X
   CM_SEMANTIC_ERR                       = 95, // X       X
// Class(110): 96-112 -> protocol error
   CM_INVALID_MANDATORY_INF              = 96, // X       X
   CM_MSG_TYPE_UNIMPL                    = 97, // X       X
   CM_MSG_TYPE_NOT_COMPATIBLE            = 98, // X       X
   CM_IE_NON_EX                          = 99, // X       X
   CM_COND_IE_ERR                       = 100, // X
   CM_INCOMP_MESG_WITH_STATE            = 101, // X
   CM_RECOVERY_ON_TIMER_EXPIRY          = 102, // X
   CM_PROTOCOL_ERR_UNSPECIFIED          = 111, // X       X
// Class(111): 113-127 -> interworking
   CM_INTER_WRK_UNSPECIFIED             = 127, // X       X
/* ============================== ~CM cause ============================== */

/* ============================== SMS cause ============================== */
// 3GPP 23.040 clause 9.2.3.22 SMS TP cause values
// 128-255 -> SMS cause value
      
   /* PID error */   
   TELEMATIC_INT_WRK_NOT_SUPPORT = 0x80, /* telematic interworking not support */
   SMS_TYPE0_NOT_SUPPORT         = 0x81, /* short message type 0 not support */
   CANNOT_REPLACE_MSG            = 0x82, 
   UNSPECIFIED_PID_ERROR         = 0x8F, 
   
   /* DCS error */
   ALPHABET_NOT_SUPPORT          = 0x90, /* data coding scheme (alphabet) not support */
   MSG_CLASS_NOT_SUPPORT         = 0x91, /* message class not support */
   UNSPECIFIED_TP_DCS            = 0x9f,

   /* Command error */
   CMD_CANNOT_ACTION             = 0xa0, /* command cannot be actioned */
   CMD_NOT_SUPPORT               = 0xa1, /* command unsupported */
   UNSPECIFIED_CMD_ERROR         = 0xaf, 
   
   TPDU_NOT_SUPPORT              = 0xb0,   
   SC_BUSY                       = 0xc0,
   NO_SC_SUBSCRIPTION            = 0xc1,
   SC_SYS_FAILURE                = 0xc2,  /* SC system failure */
   INVALID_SME_ADDR              = 0xc3,  
   DEST_SME_BARRED               = 0xc4,  /* destination SME barred */
   SM_REJ_DUP_SM                 = 0xc5,  /* SM rejected-duplicate SM */
   TP_VPF_NOT_SUPPORT            = 0xc6,
   TP_VP_NOT_SUPPORT             = 0xc7,
      
   SIM_STORAGE_FULL              = 0xd0,
   NO_SMS_STO_IN_SIM             = 0xd1,  /* No SMS storage capability in SIM */
   ERROR_IN_MS                   = 0xd2, 
   MEM_CAP_EXCEEDED              = 0xd3,  /* Memory Capacity Exceeded */
   
   /* SIM Application Toolkit */
   SMS_SAT_BUSY                  = 0xd4,  /* SIM Application Toolkit Busy */
   SMS_SAT_DL_ERROR              = 0xd5,  /* SIM data download error */
   
   /* 0XE0 to 0xFE are specific to application */
   SMS_CP_RETRY_EXCEED           = 0xe0,
   SMS_RP_TR1M_TIMEOUT           = 0xe1,
   SMS_CONNECTION_BROKEN         = 0xe2,
   
   UNSPECIFIED_TP_FCS            = 0xff,

// 3GPP 27.005 clause 3.2.5 SMS ATcmd cause values
// 256-511 -> SMS AT command cause value
   INVALID_PDU_MODE_PARA  = 304,   /* invalid pdu mode parameter */
   INVALID_TEXT_MODE_PARA = 305,   /* invalid text mode parameter */
   SMS_SIM_FAILURE        = 313,   /* sim failure */
   MEM_FAILURE              = 320,   /* memory failure */
   INVALID_MEM_INDEX      = 321,   /* invalid memory index */
   MEM_FULL              = 322,   /* memory full */
   SCA_UNKNOWN              = 330,   /* SMSC address unknown */
   NO_CNMA_EXPECTED       = 340,   /* no +CNMA acknowledgement expected */
   SMSAL_UNSPECIFIED_ERROR_CAUSE = 500,
   
// 512-767 -> SMS proprietary cause values
   SMSAL_NO_ERROR             = 512,
   MSG_LEN_EXCEEDED           = 513,   /* message length exceeds 
                                          maximum length */
   INVALID_REQ_PARAMETER      = 514,   /* invalid request parameters */
   SMS_ME_STORAGE_FAILURE     = 515,   /* ME storage failure */
   SMSAL_INVALID_BEARER       = 516,   /* Invalid bearer service */
   SMSAL_INVALID_SERVICE_MODE = 517,   /* Invalid service mode */
   SMSAL_INVALID_STORAGE_TYPE = 518,   /* Invalid storage type */
   SMSAL_INVALID_MSG_FORMAT   = 519,   /* Invalid message format */
   TOO_MANY_MO_CONCAT_MSG     = 520,   /* Too many MO concatenated messages */
   SMSAL_NOT_READY            = 521,
   SMSAL_NO_MO_SERVICE        = 522,
   NOT_SUPP_SR_CMD_IN_STORAGE = 523,   /* not support TP-Status-Report & TP-Command in storage */   
   SMSAL_NOT_SUPPORT_MSG_TYPE = 524,   /* Reserved MTI */
   SMS_RL_NO_FREE_ENTITY      = 525,   /* no free entity in RL layer */
   SMS_PORT_NUM_ALREADY_REG   = 526,   /* The port number is already registerred */
   SMS_PORT_NUM_NO_FREE_ENTITY= 527,   /* There is no free entity for port number */
   SMS_RL_MMS_STATE_ERR       = 528,   /* More Message to Send state error */
   SMS_MO_SMS_NOT_ALLOW       = 529,   /* MO SMS is not allow */
   SMS_PS_SUSPENDED           = 530,   /* GPRS is suspended */
   SMS_ME_STORAGE_FULL        = 531,   /* ME storage full */
   SMS_SIM_REFRESHING         = 532,   /* doing SIM refresh */
/* ============================== ~SMS cause ============================== */
   
/* ============================== CC cause ============================== */   
// 768-1023 -> CC proprietary cause values
    CSMCC_CMD_NOT_ALLOW = 768,
    CSMCC_ILLEGAL_CALL_ID,
    CSMCC_CALL_ALLOC_FAIL,
    CSMCC_BC_FILL_FAIL,
    CSMCC_CALL_RE_EST,
    CSMCC_ILLEGAL_DTMF_TONE,
    CSMCC_ILLEGAL_BC,
    CSMCC_MODIFY_ACTUAL_MODE,   
    CSMCC_DATA_ACT_FAIL,
    CSMCC_NO_RESPONSE_FROM_NW,
    CSMCC_CALL_ACCEPT_NOT_ALLOW,

   CM_L4C_GEN_CAUSE = 896,
   CM_L4C_CC_CALL_ABORT = 897,  /*CSD call is aborted by user during call establishment
                   or MT call abort MO call/USSD*/
   CM_L4C_CC_CALL_DISC_IND = 898,  /*CSD call is disconnected due to lower layer failure*/
/* ============================== ~CC cause ============================== */
   
/* ============================== SS cause ============================== */   
// 1024-1279 -> SS proprietary cause values
// 3GPP 23.080 clause 4.3 SS Error Types
   CM_SS_ERR_START                                = 1024,
   CM_SS_ERR_UNKNOWNSUBSCRIBER                    =   1 + CM_SS_ERR_START,
   CM_SS_ERR_ILLEGALSUBSCRIBER                    =   9 + CM_SS_ERR_START,
   CM_SS_ERR_BEARERSERVICENOTPROVISIONED          =  10 + CM_SS_ERR_START,
   CM_SS_ERR_TELESERVICENOTPROVISIONED            =  11 + CM_SS_ERR_START,
   CM_SS_ERR_ILLEGALEQUIPMENT                     =  12 + CM_SS_ERR_START,
   CM_SS_ERR_CALLBARRED                           =  13 + CM_SS_ERR_START,
   CM_SS_ERR_ILLEGALSS_OPERATION                  =  16 + CM_SS_ERR_START,
   CM_SS_ERR_SS_ERRORSTATUS                       =  17 + CM_SS_ERR_START,
   CM_SS_ERR_SS_NOTAVAILABLE                      =  18 + CM_SS_ERR_START,
   CM_SS_ERR_SS_SUBSCRIPTIONVIOLATION             =  19 + CM_SS_ERR_START,
   CM_SS_ERR_SS_INCOMPABILITY                     =  20 + CM_SS_ERR_START,
   CM_SS_ERR_FACILITYNOTSUPPORTED                 =  21 + CM_SS_ERR_START,
   CM_SS_ERR_ABSENTSUBSCRIBER                     =  27 + CM_SS_ERR_START,
   CM_SS_ERR_SHORTTERMDENIAL                      =  29 + CM_SS_ERR_START,
   CM_SS_ERR_LONGTERMDENIAL                       =  30 + CM_SS_ERR_START,
   CM_SS_ERR_SYSTEMFAILURE                        =  34 + CM_SS_ERR_START,
   CM_SS_ERR_DATAMISSING                          =  35 + CM_SS_ERR_START,
   CM_SS_ERR_UNEXPECTEDDATAVALUE                  =  36 + CM_SS_ERR_START,
   CM_SS_ERR_PW_REGISTRATIONFAILURE               =  37 + CM_SS_ERR_START,
   CM_SS_ERR_NEGATIVEPW_CHECK                     =  38 + CM_SS_ERR_START,
   CM_SS_ERR_NUMBEROFPW_ATTEMPTSVIOLATION         =  43 + CM_SS_ERR_START,
   CM_SS_ERR_POSITIONMETHODFAILURE                =  54 + CM_SS_ERR_START,
   CM_SS_ERR_UNKNOWNALPHABET                      =  71 + CM_SS_ERR_START,
   CM_SS_ERR_USSD_BUSY                            =  72 + CM_SS_ERR_START,
   CM_SS_ERR_REJECTEDBYUSER                       = 121 + CM_SS_ERR_START,
   CM_SS_ERR_REJECTEDBYNETWORK                    = 122 + CM_SS_ERR_START,
   CM_SS_ERR_DEFLECTIONTOSERVEDSUBSCRIBER         = 123 + CM_SS_ERR_START,
   CM_SS_ERR_SPECIALSERVICECODE                   = 124 + CM_SS_ERR_START,
   CM_SS_ERR_INVALIDDEFLECTIONTONUMBER            = 125 + CM_SS_ERR_START,
   CM_SS_ERR_MAXNUMBEROFMPTY_PARTICIPANTSEXCEEDED = 126 + CM_SS_ERR_START,
   CM_SS_ERR_RESOURCESNOTAVAILABLE                = 127 + CM_SS_ERR_START,

   CM_SS_GENERALPROBLEM_START                     = 1152,
   CM_SS_GENERALPROBLEM_UNRECOGNIZEDCOMPONENT     =       CM_SS_GENERALPROBLEM_START,
   CM_SS_GENERALPROBLEM_MISTYPEDCOMPONENT         = 1   + CM_SS_GENERALPROBLEM_START,
   CM_SS_GENERALPROBLEM_BADLYSTRUCTUREDCOMPONENT  = 2   + CM_SS_GENERALPROBLEM_START,

   CM_SS_INVOKEPROBLEM_START                      = 1155,
   CM_SS_INVOKEPROBLEM_DUPLICATEINVOKEID          =       CM_SS_INVOKEPROBLEM_START,
   CM_SS_INVOKEPROBLEM_UNRECOGNIZEDOPERATION      = 1  +  CM_SS_INVOKEPROBLEM_START,
   CM_SS_INVOKEPROBLEM_MISTYPEDPARAMETER          = 2  +  CM_SS_INVOKEPROBLEM_START,
   CM_SS_INVOKEPROBLEM_RESOURCELIMITATION         = 3  +  CM_SS_INVOKEPROBLEM_START,
   CM_SS_INVOKEPROBLEM_INITIATINGRELEASE          = 4  +  CM_SS_INVOKEPROBLEM_START,
   CM_SS_INVOKEPROBLEM_UNRECOGNIZEDLINKDID        = 5  +  CM_SS_INVOKEPROBLEM_START,
   CM_SS_INVOKEPROBLEM_LINKEDRESONSEUNEXPECTED    = 6  +  CM_SS_INVOKEPROBLEM_START,
   CM_SS_INVOKEPROBLEM_UNEXPECTEDLINKEDOPERATION  = 7  +  CM_SS_INVOKEPROBLEM_START,

   CM_SS_RETURNRESULTPROBLEM_START                     = 1163,
   CM_SS_RETURNRESULTPROBLEM_RR_UNRECOGNIZEDINVOKEID   =      CM_SS_RETURNRESULTPROBLEM_START,
   CM_SS_RETURNRESULTPROBLEM_RR_RETURNRESULTUNEXPECTED = 1 +  CM_SS_RETURNRESULTPROBLEM_START,
   CM_SS_RETURNRESULTPROBLEM_RR_MISTYPEDPARAMETER      = 2 +  CM_SS_RETURNRESULTPROBLEM_START,

   CM_SS_RETURNERRORPROBLEM_START                      = 1166,
   CM_SS_RETURNERRORPROBLEM_RE_UNRECOGNIZEDINVOKEID    =      CM_SS_RETURNERRORPROBLEM_START,
   CM_SS_RETURNERRORPROBLEM_RE_RETURNERRORUNEXPECTED   = 1 +  CM_SS_RETURNERRORPROBLEM_START,
   CM_SS_RETURNERRORPROBLEM_RE_UNRECOGNIZEDERROR       = 2 +  CM_SS_RETURNERRORPROBLEM_START,
   CM_SS_RETURNERRORPROBLEM_RE_UNEXPECTEDERROR         = 3 +  CM_SS_RETURNERRORPROBLEM_START,
   CM_SS_RETURNERRORPROBLEM_RE_MISTYPEDPARAMETER       = 4 +  CM_SS_RETURNERRORPROBLEM_START,
/* ============================== ~SS cause ============================== */

/* ============================== MM cause ============================== */
// 3GPP 24.008 clause 10.5.3.6(Annex G) MM reject cause values
// 3GPP 24.008 clause 10.5.5.14(Annex G) GMM cause values
// 2048 -> MM cause values
   CM_MM_CAUSE_START                     = 2048,
   CM_MM_CAUSE_NONE                     =     0x00 + CM_MM_CAUSE_START,
   CM_MM_IMSI_UNKNOWN_IN_HLR            =     0x02 + CM_MM_CAUSE_START,
   CM_MM_ILLEGAL_MS                     =     0x03 + CM_MM_CAUSE_START,
   CM_MM_IMSI_UNKNOWN_IN_VLR            =     0x04 + CM_MM_CAUSE_START,
   CM_MM_IMEI_NOT_ACCEPTED              =     0x05 + CM_MM_CAUSE_START,
   CM_MM_ILLEGAL_ME                     =     0x06 + CM_MM_CAUSE_START,
   CM_MM_GPRS_NOT_ALLOWED               =     0x07 + CM_MM_CAUSE_START,
   CM_MM_GPRS_NON_GPRS_NOT_ALLOWED      =     0x08 + CM_MM_CAUSE_START,
   CM_MM_MS_ID_NOT_DERIVED_BY_NW        =     0x09 + CM_MM_CAUSE_START,
   CM_MM_IMPLICIT_DETACH                =     0x0a + CM_MM_CAUSE_START,
   CM_MM_PLMN_NOT_ALLOWED               =     0x0b + CM_MM_CAUSE_START,
   CM_MM_LOCATION_AREA_NOT_ALLOWED      =     0x0c + CM_MM_CAUSE_START,
   CM_MM_ROAMING_AREA_NOT_ALLOWED       =     0x0d + CM_MM_CAUSE_START,
   CM_MM_GPRS_NOT_ALLOWED_IN_PLMN       =     0x0e + CM_MM_CAUSE_START,
   CM_MM_NO_SUITABLE_CELLS_IN_LA        =     0x0f + CM_MM_CAUSE_START,
   CM_MM_MSC_TEMP_NOT_REACHABLE         =     0x10 + CM_MM_CAUSE_START,
   CM_MM_NETWORK_FAILURE                =     0x11 + CM_MM_CAUSE_START,
   CM_MM_MAC_FAILURE                    =     0x14 + CM_MM_CAUSE_START,
   CM_MM_SYNC_FAILURE                   =     0x15 + CM_MM_CAUSE_START,
   CM_MM_CONGESTION                     =     0x16 + CM_MM_CAUSE_START,
   CM_MM_SERV_OPTION_NOT_SUPPORTED      =     0x20 + CM_MM_CAUSE_START,
   CM_MM_REQ_SERV_OPTION_NOT_SUBSCRIBED =     0x21 + CM_MM_CAUSE_START,
   CM_MM_SERV_OPTION_TEMP_OUT_OF_ORDER  =     0x22 + CM_MM_CAUSE_START,
   CM_MM_CALL_CANNOT_BE_IDENTIFIED      =     0x26 + CM_MM_CAUSE_START,
   CM_MM_NO_PDP_CONTEXT_ACTIVATED       =     0x28 + CM_MM_CAUSE_START,
   CM_MM_RETRY_UPON_ENTRY_INTO_NEWCELL_MIN =  0x30 + CM_MM_CAUSE_START,  /* 30..3f */
   CM_MM_RETRY_UPON_ENTRY_INTO_NEWCELL_MAX =  0x3f + CM_MM_CAUSE_START,
   CM_MM_SEMANTICALLY_INCORRECT_MSG        =  0x5f + CM_MM_CAUSE_START,
   CM_MM_INVALID_MM_MAND_INFO              =  0x60 + CM_MM_CAUSE_START,
   CM_MM_MSG_TYPE_NON_EXISTENT             =  0x61 + CM_MM_CAUSE_START,
   CM_MM_MSG_TYPE_NOT_IMPLEMENTED          =  0x61 + CM_MM_CAUSE_START,
   CM_MM_MSG_TYPE_INCOMPAT_WITH_PROTO_STATE = 0x62 + CM_MM_CAUSE_START,
   CM_MM_IE_NOT_IMPLEMENTED                 = 0x63 + CM_MM_CAUSE_START,
   CM_MM_CONDITIONAL_MM_IE_ERROR            = 0x64 + CM_MM_CAUSE_START,
   CM_MM_MSG_NOT_COMPAT_WITH_PROTO_STATE    = 0x65 + CM_MM_CAUSE_START,  /* 65..6f */
   CM_MM_PROTO_ERROR_UNSPECIFIED            = 0x6f + CM_MM_CAUSE_START,

   /* RR connection establishment failure */
   CM_MM_ACCESS_BARRED  ,
   CM_MM_ASSIGNMENT_REJECT ,
   CM_MM_RANDOM_ACCES_FAILURE ,
   CM_MM_RR_NO_SERVICE ,
   CM_RR_PLMN_SRCH_REJ_EMERGENCY,   
   CM_MM_RR_CONN_RELEASE,

   /* other cause */
   CM_MM_AUTH_FAILURE,
   CM_MM_IMSI_DETACH,
   CM_MM_ABORT_BY_NW ,
   CM_MM_CONN_TIMEOUT,
   /* MM internal casuse */
   CM_MM_CM_ENQUEUE_FAIL,
   CM_MM_NOT_UPDATED ,
   CM_MM_STATE_NOT_ALLOWED_CM,
   CM_MM_EMERGENCY_NOT_ALLOWED,
   CM_MM_NO_SERVICE,
   CM_MM_ACCESS_CLASS_BARRED,
#ifdef __GEMINI__
/* under construction !*/
/* under construction !*/
/* under construction !*/
/* under construction !*/
/* under construction !*/
 #endif
   CM_MM_CAUSE_END                       = 2048 + 256,
/* ============================== ~MM cause ============================== */

/* ============================== SIM cause ============================== */
   SIM_CAUSE_START                                     = 2560,
   SIM_CAUSE_CMD_SUCCESS                               =     SIM_CAUSE_START,
   SIM_CAUSE_CMD_FAIL                                  = 1 + SIM_CAUSE_START,
   SIM_CAUSE_FATAL_ERROR                               = 2 + SIM_CAUSE_START,
   SIM_CAUSE_NO_INSERTED                               = 3 + SIM_CAUSE_START,
   SIM_CAUSE_CHV_NOT_INIT                              = 4 + SIM_CAUSE_START,
   SIM_CAUSE_CHV_VERIFY_ERROR                          = 5 + SIM_CAUSE_START, /* SIM_CHV_RETRY_ALLOW */
   SIM_CAUSE_CHV_BLOCK                                 = 6 + SIM_CAUSE_START, /* SIM_CHV_BLOCK */
   SIM_CAUSE_ACCESS_NOT_ALLOW                          = 7 + SIM_CAUSE_START, /* */
   SIM_CAUSE_SAT_CMD_BUSY                              = 8 + SIM_CAUSE_START, /* */
   SIM_CAUSE_DL_ERROR                                  = 9 + SIM_CAUSE_START, /* */
   SIM_CAUSE_MEMORY_PROBLEM                            = 10 + SIM_CAUSE_START, /* */
   SIM_CAUSE_TECHNICAL_PROBLEM                         = 11 + SIM_CAUSE_START, /* */
   SIM_CAUSE_PUK_UNLOCK                                = 12 + SIM_CAUSE_START, /* add new enum value to 
                                                                               distinguish whether normal poweron or PUK-unlock-poweron */
   SIM_CAUSE_END,
/* ============================== ~SIM cause ============================== */

/* ============================== PHB cause ============================== */
   PHB_CAUSE_FDN_BLOCKED = 2600,
   PHB_L4C_ERROR = 2601,  /*called number is not approved by PHB*/
/* ============================== ~PHB cause ============================== */

/* ============================== SM cause ============================== */
// 3GPP 24.008 clause 10.5.6.6(Annex I) SM/LLC/SNDCP/PDP/GPRS cause values
// 3072 -> SM cause values

    SM_CAUSE_START                   = 3072,
    OPERATOR_DETERMINED_BARRING      =   0x08 + SM_CAUSE_START, //__REL4__
    LLC_SND_FAILURE                  =   0X19 + SM_CAUSE_START,
    INSUFF_RESOURCE                  =   0X1A + SM_CAUSE_START,
    UNKNOWN_APN                      =   0X1B + SM_CAUSE_START,
    UNKNOWN_PDP_ADDR_OR_TYPE         =   0X1C + SM_CAUSE_START,
    AUTHENTICATION_FAILURE           =   0X1D + SM_CAUSE_START,
    ACTIVATION_REJ_GGSN              =   0X1E + SM_CAUSE_START,
    ACTIVATION_REJ                   =   0X1F + SM_CAUSE_START,
    UNSUPPORTED_SERVICE_OPTION       =   0X20 + SM_CAUSE_START,
    UNSUBSCRIBED_SERVICE_OPTION      =   0X21 + SM_CAUSE_START,
    OUT_OF_ORDER_SERVICE_OPTION      =   0X22 + SM_CAUSE_START,
    REGULAR_DEACTIVATION             =   0X24 + SM_CAUSE_START,
    QOS_NOT_ACCEPTED                 =   0X25 + SM_CAUSE_START,
    NETWORK_FAIL                     =   0X26 + SM_CAUSE_START,
    REACTIVATION_REQD                =   0X27 + SM_CAUSE_START,
    UNSUPPORTED_NW_CONTEXT_ACTIVATION=   0X28 + SM_CAUSE_START, /* Came and went from 4.08 + SM_CAUSE_START,
                                                  should this be removed? */
    SEMANTIC_ERROR_IN_TFT_OP         =   0X29 + SM_CAUSE_START,
    SYNTACTICAL_ERROR_IN_TFT_OP      =   0X2A + SM_CAUSE_START,
    UNKNOWN_PDP_CONTEXT              =   0X2B + SM_CAUSE_START,
    SEMANTIC_ERROR_IN_PACKET_FILTER  =   0X2C + SM_CAUSE_START,
    SYNTAX_ERROR_IN_PACKET_FILTER    =   0X2D + SM_CAUSE_START,
    PDP_CONTEXT_WO_TFT_ALREADY_ACT   =   0X2E + SM_CAUSE_START,
    /* The following cause values are only for sm internal usage */
    INVALID_TI                       =   0X51 + SM_CAUSE_START,
    INCORRECT_MSG                    =   0X5F + SM_CAUSE_START,
    INVALID_MAND_INFO                =   0X60 + SM_CAUSE_START,
    UNIMPLE_MSG_TYPE                 =   0X61 + SM_CAUSE_START,
    INCOMPAT_MSG_TYP_PROTO_STAT      =   0X62 + SM_CAUSE_START,
    UNIMPLE_IE                       =   0X63 + SM_CAUSE_START,
    CONDITIONAL_IE_ERROR             =   0X64 + SM_CAUSE_START,
    INCOMP_MSG_PROTO_STAT            =   0X65 + SM_CAUSE_START,
    UNSPECIFIED                      =   0X6F + SM_CAUSE_START,
    STARTUP_FAILURE , /* cause enum when the link layer startup fails */ 
    SM_CAUSE_END,
/* ============================== ~SM cause ============================== */

/* ============================== ABM cause ============================== */
//
// ABM error causes, Kevin Chien
//    
    ABM_CAUSE_START = 3272,
    ABM_E_SUCCESS,
    ABM_E_INVALID_NW_ACC_ID,
    ABM_E_GPRS_REACTIVATE,
    ABM_E_GPRS_PROTOCOL_REJECTION,
    ABM_E_CSD_REACTIVATE,
    ABM_E_CSD_PPP_NEGOTIATED_FAILED,
    ABM_E_CSD_ACT_FAILED,
    ABM_E_CSD_CALL_SETUP_FAILED,
    ABM_E_WIFI_REACTIVATE,
    ABM_E_WIFI_NOTREADY,
    ABM_E_REJECTED,
    ABM_E_SLOT_LIMITED,
    ABM_E_ABORT,
    ABM_E_NON_AUTO_DEACT,
/* ============================== ~ABM cause ============================== */

/* ============================== TCM cause ============================== */
    TCM_L4C_INVALID_PARAMETER = 3372,
    TCM_L4C_NSAPI_NOT_IN_USE,
    TCM_ACL_ACTION_NOT_ALLOWED,
    TCM_ACL_SIM_FILE_FULL,
    TCM_ACL_ADD_ENTRY_FAILED,
    TCM_ACL_DEL_ENTRY_FAILED,
    TCM_ACL_SET_ENTRY_FAILED,
    TCM_ACL_SIM_READ_FAILED,
    TCM_ACL_SIM_WRITE_FAILED,
/* ============================== ~TCM cause ============================== */

/* ============================== L4C cause ============================== */
    L4C_CMD_CONFLICT = 3472
/* ============================== ~L4C cause ============================== */
}ps_cause_enum;

#endif

#endif // __EXAMPLE_TCPLONG__

