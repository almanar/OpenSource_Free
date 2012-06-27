#ifdef __EXAMPLE_TCPIP__

/**************************************************************
*
*1. use Ql_GprsAPNGet and Ql_GprsAPNSet to cinfig APN
*2. use Ql_GprsNetworkInitialize to config context
*3. use Ql_GprsNetworkGetState(0, NULL, &ps_status) to get network state, 
*4. use Ql_GprsNetworkActive to active context when REG_STATE_REGISTERED == ps_status in 3
*5. now , you can use Ql_SocketCreate, Ql_SocketConnect, Ql_GetHostIpbyName .... to operate
*
***************************************************************/

#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_tcpip.h"
#include "ql_timer.h"
#include "ql_stdlib.h"
#include "ql_sms.h"
#include "ql_fcm.h"

//#include "ql_pin.h"
#include "ql_filesystem.h"
#include "ql_error.h"
#include "ql_fota.h"
#include "ql_pin.h"
#include <math.h>




#define OUT_DEBUG(x,...)  \
    Ql_memset((x),0,100);  \
    Ql_sprintf((x),__VA_ARGS__);   \
    Ql_SendToUart(ql_uart_port1,(u8 *)(x),Ql_strlen(x));


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

void Callback_Read_ADC(QlADCPin adc_pin, u8 status, u16 adc_val);

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

void uart2_datahandler(u16 Datalen,char *data);
bool uart1_datahandler(u16 datlength,char * data);
void GPIOPin_Config(void);
void filesystem_function(void);
void key_send(void);
void FileSyatem_ReadFunction(void);
void GPIO_ReadFunction(void);
void io_send(void);
void iosend_function(void);
void Send_GSMVersionString(void);
void Socket_DataSend_Function(void);
u8 Uart1_setting_received(void);
void save_new_ip_settings(void);
void OpenSetting_File(void);
void save_new_OwnNumber_settings(void);
void save_new_Update_timer_settings(void);
void save_new_Apn_settings(void);

void SendAtCmd(void);
void SendAtCmd_Message_Read(void);
void SendGPS_Setting(void);
void Power_Supply_Pulse(void);
void adcsend_function(void);
void  Create_WavePointFile();

void INTtoHextoASCII(ascii *buf,u32 IntNumber,u8 Numberofbyte);
u32 ASCIItoINT(ascii *buf,u8 count);
void INTtoASCII(u32 IntNumber,ascii *buf);
s32 lat_calc(ascii *buff);
void distance_calc_TimerHandler(void);



#define RXBUF_SIZE 80
#define NO_OF_WAVE_POINT_STORED 5
#define PANIC_PRESS_DETECTED (1<<0)

u8 data_key=0;
u8 ispressed_varible=0xff;
u16 temp_key=0;
u16 New_temp_key=0;
u8 key_buffer[2];


char Uart1_Setting_Buffer[50];
u8 setting1_counter=0;
u8 setting_length_counter=0;
u8 GPRMC_STRING_FLAG=FALSE;
u8 FTP_START_FLAG=FALSE;
u8 MESSAGE_RECEIVED_FLAG=FALSE;
u8 MESSAGE_SETTING_OK=FALSE;
u8 MESSAGE_DELETE_COMMAND=FALSE;
char Message_No='\0';
char Message_Buffer[100];



char textBuf[100];
u8 strBuf[100];
s32 filehandle = -1;
s32 filehandle1 = -1;
s32 filehandle2 = -1;


 s32 c=0;

u8 Power_Previous_State=0;
u8 Power_Current_State=0;
u8 AC_Previous_State=0;
u8 AC_Current_State=0;
u8 Ignition_Previous_State=0;
u8 Ignition_Current_State=0;
u8 Door_Previous_State=0;
u8 Door_Current_State=0;
u8 IP3_Previous_State=1;
u8 IP3_Current_State=0;
u8 IO_STATUS=0x27;
u8 IO_Status_Change=0;

u8 Not_Dota_Command_Receive=1;
		
char End_Array[]="EEEEEEEEEE";


 u32 ReadPositionStatus=0;
 u32 WritePositionStatus=0;
  
 u32 Position=0;
 u32 Position1=0;
 u32 Read_Position=0;
 u32 nextposition=0;
 u32 Read_nextposition=0;
 u16 GPRS_StringLength=0;
s32 globalpos=0;
u8  gpscount=0;
 QlEventBuffer  flSignalBuffer;
ascii prox_data[95];
char GPSString[82];
char localdata[82];



char Uart1_Event_Buffer[82];
s32 Uart1_Event_variable=0;
char Uart1_Event_Local_Buffer[82];
s32 Uart1_Event_Local_variable=0;
//char localdata[]="$GPRMC,055129.00,A,1828.91021,N,07353.89991,E,0.077,,270111,,,A*74\n";
//char GPSString[]="$GPRMC,055129.00,A,1828.91021,N,07353.89991,E,0.077,,270111,,,A*74\n";
u8 gprsupdate_flag=1;

char uart1_buff[2000];
static int u_cnt=0;
static int uart1_ptr1=0;
ascii set_para[39];
//u8 finalF[53];
u32 sms_val,fall_cnt,fix_cnt,op_cnt;
ascii hexbuff[6]="000000",hex_buff[6],hex_input[2],hex_input_status[2];
ascii SERVER_ADD[15];
//float fresult=0;
u32 fresult=0;
char finalp[79];//,date[9],time[11];
bool input_status,suppy_removed,suppy_connected,intr_check,ac_sense_on,ac_sense_off,panic_external,mobiliser,key_talk,panic_status,fuel,ignition,output2,key_pickup;
char glo_RX_buff[80];
u8 RXbuf_end_ptr1=0;
u8 send;


ascii speed2[10];
ascii speed3[10];

ascii lat[11],lon[11],status,head[9],date[9]="000000000",time[11]="00000000000",speed1[9]="000000000",speed[9]="000000000";//static change
ascii finalF[63];
ascii TempBuffer[100];
ascii adc_value[4];
u8 Socket_Buffer_API[100];
u16  Socket_Buffer_Length=0;
u8 fix=0;
u8 fix_status=FALSE;
u8 GPRS_CONNECTION=FALSE;
u8 Not_Parameter_Update=TRUE;

s32 iret;
char string_start_head[]="!*0109850421299,000000#*35";
char owen_number[]="7387080434";
char versionstring[]="#*05M30FVERSION008*#";
u8 StringtoSendGprs[100];
u8 Global_sock=0;
u16 port = 2020;    //7008
//u16 port = 2023;    //7008
char port1_buffer[]="2020";
char port2_buffer[]="2020";
char address1_buffer[] ="115.112.176.089";
//char address1_buffer[] ="122.169.104.005";
char address2_buffer[] ="122.169.104.005";
char APN[]="airtelgprs.com\0";
char Apn_Setting_Buffer[30];
char timer_update_rate[]="600";
u32 Update_Timer_Variable=0;
//u16 port = 3456;
//u8 address[4] = {122,169,104,5};

char GPRMC_STRING_SETTING[55];


u8 address[4] = {202,56,200,98};        //3456 port 
//u8 address[4] = {115,112,176,89};       //rahul enterprises
//u8 address[4] = {115,112,241,148};   //MDATEST
//u8 address[4] = {115,112,176,89};      //Prr Pune


#define DATA_LEN 80/*1000*/

s8  socketonly[2] = {0xFF, 0xFF}; // tcp socket
s8  udponly[2] = {0xFF, 0xFF}; // udp socket

s32 dataLen[2] ={DATA_LEN};
s32 send_dataLen[2] ={0};

u8 data_p[DATA_LEN]={0};
char *p2=NULL;
char *p3=NULL;
char status_check='\0';
QlTimer tm;
QlTimer timer1min;
QlTimer statustimer;
QlTimer fota_timer;
QlTimer filetimer;
QlTimer versiontimer;
QlTimer IpSettingTimer;
QlTimer PulseTimer;
QlTimer AdcTimer;
QlTimer SpeedCalTimer;

char buffer[100];
char at_response[20];
char at_Message_response[100];

char FTP_USER[]="ftphardware";
char FTP_PASS[]="deepakhemant";
char FTP_ADDR[]="202.56.200.98";
char FTP_PORT[]="21";
char FTP_UFS_PATH[]="/UFS/";
char FTP_PATH[]="/";
char FTP_FILE[]="QL_CUSTOM_V01.bin";
//char FTP_FILE[]="QUECTEL_CORE.bin";
u8 deltabuf[512];

u16 idxCmd=0;
u16 msg_idxCmd=0;
ascii coreBinFile[100];
ascii appBinFile[100];
u32 GlobalFlags;

ascii buff2[6],buff3[6],latf1[2],dfinal[8];
u8 check_dis=0,check_gprs=0,check_coverage=0;
bool speed_flag=FALSE,gps_err=FALSE,start_gpscalc=FALSE;
u32 sms_val=18000,fall_cnt=0,fix_cnt=0,op_cnt=0,over_speed;
float result=0,result_temp=0,result_ref=0,result_ref1;
float sresult=0;
u32 disty,latx,laty;
//float distx;
u32 distx;
u32 spret1,spret2,spret,pret;
static u8 speed_flag_cnt=0,speed_mem_cnt=0;
u32 input;



u8 CHECKFLAG( u32 Flag )
{
	if ( GlobalFlags & Flag )	
	{	return 1;		}
	else						
	{	return 0;	}
}

void SETFLAG( u32 Flag )
{
	GlobalFlags |= Flag;
	return;	
}

void RESETFLAG( u32 Flag )
{
	GlobalFlags &= ~Flag;
	return;	
}
  
void INTtoHextoASCII(ascii *buf,u32 IntNumber,u8 Numberofbyte)
{
  u8 TemoData=0;
  u8 Tempvar=0,i=0;

 
    while(Numberofbyte--)
	{
		Tempvar=(0x0000000F &(IntNumber>>(4*Numberofbyte)));
		if(Tempvar >=0x00 && Tempvar <=0x09)
		{
		 Tempvar+=0x30;
		 *(buf+i)= Tempvar;
		}
		else
		{
		  Tempvar+=0x37;
		  *(buf+i)= Tempvar;
		}
		i++;
	}
}

void INTtoASCII(u32 IntNumber,ascii *buf)
{
	u32 TempIntNumber=0,Multiplier=1;
	u8 Dividen=0,i=0,Temp_count=0;
	TempIntNumber=IntNumber;
	for(i=0;i<15;i++)
	{
		TempIntNumber=IntNumber/Multiplier;
		if(TempIntNumber <=9)
		{
		break;
		}
		Multiplier*=10;
	}
	Temp_count=i;
	TempIntNumber=IntNumber;
	for(i=0;i<Temp_count;i++)
	{
		Dividen=TempIntNumber/Multiplier;
		*(buf+i)=Dividen+0x30;
		TempIntNumber%=Multiplier;
		Multiplier/=10;
		
	}
	
}

u32 ASCIItoINT(ascii *buf,u8 count)
{
	u32 IntNumber=0,Multiplier=1;
	u8 Temp_count=0,i=0;
	Temp_count=count;
	while(count--)
	{
	  Multiplier*=10;
	}
	
	for(i=0;i<Temp_count;i++)
	{
		Multiplier/=10;
		IntNumber+=	(Multiplier * ((*(buf+i))-0x30));
		
	}
	return 	IntNumber;
}


/******************************ODOMETER FUNCTION***********************************************/

s32 lat_calc(ascii *buff)
{
	int i,j=0,k=0;
	float ret=0;
	s32 ret1=0;
	if(fix==TRUE)
	{
		for(i=0;i<=1;i++)
		{
			latf1[k]=buff[i];
			k++;
		}
		
		for(i=2;i<=7;i++)
		{
			buff2[j]=buff[i];
			j++;
		}
		
		
		//ret1= wm_atoi(buff2); //o'th index msb
		ret1=ASCIItoINT(buff2,6);
		Ql_sprintf(buffer, "\r\nFIRST OUT=%d\r\n",ret1);
        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
		ret=(float)ret1/60;
		ret1=(s32)ret*100;
	//	wm_itoa(ret1,buff3);
		INTtoASCII(ret1,buff3);
		Ql_SendToUart(ql_uart_port1,(u8*)"ASCII_VALUE",Ql_strlen("ASCII_VALUE"));
		Ql_SendToUart(ql_uart_port1,(u8*)buff3,6);
		Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

		dfinal[0]=latf1[0];
		dfinal[1]=latf1[1];
		dfinal[2]=buff3[0];
		dfinal[3]=buff3[1];
		dfinal[4]=buff3[2];
		dfinal[5]=buff3[3];
		dfinal[6]=buff3[4];
		dfinal[7]=buff3[5];
		ret1=ASCIItoINT(dfinal,8);
		Ql_sprintf(buffer, "\r\nSECOND OUT=%d\r\n",ret1);
        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
		//ret1= wm_atoi(dfinal);
		
	}
	return  ret1;
}
void distance_calc_TimerHandler (void)
{
	static s8 z;
	static s32 lattf1,prevlat,protlat;
	static s32 lonnf1,prevlon,protlon;
	static char lat1[8],lon1[8],tstbuff1[8],mon1[8];
	
	check_dis=0;
	
	if(speed_flag==TRUE)
	{
		result_ref=0;
		result_ref1=0;
	}
	else
	{
		result_ref=300;//1200
		result_ref1=500;//1800
	}
	
	
	if(fix==TRUE)
		
	{
		if(start_gpscalc==TRUE)
		{
			if(speed_flag==FALSE)
			{
				
				//wm_strcpy(lat1,lat);
				lat1[0]=lat[0];
				lat1[1]=lat[1];
				lat1[2]=lat[2];
				lat1[3]=lat[3];
				lat1[4]=lat[4];
				lat1[5]=lat[5];
				lat1[6]=lat[6];
				lat1[7]=lat[7];

				
				lon1[0]=finalp[35];
				lon1[1]=finalp[36];
				lon1[2]=finalp[37];
				lon1[3]=finalp[38];
				lon1[4]=finalp[39];
				lon1[5]=finalp[40];
				lon1[6]=finalp[41];
				lon1[7]=finalp[42];
				//wm_strncpy(mon1,lon1,8);
				mon1[0]=lon1[0];
				mon1[1]=lon1[1];
				mon1[2]=lon1[2];
				mon1[3]=lon1[3];
				mon1[4]=lon1[4];
				mon1[5]=lon1[5];
				mon1[6]=lon1[6];
				mon1[7]=lon1[7];
				
				lattf1=lat_calc(lat1);
				lonnf1=lat_calc(mon1);

				Ql_sprintf(buffer, "\r\nlatf1=%d\r\n",lattf1);
        		Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
				Ql_sprintf(buffer, "\r\nlonnf1=%d\r\n",lonnf1);
        		Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
				
				
				if(prevlat==0)
				{
					prevlat=lattf1;
					prevlon=lonnf1;
					protlat=lattf1;
					protlon=lonnf1;
					Ql_SendToUart(ql_uart_port1,(u8*)"I AM IN PREVLAT=0",Ql_strlen("I AM IN PREVLAT=0"));
					Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
				}
				else
				{
					Ql_SendToUart(ql_uart_port1,(u8*)"I AM IN PREVLAT=1",Ql_strlen("I AM IN PREVLAT=1"));
					Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
					latx=(lattf1-prevlat);
					
					laty=(lonnf1-prevlon);
					

					distx=125.2*(latx*latx);
					//distx=126*(latx*latx);
					
					disty=116*(laty*laty);
					


					input=distx+disty;

					result=sqrt(input);
					Ql_sprintf(buffer, "\r\nresult=%f\r\n",result);
        			Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
					
					result=result/100;
					/*adl_atSendResponse ( ADL_AT_RSP,"\r\n 1st Result= ");
					wm_itoa(result,tstbuff1);
					adl_atSendResponse ( ADL_AT_RSP,tstbuff1);*/
					
					if(result> result_ref)
						
					{

						gps_err=TRUE;
						prevlat=lattf1;
						prevlon=lonnf1;
					} 
					else
					{
						if(gps_err==FALSE) 
						{
							if(speed_flag==FALSE)
							{
								if(fresult>=16000001)
								{
									Ql_SendToUart(ql_uart_port1,(u8*)"I AM IN FRESULT16",Ql_strlen("I AM IN FRESULT16"));
									Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
									fresult=0;
								}
								else
								{
									fresult=fresult+result;
								}
								
								//wm_itohexa(hexbuff,fresult,6); need to place
								Ql_sprintf(buffer, "\r\nfresult=%d\r\n",fresult);
        						Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
								INTtoHextoASCII(hexbuff,fresult,6);
								Ql_SendToUart(ql_uart_port1,(u8*)"SPEED RECEIVED:",Ql_strlen("SPEED RECEIVED:"));
								Ql_SendToUart(ql_uart_port1,(u8*)hexbuff,6);
								Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
								
							}
							else
							{
								//adl_atSendResponse ( ADL_AT_RSP,"\r\n **NO distance CALULATIONS** ");
							}
							
							//adl_atSendResponse ( ADL_AT_RSP,"\r\n1st  original_distance is = ");
							//adl_atSendResponse ( ADL_AT_RSP,hexbuff);
							prevlat=lattf1;
							prevlon=lonnf1;
							protlat=lattf1;
							protlon=lonnf1;
						}
						else
						{

							gps_err=FALSE;
							latx=(lattf1-protlat);
							laty=(lonnf1-protlon);
							distx=125.2*(latx*latx);
							//distx=126*(latx*latx);
							
							disty=116*(laty*laty);
							
							input=distx+disty;
							result_temp=sqrt(input);
							result_temp=result_temp/100;
							//adl_atSendResponse ( ADL_AT_RSP,"\r\n2nd Result= ");
						//	wm_itoa(result,tstbuff1); needed to place
						//	adl_atSendResponse ( ADL_AT_RSP,tstbuff1);
							if(result_temp>result_ref1)
								
							{
								prevlat=lattf1;
								prevlon=lonnf1;
								protlat=lattf1;
								protlon=lonnf1;
							}
							else
							{
								result=result_temp;
								if(speed_flag==FALSE)
								{
									fresult=fresult+result;
									//wm_itohexa(hexbuff,fresult,6); needed to place
									INTtoHextoASCII(hexbuff,fresult,6);

								}
								else
								{
									//adl_atSendResponse ( ADL_AT_RSP,"\r\n **NO distance CALULATIONS** ");
								}
								
								
								//adl_atSendResponse ( ADL_AT_RSP,"\r\n2nd  original_distance is = ");
								//adl_atSendResponse ( ADL_AT_RSP,hexbuff);
								prevlat=lattf1;
								prevlon=lonnf1;
								protlat=lattf1;
								protlon=lonnf1;
							}
							
							
						}
						
					}
					
		}   
	}
}  

else
{
	prevlat=0;
	prevlon=0;
	protlat=0;
	protlon=0;
	

}

} 

			
			  


} 
/******************************ODOMETER FUNCTION END*******************************************/


/*******************************Gps Setting Function*******************************************/
void SendGPS_Setting(void)
{
		 GPRMC_STRING_SETTING[0]='$';
		 GPRMC_STRING_SETTING[1]='P';
		 GPRMC_STRING_SETTING[2]='M';
		 GPRMC_STRING_SETTING[3]='T';
		 GPRMC_STRING_SETTING[4]='K';
		 GPRMC_STRING_SETTING[5]='3';
		 GPRMC_STRING_SETTING[6]='1';
		 GPRMC_STRING_SETTING[7]='4';
		 GPRMC_STRING_SETTING[8]=',';
		 GPRMC_STRING_SETTING[9]='0';
		 GPRMC_STRING_SETTING[10]=',';
		 GPRMC_STRING_SETTING[11]='1';
		 GPRMC_STRING_SETTING[12]=',';
		 GPRMC_STRING_SETTING[13]='0';
		 GPRMC_STRING_SETTING[14]=',';
		 GPRMC_STRING_SETTING[15]='0';
		 GPRMC_STRING_SETTING[16]=',';
		 GPRMC_STRING_SETTING[17]='0';
		 GPRMC_STRING_SETTING[18]=',';
		 GPRMC_STRING_SETTING[19]='0';
		 GPRMC_STRING_SETTING[20]=',';
		 GPRMC_STRING_SETTING[21]='0';
		 GPRMC_STRING_SETTING[22]=',';
		 GPRMC_STRING_SETTING[23]='0';
		 GPRMC_STRING_SETTING[24]=',';
		 GPRMC_STRING_SETTING[25]='0';
		 GPRMC_STRING_SETTING[26]=',';
		 GPRMC_STRING_SETTING[27]='0';
		 GPRMC_STRING_SETTING[28]=',';
		 GPRMC_STRING_SETTING[29]='0';
		 GPRMC_STRING_SETTING[30]=',';
		 GPRMC_STRING_SETTING[31]='0';
		 GPRMC_STRING_SETTING[32]=',';
		 GPRMC_STRING_SETTING[33]='0';
		 GPRMC_STRING_SETTING[34]=',';
		 GPRMC_STRING_SETTING[35]='0';
		 GPRMC_STRING_SETTING[36]=',';
		 GPRMC_STRING_SETTING[37]='0';
		 GPRMC_STRING_SETTING[38]=',';
		 GPRMC_STRING_SETTING[39]='0';
		 GPRMC_STRING_SETTING[40]=',';
		 GPRMC_STRING_SETTING[41]='0';
		 GPRMC_STRING_SETTING[42]=',';
		 GPRMC_STRING_SETTING[43]='0';
		 GPRMC_STRING_SETTING[44]=',';
		 GPRMC_STRING_SETTING[45]='0';
		 GPRMC_STRING_SETTING[46]=0x2A;
		 GPRMC_STRING_SETTING[47]='2';
		 GPRMC_STRING_SETTING[48]='9';
		 GPRMC_STRING_SETTING[49]=0X0D;
		 GPRMC_STRING_SETTING[50]=0X0A;
		 Ql_SendToUart(ql_uart_port2,(u8*)GPRMC_STRING_SETTING,52);
		 Ql_SendToUart(ql_uart_port1,(u8*)GPRMC_STRING_SETTING,52);
		 
}
/************************************end******************************************************/



/**********************************Uart1 Setting Function***************************************/
u8 Uart1_setting_received(void)
{
	u8 i,j;
	u8 Dummy_Length=0;
	s32 ret;
	
	if(Uart1_Setting_Buffer[0]=='#' && Uart1_Setting_Buffer[1]=='*' && Uart1_Setting_Buffer[setting_length_counter-3]=='*' && Uart1_Setting_Buffer[setting_length_counter-2]=='#' && Uart1_Setting_Buffer[setting_length_counter-1]==0x03)
	{
	Dummy_Length=setting_length_counter;
	setting_length_counter=0;
	switch(Uart1_Setting_Buffer[2])
	{
		
		
		
	
		case 'Y':
		case 'y':
		if(Dummy_Length==6)
		{
		ret = Ql_pinWrite( QL_PINNAME_KBC4, QL_PINLEVEL_LOW);
        Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",ret,QL_PINNAME_KBC4);
        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
        Ql_SendToUart(ql_uart_port1,(u8*)"Employe Panic Ack Received\r\n",Ql_strlen("Employe Panic Ack Received\r\n"));
        }
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;
		case 'S':
		case 's':


		if(Dummy_Length==30)
		{
		if(Uart1_Setting_Buffer[6]=='.' && Uart1_Setting_Buffer[10]=='.' && Uart1_Setting_Buffer[14]=='.')
		{
		for(i=0;i<15;i++)
		address1_buffer[i]=Uart1_Setting_Buffer[i+3];
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		if(Uart1_Setting_Buffer[18]=='P' || Uart1_Setting_Buffer[18]=='p')
		{
		for(i=0;i<4;i++)
		port1_buffer[i]=Uart1_Setting_Buffer[i+19];
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		
		if(Uart1_Setting_Buffer[23]=='M' || Uart1_Setting_Buffer[23]=='m')
		{
		for(i=0;i<3;i++)
		timer_update_rate[i]=Uart1_Setting_Buffer[i+24];
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		
		
	
		save_new_ip_settings();
		break;


		case 'A':
		case 'a':
		
		if(1)
		{
		for(i=0;Uart1_Setting_Buffer[i+3]!='*';i++)
		{
		Apn_Setting_Buffer[i]=Uart1_Setting_Buffer[i+3];
			if(i>=100)
			{
			return 0;
			}
		}
		Apn_Setting_Buffer[i]='\0';
		save_new_Apn_settings();
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;


		case 'O':
		case 'o':
		if(Dummy_Length==16)
		{
		for(i=0;i<10;i++)
		owen_number[i]=Uart1_Setting_Buffer[i+3];
		save_new_OwnNumber_settings();
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;

		case 'D':
		case 'd':
		if(Dummy_Length==6)
		{
		Ql_StopTimer(&timer1min);
        Ql_StopTimer(&statustimer);
        Ql_SocketClose(Global_sock);
        ret = Ql_GprsNetworkDeactive(1);
//        OUT_DEBUG(textBuf,"Ql_GprsNetworkDeactive(contxtid=%d)=%d\r\n",1,ret); 
		FTP_START_FLAG=TRUE;
        idxCmd = 1;
        Ql_SendToUart(ql_uart_port1,(u8*)"DOTA COMMAND RECEIVED\r\n",Ql_strlen("DOTA COMMAND RECEIVED\r\n"));
        SendAtCmd();
		
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;

		case 'T':
		case 't':
		if(Dummy_Length==9)
		{
		for(i=0;i<3;i++)
		timer_update_rate[i]=Uart1_Setting_Buffer[i+3];
		save_new_Update_timer_settings();
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;

		case 'H':
		case 'h':
		if(Dummy_Length==7)
		{
		if(Uart1_Setting_Buffer[3]=='1')
			{
				GPRMC_STRING_FLAG=TRUE;
			}

		else if(Uart1_Setting_Buffer[3]=='0')
			{
				GPRMC_STRING_FLAG=FALSE;
			}
		else
			{
			Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
			return 0;	
			}
		
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;

		case 'C':   
		case 'c':
		if(Dummy_Length==7)
		{
		
			if(Uart1_Setting_Buffer[3]=='1')
			{
					iret = Ql_pinWrite( QL_PINNAME_M10_GPIO1, QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret, QL_PINNAME_M10_GPIO1);
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 
			}

		else if(Uart1_Setting_Buffer[3]=='0')
			{
					iret = Ql_pinWrite( QL_PINNAME_M10_GPIO1, QL_PINLEVEL_LOW);
                	Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret, QL_PINNAME_M10_GPIO1 );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 
			}
		else
			{
			Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
			return 0;	
			}

			/*if(Uart1_Setting_Buffer[3]=='1')
			{
					//iret = Ql_pinWrite( QL_PINNAME_M30_LIGHT_MOS, QL_PINLEVEL_HIGH);
                	//Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret, QL_PINNAME_M30_LIGHT_MOS);
                	//Ql_SendToUart(ql_uart_port1,buffer,Ql_strlen(buffer));
                	
                	iret = Ql_pinControl( QL_PINNAME_M30_LIGHT_MOS, QL_PINCONTROL_START);
                    Ql_sprintf(buffer, "Ql_pinControl(pin=%d, ctrl=%d)=%d\r\n", QL_PINNAME_M30_LIGHT_MOS, QL_PINCONTROL_START, iret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));  
			}

		else if(Uart1_Setting_Buffer[3]=='0')
			{
					//iret = Ql_pinWrite( QL_PINNAME_M30_LIGHT_MOS, QL_PINLEVEL_LOW);
                	//Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret,  QL_PINNAME_M30_LIGHT_MOS );
                	//Ql_SendToUart(ql_uart_port1,buffer,Ql_strlen(buffer));
                	
                	iret = Ql_pinControl( QL_PINNAME_M30_LIGHT_MOS, QL_PINCONTROL_STOP);
                    Ql_sprintf(buffer, "Ql_pinControl(pin=%d, ctrl=%d)=%d\r\n", QL_PINNAME_M30_LIGHT_MOS,QL_PINCONTROL_STOP, iret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));
			} */
		
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;

		case 'B':
		case 'b':
		if(Dummy_Length==7)
		{
		if(Uart1_Setting_Buffer[3]=='1')
			{
					iret = Ql_pinWrite( QL_PINNAME_M10_GPIO0, QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret,QL_PINNAME_M10_GPIO0);
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 
                	
                	/*iret = Ql_pinControl( QL_PINNAME_M30_LED_B, QL_PINCONTROL_START);
                    Ql_sprintf(buffer, "Ql_pinControl(pin=%d, ctrl=%d)=%d\r\n", QL_PINNAME_M30_LED_B, QL_PINCONTROL_START, iret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer)); */
			}

		else if(Uart1_Setting_Buffer[3]=='0')
			{
					iret = Ql_pinWrite( QL_PINNAME_M10_GPIO0, QL_PINLEVEL_LOW);
                	Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret,  QL_PINNAME_M10_GPIO0 );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
                	
                	/*iret = Ql_pinControl( QL_PINNAME_M30_LED_B, QL_PINCONTROL_STOP);
                    Ql_sprintf(buffer, "Ql_pinControl(pin=%d, ctrl=%d)=%d\r\n", QL_PINNAME_M30_LED_B,QL_PINCONTROL_STOP, iret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));*/
			}
		else
			{
			Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
			return 0;	
			}
		
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;

		case 'E':
		case 'e':
		if(Dummy_Length==7)
		{
		if(Uart1_Setting_Buffer[3]=='1')
			{
					/*iret = Ql_pinWrite( QL_PINNAME_M30_GPIO2, QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret, QL_PINNAME_M30_GPIO2);
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); */
			}

		else if(Uart1_Setting_Buffer[3]=='0')
			{
					/*iret = Ql_pinWrite( QL_PINNAME_M30_GPIO2, QL_PINLEVEL_LOW);
                	Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret, QL_PINNAME_M30_GPIO2 );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); */
			}
		else
			{
			Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
			return 0;	
			}
		
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;

		case 'L':
		case 'i':
		if(Dummy_Length==7)
		{
		if(Uart1_Setting_Buffer[3]=='1')
			{
					iret = Ql_pinWrite( QL_PINNAME_M30_PCM_SYNC, QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret, QL_PINNAME_M30_PCM_SYNC);
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 
			}

		else if(Uart1_Setting_Buffer[3]=='0')
			{
					iret = Ql_pinWrite( QL_PINNAME_M30_PCM_SYNC, QL_PINLEVEL_LOW);
                	Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret, QL_PINNAME_M30_PCM_SYNC );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 
			}
		else
			{
			Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
			return 0;	
			}
		
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;

		case 'P':
		case 'p':
		if(Dummy_Length==7)
		{
		if(Uart1_Setting_Buffer[3]=='1')
			{
					
                	
                	iret = Ql_pinControl(QL_PINNAME_LIGHT_MOS, QL_PINCONTROL_START);
                    Ql_sprintf(buffer, "Ql_pinControl(pin=%d, ctrl=%d)=%d\r\n",QL_PINNAME_LIGHT_MOS, QL_PINCONTROL_START, iret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer)); 
                	/*iret = Ql_pinWrite( QL_PINNAME_M10_DISP_CLK , QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret,QL_PINNAME_M10_DISP_CS );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
                	
                	iret = Ql_pinWrite( QL_PINNAME_M10_DCD , QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret,QL_PINNAME_M10_DCD );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));*/
                	
                	
                	
                	
                	iret = Ql_pinWrite( QL_PINNAME_KBR4 , QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret,QL_PINNAME_KBR4 );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
                	
                	
                	
                	iret = Ql_pinWrite( QL_PINNAME_KBC4 , QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret,QL_PINNAME_KBC4 );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
                	
                	
                	
                	
                	
                	
                	
                	
                	
                	
						
			}

		else if(Uart1_Setting_Buffer[3]=='0')
			{
					
                	/*iret = Ql_pinWrite(QL_PINNAME_M10_DISP_CLK , QL_PINLEVEL_LOW);
                	Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret, QL_PINNAME_M10_DISP_CS  );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
                	
                	
                	iret = Ql_pinWrite(QL_PINNAME_M10_DCD , QL_PINLEVEL_LOW);
                	Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret,QL_PINNAME_M10_DCD );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));*/
                	
                	
                	iret = Ql_pinWrite(QL_PINNAME_KBR4 , QL_PINLEVEL_LOW);
                	Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret,QL_PINNAME_KBR4 );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
                
                	
                	iret = Ql_pinWrite(QL_PINNAME_KBC4, QL_PINLEVEL_LOW);
                	Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret, QL_PINNAME_KBC4 );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 
                	
                	iret = Ql_pinControl(QL_PINNAME_LIGHT_MOS, QL_PINCONTROL_STOP);
                    Ql_sprintf(buffer, "Ql_pinControl(pin=%d, ctrl=%d)=%d\r\n",QL_PINNAME_LIGHT_MOS, QL_PINCONTROL_STOP, iret);
                    Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));
                	
                	
                	
                	
                	 
                		
                	
			}
		else
			{
			Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
			return 0;	
			}
		
		}
		else
		{
		Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
		return 0;
		}
		break;

		

		


	
		
		
	}
	}
	else
	{
	  setting_length_counter=0;
	  Ql_SendToUart(ql_uart_port1,(u8*)"You Have Entered Wrong Setting ,Please Enter Write\r\n",Ql_strlen("You Have Entered Wrong Setting ,Please Enter Write\r\n"));
	}
}

void save_new_Apn_settings(void)
{
		char *p=NULL;
	   u32 writeedlen;
       u32 readedlen;
	    s32 ret;
		u8 apn_length=0;
		
		OpenSetting_File();
		
		
	{
	 p=Apn_Setting_Buffer;
	 Ql_FileSeek(filehandle2,32, QL_FS_FILE_BEGIN);
     apn_length=Ql_strlen(Apn_Setting_Buffer); 
     ret = Ql_FileWrite(filehandle2, (u8*)p,(apn_length+1), &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	}

	{
	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,32, QL_FS_FILE_BEGIN);
	 ret = Ql_FileRead(filehandle2, strBuf,30,&readedlen);
	 for(apn_length=0;strBuf[apn_length]!='\0';apn_length++)
	 {
	 Apn_Setting_Buffer[apn_length]=strBuf[apn_length];
	 }
	 Apn_Setting_Buffer[apn_length]='\0';

	 // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"New_APN_Setting=",Ql_strlen("New_APN_Setting="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
     }
      	 
	 
      Ql_FileClose(filehandle2);
      filehandle2 = -1;
      Ql_memset(textBuf,0,63);
      Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); 
	  
	  IpSettingTimer.timeoutPeriod=Ql_SecondToTicks(5);
	  Ql_StartTimer(&IpSettingTimer); 
}

void save_new_Update_timer_settings(void)
{
	 char *p=NULL;
	   u32 writeedlen;
       u32 readedlen;
	    s32 ret;
		
		OpenSetting_File();
		
		
	{
	 p=timer_update_rate;
	 Ql_FileSeek(filehandle2,19, QL_FS_FILE_BEGIN);
      
     ret = Ql_FileWrite(filehandle2, (u8*)p,3, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	}
     
      	 
	 {
	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,19, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,3,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
	 Update_Timer_Variable=((((strBuf[0]-0x30)*100)+((strBuf[1]-0x30)*10)+((strBuf[2]-0x30)*1))/10);
     Ql_SendToUart(ql_uart_port1,(u8*)"New_Timer_Update_Rate=",Ql_strlen("New_Timer_Update_Rate="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
 	 }
      Ql_FileClose(filehandle2);
      filehandle2 = -1;
      Ql_memset(textBuf,0,63);
      Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));  
      
}

void save_new_OwnNumber_settings(void)
{
	   char *p=NULL;
	   u32 writeedlen;
       u32 readedlen;
	    s32 ret;
		
		OpenSetting_File();
		
		 
     if(1)
     {
	 p=owen_number;
	 Ql_FileSeek(filehandle2,22, QL_FS_FILE_BEGIN);
      
     ret = Ql_FileWrite(filehandle2, (u8*)p,10, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
     
     }
     
      
    

	
	{
	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,22, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,10,&readedlen);

	 owen_number[0]=strBuf[0];
	 owen_number[1]=strBuf[1];
	 owen_number[2]=strBuf[2];
	 owen_number[3]=strBuf[3];
	 owen_number[4]=strBuf[4];
	 owen_number[5]=strBuf[5];
	 owen_number[6]=strBuf[6];
	 owen_number[7]=strBuf[7];
	 owen_number[8]=strBuf[8];
	 owen_number[9]=strBuf[9];
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"New_OWN_Number=",Ql_strlen("New_OWN_Number="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

     
     
      Ql_FileClose(filehandle2);
      filehandle2 = -1;
      Ql_memset(textBuf,0,63);
      Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));  
      }
}
void save_new_ip_settings(void)
{
	   char *p=NULL;
	   u32 writeedlen;
       u32 readedlen;
	    s32 ret;
		
		OpenSetting_File();
		
		 p=port1_buffer;
     if(p)
     {
      
     Ql_memset(textBuf,0,63);
    
     ret = Ql_FileWrite(filehandle2, (u8*)p,4, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
     
     p=address1_buffer;
	 Ql_FileSeek(filehandle2,4, QL_FS_FILE_BEGIN);
      
     ret = Ql_FileWrite(filehandle2, (u8*)p,15, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));

	 p=timer_update_rate;
	 Ql_FileSeek(filehandle2,19, QL_FS_FILE_BEGIN);
      
     ret = Ql_FileWrite(filehandle2, (u8*)p,3, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));



     }
     
      
    

	
	{
	  

	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,0, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,4,&readedlen);

	 port= (((strBuf[0]-0x30)*1000)+((strBuf[1]-0x30)*100)+((strBuf[2]-0x30)*10)+((strBuf[3]-0x30)*1));
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"New_Port_Number=",Ql_strlen("New_Port_Number="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
     
     Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,4, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,15,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);

	 address[0]=(((strBuf[0]-0x30)*100)+((strBuf[1]-0x30)*10)+((strBuf[2]-0x30)*1));
	 address[1]=(((strBuf[4]-0x30)*100)+((strBuf[5]-0x30)*10)+((strBuf[6]-0x30)*1));
	 address[2]=(((strBuf[8]-0x30)*100)+((strBuf[9]-0x30)*10)+((strBuf[10]-0x30)*1));
	 address[3]=(((strBuf[12]-0x30)*100)+((strBuf[13]-0x30)*10)+((strBuf[14]-0x30)*1));

     Ql_SendToUart(ql_uart_port1,(u8*)"New_IP_Address=",Ql_strlen("New_IP_Address="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,19, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,3,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
	 Update_Timer_Variable=((((strBuf[0]-0x30)*100)+((strBuf[1]-0x30)*10)+((strBuf[2]-0x30)*1))/10);
     Ql_SendToUart(ql_uart_port1,(u8*)"New_Timer_Update_Rate=",Ql_strlen("New_Timer_Update_Rate="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
 
      Ql_FileClose(filehandle2);
      filehandle2 = -1;
      Ql_memset(textBuf,0,63);
      Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));  
      }
	  
	  IpSettingTimer.timeoutPeriod=Ql_SecondToTicks(15);
	  Ql_StartTimer(&IpSettingTimer);
	  //Ql_Reset(0);
}
/*****************************************END of Function***************************************/

/***************************************Message Setting Area*************************************/
void SendAtCmd_Message_Read()
{
  bool exec = 1;
  u8 i=0;
  Ql_memset(buffer,0,100);
  switch (msg_idxCmd)
  {
    
     //1. Get...
    case 1:// Ver
        Ql_sprintf((char *)buffer,"AT+CMGF=1\r\n");
        Ql_sprintf((char *)at_Message_response,"OK\r\n"); 
        break; 
		  
    case 2:// Ver
        Ql_sprintf((char *)buffer,"AT+CMGR=");
        buffer[8]=Message_No;
        buffer[9]='\r';
       	buffer[10]='\n';
        Ql_sprintf((char *)at_Message_response,"00000"); 
        break;
        
        
    case 3:// Ver
        Ql_sprintf((char *)buffer,"AT+QMGDA=");
        buffer[9]='"';
        buffer[10]='D';
		buffer[11]='E';
        buffer[12]='L';
		buffer[13]=' ';
        buffer[14]='A';
		buffer[15]='L';
        buffer[16]='L';
		buffer[17]='"';
        buffer[18]='\r';
        buffer[19]='\n';
        Ql_sprintf((char *)at_Message_response,"OK\r\n"); 
		MESSAGE_DELETE_COMMAND=TRUE;
        break;
        
   
     
      
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
/*********************************************END************************************************/

/****************************Fota Function Area Start********************************************/
void SendAtCmd()
{
  bool exec = 1;
  u8 i=0;
  Ql_memset(buffer,0,100);
  switch (idxCmd)
  {
    
    //1. Get...
    case 1:// Ver
        Ql_sprintf((char *)buffer,"AT+QIFGCNT=0\r\n");
        Ql_sprintf((char *)at_response,"OK\r\n"); 
        break;
		    
    case 2:// Ver
        Ql_sprintf((char *)buffer,"AT+QICSGP=1");
        buffer[11]=',';
        buffer[12]='"';
        for(i=0;Apn_Setting_Buffer[i]!='\0';i++)
        buffer[i+13]=Apn_Setting_Buffer[i];
        buffer[i+13]='"';
        i++;
        buffer[i+13]='\r';
        i++;
        buffer[i+13]='\n';
        Ql_sprintf((char *)at_response,"OK\r\n"); 
        break;
        
        
    case 3:// Ver
        Ql_sprintf((char *)buffer,"AT+QFTPUSER");
        buffer[11]='=';
        buffer[12]='"';
        for(i=0;FTP_USER[i]!='\0';i++)
        buffer[i+13]=FTP_USER[i];
        buffer[i+13]='"';
        i++;
        buffer[i+13]='\r';
        i++;
        buffer[i+13]='\n';
        Ql_sprintf((char *)at_response,"OK\r\n"); 
        break;
        
    case 4:// Ver
        Ql_sprintf((char *)buffer,"AT+QFTPPASS");
        buffer[11]='=';
        buffer[12]='"';
        for(i=0;FTP_PASS[i]!='\0';i++)
        buffer[i+13]=FTP_PASS[i];
        buffer[i+13]='"';
        i++;
        buffer[i+13]='\r';
        i++;
        buffer[i+13]='\n';
        Ql_sprintf((char *)at_response,"OK\r\n"); 
        break;
        
     case 5:// Ver
        Ql_sprintf((char *)buffer,"AT+QFTPOPEN");
        buffer[11]='=';
        buffer[12]='"';
        for(i=0;FTP_ADDR[i]!='\0';i++)
        buffer[i+13]=FTP_ADDR[i];
        buffer[i+13]='"';
        i++;
        buffer[i+13]=',';
        i++;
        buffer[i+13]=FTP_PORT[0];
        i++;
        buffer[i+13]=FTP_PORT[1];
        i++;
        buffer[i+13]='\r';
        i++;
        buffer[i+13]='\n';
        Ql_sprintf((char *)at_response,"+QFTPOPEN:0\r\n"); 
        break;
      
      case 6:// Ver
        Ql_sprintf((char *)buffer,"AT+QFTPCFG");
        buffer[10]='=';
        buffer[11]='4';
        buffer[12]=',';
        buffer[13]='"';
        for(i=0;FTP_UFS_PATH[i]!='\0';i++)
        buffer[i+14]=FTP_UFS_PATH[i];
        buffer[i+14]='"';
        i++;
        buffer[i+14]='\r';
        i++;
        buffer[i+14]='\n';
        Ql_sprintf((char *)at_response,"+QFTPCFG:0\r\n"); 
        break;
        
       case 7:// Ver
        Ql_sprintf((char *)buffer,"AT+QFTPPATH");
        buffer[11]='=';
        buffer[12]='"';
        for(i=0;FTP_PATH[i]!='\0';i++)
        buffer[i+13]=FTP_PATH[i];
        buffer[i+13]='"';
        i++;
        buffer[i+13]='\r';
        i++;
        buffer[i+13]='\n';
        Ql_sprintf((char *)at_response,"+QFTPPATH:0\r\n"); 
        break;
        
        case 8:// Ver
        Ql_sprintf((char *)buffer,"AT+QFTPGET");
        buffer[10]='=';
        buffer[11]='"';
        for(i=0;FTP_FILE[i]!='\0';i++)
        buffer[i+12]=FTP_FILE[i];
        buffer[i+12]='"';
        i++;
        buffer[i+12]='\r';
        i++;
        buffer[i+12]='\n';
        Ql_sprintf((char *)at_response,"+QFTPGET:\r\n"); 
        break;
        //+QFTPOPEN:0
        //+QFTPCFG:0
        //+QFTPPATH:0
   /* case 2:// UFS
        Ql_sprintf((char *)buffer,"AT+QFLDS=\"UFS\"\r\n");
        break;*/
    /*    case 1:// ATI
        Ql_sprintf((char *)buffer,"ATI\r\n");
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
      Ql_SendToModem(ql_md_port1,(u8*)buffer, Ql_strlen(buffer));
  }
}

/****************************Fota Function Area End  ********************************************/


/***************************GPIO PIN AREA START********************************************************/
void GPIOPin_Config(void)
{
          
   
    QlPinParameter pinparameter;      
          
          
          
         				Ql_memset(buffer,0,100);
         				
         				
         				//Test Pin
         				iret = Ql_pinSubscribe(QL_PINNAME_LIGHT_MOS, QL_PINMODE_1, NULL);    
                    	Ql_sprintf(buffer, "Ql_pinSubscribe(pin=%d,mod=%d)=%d\r\n",QL_PINNAME_LIGHT_MOS, QL_PINMODE_1, iret);
                    	Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer));  
                    
          

						//GPRS LED PIN
						pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_OUT;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_LOW;
                        iret = Ql_pinSubscribe( QL_PINNAME_KBR4, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_KBR4,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 

						//FIX LED PIN
						pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_OUT;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_LOW;
                        iret = Ql_pinSubscribe(QL_PINNAME_M10_DCD, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_M10_DCD,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
                        
                        
                         

						// GSM ON-OFF PIN
						pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_OUT;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_LOW;
                        iret = Ql_pinSubscribe(QL_PINNAME_KBC3, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_KBC3,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 

						
                        
                           
                        
                        


						// INPUT    IP1
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_IN;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_HIGH;
                        iret = Ql_pinSubscribe( QL_PINNAME_RI, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret, QL_PINNAME_RI,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_IN,QL_PINLEVEL_HIGH);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 
                        
                      
                        //PANIC KEY ON INTURRUPT
                        /*pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.eintparameter.eintsensitivetype = QL_EINTSENSITIVETYPE_LEVEL;
                        pinparameter.pinparameterunion.eintparameter.hardware_de_bounce = 10; //10 ms
                        pinparameter.pinparameterunion.eintparameter.software_de_bounce = 0; // unit is ms , max is 2559
                        iret = Ql_pinSubscribe(QL_PINNAME_RI, QL_PINMODE_3, &pinparameter); 
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d\r\n",iret,QL_PINNAME_RI,QL_PINMODE_3);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));*/  
						
						
						
                        
                        //IP2	
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_IN;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_HIGH;
                        iret = Ql_pinSubscribe( QL_PINNAME_M10_DISP_RST, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_M10_DISP_RST ,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_IN,QL_PINLEVEL_HIGH);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
                        
                        
                        //IP4  
						pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_IN;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_HIGH;
                        iret = Ql_pinSubscribe(QL_PINNAME_M10_DISP_CS, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_M10_DISP_CS,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_HIGH);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 
						
						 
						

						//INPUT	---Power Detection
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_IN;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_HIGH;
                        iret = Ql_pinSubscribe(QL_PINNAME_DSR, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_DSR,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_IN,QL_PINLEVEL_HIGH);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 

						//INPUT ---	 PANIC  IP3   
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_IN;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_HIGH;
                        iret = Ql_pinSubscribe(QL_PINNAME_M10_DISP_DC , QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_M10_DISP_DC ,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_IN,QL_PINLEVEL_HIGH);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
						

						//OUTPUT  ---FUEL IMMOBILIZER
						
																																					   
						pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_DISABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_OUT;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_LOW;
                        iret = Ql_pinSubscribe( QL_PINNAME_M10_GPIO0, QL_PINMODE_1, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_M10_GPIO0,QL_PINMODE_1,QL_PINPULLENABLE_DISABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                       

						//OUTPUT  ---IGNITION IMMOBILIZER
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_OUT;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_LOW;
                        iret = Ql_pinSubscribe(QL_PINNAME_M10_GPIO1 , QL_PINMODE_1, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret, QL_PINNAME_M10_GPIO1 ,QL_PINMODE_1,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));


						//OUTPUT  ---DRIVER PANIC
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_OUT;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_LOW;
                        iret = Ql_pinSubscribe(QL_PINNAME_KBC4 , QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_KBC4 ,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));

						iret = Ql_pinWrite(QL_PINNAME_KBC4, QL_PINLEVEL_LOW);
                		Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret, QL_PINNAME_KBC4 );
                		Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));



						//OUTPUT---LDO POWER
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_OUT;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_HIGH;
                        iret = Ql_pinSubscribe( QL_PINNAME_M10_DISP_CLK, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret, QL_PINNAME_M10_DISP_CLK,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_HIGH);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));

						//OUTPUT---POWER PULSE
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_DISABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_OUT;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_LOW;
                        iret = Ql_pinSubscribe( QL_PINNAME_M10_DISP_DATA, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret, QL_PINNAME_M10_DISP_DATA,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_LOW);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));


					    //Buzzer control
                        pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.alertparameter.alertclock = QL_ALERTCLOCK_13DIV8M;
                        pinparameter.pinparameterunion.alertparameter.alertmode = QL_ALERTMODE_2;
                        pinparameter.pinparameterunion.alertparameter.alertcounter1 = 50;
                        pinparameter.pinparameterunion.alertparameter.alertcounter2 = 600;
                        pinparameter.pinparameterunion.alertparameter.alertthreshold = 20;
                        iret = Ql_pinSubscribe(QL_PINNAME_BUZZER, QL_PINMODE_3, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d\r\n",iret,QL_PINNAME_BUZZER,QL_PINMODE_3);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer)); 
                        
                        
                        //OUTPUT---LDO RESET
					   //	This is for Quctel and U-blox GPS
                        /*pinparameter.pinconfigversion = QL_PIN_VERSION;
                        pinparameter.pinparameterunion.gpioparameter.pinpullenable = QL_PINPULLENABLE_ENABLE;
                        pinparameter.pinparameterunion.gpioparameter.pindirection = QL_PINDIRECTION_OUT;
                        pinparameter.pinparameterunion.gpioparameter.pinlevel = QL_PINLEVEL_HIGH;
                        iret = Ql_pinSubscribe( QL_PINNAME_KBC3, QL_PINMODE_2, &pinparameter);    
                        Ql_sprintf(buffer, "\r\nSubscribe(%d),pin=%d,mod=%d,pul=%d,dir=%d,lev=%d\r\n",iret,QL_PINNAME_KBC3,QL_PINMODE_2,QL_PINPULLENABLE_ENABLE,QL_PINDIRECTION_OUT,QL_PINLEVEL_HIGH);
                        Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));	*/
						
						
					 
                        
 }

 void GPIO_ReadFunction(void)
 {
 					//s32 pin;
                    //s32 mod = 0;
                    //s32 pinpullenable = 0;
                    //s32 pindirection = 0;
                    
                    QlPinLevel pinlevel;
                    s32 iret;
					u8 IO_Temp;
					
					

			
 

					//Power Pin Status
					iret =Ql_pinRead(QL_PINNAME_DSR,&pinlevel);
					Power_Current_State=pinlevel;
					if(Power_Current_State==0)
					IO_STATUS&=0xdf;
					else
					IO_STATUS|=0x20;
					if(Power_Previous_State!=Power_Current_State)
					{
					Power_Previous_State=Power_Current_State;
					
					Ql_sprintf(buffer, "\r\n---POWER PIN STATUS CHANGE---(%d),pin=%d,level=%d\r\n",iret,QL_PINNAME_DSR,pinlevel);
                    Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
					IO_Status_Change=1;
				
					}

					//IP1 ----A/C Input status
					iret =Ql_pinRead(QL_PINNAME_RI,&pinlevel);
					AC_Current_State=pinlevel;
					if(AC_Current_State==0)
					IO_STATUS&=0xfe;
					else
					IO_STATUS|=0x01;
					if(AC_Previous_State!=AC_Current_State)
					{
					
					AC_Previous_State=AC_Current_State;
					Ql_sprintf(buffer, "\r\n---AC PIN STATUS CHANGE---(%d),pin=%d,level=%d\r\n",iret,QL_PINNAME_RI,pinlevel);
                    Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
					IO_Status_Change=1;
					}

					//IP2 ---Ignition Input Status
					iret =Ql_pinRead(QL_PINNAME_M10_DISP_RST,&pinlevel);
					Ignition_Current_State=pinlevel;
					if(Ignition_Current_State==0)
					IO_STATUS&=0xfd;
					else
					IO_STATUS|=0x02;
					if(Ignition_Previous_State!=Ignition_Current_State)
					{
					
					Ignition_Previous_State=Ignition_Current_State;
					Ql_sprintf(buffer, "\r\n---IGNITION PIN STATUS CHANGE---(%d),pin=%d,level=%d\r\n",iret,	QL_PINNAME_M10_DISP_RST,pinlevel);
                    Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
					IO_Status_Change=1;
					}
					
					
					//IP4 ---Door Input Status
					iret =Ql_pinRead(QL_PINNAME_M10_DISP_CS,&pinlevel);
					Door_Current_State=pinlevel;
					if(Door_Current_State==0)
					IO_STATUS&=0xfb;
					else
					IO_STATUS|=0x04;
					if(Door_Previous_State!=Door_Current_State)
					{
					
					Door_Previous_State=Door_Current_State;
					Ql_sprintf(buffer, "\r\n---DOOR PIN STATUS CHANGE---(%d),pin=%d,level=%d\r\n",iret,QL_PINNAME_M10_DISP_CS,pinlevel);
                    Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
					IO_Status_Change=1;
					}
					
					
					

					//IP3 PANIC Input status
					iret =Ql_pinRead(QL_PINNAME_M10_DISP_DC,&pinlevel);
					IP3_Current_State=pinlevel;
					if(IP3_Previous_State!=IP3_Current_State)
					{
					IP3_Previous_State=IP3_Current_State;
					Ql_sprintf(buffer, "\r\n---PANIC PIN STATUS CHANGE---(%d),pin=%d,level=%d\r\n",iret,QL_PINNAME_M10_DISP_DC,pinlevel);
                    Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
					iret = Ql_pinWrite( QL_PINNAME_KBC4, QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret,QL_PINNAME_KBC4);
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
					if(IP3_Current_State==0) 
					SETFLAG(PANIC_PRESS_DETECTED);
					}


				    if(IO_Status_Change==1)
					{
					IO_Temp=(IO_STATUS & 0xff);
					IO_Temp=((IO_Temp & 0xF0)>>4);
					if(IO_Temp>=0 && IO_Temp <=9)
					{
					 	hex_input[0]=(0x30 | IO_Temp);
					}
					else
					{
						hex_input[0]=(0x37 + IO_Temp);
					}

					IO_Temp=(IO_STATUS & 0x0F);
					if(IO_Temp>=0 && IO_Temp <=9)
					{
					 	hex_input[1]=(0x30 | IO_Temp);
					}
					else
					{
						hex_input[1]=(0x37 + IO_Temp);
					}
					io_send();
					IO_Status_Change=0;
					IO_STATUS=0;
					}
					else
					{
					 hex_input[0]='F';
					 hex_input[1]='F';
					 IO_Status_Change=0;
					    if(CHECKFLAG(PANIC_PRESS_DETECTED))
					 	{
						Not_Parameter_Update=FALSE;
	  					iosend_function();
					 	}
					}

					
 }

                        
/***************************GPIO PIN AREA END  ********************************************************/

void Power_Supply_Pulse(void)
{
			        u16 i,j;

				
				    
					
					 
					iret = Ql_pinWrite( QL_PINNAME_M10_DISP_DATA , QL_PINLEVEL_HIGH);
                	Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret,QL_PINNAME_M10_DISP_DATA );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));

					for(i=0;i<2000;i++)
					for(j=0;j<3000;j++);

					iret = Ql_pinWrite( QL_PINNAME_M10_DISP_DATA , QL_PINLEVEL_LOW);
                	Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret,QL_PINNAME_M10_DISP_DATA );
                	Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
}

/*******************************UART1 DATAPROCESS AREA START******************************************/
 bool uart1_datahandler(u16 datlength,char * data)
{  
	int i,j,k,m;
	s32 U1ret3,U1ret1,U1ret2;
	static bool send=FALSE;
	ascii sersettings[150],timerval[6],bufff0='0',smsval[6], port[4],t_val[3],showcard[150],fset[2],buff[6];
	
	ascii *send1;
	
	static ascii buff0='0',buff1='1';
	
	for(i=0;i<datlength;i++) 
	{ 
		
		
		uart1_buff[uart1_ptr1] = data[i];
		if(uart1_ptr1 >= 1000)
		{
			uart1_ptr1 = 0;
		}
		if(uart1_buff[uart1_ptr1]==0x03)
		{
			send=TRUE;
			uart1_ptr1=0;
		}
		else
		{
			
			uart1_ptr1++;
			u_cnt++;
		}
	} 
	
	
	
	
	if(send==TRUE)
	{
		
		for ( m = 0 ; m <= u_cnt ; m++ )
		{    
			
			
			
			if(uart1_buff[m]==0x02)
			{
				for(j=0;j<(u_cnt);j++)
				{
					showcard[j]= uart1_buff[m+j+1];
				}
			}
		}
		////--------------------------------------------
		
		if((showcard[0]=='P')||(showcard[0]=='0')||(showcard[0]=='1'))
		{
			prox_data[0]='A';
			prox_data[1]='F';
			prox_data[2]=finalF [0];
			prox_data[3]=finalF [1];
			prox_data[4]=finalF [2];
			prox_data[5]=finalF [3];
			prox_data[6]=finalF [4];
			prox_data[7]=finalF [5];
			prox_data[8]=finalF [6];
			prox_data[9]=finalF [7];
			prox_data[10]=finalF [8];
			prox_data[11]=finalF [9];
			prox_data[12]=finalF [10];
			prox_data[13]=finalF [11];
			prox_data[14]=finalF [12];
			prox_data[15]=finalF [13];
			prox_data[16]=finalF [14];
			prox_data[17]=finalF [15];
			prox_data[18]=finalF [16];
			prox_data[19]=finalF [17];
			prox_data[20]=finalF [18];
			prox_data[21]=finalF [19];
			prox_data[22]=finalF [20];
			prox_data[23]=finalF [21];
			prox_data[24]=finalF [22];
			prox_data[25]=finalF [23];
			prox_data[26]=finalF [24];
			prox_data[27]=finalF [25];
			prox_data[28]=finalF [26];
			prox_data[29]=finalF [27];
			prox_data[30]=finalF [28];
			prox_data[31]=finalF [29];
			prox_data[32]=finalF [30];
			prox_data[33]=finalF [31];
			prox_data[34]=finalF [32];
			prox_data[35]=finalF [33];
			prox_data[36]=finalF [34];
			prox_data[37]=finalF [35];
			prox_data[38]=finalF [36];
			prox_data[39]=finalF [37];
			prox_data[40]=finalF [38];
			prox_data[41]=finalF [39];
			prox_data[42]=finalF [40];
			prox_data[43]=finalF [41];
			prox_data[44]=finalF [42];
			prox_data[45]=finalF [43];
			prox_data[46]=finalF [44];
			prox_data[47]=finalF [45];
			prox_data[48]=finalF [46];
			prox_data[49]=finalF [47];
			prox_data[50]=finalF [48];
			prox_data[51]=finalF [49];
			prox_data[52]=finalF [50];
			if(showcard[0]=='P')
			{
				if(u_cnt==16)  /// 54 bit HID
				{
					
					prox_data[53]='K';
					prox_data[54]='1';
					prox_data[55]='2';
					prox_data[56]='3';
					prox_data[57]='4';
					prox_data[58]='A';
					prox_data[59]='1';
					
					prox_data[60]=showcard[1];
					prox_data[61]=showcard[2];
					prox_data[62]=showcard[3];
					prox_data[63]=showcard[4];
					prox_data[64]=showcard[5];
					prox_data[65]=showcard[6];
					prox_data[66]=showcard[7];
					prox_data[67]=showcard[8];
					prox_data[68]=showcard[9];
					prox_data[69]=showcard[10];
					prox_data[70]=showcard[11];
					prox_data[71]=showcard[12];
					prox_data[72]=showcard[13];
					
					if(showcard[14]=='I')
					{
						prox_data[73]='I';
					}
					else if(showcard[14]=='O')
					{
						prox_data[73]='O';   
					}
					else
					{
						prox_data[73]='O';   
					}
					
					prox_data[74]='I';
					prox_data[75]='D';
					
					prox_data[76]='*';
					prox_data[77]='#';
					
					
					
					/*U1ret1=adl_flhWrite ( fhandle, ID[key_wr],78, prox_data);
					if(U1ret1==OK)
					{
						
						key_wr++;
						if(key_wr>=1968)
						{
							key_wr=1901;
						}
						else
						{
							wm_itoa(key_wr,key_ptr);
							adl_flhWrite ( fhandle, ID[1992],wm_strlen(key_ptr), key_ptr);
						}
						
						
						
						
						
						
					}
					adl_fcmSendData(uart1_handler,"%R%",3);*/
					u_cnt=0;///chANGE
					send=FALSE;
					
				}
				
				else if((u_cnt==10)||(u_cnt==9))  ///24/26 bit HID
				{
					
					prox_data[53]='K';
					prox_data[54]='1';
					prox_data[55]='2';
					prox_data[56]='3';
					prox_data[57]='4';
					prox_data[58]='A';
					prox_data[59]='1';
					
					prox_data[60]=showcard[1];//26 bit
					prox_data[61]=showcard[2];
					prox_data[62]=showcard[3];
					prox_data[63]=showcard[4];
					prox_data[64]=showcard[5];
					prox_data[65]=showcard[6];
					if(showcard[7]=='I')
					{
						prox_data[66]='I';
					}
					else if(showcard[7]=='O')
					{
						prox_data[66]='O';   
					}
					else
					{
						prox_data[66]='O';   
					}
					
					prox_data[67]='I';
					prox_data[68]='D';
					
					prox_data[69]='*';
					prox_data[70]='#';
					
					
					
					/*U1ret1=adl_flhWrite ( fhandle, ID[key_wr],71, prox_data);
					if(U1ret1==OK)
					{
						
						key_wr++;
						if(key_wr>=1968)
						{
							key_wr=1901;
						}
						else
						{
							wm_itoa(key_wr,key_ptr);
							adl_flhWrite ( fhandle, ID[1992],wm_strlen(key_ptr), key_ptr);
						}
						
						
						
						
						
						
					}
					adl_fcmSendData(uart1_handler,"%R%",3);*/
					u_cnt=0;///chANGE
					send=FALSE;
					
				}
				
				
				
				else if(u_cnt==12)  ///34/35  bit HID
				{
					prox_data[53]='K';
					prox_data[54]='1';
					prox_data[55]='2';
					prox_data[56]='3';
					prox_data[57]='4';
					prox_data[58]='A';
					prox_data[59]='1';
					
					prox_data[60]=showcard[1];
					prox_data[61]=showcard[2];
					prox_data[62]=showcard[3];
					prox_data[63]=showcard[4];
					prox_data[64]=showcard[5];
					prox_data[65]=showcard[6];
					prox_data[66]=showcard[7];
					prox_data[67]=showcard[8];
					prox_data[68]=showcard[9];
					
					if(showcard[10]=='I')
					{
						prox_data[69]='I';
					}
					else if(showcard[10]=='O')
					{
						prox_data[69]='O';   
					}
					else
					{
						prox_data[69]='O';   
					}
					
					prox_data[70]='I';
					prox_data[71]='D';
					prox_data[72]='*';
					prox_data[73]='#';
					
					/*U1ret1=adl_flhWrite ( fhandle, ID[key_wr],74, prox_data);
					if(U1ret1==OK)
					{
						
						key_wr++;
						if(key_wr>=1968)
						{
							key_wr=1901;
						}
						else
						{
							wm_itoa(key_wr,key_ptr);
							adl_flhWrite ( fhandle, ID[1992],wm_strlen(key_ptr), key_ptr);
						}
						
						
						
						
						
						
					}
					adl_fcmSendData(uart1_handler,"%R%",3);*/
					u_cnt=0;///chANGE
					send=FALSE;
					
				}
				
				else
					
				{
					u_cnt=0;///chANGE
					send=FALSE;
					
				}
				
				
				
}
else if((showcard[0]=='1')||(showcard[0]=='0'))  // NI Reader
{
	
	prox_data[53]='K';  
	prox_data[54]='1';
	prox_data[55]='2';
	prox_data[56]='3';
	prox_data[57]='4';
	prox_data[58]='A';
	prox_data[59]='2';
	
	prox_data[60]=showcard[0];
	prox_data[61]=showcard[1];
	prox_data[62]=showcard[2];
	prox_data[63]=showcard[3];
	prox_data[64]=showcard[4];
	prox_data[65]=showcard[5];
	prox_data[66]=showcard[6];   
	prox_data[67]=showcard[7];
	prox_data[68]=showcard[8];
	prox_data[69]=showcard[9];
	prox_data[70]=showcard[10];
	prox_data[71]='*';
	prox_data[72]='#';
	
	/*U1ret1=adl_flhWrite ( fhandle, ID[key_wr],73, prox_data);
		  if(U1ret1==OK)
		  {
			  
			  key_wr++;
			  if(key_wr>=1968)
			  {
				  key_wr=1901;
			  }
			  else
			  {
				  wm_itoa(key_wr,key_ptr);
				  adl_flhWrite ( fhandle, ID[1992],wm_strlen(key_ptr), key_ptr);
			  }
		  }
		  adl_fcmSendData(uart1_handler,"%R%",3);*/
		  u_cnt=0;///chANGE
		  send=FALSE;
		  
		  
}

}
}

return TRUE;

return TRUE;
}








/*******************************UART1 DATAPROCESS AREA END  ******************************************/

void PrepareString(u8 *pt1)
{
	u8 i1,j1;

	
	StringtoSendGprs[0]='!';
	StringtoSendGprs[1]='*';
	StringtoSendGprs[2]='0';
	StringtoSendGprs[3]='1';
	StringtoSendGprs[4]='0';
	StringtoSendGprs[5]=owen_number[0];
	StringtoSendGprs[6]=owen_number[1];
	StringtoSendGprs[7]=owen_number[2];
	StringtoSendGprs[8]=owen_number[3];
	StringtoSendGprs[9]=owen_number[4];
	StringtoSendGprs[10]=owen_number[5];
	StringtoSendGprs[11]=owen_number[6];
	StringtoSendGprs[12]=owen_number[7];
	StringtoSendGprs[13]=owen_number[8];
	StringtoSendGprs[14]=owen_number[9];
	StringtoSendGprs[15]=',';
/*	StringtoSendGprs[16]='0';
	StringtoSendGprs[17]='0';
	StringtoSendGprs[18]='0';
	StringtoSendGprs[19]='0';
	StringtoSendGprs[20]='0';
	StringtoSendGprs[21]='0';
	StringtoSendGprs[22]='#';
	StringtoSendGprs[23]='*';
	StringtoSendGprs[24]='3';
	StringtoSendGprs[25]='5';*/
	

	
	for(i1=16;i1<79;i1++)
	{
	StringtoSendGprs[i1]= *pt1;
	pt1++;
	}
	StringtoSendGprs[i1]='\0';
	
	
	for(j1=0;StringtoSendGprs[j1]!='\0';j1++)
    Socket_Buffer_API[j1]=StringtoSendGprs[j1];
    Socket_Buffer_Length=j1;
    
    Socket_Buffer_API[j1]='\0';
    
    Ql_sprintf(textBuf,"Wave Point String=%s\r\n",Socket_Buffer_API);
    Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); 
                         
	
    
    
}

void AdcPrepareString(u8 *pt1)
{
	u8 i1,j1;

	
	StringtoSendGprs[0]='!';
	StringtoSendGprs[1]='*';
	StringtoSendGprs[2]='0';
	StringtoSendGprs[3]='1';
	StringtoSendGprs[4]='0';
	StringtoSendGprs[5]=owen_number[0];
	StringtoSendGprs[6]=owen_number[1];
	StringtoSendGprs[7]=owen_number[2];
	StringtoSendGprs[8]=owen_number[3];
	StringtoSendGprs[9]=owen_number[4];
	StringtoSendGprs[10]=owen_number[5];
	StringtoSendGprs[11]=owen_number[6];
	StringtoSendGprs[12]=owen_number[7];
	StringtoSendGprs[13]=owen_number[8];
	StringtoSendGprs[14]=owen_number[9];
	StringtoSendGprs[15]=',';
/*	StringtoSendGprs[16]='0';
	StringtoSendGprs[17]='0';
	StringtoSendGprs[18]='0';
	StringtoSendGprs[19]='0';
	StringtoSendGprs[20]='0';
	StringtoSendGprs[21]='0';
	StringtoSendGprs[22]='#';
	StringtoSendGprs[23]='*';
	StringtoSendGprs[24]='3';
	StringtoSendGprs[25]='5';*/
	

	
	for(i1=16;i1<60;i1++)
	{
	StringtoSendGprs[i1]= *pt1;
	pt1++;
	}
	StringtoSendGprs[i1]='*';
	i1++;
	StringtoSendGprs[i1]='#';
	i1++;

	StringtoSendGprs[i1]='\0';
	
	
	for(j1=0;StringtoSendGprs[j1]!='\0';j1++)
    Socket_Buffer_API[j1]=StringtoSendGprs[j1];
    Socket_Buffer_Length=j1;
    
    Socket_Buffer_API[j1]='\0';
    
    Ql_sprintf(textBuf,"Adc Point String=%s\r\n",Socket_Buffer_API);
    Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); 
                         
	
    
    
}



void Send_GSMVersionString(void)
{
	u8 i1,j1;
	


	  Socket_Buffer_API[0]='!';
	  Socket_Buffer_API[1]='*';
	  Socket_Buffer_API[2]='0';
	  Socket_Buffer_API[3]='1';
	  Socket_Buffer_API[4]='0';
	  Socket_Buffer_API[5]=owen_number[0];
	  Socket_Buffer_API[6]=owen_number[1];
	  Socket_Buffer_API[7]=owen_number[2];
	  Socket_Buffer_API[8]=owen_number[3];
	  Socket_Buffer_API[9]=owen_number[4];
	 Socket_Buffer_API[10]=owen_number[5];
	 Socket_Buffer_API[11]=owen_number[6];
	 Socket_Buffer_API[12]=owen_number[7];
	 Socket_Buffer_API[13]=owen_number[8];
	 Socket_Buffer_API[14]=owen_number[9];
	 Socket_Buffer_API[15]=',';
	for(j1=0;versionstring[j1]!='\0';j1++)
    Socket_Buffer_API[j1+16]=versionstring[j1];
    Socket_Buffer_Length=(j1+16);
    
    Socket_Buffer_API[j1+16]='\0';
    
    Ql_sprintf(textBuf,"GSM FIRMWARE VERSION STRING=%s\r\n",Socket_Buffer_API);
    Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); 
    Socket_DataSend_Function();
    
}

void OpenSetting_File(void)
{
	                
	                char *p4,*p5;
	                
	               
	                
	                
	                
	                p4="Ql_FileOpen=csettings.txt";
    				p5 = Ql_strstr(p4,"Ql_FileOpen=c");
    				if (p5)
    				{
      				 
      				 s32 ret;
      				p5 = Ql_strstr(p4,"=c");
      				p5 += 2;
      				ret = Ql_FileOpen((u8*)p5 ,FALSE, FALSE);
      				if(ret >= QL_RET_OK)
      				filehandle2 = ret;
      				Ql_sprintf(textBuf,"Ql_FileOpen (%s)=%d\r\n", p5, ret);
      				Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
      				}
      				
	                
	               
}
void openfile1(void)
{
	                
	                char *p4,*p5;
	                
	               
	                
	                
	                
	                p4="Ql_FileOpen=cFile1.txt";
    				p5 = Ql_strstr(p4,"Ql_FileOpen=c");
    				if (p5)
    				{
      				 
      				 s32 ret;
      				p5 = Ql_strstr(p4,"=c");
      				p5 += 2;
      				ret = Ql_FileOpen((u8*)p5 ,FALSE, FALSE);
      				if(ret >= QL_RET_OK)
      				filehandle = ret;
      				Ql_sprintf(textBuf,"Ql_FileOpen (%s)=%d\r\n", p5, ret);
      				Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
      				}
      				
	                
	               
}

void openfile2(void)
{
	              char *p6,*p7;
	 
	 				p6="Ql_FileOpen=cconfig.txt";
    				p7 = Ql_strstr(p6,"Ql_FileOpen=c");
    				if (p7)
    				{
      				 
      				 s32 ret;
      				p7 = Ql_strstr(p6,"=c");
      				p7 += 2;
      				ret = Ql_FileOpen((u8*)p7 ,FALSE, FALSE);
      				if(ret >= QL_RET_OK)
      				filehandle1 = ret;
      				Ql_sprintf(textBuf,"Ql_FileOpen (%s)=%d\r\n", p7, ret);
      				Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
      				}
      				
}


void Socket_DataSend_Function(void)
{

	 	    u32 ret;
	 	    if(socketonly[1]  == 0xFF)
                    {
                    //   OUT_DEBUG(textBuf,(u8*)"socket not create\r\n");
                        return;
                    }

                    if(send_dataLen[1]  > 0)
                    {
                   //     OUT_DEBUG(textBuf,(u8*)"socket now busy, sending\r\n");
                        return;
                    }
                    
                    
                    
                  
                    
                    send_dataLen[1]  = 0;
                    dataLen[1]  =Socket_Buffer_Length;
                    do
                    {
                        ret = Ql_SocketSend(socketonly[1] , Socket_Buffer_API  ,Socket_Buffer_Length );
                      //  OUT_DEBUG(textBuf,(u8*)"Ql_SocketSend(socket=%d,dataLen=%d)=%d\r\n",socketonly[1] ,dataLen[1] ,ret);
                        //i=0;
                        //while(array[i]!='\0')
   						//{
						//data_p[i]=array[i];
						//i++;
						//}
                    
                        if(ret == (dataLen[1]  - send_dataLen[1] ))
                        {
                            //send compelete
                            send_dataLen[1]  = 0;
                            break;
                        }
                        else if((ret < 0) && (ret == -2)) 
                        {
                            //you must wait CallBack_socket_write, then send data;                                                    
                            break;
                        }
                        else if(ret < 0)
                        {
                            //error , Ql_SocketClose
                            Ql_SocketClose(socketonly[1]);
                            socketonly[1] = 0xFF;
                            send_dataLen[1]  = 0;
                            break;
                        }
                        else if(ret <= dataLen[1] )
                        {
                            send_dataLen[1]  += ret;
                            //continue send
                        }
                    }while(1);
                    
                    //timer for 30 seconds
                    if(Not_Parameter_Update==TRUE)
                    {
                    timer1min.timeoutPeriod=Ql_SecondToTicks(Update_Timer_Variable);
                    Ql_StartTimer(&timer1min);
                    }
}


void FileSyatem_ReadFunction(void)
{


	char *p2;
	char *pt;
	s8 ret = -1;

	    
                   openfile1();
                   openfile2();
                   
                    
                    Ql_memset(textBuf,0,63); 
                    if (1)
                    {
                        s32 ret;
                        u32 readedlen;
                        u8 i;
                        Ql_memset(strBuf,0,63);
                        Ql_memset(textBuf,0,63);
                        Ql_FileSeek(filehandle1,10, QL_FS_FILE_BEGIN);
                        ret = Ql_FileRead(filehandle1, strBuf,10,&readedlen);
                       // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)"ReadPointer=",Ql_strlen("ReadPointer="));
                        Ql_sprintf(textBuf,"%s",strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));  
                       // Ql_SendToUart(ql_uart_port1,strBuf,Ql_strlen( strBuf));
                       
                        //Ql_memset(strBuf,0,53);
                        //Ql_memset(textBuf,0,53);
                        Read_Position=0;
						Read_nextposition=0;
                        i=0;
                        while(textBuf[i]!='E')
                        {
                        Read_Position++;
                        i++;
                        }
                       if(Read_Position>0 && Read_Position<=2)
                       Read_nextposition=(((textBuf[0]-0x30)*10)+(textBuf[1]-0x30)); 
                       else if(Read_Position>2 && Read_Position<=3)
                       Read_nextposition=(((textBuf[0]-0x30)*100)+((textBuf[1]-0x30)*10)+(textBuf[2]-0x30)); 
                       else if(Read_Position>3 && Read_Position<=4)
                       Read_nextposition=(((textBuf[0]-0x30)*1000)+((textBuf[1]-0x30)*100)+((textBuf[2]-0x30)*10)+(textBuf[3]-0x30));
                       else if(Read_Position>4 && Read_Position<=5)
                       Read_nextposition=(((textBuf[0]-0x30)*10000)+((textBuf[1]-0x30)*1000)+((textBuf[2]-0x30)*100)+((textBuf[3]-0x30)*10)+(textBuf[4]-0x30));      
                       else if(Read_Position>5 && Read_Position<=6)
                       Read_nextposition=(((textBuf[0]-0x30)*100000)+((textBuf[1]-0x30)*10000)+((textBuf[2]-0x30)*1000)+((textBuf[3]-0x30)*100)+((textBuf[4]-0x30)*10)+(textBuf[5]-0x30));
                       else if(Read_Position>6 && Read_Position<=7)
                       Read_nextposition=(((textBuf[0]-0x30)*1000000)+((textBuf[1]-0x30)*100000)+((textBuf[2]-0x30)*10000)+((textBuf[3]-0x30)*1000)+((textBuf[4]-0x30)*100)+((textBuf[5]-0x30)*10)+(textBuf[6]-0x30));
                       
                       else if(Read_Position>7 && Read_Position<=8)
                       Read_nextposition=(((textBuf[0]-0x30)*10000000)+((textBuf[1]-0x30)*1000000)+((textBuf[2]-0x30)*100000)+((textBuf[3]-0x30)*10000)+((textBuf[4]-0x30)*1000)+((textBuf[5]-0x30)*100)+((textBuf[6]-0x30)*10)+(textBuf[7]-0x30));
                       
                       else if(Read_Position>8 && Read_Position<=9)
                       Read_nextposition=(((textBuf[0]-0x30)*100000000)+((textBuf[1]-0x30)*10000000)+((textBuf[2]-0x30)*1000000)+((textBuf[3]-0x30)*100000)+((textBuf[4]-0x30)*10000)+((textBuf[5]-0x30)*1000)+((textBuf[6]-0x30)*100)+((textBuf[7]-0x30)*10)+(textBuf[8]-0x30));
                       
                       else if(Read_Position>9 && Read_Position<=10)
                       Read_nextposition=(((textBuf[0]-0x30)*1000000000)+((textBuf[1]-0x30)*100000000)+((textBuf[2]-0x30)*10000000)+((textBuf[3]-0x30)*1000000)+((textBuf[4]-0x30)*100000)+((textBuf[5]-0x30)*10000)+((textBuf[6]-0x30)*1000)+((textBuf[7]-0x30)*100)+((textBuf[8]-0x30)*10)+(textBuf[9]-0x30));
                       
                       ReadPositionStatus=Read_nextposition;
                             
                             
                             
                             
                        
                        
                    }
                    
                   if(GPRS_CONNECTION==TRUE)
                   {
                    if (1)
                    {
                        s32 ret;
                        //u32 writeedlen;
                        //extern int atoi(const char*);
                        //p = Ql_strstr(pData,"=");
                        //p += 1;
                        //Position-=53;
                        
                        Ql_memset(textBuf,0,100);
                        ret = Ql_FileSeek(filehandle, Read_nextposition , QL_FS_FILE_BEGIN);
                        Ql_sprintf(textBuf,"Ql_FileSeek()=%d: Read_Position=%d\r\n",ret,Read_nextposition);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                       
                    }

                    
                   // pt = Ql_strstr(p2,"Ql_FileRead");
                    if (1)
                    {
                        s32 ret;
                        u32 readedlen;
                        //u8 Glen=53;
                        u8 j=0;
                
                        Ql_memset(strBuf,0,63);
                        Ql_memset(textBuf,0,63);
                        ret = Ql_FileRead(filehandle, strBuf,63, &readedlen);
                        if(strBuf[8]=='A' && strBuf[9]=='B')
						{
						AdcPrepareString(strBuf);
						}
						else
						{
						PrepareString(strBuf);
						}
                        Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); 
                        
                        
                        
                        Ql_memset(textBuf,0,63);
                        Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d\r\n",ret, readedlen);
                      //  Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); 
                        
                        
                        
                      
                        
                    }
                    
                    
                    if (1)
                    {
                       s32 ret;
                       Ql_memset(textBuf,0,63);
                       ret = Ql_FileGetFilePosition(filehandle, &Position);
                        
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)"NewReadPosition=",Ql_strlen("NewReadPosition="));
                       
                       Ql_sprintf(textBuf,"%d", Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                           
                        Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
                    }

                    
                    pt=textBuf;
                    if (pt)
                    {
                        s32 ret1;
                        u32 writeedlen;
                        //pt = Ql_strstr(p2,"=");
                        //pt += 1;
                       // ret = Ql_FileGetFilePosition(filehandle, &Position);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       	Ql_FileSeek(filehandle1,10, QL_FS_FILE_BEGIN);
                        ret1 = Ql_FileWrite(filehandle1, (u8*)pt,Ql_strlen(textBuf), &writeedlen);
                        Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret1, writeedlen);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        globalpos=writeedlen;
                        
                       Ql_memset(textBuf,0,63); 
                        ret = Ql_FileGetFilePosition(filehandle1, &Read_Position);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)"Read_Position=",Ql_strlen("Read_Position="));
                       Ql_sprintf(textBuf,"%d", Read_Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); 
                       Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));    
                       
                       
                    }
                    Socket_DataSend_Function();

						if(WritePositionStatus>=(63*NO_OF_WAVE_POINT_STORED))
						{
							if(WritePositionStatus==ReadPositionStatus)
							{
							 
							  
							  Ql_FileClose(filehandle);
			     				filehandle = -1;
			      				Ql_memset(textBuf,0,63);
			      				Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
			      				Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
			                    
						
						       Ql_FileClose(filehandle1);
			     			   filehandle1 = -1;
			      			   Ql_memset(textBuf,0,63);
			      			   Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
			      			   Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
							  
							  ret = Ql_FileDelete((u8*)"File1.txt" );
	   						  Ql_sprintf(textBuf,"Ql_FileDelete(%s)=%d\r\n", "File1.txt", ret);
	   						  Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
							  
							  ret = Ql_FileDelete((u8*)"config.txt" );
	   						  Ql_sprintf(textBuf,"Ql_FileDelete(%s)=%d\r\n", "config.txt", ret);
	   						  Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); 
							  Create_WavePointFile();
							  return;
							}
						}
                    }

 
                    Ql_FileClose(filehandle);
     				filehandle = -1;
      				Ql_memset(textBuf,0,63);
      				Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      				Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                    
			
			       Ql_FileClose(filehandle1);
     			   filehandle1 = -1;
      			   Ql_memset(textBuf,0,63);
      			   Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      			   Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
      					
      					
                    
                   
                   

}

void AdcFunction(void)
{
	   finalF[0]='0';
	   finalF[1]='0';
	   finalF[2]='0';
	   finalF[3]='0';
	   finalF[4]='0';
	   finalF[5]='0';
	   finalF[6]='#';
	   finalF[7]='*';
	   finalF[8]='A';
	   finalF[9]='B';
	   finalF[10]='A';
	   finalF[11]='D';
	   finalF[12]='C';
	   finalF[13]=':';
	   finalF[14]='6';
	   finalF[15]='5';
	   finalF[16]='5';
	   finalF[17]='5';
	   finalF[18]='5';
	   finalF[19]=',';
	   finalF[20]='6';
	   finalF[21]='5';
	   finalF[22]='5';
	   finalF[23]='5';
	   finalF[24]='5';
	   finalF[25]=',';
	   finalF[26]=adc_value[3];
	   finalF[27]=adc_value[2];
	   finalF[28]=adc_value[1];
	   finalF[29]=adc_value[0];
	   finalF[30]=date[0];
	   finalF[31]=date[1];
	   finalF[32]=date[2];
	   finalF[33]=date[3];
	   finalF[34]=date[4];
	   finalF[35]=date[5];
	   finalF[36]=time[0];
	   finalF[37]=time[1];
	   finalF[38]=time[2];
	   finalF[39]=time[3];
	   finalF[40]=time[4];
	   finalF[41]=time[5];
	   finalF[42]='_';
	   finalF[43]='1';
	   finalF[44]='0';
	   finalF[45]='0';
	   finalF[46]='0';
	   finalF[47]='0';
	   finalF[48]='0';
	   finalF[49]='0';
	   finalF[50]='0';
	   finalF[51]='0';
	   finalF[52]='0';
	   finalF[53]='0';
	   finalF[54]='0';
	   finalF[55]='0';
	   finalF[56]='0';
	   finalF[57]='0';
	   finalF[58]='0';
	   finalF[59]='0';
	   finalF[60]='0';
	   finalF[61]='*';
	   finalF[62]='#';
	   adcsend_function();
}

void iosend_function(void)
{
	char *p2;
	char *pt;
	s8 ret = -1;
	u8 i,j;

	    
                   openfile1();
                   openfile2();

				   for(i=0;i<53;i++)
				   {
				   TempBuffer[i]=finalF[i];
				   }
				   finalF[0]='0';
				   finalF[1]='0';
				   finalF[2]='0';
				   finalF[3]='0';
				   finalF[4]='0';
				   finalF[5]='0';
				   finalF[6]='#';
				   finalF[7]='*';
				   if(!CHECKFLAG(PANIC_PRESS_DETECTED))
				   {
				   finalF[8]='3';
				   finalF[9]='5';
				   }
				   else
				   {
				   finalF[8]='A';
				   finalF[9]='2';
				   RESETFLAG(PANIC_PRESS_DETECTED);
				   }
	
				   for(i=0;i<53;i++)
				   {
					finalF[10+i]=TempBuffer[i];
				   }
                    pt=finalF;
                    
                   
                   /* Ql_memset(textBuf,0,53); 
                        ret = Ql_FileGetFilePosition(filehandle1, &Position1);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,"Position1=",Ql_strlen("Position1="));
                       Ql_sprintf(textBuf,"%d", Position1);
                       Ql_SendToUart(ql_uart_port1,textBuf,Ql_strlen(textBuf)); ; */   
                    
                     
                    /*  if (1)
                    {
                        s32 ret;
                        u32 readedlen;
                        u8 i;
                        Ql_memset(strBuf,0,53);
                        Ql_memset(textBuf,0,53);
                        Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
                        ret = Ql_FileRead(filehandle1, strBuf,10,&readedlen);
                       // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,"Writepointer=",Ql_strlen("Writepointer="));
                        Ql_sprintf(textBuf,"%s",strBuf);
                        Ql_SendToUart(ql_uart_port1,textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,"\r\n",Ql_strlen("\r\n")); 
                        Position1= NewWritePointerCount; 
                       // Ql_SendToUart(ql_uart_port1,strBuf,Ql_strlen( strBuf));
                       
                        //Ql_memset(strBuf,0,53);
                        //Ql_memset(textBuf,0,53);
                        Position1=0;
                        i=0;
                        while(textBuf[i]!='E')
                        {
                        	
                        Position1++;
                        i++;
                        }
                       if(Position1>0 && Position1<=2)
                       nextposition=(((textBuf[0]-0x30)*10)+(textBuf[1]-0x30)); 
                       else if(Position1>2 && Position1<=3)
                       nextposition=(((textBuf[0]-0x30)*100)+((textBuf[1]-0x30)*10)+(textBuf[2]-0x30)); 
                       else if(Position1>3 && Position1<=4)
                       nextposition=(((textBuf[0]-0x30)*1000)+((textBuf[1]-0x30)*100)+((textBuf[2]-0x30)*10)+(textBuf[3]-0x30));
                       else if(Position1>4 && Position1<=5)
                       nextposition=(((textBuf[0]-0x30)*10000)+((textBuf[1]-0x30)*1000)+((textBuf[2]-0x30)*100)+((textBuf[3]-0x30)*10)+(textBuf[4]-0x30));      
                       else if(Position1>5 && Position1<=6)
                       nextposition=(((textBuf[0]-0x30)*100000)+((textBuf[1]-0x30)*10000)+((textBuf[2]-0x30)*1000)+((textBuf[3]-0x30)*100)+((textBuf[4]-0x30)*10)+(textBuf[5]-0x30));
                       else if(Position1>6 && Position1<=7)
                       nextposition=(((textBuf[0]-0x30)*1000000)+((textBuf[1]-0x30)*100000)+((textBuf[2]-0x30)*10000)+((textBuf[3]-0x30)*1000)+((textBuf[4]-0x30)*100)+((textBuf[5]-0x30)*10)+(textBuf[6]-0x30));
                       
                       else if(Position1>7 && Position1<=8)
                       nextposition=(((textBuf[0]-0x30)*10000000)+((textBuf[1]-0x30)*1000000)+((textBuf[2]-0x30)*100000)+((textBuf[3]-0x30)*10000)+((textBuf[4]-0x30)*1000)+((textBuf[5]-0x30)*100)+((textBuf[6]-0x30)*10)+(textBuf[7]-0x30));
                       
                       else if(Position1>8 && Position1<=9)
                       nextposition=(((textBuf[0]-0x30)*100000000)+((textBuf[1]-0x30)*10000000)+((textBuf[2]-0x30)*1000000)+((textBuf[3]-0x30)*100000)+((textBuf[4]-0x30)*10000)+((textBuf[5]-0x30)*1000)+((textBuf[6]-0x30)*100)+((textBuf[7]-0x30)*10)+(textBuf[8]-0x30));
                       
                       else if(Position1>9 && Position1<=10)
                       nextposition=(((textBuf[0]-0x30)*1000000000)+((textBuf[1]-0x30)*100000000)+((textBuf[2]-0x30)*10000000)+((textBuf[3]-0x30)*1000000)+((textBuf[4]-0x30)*100000)+((textBuf[5]-0x30)*10000)+((textBuf[6]-0x30)*1000)+((textBuf[7]-0x30)*100)+((textBuf[8]-0x30)*10)+(textBuf[9]-0x30));
                       
                             
                             
                             
                             
                        
                        
                    }*/  
                     Ql_memset(textBuf,0,63); 
                    if (1)
                    {
                        s32 ret;
                        u32 readedlen;
                        u8 i;
                        Ql_memset(strBuf,0,63);
                        Ql_memset(textBuf,0,63);
                        Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
                        ret = Ql_FileRead(filehandle1, strBuf,10,&readedlen);
                       // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)"Writepointer=",Ql_strlen("Writepointer="));
                        Ql_sprintf(textBuf,"%s",strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));  
                       // Ql_SendToUart(ql_uart_port1,strBuf,Ql_strlen( strBuf));
                       
                        //Ql_memset(strBuf,0,53);
                        //Ql_memset(textBuf,0,53);
                        Position1=0;
						nextposition=0;
                        i=0;
                        while(textBuf[i]!='E')
                        {
                        	
                        Position1++;
                        i++;
                        }
                       if(Position1>0 && Position1<=2)
                       nextposition=(((textBuf[0]-0x30)*10)+(textBuf[1]-0x30)); 
                       else if(Position1>2 && Position1<=3)
                       nextposition=(((textBuf[0]-0x30)*100)+((textBuf[1]-0x30)*10)+(textBuf[2]-0x30)); 
                       else if(Position1>3 && Position1<=4)
                       nextposition=(((textBuf[0]-0x30)*1000)+((textBuf[1]-0x30)*100)+((textBuf[2]-0x30)*10)+(textBuf[3]-0x30));
                       else if(Position1>4 && Position1<=5)
                       nextposition=(((textBuf[0]-0x30)*10000)+((textBuf[1]-0x30)*1000)+((textBuf[2]-0x30)*100)+((textBuf[3]-0x30)*10)+(textBuf[4]-0x30));      
                       else if(Position1>5 && Position1<=6)
                       nextposition=(((textBuf[0]-0x30)*100000)+((textBuf[1]-0x30)*10000)+((textBuf[2]-0x30)*1000)+((textBuf[3]-0x30)*100)+((textBuf[4]-0x30)*10)+(textBuf[5]-0x30));
                       else if(Position1>6 && Position1<=7)
                       nextposition=(((textBuf[0]-0x30)*1000000)+((textBuf[1]-0x30)*100000)+((textBuf[2]-0x30)*10000)+((textBuf[3]-0x30)*1000)+((textBuf[4]-0x30)*100)+((textBuf[5]-0x30)*10)+(textBuf[6]-0x30));
                       
                       else if(Position1>7 && Position1<=8)
                       nextposition=(((textBuf[0]-0x30)*10000000)+((textBuf[1]-0x30)*1000000)+((textBuf[2]-0x30)*100000)+((textBuf[3]-0x30)*10000)+((textBuf[4]-0x30)*1000)+((textBuf[5]-0x30)*100)+((textBuf[6]-0x30)*10)+(textBuf[7]-0x30));
                       
                       else if(Position1>8 && Position1<=9)
                       nextposition=(((textBuf[0]-0x30)*100000000)+((textBuf[1]-0x30)*10000000)+((textBuf[2]-0x30)*1000000)+((textBuf[3]-0x30)*100000)+((textBuf[4]-0x30)*10000)+((textBuf[5]-0x30)*1000)+((textBuf[6]-0x30)*100)+((textBuf[7]-0x30)*10)+(textBuf[8]-0x30));
                       
                       else if(Position1>9 && Position1<=10)
                       nextposition=(((textBuf[0]-0x30)*1000000000)+((textBuf[1]-0x30)*100000000)+((textBuf[2]-0x30)*10000000)+((textBuf[3]-0x30)*1000000)+((textBuf[4]-0x30)*100000)+((textBuf[5]-0x30)*10000)+((textBuf[6]-0x30)*1000)+((textBuf[7]-0x30)*100)+((textBuf[8]-0x30)*10)+(textBuf[9]-0x30));
                       
                       WritePositionStatus=nextposition;      
                             
                             
                             
                        
                        
                    }
                    
                   
                    if (pt)
                    {
                        s32 ret1;
                        u32 writeedlen;
                        //pt = Ql_strstr(p2,"=");
                        //pt += 1;
                        Ql_memset(textBuf,0,63);
                        Ql_FileSeek(filehandle,nextposition, QL_FS_FILE_BEGIN);
                        ret1 = Ql_FileWrite(filehandle, (u8*)pt,63, &writeedlen);
                        Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret1, writeedlen);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                       
                    }
                     
                    
                
        			 //p = Ql_strstr(pData,"Ql_FileGetFilePosition");
                    if (1)
                    {
                       s32 ret;
                       Ql_memset(textBuf,0,63);
                       ret = Ql_FileGetFilePosition(filehandle, &Position);
                        
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)"NewWritePosition=",Ql_strlen("NewWritePosition="));
                       
                       Ql_sprintf(textBuf,"%d", Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));    
                       
                    }

                    
                    pt=textBuf;
                    if (pt)
                    {
                        s32 ret1;
                        u32 writeedlen;
                        //pt = Ql_strstr(p2,"=");
                        //pt += 1;
                       // ret = Ql_FileGetFilePosition(filehandle, &Position);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       	Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
                        ret1 = Ql_FileWrite(filehandle1, (u8*)pt,Ql_strlen(textBuf), &writeedlen);
                        Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret1, writeedlen);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        globalpos=writeedlen;
                        
                       Ql_memset(textBuf,0,63); 
                       ret = Ql_FileGetFilePosition(filehandle1, &Position1);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)"NewWritePointerCount=",Ql_strlen("NewWritePointerCount="));
                       Ql_sprintf(textBuf,"%d", Position1);
                       Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); ; 
                       Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));   
                       
                    }
                    
                    
                    //p = Ql_strstr(pData,"Ql_FileClose");
                    
                    
                    //pt="Ql_FileSeek=53"
                   // p = Ql_strstr(pData,"Ql_FileSeek=");
                   
            
                    
                    Ql_FileClose(filehandle);
     				 filehandle = -1;
      				 Ql_memset(textBuf,0,63);
      					Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      					Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                    
			
			       Ql_FileClose(filehandle1);
     				 filehandle1 = -1;
      				 Ql_memset(textBuf,0,63);
      					Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      					Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
      					
      					FileSyatem_ReadFunction();
      					
      					 finalF[37]= 'F';
	 					 finalF[38]= 'F';
						 finalF[43]= '0';
	  					 finalF[44]= '0';
                    
           
}

void adcsend_function(void)
{
	char *p2;
	char *pt;
	s8 ret = -1;
	u8 i,j;

	    
                   openfile1();
                   openfile2();

				  
                    pt=finalF;
                    
                   
                   /* Ql_memset(textBuf,0,53); 
                        ret = Ql_FileGetFilePosition(filehandle1, &Position1);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,"Position1=",Ql_strlen("Position1="));
                       Ql_sprintf(textBuf,"%d", Position1);
                       Ql_SendToUart(ql_uart_port1,textBuf,Ql_strlen(textBuf)); ; */   
                    
                     
                    /*  if (1)
                    {
                        s32 ret;
                        u32 readedlen;
                        u8 i;
                        Ql_memset(strBuf,0,53);
                        Ql_memset(textBuf,0,53);
                        Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
                        ret = Ql_FileRead(filehandle1, strBuf,10,&readedlen);
                       // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,"Writepointer=",Ql_strlen("Writepointer="));
                        Ql_sprintf(textBuf,"%s",strBuf);
                        Ql_SendToUart(ql_uart_port1,textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,"\r\n",Ql_strlen("\r\n")); 
                        Position1= NewWritePointerCount; 
                       // Ql_SendToUart(ql_uart_port1,strBuf,Ql_strlen( strBuf));
                       
                        //Ql_memset(strBuf,0,53);
                        //Ql_memset(textBuf,0,53);
                        Position1=0;
                        i=0;
                        while(textBuf[i]!='E')
                        {
                        	
                        Position1++;
                        i++;
                        }
                       if(Position1>0 && Position1<=2)
                       nextposition=(((textBuf[0]-0x30)*10)+(textBuf[1]-0x30)); 
                       else if(Position1>2 && Position1<=3)
                       nextposition=(((textBuf[0]-0x30)*100)+((textBuf[1]-0x30)*10)+(textBuf[2]-0x30)); 
                       else if(Position1>3 && Position1<=4)
                       nextposition=(((textBuf[0]-0x30)*1000)+((textBuf[1]-0x30)*100)+((textBuf[2]-0x30)*10)+(textBuf[3]-0x30));
                       else if(Position1>4 && Position1<=5)
                       nextposition=(((textBuf[0]-0x30)*10000)+((textBuf[1]-0x30)*1000)+((textBuf[2]-0x30)*100)+((textBuf[3]-0x30)*10)+(textBuf[4]-0x30));      
                       else if(Position1>5 && Position1<=6)
                       nextposition=(((textBuf[0]-0x30)*100000)+((textBuf[1]-0x30)*10000)+((textBuf[2]-0x30)*1000)+((textBuf[3]-0x30)*100)+((textBuf[4]-0x30)*10)+(textBuf[5]-0x30));
                       else if(Position1>6 && Position1<=7)
                       nextposition=(((textBuf[0]-0x30)*1000000)+((textBuf[1]-0x30)*100000)+((textBuf[2]-0x30)*10000)+((textBuf[3]-0x30)*1000)+((textBuf[4]-0x30)*100)+((textBuf[5]-0x30)*10)+(textBuf[6]-0x30));
                       
                       else if(Position1>7 && Position1<=8)
                       nextposition=(((textBuf[0]-0x30)*10000000)+((textBuf[1]-0x30)*1000000)+((textBuf[2]-0x30)*100000)+((textBuf[3]-0x30)*10000)+((textBuf[4]-0x30)*1000)+((textBuf[5]-0x30)*100)+((textBuf[6]-0x30)*10)+(textBuf[7]-0x30));
                       
                       else if(Position1>8 && Position1<=9)
                       nextposition=(((textBuf[0]-0x30)*100000000)+((textBuf[1]-0x30)*10000000)+((textBuf[2]-0x30)*1000000)+((textBuf[3]-0x30)*100000)+((textBuf[4]-0x30)*10000)+((textBuf[5]-0x30)*1000)+((textBuf[6]-0x30)*100)+((textBuf[7]-0x30)*10)+(textBuf[8]-0x30));
                       
                       else if(Position1>9 && Position1<=10)
                       nextposition=(((textBuf[0]-0x30)*1000000000)+((textBuf[1]-0x30)*100000000)+((textBuf[2]-0x30)*10000000)+((textBuf[3]-0x30)*1000000)+((textBuf[4]-0x30)*100000)+((textBuf[5]-0x30)*10000)+((textBuf[6]-0x30)*1000)+((textBuf[7]-0x30)*100)+((textBuf[8]-0x30)*10)+(textBuf[9]-0x30));
                       
                             
                             
                             
                             
                        
                        
                    }*/  
                     Ql_memset(textBuf,0,63); 
                    if (1)
                    {
                        s32 ret;
                        u32 readedlen;
                        u8 i;
                        Ql_memset(strBuf,0,63);
                        Ql_memset(textBuf,0,63);
                        Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
                        ret = Ql_FileRead(filehandle1, strBuf,10,&readedlen);
                       // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)"Writepointer=",Ql_strlen("Writepointer="));
                        Ql_sprintf(textBuf,"%s",strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));  
                       // Ql_SendToUart(ql_uart_port1,strBuf,Ql_strlen( strBuf));
                       
                        //Ql_memset(strBuf,0,53);
                        //Ql_memset(textBuf,0,53);
                        Position1=0;
						nextposition=0;
                        i=0;
                        while(textBuf[i]!='E')
                        {
                        	
                        Position1++;
                        i++;
                        }
                       if(Position1>0 && Position1<=2)
                       nextposition=(((textBuf[0]-0x30)*10)+(textBuf[1]-0x30)); 
                       else if(Position1>2 && Position1<=3)
                       nextposition=(((textBuf[0]-0x30)*100)+((textBuf[1]-0x30)*10)+(textBuf[2]-0x30)); 
                       else if(Position1>3 && Position1<=4)
                       nextposition=(((textBuf[0]-0x30)*1000)+((textBuf[1]-0x30)*100)+((textBuf[2]-0x30)*10)+(textBuf[3]-0x30));
                       else if(Position1>4 && Position1<=5)
                       nextposition=(((textBuf[0]-0x30)*10000)+((textBuf[1]-0x30)*1000)+((textBuf[2]-0x30)*100)+((textBuf[3]-0x30)*10)+(textBuf[4]-0x30));      
                       else if(Position1>5 && Position1<=6)
                       nextposition=(((textBuf[0]-0x30)*100000)+((textBuf[1]-0x30)*10000)+((textBuf[2]-0x30)*1000)+((textBuf[3]-0x30)*100)+((textBuf[4]-0x30)*10)+(textBuf[5]-0x30));
                       else if(Position1>6 && Position1<=7)
                       nextposition=(((textBuf[0]-0x30)*1000000)+((textBuf[1]-0x30)*100000)+((textBuf[2]-0x30)*10000)+((textBuf[3]-0x30)*1000)+((textBuf[4]-0x30)*100)+((textBuf[5]-0x30)*10)+(textBuf[6]-0x30));
                       
                       else if(Position1>7 && Position1<=8)
                       nextposition=(((textBuf[0]-0x30)*10000000)+((textBuf[1]-0x30)*1000000)+((textBuf[2]-0x30)*100000)+((textBuf[3]-0x30)*10000)+((textBuf[4]-0x30)*1000)+((textBuf[5]-0x30)*100)+((textBuf[6]-0x30)*10)+(textBuf[7]-0x30));
                       
                       else if(Position1>8 && Position1<=9)
                       nextposition=(((textBuf[0]-0x30)*100000000)+((textBuf[1]-0x30)*10000000)+((textBuf[2]-0x30)*1000000)+((textBuf[3]-0x30)*100000)+((textBuf[4]-0x30)*10000)+((textBuf[5]-0x30)*1000)+((textBuf[6]-0x30)*100)+((textBuf[7]-0x30)*10)+(textBuf[8]-0x30));
                       
                       else if(Position1>9 && Position1<=10)
                       nextposition=(((textBuf[0]-0x30)*1000000000)+((textBuf[1]-0x30)*100000000)+((textBuf[2]-0x30)*10000000)+((textBuf[3]-0x30)*1000000)+((textBuf[4]-0x30)*100000)+((textBuf[5]-0x30)*10000)+((textBuf[6]-0x30)*1000)+((textBuf[7]-0x30)*100)+((textBuf[8]-0x30)*10)+(textBuf[9]-0x30));
                       
                       WritePositionStatus=nextposition;      
                             
                             
                             
                        
                        
                    }
                    
                   
                    if (pt)
                    {
                        s32 ret1;
                        u32 writeedlen;
                        //pt = Ql_strstr(p2,"=");
                        //pt += 1;
                        Ql_memset(textBuf,0,63);
                        Ql_FileSeek(filehandle,nextposition, QL_FS_FILE_BEGIN);
                        ret1 = Ql_FileWrite(filehandle, (u8*)pt,63, &writeedlen);
                        Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret1, writeedlen);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                       
                    }
                     
                    
                
        			 //p = Ql_strstr(pData,"Ql_FileGetFilePosition");
                    if (1)
                    {
                       s32 ret;
                       Ql_memset(textBuf,0,63);
                       ret = Ql_FileGetFilePosition(filehandle, &Position);
                        
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)"NewWritePosition=",Ql_strlen("NewWritePosition="));
                       
                       Ql_sprintf(textBuf,"%d", Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));    
                       
                    }

                    
                    pt=textBuf;
                    if (pt)
                    {
                        s32 ret1;
                        u32 writeedlen;
                        //pt = Ql_strstr(p2,"=");
                        //pt += 1;
                       // ret = Ql_FileGetFilePosition(filehandle, &Position);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       	Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
                        ret1 = Ql_FileWrite(filehandle1, (u8*)pt,Ql_strlen(textBuf), &writeedlen);
                        Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret1, writeedlen);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        globalpos=writeedlen;
                        
                       Ql_memset(textBuf,0,63); 
                       ret = Ql_FileGetFilePosition(filehandle1, &Position1);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)"NewWritePointerCount=",Ql_strlen("NewWritePointerCount="));
                       Ql_sprintf(textBuf,"%d", Position1);
                       Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); ; 
                       Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));   
                       
                    }
                    
                    
                    //p = Ql_strstr(pData,"Ql_FileClose");
                    
                    
                    //pt="Ql_FileSeek=53"
                   // p = Ql_strstr(pData,"Ql_FileSeek=");
                   
            
                    
                    Ql_FileClose(filehandle);
     				 filehandle = -1;
      				 Ql_memset(textBuf,0,63);
      					Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      					Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                    
			
			       Ql_FileClose(filehandle1);
     				 filehandle1 = -1;
      				 Ql_memset(textBuf,0,63);
      					Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      					Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
      					
      					FileSyatem_ReadFunction();
      					
      					 finalF[37]= 'F';
	 					 finalF[38]= 'F';
						 finalF[43]= '0';
	  					 finalF[44]= '0';
                    
           
}
void filesystem_function(void)
{
	char *p2;
	char *pt;
	s8 ret = -1;
	u8 i,j;


	    
                   openfile1();
                   openfile2();
				   for(i=0;i<53;i++)
				   {
				   TempBuffer[i]=finalF[i];
				   }
				   finalF[0]='0';
				   finalF[1]='0';
				   finalF[2]='0';
				   finalF[3]='0';
				   finalF[4]='0';
				   finalF[5]='0';
				   finalF[6]='#';
				   finalF[7]='*';
				   finalF[8]='3';
				   finalF[9]='5';
	
				   for(i=0;i<53;i++)
				   {
					finalF[10+i]=TempBuffer[i];
				   }
                    pt=finalF;
                    
                   
                   /* Ql_memset(textBuf,0,53); 
                        ret = Ql_FileGetFilePosition(filehandle1, &Position1);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,"Position1=",Ql_strlen("Position1="));
                       Ql_sprintf(textBuf,"%d", Position1);
                       Ql_SendToUart(ql_uart_port1,textBuf,Ql_strlen(textBuf)); ; */   
                    
                     
                    /*  if (1)
                    {
                        s32 ret;
                        u32 readedlen;
                        u8 i;
                        Ql_memset(strBuf,0,53);
                        Ql_memset(textBuf,0,53);
                        Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
                        ret = Ql_FileRead(filehandle1, strBuf,10,&readedlen);
                       // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,"Writepointer=",Ql_strlen("Writepointer="));
                        Ql_sprintf(textBuf,"%s",strBuf);
                        Ql_SendToUart(ql_uart_port1,textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,"\r\n",Ql_strlen("\r\n")); 
                        Position1= NewWritePointerCount; 
                       // Ql_SendToUart(ql_uart_port1,strBuf,Ql_strlen( strBuf));
                       
                        //Ql_memset(strBuf,0,53);
                        //Ql_memset(textBuf,0,53);
                        Position1=0;
                        i=0;
                        while(textBuf[i]!='E')
                        {
                        	
                        Position1++;
                        i++;
                        }
                       if(Position1>0 && Position1<=2)
                       nextposition=(((textBuf[0]-0x30)*10)+(textBuf[1]-0x30)); 
                       else if(Position1>2 && Position1<=3)
                       nextposition=(((textBuf[0]-0x30)*100)+((textBuf[1]-0x30)*10)+(textBuf[2]-0x30)); 
                       else if(Position1>3 && Position1<=4)
                       nextposition=(((textBuf[0]-0x30)*1000)+((textBuf[1]-0x30)*100)+((textBuf[2]-0x30)*10)+(textBuf[3]-0x30));
                       else if(Position1>4 && Position1<=5)
                       nextposition=(((textBuf[0]-0x30)*10000)+((textBuf[1]-0x30)*1000)+((textBuf[2]-0x30)*100)+((textBuf[3]-0x30)*10)+(textBuf[4]-0x30));      
                       else if(Position1>5 && Position1<=6)
                       nextposition=(((textBuf[0]-0x30)*100000)+((textBuf[1]-0x30)*10000)+((textBuf[2]-0x30)*1000)+((textBuf[3]-0x30)*100)+((textBuf[4]-0x30)*10)+(textBuf[5]-0x30));
                       else if(Position1>6 && Position1<=7)
                       nextposition=(((textBuf[0]-0x30)*1000000)+((textBuf[1]-0x30)*100000)+((textBuf[2]-0x30)*10000)+((textBuf[3]-0x30)*1000)+((textBuf[4]-0x30)*100)+((textBuf[5]-0x30)*10)+(textBuf[6]-0x30));
                       
                       else if(Position1>7 && Position1<=8)
                       nextposition=(((textBuf[0]-0x30)*10000000)+((textBuf[1]-0x30)*1000000)+((textBuf[2]-0x30)*100000)+((textBuf[3]-0x30)*10000)+((textBuf[4]-0x30)*1000)+((textBuf[5]-0x30)*100)+((textBuf[6]-0x30)*10)+(textBuf[7]-0x30));
                       
                       else if(Position1>8 && Position1<=9)
                       nextposition=(((textBuf[0]-0x30)*100000000)+((textBuf[1]-0x30)*10000000)+((textBuf[2]-0x30)*1000000)+((textBuf[3]-0x30)*100000)+((textBuf[4]-0x30)*10000)+((textBuf[5]-0x30)*1000)+((textBuf[6]-0x30)*100)+((textBuf[7]-0x30)*10)+(textBuf[8]-0x30));
                       
                       else if(Position1>9 && Position1<=10)
                       nextposition=(((textBuf[0]-0x30)*1000000000)+((textBuf[1]-0x30)*100000000)+((textBuf[2]-0x30)*10000000)+((textBuf[3]-0x30)*1000000)+((textBuf[4]-0x30)*100000)+((textBuf[5]-0x30)*10000)+((textBuf[6]-0x30)*1000)+((textBuf[7]-0x30)*100)+((textBuf[8]-0x30)*10)+(textBuf[9]-0x30));
                       
                             
                             
                             
                             
                        
                        
                    }*/  
                     Ql_memset(textBuf,0,63); 
                    if (1)
                    {
                        s32 ret;
                        u32 readedlen;
                        u8 i;
                        Ql_memset(strBuf,0,63);
                        Ql_memset(textBuf,0,63);
                        Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
                        ret = Ql_FileRead(filehandle1, strBuf,10,&readedlen);
                       // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)"Writepointer=",Ql_strlen("Writepointer="));
                        Ql_sprintf(textBuf,"%s",strBuf);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));  
                       // Ql_SendToUart(ql_uart_port1,strBuf,Ql_strlen( strBuf));
                       
                        //Ql_memset(strBuf,0,53);
                        //Ql_memset(textBuf,0,53);
                        Position1=0;
                        i=0;
                        while(textBuf[i]!='E')
                        {
                        	
                        Position1++;
                        i++;
                        }
                       if(Position1>0 && Position1<=2)
                       nextposition=(((textBuf[0]-0x30)*10)+(textBuf[1]-0x30)); 
                       else if(Position1>2 && Position1<=3)
                       nextposition=(((textBuf[0]-0x30)*100)+((textBuf[1]-0x30)*10)+(textBuf[2]-0x30)); 
                       else if(Position1>3 && Position1<=4)
                       nextposition=(((textBuf[0]-0x30)*1000)+((textBuf[1]-0x30)*100)+((textBuf[2]-0x30)*10)+(textBuf[3]-0x30));
                       else if(Position1>4 && Position1<=5)
                       nextposition=(((textBuf[0]-0x30)*10000)+((textBuf[1]-0x30)*1000)+((textBuf[2]-0x30)*100)+((textBuf[3]-0x30)*10)+(textBuf[4]-0x30));      
                       else if(Position1>5 && Position1<=6)
                       nextposition=(((textBuf[0]-0x30)*100000)+((textBuf[1]-0x30)*10000)+((textBuf[2]-0x30)*1000)+((textBuf[3]-0x30)*100)+((textBuf[4]-0x30)*10)+(textBuf[5]-0x30));
                       else if(Position1>6 && Position1<=7)
                       nextposition=(((textBuf[0]-0x30)*1000000)+((textBuf[1]-0x30)*100000)+((textBuf[2]-0x30)*10000)+((textBuf[3]-0x30)*1000)+((textBuf[4]-0x30)*100)+((textBuf[5]-0x30)*10)+(textBuf[6]-0x30));
                       
                       else if(Position1>7 && Position1<=8)
                       nextposition=(((textBuf[0]-0x30)*10000000)+((textBuf[1]-0x30)*1000000)+((textBuf[2]-0x30)*100000)+((textBuf[3]-0x30)*10000)+((textBuf[4]-0x30)*1000)+((textBuf[5]-0x30)*100)+((textBuf[6]-0x30)*10)+(textBuf[7]-0x30));
                       
                       else if(Position1>8 && Position1<=9)
                       nextposition=(((textBuf[0]-0x30)*100000000)+((textBuf[1]-0x30)*10000000)+((textBuf[2]-0x30)*1000000)+((textBuf[3]-0x30)*100000)+((textBuf[4]-0x30)*10000)+((textBuf[5]-0x30)*1000)+((textBuf[6]-0x30)*100)+((textBuf[7]-0x30)*10)+(textBuf[8]-0x30));
                       
                       else if(Position1>9 && Position1<=10)
                       nextposition=(((textBuf[0]-0x30)*1000000000)+((textBuf[1]-0x30)*100000000)+((textBuf[2]-0x30)*10000000)+((textBuf[3]-0x30)*1000000)+((textBuf[4]-0x30)*100000)+((textBuf[5]-0x30)*10000)+((textBuf[6]-0x30)*1000)+((textBuf[7]-0x30)*100)+((textBuf[8]-0x30)*10)+(textBuf[9]-0x30));
                       
                       WritePositionStatus=nextposition;      
                             
                             
                             
                        
                        
                    }
                    
                   
                    if (pt)
                    {
                        s32 ret1;
                        u32 writeedlen;
                        //pt = Ql_strstr(p2,"=");
                        //pt += 1;
                        Ql_memset(textBuf,0,63);
                        Ql_FileSeek(filehandle,nextposition, QL_FS_FILE_BEGIN);
                        ret1 = Ql_FileWrite(filehandle, (u8*)pt,63, &writeedlen);
                        Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret1, writeedlen);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
                       
                    }
                     
                    
                
        			 //p = Ql_strstr(pData,"Ql_FileGetFilePosition");
                    if (1)
                    {
                       s32 ret;
                       Ql_memset(textBuf,0,63);
                       ret = Ql_FileGetFilePosition(filehandle, &Position);
                        
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)"NewWritePosition=",Ql_strlen("NewWritePosition="));
                       
                       Ql_sprintf(textBuf,"%d", Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));    
                       
                    }

                    
                    pt=textBuf;
                    if (pt)
                    {
                        s32 ret1;
                        u32 writeedlen;
                        //pt = Ql_strstr(p2,"=");
                        //pt += 1;
                       // ret = Ql_FileGetFilePosition(filehandle, &Position);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       	Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
                        ret1 = Ql_FileWrite(filehandle1, (u8*)pt,Ql_strlen(textBuf), &writeedlen);
                        Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret1, writeedlen);
                        Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                        globalpos=writeedlen;
                        
                       Ql_memset(textBuf,0,63); 
                       ret = Ql_FileGetFilePosition(filehandle1, &Position1);
                       // Ql_sprintf(textBuf,"Ql_FileGetFilePosition()=%d: Position=%d\r\n",ret, Position);
                       Ql_SendToUart(ql_uart_port1,(u8*)"NewWritePointerCount=",Ql_strlen("NewWritePointerCount="));
                       Ql_sprintf(textBuf,"%d", Position1);
                       Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); ; 
                       Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));   
                       
                    }
                    
                    
                    //p = Ql_strstr(pData,"Ql_FileClose");
                    
                    
                    //pt="Ql_FileSeek=53"
                   // p = Ql_strstr(pData,"Ql_FileSeek=");
                   
            
                    
                    Ql_FileClose(filehandle);
     				 filehandle = -1;
      				 Ql_memset(textBuf,0,63);
      					Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      					Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
                    
			
			       Ql_FileClose(filehandle1);
     				 filehandle1 = -1;
      				 Ql_memset(textBuf,0,63);
      					Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      					Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
      					
      					FileSyatem_ReadFunction();
      					
      					 finalF[35]= '0';
	 					 finalF[36]= '0';
                    
           
}
void key_send(void)
{
	  Not_Parameter_Update=FALSE;
	  finalF[35]= key_buffer[0];
	  finalF[36]= key_buffer[1];
	  filesystem_function();
}

void io_send(void)
{
	  finalF[37]= hex_input[0];
	  finalF[38]= hex_input[1];
	  finalF[43]= 'A';
	  finalF[44]= 'E';

	  Not_Parameter_Update=FALSE;
	  iosend_function();
}
			

void uart2_datahandler(u16 Datalen,char *data)

{
	int i=0,k=0,x,z=0,a=0,b=0,c=0,d=0,e=0,f=0,jj=0,angle1,angle2;
	
	static int chek_notfix; 
	
	static ascii ant_status[4];//----change from 4 to 6
	
	s8 ret = -1;
	
	ascii dummy_head[5],test1[4];
	static ascii buff0='0',buff1='1';
	
	s8 ret3;
	hex_input[0]='F';
	hex_input[1]='F';
	 
	 		

	// Ql_SendToUart(ql_uart_port1,data,Ql_strlen(data));
	 //Ql_SendToUart(ql_uart_port1,&Datalen,1);
	{
		
		for(i=0;i<Datalen;i++) 
		{ 
			glo_RX_buff[RXbuf_end_ptr1] = data[i];
			
			
			if(RXbuf_end_ptr1 >= RXBUF_SIZE)
			{
				RXbuf_end_ptr1 = 0;
			}
			
			if(glo_RX_buff[RXbuf_end_ptr1]==0x0a)
			{
				send=TRUE;
				// adl_atSendResponse(ADL_AT_UNS,"got data");
				
			}
			else
			{
				send=FALSE;
				RXbuf_end_ptr1++;
			}
		}
		
		
		/*if(check_dis>=100)//------distance calculator
		{
			//adl_atSendResponse(ADL_AT_UNS,"Initializing all timers again distance");
			if(dota_flag==FALSE)
			{
				//adl_atCmdCreate( "at+wrst=1,\"000:01\"", FALSE , wbci_enable_handler3, "*", NULL );
			}
		}
		else
		{
			check_dis++;
		}
		
		if(check_coverage>=200)//---------coverage_timerhandler
		{
			adl_atSendResponse(ADL_AT_UNS,"Initializing all timers again coverage");
			
			if(dota_flag==FALSE)
			{
				adl_atCmdCreate( "at+wrst=1,\"000:01\"", FALSE , wbci_enable_handler3, "*", NULL );
			}
		}
		else
		{
			check_coverage++;
		}
		if(check_gprs>=300)//------gprs_timerhandler
			
		{
			adl_atSendResponse(ADL_AT_UNS,"Data write timer has been stopped");
			adl_atSendResponse(ADL_AT_UNS,"Initializing all timers again");
			
			if(dota_flag==FALSE)
			{
				adl_atCmdCreate( "at+wrst=1,\"000:01\"", FALSE , wbci_enable_handler3, "*", NULL );
			}
		}
		else
		{
			check_gprs++;
		}*/
		
		if((send==TRUE))
		{
			//adl_atSendResponse(ADL_AT_UNS,"checking data");
			
			        
			for(x=6;x<RXbuf_end_ptr1;x++)
			{
				if(glo_RX_buff[x]=='.')
				{
					
				}
				else if(glo_RX_buff[x]==',')
				{
					z++;
				}
				else
				{
					switch(z)
					{
					case 1:
						time[a]=glo_RX_buff[x];
						a++;
						break;
						
						
					case 2:
						
						status=glo_RX_buff[x];
						
						
						
						break;
						
					case 3:
						lat[b]=glo_RX_buff[x];
						b++;
						break;
						
					case 4://--change
						angle1=glo_RX_buff[x];
						break;
						
					case 5:
						lon[c]=glo_RX_buff[x];
						c++;
						break;
						
					case 6://change
						angle2=glo_RX_buff[x];
						break;
						
						
					case 7:
						speed1[d]=glo_RX_buff[x];
						d++;
						break;
						
					case 8:
						head[e]=glo_RX_buff[x];
						e++;
						break;
						
					case 9:
						date[f]=glo_RX_buff[x];
						f++;
						break;
						
					}//switch
					
				}//if
			} //for
			
			
			if(d<=3) 
			{
				if(fix==TRUE)
				{
					speed2[2]=speed1[0];
					speed2[1]='0';
					speed2[0]='0';
				}
				else
				{
					speed2[2]='0';
					speed2[1]='0';
					speed2[0]='0';
					
				}
				
				//spret=wm_atoi(speed2);	   //Convert  this into int
				spret=ASCIItoINT(speed2,3);
				//spret=spret*1.5;
				pret=(u32)spret;
				if(pret<=4)
					
				{
					speed2[2]='0';
					speed2[1]='0';
					speed2[0]='0';
					pret=0;
					
				
					speed_flag_cnt++;
					speed_mem_cnt=0;
					if(speed_flag_cnt>=60)
					{
						speed_flag=TRUE;
						speed_flag_cnt=0;
						
						
					}
					
				}
				else
				{
					speed_flag=FALSE;
					if(speed_mem_cnt==0)
					{
						speed_mem_cnt++;
						
					}

					/*wm_sprintf(test1,"%d",pret);
					adl_atSendResponse ( ADL_AT_RSP,"\r\n pret value=");
					adl_atSendResponse ( ADL_AT_RSP,test1);
					wm_sprintf(test1,"%d",over_speed);
					adl_atSendResponse ( ADL_AT_RSP,"\r\n over_speed value=");
					adl_atSendResponse ( ADL_AT_RSP,test1);
					if(pret>=over_speed)
					{
						beep_buz=TRUE;
					}
					else
					{
						beep_buz=FALSE;
					} */
				} 
			
			
			//	wm_itohexa(speed,pret,2);  place its meanning
			INTtoHextoASCII(speed,pret,2);
				
				
				
			}
			else if(d==4)
			{
				speed2[0]='0';//speed
				speed2[1]=speed1[0];
				speed2[2]=speed1[1];
				//adl_atSendResponse ( ADL_AT_RSP,"\r\nspeed inside_5=");
				//adl_atSendResponse ( ADL_AT_RSP,speed2);
				
				//spret=wm_atoi(speed2);
				spret=ASCIItoINT(speed2,3);
				//spret=spret*1.5;
				pret=(u32)spret;
				
				//wm_itohexa(speed,pret,2); place meanning
				INTtoHextoASCII(speed,pret,2);
				
				if(pret<=4)
				{
					
					speed_flag_cnt++;
					speed_mem_cnt=0;
					if(speed_flag_cnt>=60)
					{
						speed_flag=TRUE;
						speed_flag_cnt=0;
						
					}
					
				}
				else
				{
					speed_flag=FALSE;
					if(speed_mem_cnt==0)
					{
						speed_mem_cnt++;
						/*ret3=adl_flhWrite ( fhandle, ID[1984],1,&buff0);
						if(ret3==OK)
						{
							adl_atSendResponse ( ADL_AT_RSP,"\r\n speed_flag sucsessfull stored in memory ");
						}
						else
						{
							adl_atSendResponse ( ADL_AT_RSP,"\r\n error in writting speed_flag");
						}*/
					}
					/*wm_sprintf(test1,"%d",pret);
					adl_atSendResponse ( ADL_AT_RSP,"\r\n pret value=");
					adl_atSendResponse ( ADL_AT_RSP,test1);
					wm_sprintf(test1,"%d",over_speed);
					adl_atSendResponse ( ADL_AT_RSP,"\r\n over_speed value=");
					adl_atSendResponse ( ADL_AT_RSP,test1);	 */
				/*	if(pret>=over_speed)
					{
						beep_buz=TRUE;
					}
					else
					{
						beep_buz=FALSE;
					} */
				}
				
				
				
			}
			else if(d==5)
			{
				
				speed2[0]= speed1[0];//speed
				speed2[1]= speed1[1];
				speed2[2]= speed1[2];//speed
			
				
			//	spret=wm_atoi(speed2);
				spret=ASCIItoINT(speed2,3);
				//spret=spret*1.5;
				pret=(u32)spret;
				
			//	wm_itohexa(speed,pret,2); need to place meanning
			INTtoHextoASCII(speed,pret,2);
				//adl_atSendResponse ( ADL_AT_RSP,"\r\nspeed=");
				//adl_atSendResponse ( ADL_AT_RSP,speed);
				
				if(pret==0)
				{
					
					speed_flag_cnt++;
					speed_mem_cnt=0;
					if(speed_flag_cnt>=60)
					{
						speed_flag=TRUE;
						speed_flag_cnt=0;
						
						
					}
				}
				else
				{
					speed_flag=FALSE;
					if(speed_mem_cnt==0)
					{
						speed_mem_cnt++;
					/*	ret3=adl_flhWrite ( fhandle, ID[1984],1,&buff0);
						if(ret3==OK)
						{
							adl_atSendResponse ( ADL_AT_RSP,"\r\n speed_flag sucsessfull stored in memory ");
						}
						else
						{
							adl_atSendResponse ( ADL_AT_RSP,"\r\n error in writting speed_flag");
						}*/
					}
					/*wm_sprintf(test1,"%d",pret);
					adl_atSendResponse ( ADL_AT_RSP,"\r\n pret value=");
					adl_atSendResponse ( ADL_AT_RSP,test1);
					wm_sprintf(test1,"%d",over_speed);
					adl_atSendResponse ( ADL_AT_RSP,"\r\n over_speed value=");
					adl_atSendResponse ( ADL_AT_RSP,test1);
					if(pret>=over_speed)
					{
						beep_buz=TRUE;
					}
					else
					{
						beep_buz=FALSE;
					}*/
				}
				
			}

			Ql_SendToUart(ql_uart_port1,(u8*)"SPEED=",Ql_strlen("SPEED="));
			Ql_SendToUart(ql_uart_port1,(u8*)speed2,3);
			Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
			Ql_memset(buffer,0,100);
			if(status=='A')
			{
			
				fix=TRUE;
				
				if(fix_status==FALSE)
				{
				iret = Ql_pinWrite( QL_PINNAME_M10_DCD, QL_PINLEVEL_HIGH);
                //Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret, QL_PINNAME_M10_DCD);
                //Ql_SendToUart(ql_uart_port1,buffer,Ql_strlen(buffer)); 
                 Ql_SendToUart(ql_uart_port1,(u8*)"GPS FIX\r\n",Ql_strlen("GPS FIX\r\n"));
                }
                
                fix_status=TRUE;

				fix_cnt++;
				if(fix_cnt==5)
				{
					start_gpscalc=TRUE;
				
				}
				if(fix_cnt>5)
				{
					fix_cnt=5;
				}

				
				
			}
			else
			{
				fix=FALSE;
				fix_status=FALSE;
				fix_cnt=0;
		
				iret = Ql_pinWrite(   QL_PINNAME_M10_DCD , QL_PINLEVEL_LOW);
                //Ql_sprintf(buffer, "\r\nWriteLow(%d),pin=%d\r\n",iret,   QL_PINNAME_M10_DCD );
                //Ql_SendToUart(ql_uart_port1,buffer,Ql_strlen(buffer));
                Ql_SendToUart(ql_uart_port1,(u8*)"GPS NO FIX\r\n",Ql_strlen("GPS NO FIX\r\n"));
               
			}
			
			//-----------------------------------------  
			//wm_strcpy(finalp, own_num); 
			
			if(fix==TRUE)
			{
			finalp[26]= status;//----status A or N
			finalF[0]=status;
			
			
			finalp[27]= lat[0];//-----LAT
			finalF[1]= lat[0];
			finalp[28]= lat[1];
			finalF[2]= lat[1];
			finalp[29]= lat[2];
			finalF[3]= lat[2];
			finalp[30]= lat[3];
			finalF[4]= lat[3];
			finalp[31]= lat[4];
			finalF[5]= lat[4];
			finalp[32]= lat[5];
			finalF[6]= lat[5];
			finalp[33]= lat[6];
			finalF[7]= lat[6];
			finalp[34]= lat[7];
			finalF[8]= lat[7];
			
			finalp[35]= lon[1];//---LON
			finalF[9]= lon[1];
			finalp[36]= lon[2];
			finalF[10]= lon[2];
			finalp[37]= lon[3];
			finalF[11]= lon[3];
			finalp[38]= lon[4];
			finalF[12]= lon[4];
			finalp[39]= lon[5];
			finalF[13]= lon[5];
			finalp[40]= lon[6];
			finalF[14]= lon[6];
			finalp[41]= lon[7];
			finalF[15]= lon[7];
			finalp[42]= lon[8];
			finalF[16]= lon[8];
			
			if(e==0)
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]='0';
				dummy_head[3]='0';
				
				//adl_fcmSendData(FCM_Handler1,"case_0",6); //dummy heading
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= '0';
				finalF[19]= '0';
				finalp[46]= '0';
				finalF[20]= '0';
			}
			else if(e==1)
				
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]='0';
				dummy_head[3]='0';
				
				//adl_fcmSendData(FCM_Handler1,"case_1",6); //dummy heading
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= '0';
				finalF[19]= '0';
				finalp[46]= '0';
				finalF[20]= '0';
				
			}
			else if(e==2)
				
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]='0';
				dummy_head[3]='0';
				
				//adl_fcmSendData(FCM_Handler1,"case_2",6); //dummy heading
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= '0';
				finalF[19]= '0';
				finalp[46]= '0';
				finalF[20]= '0';
				
			}
			else if(e==3)
				
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]=head[0];
				dummy_head[3]=head[1];
				
				//adl_fcmSendData(FCM_Handler1,dummy_head,4); //dummy heading
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= head[0];
				finalF[19]= head[0];
				finalp[46]= head[1];
				finalF[20]= head[1];
				
			}
			
			else if(e==4)
				
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]=head[0];
				dummy_head[3]=head[1];
				
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= head[0];
				finalF[19]= head[0];
				finalp[46]= head[1];
				finalF[20]= head[1];
				//adl_fcmSendData(FCM_Handler1,dummy_head,4); //dummy heading
			}
			else if(e==5)
				
			{
				dummy_head[0]='0';
				dummy_head[1]=head[0];
				dummy_head[2]=head[1];
				dummy_head[3]=head[2];
				
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= head[0];
				finalF[18]= head[0];
				finalp[45]= head[1];
				finalF[19]= head[1];
				finalp[46]= head[2];
				finalF[20]= head[2];
				//adl_fcmSendData(FCM_Handler1,dummy_head,4); //dummy heading
			}
			
			///  -change
			
			
			/* 
			finalp[43]= head[0];//---heading
			finalF[17]= head[0];
			finalp[44]= head[1];
			finalF[18]= head[1];
			finalp[45]= head[2];
			finalF[19]= head[2];
			if(e<3)
			{
			finalp[46]= '0';
			finalF[20]= '0';
			}
			else
			{
			finalp[46]= head[3];
			finalF[20]= head[3];
			}
			
			*/
			finalp[47]= speed[0];//speed
			finalF[21]= speed[0];
			finalp[48]= speed[1];
			finalF[22]= speed[1];
			
			finalp[49]= time[0];
			finalF[23]= time[0];
			finalp[50]= time[1];
			finalF[24]= time[1];
			finalp[51]= time[2];
			finalF[25]= time[2];
			
			finalp[52]= time[3];
			finalF[26]= time[3];
			finalp[53]= time[4];
			finalF[27]= time[4];
			finalp[54]= time[5];
			finalF[28]= time[5];
			
			finalp[55]= date[0];
			finalF[29]= date[0];
			finalp[56]= date[1];
			finalF[30]= date[1];
			finalp[57]= date[2];
			finalF[31]= date[2];
			
			finalp[58]= date[3];
			finalF[32]= date[3];
			finalp[59]= date[4];
			finalF[33]= date[4];
			finalp[60]= date[5];
			finalF[34]= date[5];
			
			finalp[61]= '0';
			//finalF[35]= '0';
			finalp[62]= '0';
			//finalF[36]= '0';
			
			
			// ok for change
			finalF[35]= key_buffer[0];
	  		finalF[36]= key_buffer[1];
			finalp[63]= hex_input[0];
			finalF[37]= hex_input[0];
			finalp[64]= hex_input[1];
			finalF[38]= hex_input[1];
			
			//This portion  is only for testing
			/*finalF[35]= '0';
	  		finalF[36]= '0';
			finalp[63]= '0';
			finalF[37]= '0';
			finalp[64]= '0';
			finalF[38]= '0';*/
			
			finalp[65]= '9';
			finalF[39]= '9';
			finalp[66]= '9';
			finalF[40]= '9';
			finalp[67]= '9';
			finalF[41]= '9';
			finalp[68]= '9';
			finalF[42]= '9';
			finalp[69]= '0';
			finalF[43]= '0';
			finalp[70]= '0';
			finalF[44]= '0';
			//wm_itohexa(hexbuff,fresult,6);
			INTtoHextoASCII(hexbuff,fresult,6);
			finalp[71]= hexbuff[4];
			finalF[45]= hexbuff[4];
			finalp[72]= hexbuff[5];
			finalF[46]= hexbuff[5];
			finalp[73]= hexbuff[2];
			finalF[47]= hexbuff[2];
			finalp[74]= hexbuff[3];
			finalF[48]= hexbuff[3];
			finalp[75]= hexbuff[0];
			finalF[49]= hexbuff[0];
			finalp[76]= hexbuff[1];
			finalF[50]= hexbuff[1];
			finalp[77]= '*';
			finalF[51]= '*';
			finalp[78]= '#';
			finalF[52]= '#';
			
			RXbuf_end_ptr1=0;
			a=0;b=0;c=0;d=0;e=0;f=0;
			}
			else
			{

			finalp[26]= status;//----status A or N
			finalF[0]=status;
			
			
			finalp[27]= '0';//-----LAT
			finalF[1]= '0';
			finalp[28]= '0';
			finalF[2]= '0';
			finalp[29]= '0';
			finalF[3]= '0';
			finalp[30]= '0';
			finalF[4]= '0';
			finalp[31]= '0';
			finalF[5]= '0';
			finalp[32]= '0';
			finalF[6]= '0';
			finalp[33]= '0';
			finalF[7]= '0';
			finalp[34]= '0';
			finalF[8]= '0';
			
			finalp[35]= '0';//---LON
			finalF[9]= '0';
			finalp[36]= '0';
			finalF[10]= '0';
			finalp[37]= '0';
			finalF[11]= '0';
			finalp[38]= '0';
			finalF[12]= '0';
			finalp[39]= '0';
			finalF[13]= '0';
			finalp[40]= '0';
			finalF[14]= '0';
			finalp[41]= '0';
			finalF[15]= '0';
			finalp[42]= '0';
			finalF[16]= '0';
			
			if(e==0)
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]='0';
				dummy_head[3]='0';
				
				//adl_fcmSendData(FCM_Handler1,"case_0",6); //dummy heading
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= '0';
				finalF[19]= '0';
				finalp[46]= '0';
				finalF[20]= '0';
			}
			else if(e==1)
				
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]='0';
				dummy_head[3]='0';
				
				//adl_fcmSendData(FCM_Handler1,"case_1",6); //dummy heading
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= '0';
				finalF[19]= '0';
				finalp[46]= '0';
				finalF[20]= '0';
				
			}
			else if(e==2)
				
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]='0';
				dummy_head[3]='0';
				
				//adl_fcmSendData(FCM_Handler1,"case_2",6); //dummy heading
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= '0';
				finalF[19]= '0';
				finalp[46]= '0';
				finalF[20]= '0';
				
			}
			else if(e==3)
				
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]=head[0];
				dummy_head[3]=head[1];
				
				//adl_fcmSendData(FCM_Handler1,dummy_head,4); //dummy heading
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= head[0];
				finalF[19]= head[0];
				finalp[46]= head[1];
				finalF[20]= head[1];
				
			}
			
			else if(e==4)
				
			{
				dummy_head[0]='0';
				dummy_head[1]='0';
				dummy_head[2]=head[0];
				dummy_head[3]=head[1];
				
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= '0';
				finalF[18]= '0';
				finalp[45]= head[0];
				finalF[19]= head[0];
				finalp[46]= head[1];
				finalF[20]= head[1];
				//adl_fcmSendData(FCM_Handler1,dummy_head,4); //dummy heading
			}
			else if(e==5)
				
			{
				dummy_head[0]='0';
				dummy_head[1]=head[0];
				dummy_head[2]=head[1];
				dummy_head[3]=head[2];
				
				
				finalp[43]= '0';//---heading
				finalF[17]= '0';
				finalp[44]= head[0];
				finalF[18]= head[0];
				finalp[45]= head[1];
				finalF[19]= head[1];
				finalp[46]= head[2];
				finalF[20]= head[2];
				//adl_fcmSendData(FCM_Handler1,dummy_head,4); //dummy heading
			}
			
			///  -change
			
			
			/* 
			finalp[43]= head[0];//---heading
			finalF[17]= head[0];
			finalp[44]= head[1];
			finalF[18]= head[1];
			finalp[45]= head[2];
			finalF[19]= head[2];
			if(e<3)
			{
			finalp[46]= '0';
			finalF[20]= '0';
			}
			else
			{
			finalp[46]= head[3];
			finalF[20]= head[3];
			}
			
			*/
			finalp[47]= '0';//speed
			finalF[21]= '0';
			finalp[48]= '0';
			finalF[22]= '0';
			
			finalp[49]= '0';
			finalF[23]= '0';
			finalp[50]= '0';
			finalF[24]= '0';
			finalp[51]= '0';
			finalF[25]= '0';
			
			finalp[52]= '0';
			finalF[26]= '0';
			finalp[53]= '0';
			finalF[27]= '0';
			finalp[54]= '0';
			finalF[28]= '0';
			
			finalp[55]= '0';
			finalF[29]= '0';
			finalp[56]= '0';
			finalF[30]= '0';
			finalp[57]= '0';
			finalF[31]= '0';
			
			finalp[58]= '0';
			finalF[32]= '0';
			finalp[59]= '0';
			finalF[33]= '0';
			finalp[60]= '0';
			finalF[34]= '0';
			
			finalp[61]= '0';
			finalF[35]= '0';
			finalp[62]= '0';
			finalF[36]= '0';
			finalp[63]= '0';
			finalF[37]= 'F';
			finalp[64]= '0';
			finalF[38]= 'F';
			finalp[65]= '9';
			finalF[39]= '9';
			finalp[66]= '9';
			finalF[40]= '9';
			finalp[67]= '9';
			finalF[41]= '9';
			finalp[68]= '9';
			finalF[42]= '9';
			finalp[69]= '0';
			finalF[43]= '0';
			finalp[70]= '0';
			finalF[44]= '0';
			//wm_itohexa(hexbuff,fresult,6);
			finalp[71]= hexbuff[4];
			finalF[45]= hexbuff[4];
			finalp[72]= hexbuff[5];
			finalF[46]= hexbuff[5];
			finalp[73]= hexbuff[2];
			finalF[47]= hexbuff[2];
			finalp[74]= hexbuff[3];
			finalF[48]= hexbuff[3];
			finalp[75]= hexbuff[0];
			finalF[49]= hexbuff[0];
			finalp[76]= hexbuff[1];
			finalF[50]= hexbuff[1];
			finalp[77]= '*';
			finalF[51]= '*';
			finalp[78]= '#';
			finalF[52]= '#';
			
			RXbuf_end_ptr1=0;
			a=0;b=0;c=0;d=0;e=0;f=0;
			}



		
			
			
			
			
}

}
//return TRUE;

} 

void Create_WavePointFile(void)
{
  
  char *p=NULL;
   char *p1=NULL;
   u32 ret;



  

  ret=Ql_FileCheck((u8*)"File1.txt");
   if(ret!=QL_RET_OK)
    {
    p1="Ql_FileOpen=cFile1.txt";
    p = Ql_strstr(p1,"Ql_FileOpen=c");
    if (p)
    {
      // s32 ret;
      p = Ql_strstr(p1,"=c");
      p += 2;
      ret = Ql_FileOpen((u8*)p ,TRUE, FALSE);
      if(ret >= QL_RET_OK)
      filehandle = ret;
      Ql_sprintf(textBuf,"Ql_FileOpen NEW (%s)=%d\r\n", p, ret);
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
      
     }
      Ql_FileClose(filehandle);
      filehandle1 = -1;
      Ql_memset(textBuf,0,63);
      Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf)); 
	  
	  ReadPositionStatus=0;
 	  WritePositionStatus=0;
      Position=0;
      Position1=0;
      Read_Position=0;
      nextposition=0;
      Read_nextposition=0;   
     }
     
    ret=Ql_FileCheck((u8*)"config.txt");
    
    if(ret!=QL_RET_OK)
    { 
    p1="Ql_FileOpen=cconfig.txt";
    p = Ql_strstr(p1,"Ql_FileOpen=c");
    if (p)
    {
      // s32 ret;
      p = Ql_strstr(p1,"=c");
      p += 2;
      ret = Ql_FileOpen((u8*)p ,TRUE, FALSE);
      if(ret >= QL_RET_OK)
      filehandle1 = ret;
      Ql_sprintf(textBuf,"Ql_FileOpen NEW (%s)=%d\r\n", p, ret);
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
      
     }
     p=End_Array;
     if(p)
     {
      u32 writeedlen;
      u32 readedlen;
     Ql_memset(textBuf,0,63);
    
     ret = Ql_FileWrite(filehandle1, (u8*)p,10, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
     
      Ql_FileSeek(filehandle1,10, QL_FS_FILE_BEGIN);
      
     ret = Ql_FileWrite(filehandle1, (u8*)p,10, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
     
     
     Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle1,0, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle1, strBuf,10,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"Writepointer=",Ql_strlen("Writepointer="));
     Ql_sprintf(textBuf,"%s",strBuf);
     
     Ql_memset(strBuf,0,63);
     Ql_memset (textBuf,0,63);
     Ql_FileSeek(filehandle1,10, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle1, strBuf,10,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"Writepointer=",Ql_strlen("Writepointer="));
     Ql_sprintf(textBuf,"%s",strBuf);
     }
     
      Ql_FileClose(filehandle1);
      filehandle1 = -1;
      Ql_memset(textBuf,0,63);
      Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
      
    }
}

void ql_entry()
{
    bool           keepGoing = TRUE;
   
    char buffer[100];
    char *p=NULL;
    char *p1=NULL;
    char* pdata;
    char *pfile;
    char *pData= NULL;
    char ct;
    u8 array[]="!*0109850421299,000000#*35A125460457733134900000022104802011100FF9999036664F1*#";
    u16 len1;
    u32 temp_readlen=0;
    
  //  u8 reset[]="!*0109850421299#*35A2833822277074093022616130425211210001E999903CBBA01*#";
    
   //u8 array[]="!*0109850421299#*AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGMUKESH*#";
    // u8 array[]="!*0109850421299#*AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGMUKESHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF













































    u8 at_var=0;
    s32 i=0;
     u32 ret,ret1;
    s32 k,j;
     u8 k1; 
    u8 process=0;
    QlSysTimer t;
    s32 period = 10;
    u32 filesize,fd_file;
    u8 *file_buffer=NULL;
    u16 off=0;
    s32 ret2;
    u8 updateItem = 0;
    bool initialized = FALSE;
    bool bgnCheck = FALSE;
    idxCmd = 0;
	msg_idxCmd = 0;
    Ql_sprintf(coreBinFile,"QUECTEL_CORE.bin");
    Ql_sprintf(appBinFile,"QL_CUSTOM_V01.bin");
    
    
     tm.timeoutPeriod = Ql_MillisecondToTicks(7000);
    
     statustimer.timeoutPeriod=Ql_SecondToTicks(240);
     fota_timer.timeoutPeriod=Ql_SecondToTicks(3);
     filetimer.timeoutPeriod=Ql_SecondToTicks(60);
     versiontimer.timeoutPeriod=Ql_SecondToTicks(15);
	 PulseTimer.timeoutPeriod=Ql_SecondToTicks(10);
	 AdcTimer.timeoutPeriod=Ql_SecondToTicks(300);
	 SpeedCalTimer.timeoutPeriod=Ql_SecondToTicks(10);
     Ql_StartTimer(&filetimer);
	 Ql_StartTimer(&PulseTimer);
	 Ql_StartTimer(&AdcTimer);
	 Ql_StartTimer(&SpeedCalTimer);
     
     
	   
     
	 
	 Ql_StartTimer(&tm);
    
     
   // Ql_DebugTrace("The timer starts.\r\n\r\n");
    Ql_OpenModemPort(ql_md_port1);
     bgnCheck = TRUE;
    idxCmd = 0;
	msg_idxCmd=0;
    at_response[0]='\0';

	key_buffer[0]='0';
	key_buffer[1]='0';

	hex_input[0]='F';
	hex_input[1]='F';
    RESETFLAG(PANIC_PRESS_DETECTED);
    Ql_SetDebugMode(BASIC_MODE);
    Ql_SetUartBaudRate(ql_uart_port1,9600); 
    Ql_SetUartBaudRate(ql_uart_port2,9600);
    Ql_DebugTrace("Tracking Application\r\n");
    
     Ql_UartForceSendEscape(ql_uart_port1);
     Ql_UartForceSendEscape(ql_uart_port2);
	 Ql_UartClrTxBuffer(ql_uart_port2);
    Ql_UartClrRxBuffer(ql_uart_port2); 
    
    Ql_SendToUart(ql_uart_port1,(u8*)"port1\n",Ql_strlen("port1\n"));
    Ql_SendToUart(ql_uart_port2,(u8*)"port2\n",Ql_strlen("port2\n"));
	Ql_SendToUart(ql_uart_port2,(u8*)"UART SECOND PORT\r\n",Ql_strlen("UART SECOND PORT\r\n"));

   
	 Ql_UartClrTxBuffer(ql_uart_port2);
    Ql_UartClrRxBuffer(ql_uart_port2); 
           
	//Reset_GPS(1);
	
	
    
  


	Create_WavePointFile();
	//Settings file Creation.
	ret=Ql_FileCheck((u8*)"settings.txt");
    
    if(ret!=QL_RET_OK)
    { 
    p1="Ql_FileOpen=csettings.txt";
    p = Ql_strstr(p1,"Ql_FileOpen=c");
    if (p)
    {
      // s32 ret;
      p = Ql_strstr(p1,"=c");
      p += 2;
      ret = Ql_FileOpen((u8*)p ,TRUE, FALSE);
      if(ret >= QL_RET_OK)
      filehandle2 = ret;
      Ql_sprintf(textBuf,"Ql_FileOpen NEW (%s)=%d\r\n", p, ret);
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
      
     }

	
    p=port1_buffer;
     if(p)
     {
      u32 writeedlen;
      u32 readedlen;
	  u8 apn_length=0;
     Ql_memset(textBuf,0,63);
    
     ret = Ql_FileWrite(filehandle2, (u8*)p,4, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
     
     p=address1_buffer;
	 Ql_FileSeek(filehandle2,4, QL_FS_FILE_BEGIN);
      
     ret = Ql_FileWrite(filehandle2, (u8*)p,15, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));

	 p=timer_update_rate;
	 Ql_FileSeek(filehandle2,19, QL_FS_FILE_BEGIN);
      
     ret = Ql_FileWrite(filehandle2, (u8*)p,3, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));

	 p=owen_number;
	 Ql_FileSeek(filehandle2,22, QL_FS_FILE_BEGIN);
      
     ret = Ql_FileWrite(filehandle2, (u8*)p,10, &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));

	 p=APN;
	 Ql_FileSeek(filehandle2,32, QL_FS_FILE_BEGIN);
     apn_length=Ql_strlen(APN); 
     ret = Ql_FileWrite(filehandle2, (u8*)p,(apn_length+1), &writeedlen);
     Ql_sprintf(textBuf,"Ql_FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
     Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
     
     
     Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,0, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,4,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"Port_Number=",Ql_strlen("Port_Number="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
     
     Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,4, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,15,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"IP_Address=",Ql_strlen("IP_Address="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,19, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,3,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"Timer_Update_Rate=",Ql_strlen("Timer_Update_Rate="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));


	 /*Ql_memset(strBuf,0,53);
     Ql_memset(textBuf,0,53);
     Ql_FileSeek(filehandle2,19, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,15,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,"IP2_Address=",Ql_strlen("IP2_Address="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,"\r\n",Ql_strlen("\r\n"));*/

	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,22, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,10,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"OWN_Number=",Ql_strlen("OWN_Number="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,32, QL_FS_FILE_BEGIN);
	 ret = Ql_FileRead(filehandle2, strBuf,30,&readedlen);
	 
	 for(apn_length=0;strBuf[apn_length]!='\0';apn_length++)
	 {
	 Apn_Setting_Buffer[apn_length]=strBuf[apn_length];
	 }
	 Apn_Setting_Buffer[apn_length]='\0';

	 // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"APN_Setting=",Ql_strlen("APN_Setting="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));


     }
     
      Ql_FileClose(filehandle2);
      filehandle2 = -1;
      Ql_memset(textBuf,0,63);
      Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));    
      
    }

	else
	{
	  u32 readedlen;
	  u8 apn_length=0;
	 OpenSetting_File();

	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,0, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,4,&readedlen);

	 port= (((strBuf[0]-0x30)*1000)+((strBuf[1]-0x30)*100)+((strBuf[2]-0x30)*10)+((strBuf[3]-0x30)*1));
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"Port_Number=",Ql_strlen("Port_Number="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

	 
     
     Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,4, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,15,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);

	 address[0]=(((strBuf[0]-0x30)*100)+((strBuf[1]-0x30)*10)+((strBuf[2]-0x30)*1));
	 address[1]=(((strBuf[4]-0x30)*100)+((strBuf[5]-0x30)*10)+((strBuf[6]-0x30)*1));
	 address[2]=(((strBuf[8]-0x30)*100)+((strBuf[9]-0x30)*10)+((strBuf[10]-0x30)*1));
	 address[3]=(((strBuf[12]-0x30)*100)+((strBuf[13]-0x30)*10)+((strBuf[14]-0x30)*1));

     Ql_SendToUart(ql_uart_port1,(u8*)"IP_Address=",Ql_strlen("IP_Address="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,19, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,3,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
	 Update_Timer_Variable=((((strBuf[0]-0x30)*100)+((strBuf[1]-0x30)*10)+((strBuf[2]-0x30)*1))/10);
     Ql_SendToUart(ql_uart_port1,(u8*)"Timer_Update_Rate=",Ql_strlen("Timer_Update_Rate="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

	 Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,22, QL_FS_FILE_BEGIN);
     ret = Ql_FileRead(filehandle2, strBuf,10,&readedlen);
     // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
	 owen_number[0]=strBuf[0];
	 owen_number[1]=strBuf[1];
	 owen_number[2]=strBuf[2];
	 owen_number[3]=strBuf[3];
	 owen_number[4]=strBuf[4];
	 owen_number[5]=strBuf[5];
	 owen_number[6]=strBuf[6];
	 owen_number[7]=strBuf[7];
	 owen_number[8]=strBuf[8];
	 owen_number[9]=strBuf[9];

     Ql_SendToUart(ql_uart_port1,(u8*)"OWN_Number=",Ql_strlen("OWN_Number="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

      Ql_memset(strBuf,0,63);
     Ql_memset(textBuf,0,63);
     Ql_FileSeek(filehandle2,32, QL_FS_FILE_BEGIN);
	 ret = Ql_FileRead(filehandle2, strBuf,30,&readedlen);
	 for(apn_length=0;strBuf[apn_length]!='\0';apn_length++)
	 {
	 Apn_Setting_Buffer[apn_length]=strBuf[apn_length];
	 }
	 Apn_Setting_Buffer[apn_length]='\0';

	 // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
     Ql_SendToUart(ql_uart_port1,(u8*)"APN_Setting=",Ql_strlen("APN_Setting="));
     Ql_sprintf(textBuf,"%s",strBuf);
	 Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));
	 Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));
     
      Ql_FileClose(filehandle2);
      filehandle2 = -1;
      Ql_memset(textBuf,0,63);
      Ql_sprintf(textBuf,"Ql_FileClose()\r\n");
      Ql_SendToUart(ql_uart_port1,(u8*)textBuf,Ql_strlen(textBuf));   
	}
       
    
   
                        

    Ql_memset(data_p, 0, DATA_LEN);
    
   // data_p[0] = 'B';
   // data_p[1] = 'e';
   // data_p[2] = 'g';
   // data_p[3] = 'i';
   // data_p[4] = 'n';
    
   // data_p[DATA_LEN-3] = 'E';
   // data_p[DATA_LEN-2] = 'n';
   // data_p[DATA_LEN-1] = 'D';

   // Ql_SetDebugMode(BASIC_MODE);
   // Ql_DebugTrace("tcpip: ql_entry\r\n");

   // Ql_GprsNetworkInitialize(0,  3, &callback_func);
     GPIOPin_Config();
   	 Power_Supply_Pulse();
    Ql_GprsNetworkInitialize(1,  5, &callback_func);
    
    
    /*****************This Portaion for skiped for sms dota*******************/
    for(at_var=0;at_var<20;at_var++)
    at_response[at_var]=0; 
    
    at_response[0]='N';
    at_response[1]='O';
    at_response[2]='T';

	for(at_var=0;at_var<100;at_var++)
    at_Message_response[at_var]='\0'; 
    
    timer1min.timeoutPeriod=Ql_SecondToTicks(Update_Timer_Variable);
   /************************end**********************************************/

   SendGPS_Setting();
   
    while(keepGoing)
    {
        Ql_GetEvent(&flSignalBuffer);
        switch(flSignalBuffer.eventType)
        {
          	
          	
          	case EVENT_INTR:
            {
                //the EVENT will report when you execute command 3.
                Ql_sprintf(buffer, "\r\nEVENT_INTR=pinName(%d),pinState=%d\r\n",flSignalBuffer.eventData.intr_evt.pinName,flSignalBuffer.eventData.intr_evt.pinState);
                Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
                break;
            }
          	
          	
          	case EVENT_KEY:
            {
                Ql_memset(buffer,0,100);
                Ql_SendToUart(ql_uart_port1,(u8*)"Enter Into Key Event\r\n",Ql_strlen("Enter Into Key Event\r\n")); 
                Ql_sprintf(buffer, "\r\nEVENT_KEY=key_val(%x),isPressed=%d\r\n",flSignalBuffer.eventData.key_evt.key_val,flSignalBuffer.eventData.key_evt.isPressed);
                Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));  
				ispressed_varible=flSignalBuffer.eventData.key_evt.isPressed;
               // Ql_SendToUart(ql_uart_port1,&data,1);
                
                if(ispressed_varible==0x00)
                { 
                temp_key=flSignalBuffer.eventData.key_evt.key_val;
				switch(temp_key)
				{
				case 0x00:
				New_temp_key=0x06;
				break;
				case 0x10:
				New_temp_key=0x01;
				break;
				case 0x11:
				New_temp_key=0x07;
				break;
				case 0x12:
				New_temp_key=0x09;
				break;
				case 0x30:
				New_temp_key=0x08;
				break;
				case 0x31:
				New_temp_key=0x02;
				break;
				case 0x01:
				case 0x02:
				case 0x20:
				case 0x21:
				New_temp_key=0x00;
				break;
				}
                data_key=(New_temp_key & 0xff);
                Ql_SendToUart(ql_uart_port1,&data_key,1);  
                ispressed_varible=0xff;
				key_buffer[0]=(0x30 |((data_key & 0xF0)>>4));
				key_buffer[1]=(0x30 |(data_key & 0x0F));
				key_send();
                }
                
                break;
            }
            
            case EVENT_MODEMDATA:
            {                
                Ql_DebugTrace("\r\nEVENT_MODEMDATA type=%d \r\n",flSignalBuffer.eventData.modemdata_evt.type);
                
               
                 if (flSignalBuffer.eventData.modemdata_evt.type==DATA_AT)
                {
                    Ql_DebugTrace("\r\nModem data =%s\r\n",flSignalBuffer.eventData.modemdata_evt.data);
                    pData = (char*)flSignalBuffer.eventData.modemdata_evt.data;
                    Ql_SendToUart(ql_uart_port1,(u8*)pData,Ql_strlen(pData));
                    
                    
                    
					
					
					
					
					
					
					
					
					
					
					
					if((Ql_strstr((char*)flSignalBuffer.eventData.modemdata_evt.data,"Call Ready\r\n") != NULL))
                    {
                   // idxCmd = 1;
                   // SendAtCmd();																			
                    }
					if((Ql_strstr((char*)flSignalBuffer.eventData.modemdata_evt.data,"+CMTI:") != NULL))
                    {

				
					
					while(*pData !='\0')
					{
					 
					 if(*pData==',')
					 {
					 	pData++;
						Message_No=*pData;
						Ql_SendToUart(ql_uart_port1,(u8*)"Message_No:",Ql_strlen("Message_No:"));
						Ql_SendToUart(ql_uart_port1,(u8*)&Message_No,1);
						Ql_SendToUart(ql_uart_port1,(u8*)"\r\n",Ql_strlen("\r\n"));

						if(Message_No>='1' && Message_No<='9')
						{
						MESSAGE_RECEIVED_FLAG=TRUE;
						msg_idxCmd=1;
						SendAtCmd_Message_Read();
						}
					 }
					
					 pData++;
					 
					}
                   // idxCmd = 1;
                   // SendAtCmd();																			
                    }

					if(MESSAGE_RECEIVED_FLAG==TRUE)
					{
					
					if((Ql_strstr((char*)flSignalBuffer.eventData.modemdata_evt.data,"+CMGR:") != NULL))
                      
                    {
            	 
            	 	    //Ql_SendToUart(ql_uart_port1,"SENDING MESSAGE COMMAND\r\n",Ql_strlen("SENDING MESSAGE COMMAND\r\n"));
                        //SendAtCmd();
                        //msg_idxCmd++;
						u8 msg_length=0;
						//Ql_SendToUart(ql_uart_port1,pData,Ql_strlen(pData));
						
						while(*pData!='\0')
						{						
							Message_Buffer[msg_length++]=*pData;
							pData++;

						}
						Message_Buffer[msg_length]='\0';
						
						msg_length=0;


						while(Message_Buffer[msg_length]!='#')
						{
						  
						  msg_length++;
						}
						if(Message_Buffer[msg_length]=='#' && Message_Buffer[msg_length+1]=='*')
						{
						setting_length_counter=0;
						while(Message_Buffer[msg_length]!='\0')
						{
							
							Uart1_Setting_Buffer[setting_length_counter++]=Message_Buffer[msg_length];
							msg_length++;
							if(Message_Buffer[msg_length-2]=='*' && Message_Buffer[msg_length-1]=='#')
							{
							MESSAGE_SETTING_OK=TRUE;
							Uart1_Setting_Buffer[setting_length_counter++]=0x03;
							Uart1_Setting_Buffer[setting_length_counter]='\0';
							break;
							}
						
						}
						
						if(MESSAGE_SETTING_OK==TRUE)
						{
						Ql_SendToUart(ql_uart_port1,(u8*)Uart1_Setting_Buffer,setting_length_counter);
						}
						else
						{
						Ql_SendToUart(ql_uart_port1,(u8*)"You Have Sent Wrong Message Setting.Please Send Correct.\r\n",Ql_strlen("You Have Sent Wrong Message Setting.Please Send Correct.\r\n"));
						}
						}

					
						SendAtCmd_Message_Read();
                       	msg_idxCmd++;
                        
                        //for(at_var=0;at_var<20;at_var++)
                        //at_response[at_var]=0; 
                         
                    }
					
					else if((Ql_strstr((char*)flSignalBuffer.eventData.modemdata_evt.data, at_Message_response) != NULL))
                      
                    {
            	 		u8 temp_var=0;
            	 	    Ql_SendToUart(ql_uart_port1,(u8*)"SENDING MESSAGE COMMAND\r\n",Ql_strlen("SENDING MESSAGE COMMAND\r\n"));
						
						if(MESSAGE_DELETE_COMMAND==TRUE)
						{
						 MESSAGE_DELETE_COMMAND=FALSE;
						 Uart1_setting_received();
						 for(temp_var=0;temp_var<50;temp_var++)
						 Uart1_Setting_Buffer[temp_var]='\0';
						 MESSAGE_RECEIVED_FLAG==FALSE;
						}
						else
						{
						SendAtCmd_Message_Read();
                       	msg_idxCmd++;
						}
                        
                        //for(at_var=0;at_var<20;at_var++)
                        //at_response[at_var]=0; 
                         
                    }
					else if((Ql_strstr((char*)flSignalBuffer.eventData.modemdata_evt.data, "ERROR") != NULL))
                    {
                    Ql_SendToUart(ql_uart_port1,(u8*)"WRONG ENTRY\r\n",Ql_strlen("WRONG ENTRY\r\n"));
                    msg_idxCmd--;
                    SendAtCmd_Message_Read();
                    msg_idxCmd++;
                    }
						
					}
                    
					if(FTP_START_FLAG==TRUE)
					{
					if((Ql_strstr((char*)flSignalBuffer.eventData.modemdata_evt.data, at_response) != NULL))
                      
                    {
            	 
            	 	    Ql_SendToUart(ql_uart_port1,(u8*)"SENDING FTP COMMAND\r\n",Ql_strlen("SENDING FTP COMMAND\r\n"));
                        SendAtCmd();
                        idxCmd++;
                        
                        //for(at_var=0;at_var<20;at_var++)
                        //at_response[at_var]=0; 
                         
                    }
                    
                    else if((Ql_strstr((char*)flSignalBuffer.eventData.modemdata_evt.data, "ERROR") != NULL))
                    {
                    Ql_SendToUart(ql_uart_port1,(u8*)"WRONG ENTRY\r\n",Ql_strlen("WRONG ENTRY\r\n"));
                    idxCmd--;
                    SendAtCmd();
                    idxCmd++;
                    }
                   
                     else if((Ql_strstr((char*)flSignalBuffer.eventData.modemdata_evt.data, "+QFTPGET:") != NULL))
                    {
                   // idxCmd = 1;
                    //SendAtCmd();
                    Ql_SendToUart(ql_uart_port1,(u8*)"File Downloaded Succesfully\r\n",Ql_strlen("File Downloaded Succesfully\r\n"));
                    //Ql_StartTimer(&timer); 
                    Ql_StartTimer(&fota_timer);
    
               		Ql_SendToUart(ql_uart_port1,(u8*)"Timer Started\r\n",Ql_strlen(buffer));
                    // ret = Ql_GprsAPNSet(5, (u8*)"airtelgprs.com", (u8*)"", (u8*)"", CallBack_GprsAPNSet);
         			 // ret = Ql_GprsAPNSet(5, (u8*)"internet", (u8*)"", (u8*)"", CallBack_GprsAPNSet);
           			// OUT_DEBUG(textBuf,"Ql_GprsAPNSet(%d)=%d\r\n",5,ret);
                    }
					}

					
                   
                    
                    
                }
                break;
            }
            
            
            case EVENT_UARTDATA:
            {
                if (flSignalBuffer.eventData.uartdata_evt.len>0)
                {
                    s32              pin;
                    s32 mod = 0;
                    s32 pinpullenable = 0;
                    s32 pindirection = 0;
                    s32 pinlevel = 0;
                    s32 iret;
                   
                    u16 len=0;

                    pData = (char*)flSignalBuffer.eventData.uartdata_evt.data;
                    pData[flSignalBuffer.eventData.uartdata_evt.len+1] = '\0';

                    //command-->select QL_PINNAME_DTR pin function to mode1, it is DTR function
                    if(flSignalBuffer.eventData.uartdata_evt.port==2)
                    {
                    	u8 temp_var=0;
						Ql_sprintf(buffer,"\r\nEVENT_UARTDATA PORT=%d\r\n",flSignalBuffer.eventData.uartdata_evt.port);
                    	Ql_SendToUart(ql_uart_port1,(u8*)pData,Ql_strlen(pData));
                    	
                    	
                    	while(*pData!='\0')
                    	{
                    		Uart1_Setting_Buffer[setting_length_counter++]=*pData;

							if(*pData==0x03)
                    		{
                    		Uart1_setting_received();
							 for(temp_var=0;temp_var<50;temp_var++)
							 Uart1_Setting_Buffer[temp_var]='\0';
							 break;
                    		}
                    		pData++;
							
							
                    	}
                    	
                    	/*if(*pData==0x02)
                  		 Uart1_Event_variable=0;
                   
                   		while(*pData!='\0')
                   		{
                   		Uart1_Event_Buffer[Uart1_Event_variable]=*pData;
                   		pData++;
                   
                   		if(Uart1_Event_Buffer[Uart1_Event_variable]==0x03)
                   		{
                   		Uart1_Event_variable++;
                   		Uart1_Event_Buffer[Uart1_Event_variable]='\0';
                   		//Ql_SendToUart(ql_uart_port1,GPSString,Ql_strlen(GPSString));
                   		Uart1_Event_Local_variable=0;
                    	while(Uart1_Event_Buffer[Uart1_Event_Local_variable]!='\0')
                    	{
                    	Uart1_Event_Local_Buffer[Uart1_Event_Local_variable]=Uart1_Event_Buffer[Uart1_Event_Local_variable];
                    	Uart1_Event_Local_variable++;
                   		}
                    	Uart1_Event_Local_Buffer[Uart1_Event_Local_variable]='\0';
                    	
                    	len=Ql_strlen(Uart1_Event_Local_Buffer);
                    	uart1_datahandler(len,Uart1_Event_Local_Buffer);
                   		}
                   		else
                   		Uart1_Event_variable++;
                   		}*/
                    	
                    	
                    }
                    else if(flSignalBuffer.eventData.uartdata_evt.port==3)
                    {
                    //Ql_DebugTrace("\r\nEVENT_UARTDATA PORT=%d\r\n",flSignalBuffer.eventData.uartdata_evt.port);
                    //Ql_sprintf(buffer,"\r\nEVENT_UARTDATA PORT=%d\r\n",flSignalBuffer.eventData.uartdata_evt.port);
                    //Ql_SendToUart(ql_uart_port1,buffer,Ql_strlen(buffer));  
                   // Ql_SendToUart(ql_uart_port1,pData,Ql_strlen(pData));
                    
                   /* 
                    c=0;
                     //Ql_memset(GPSString,0,82);
                     
                    //if(gprsupdate_flag)
                    { 
                    len=Ql_strlen(pData);
                    GPRS_StringLength=len;
                    while(*pData!='\0')
                    {
                    GPSString[c]=*pData;
                    pData++;
                    c++;
                    }
                    GPSString[c]='\0';
                   // gprsupdate_flag=0;               
                    }
                    Ql_SendToUart(ql_uart_port1,GPSString,Ql_strlen(GPSString));*/
                    //Ql_SendToUart(ql_uart_port1,"\r\n",2);
                   // Ql_memset(localdata,0,82);
                   
                   
                   
                   if(*pData=='$')
                   c=0;
                   
                   while(*pData!='\0')
                   {
                   GPSString[c]=*pData;
                   pData++;
                   
                   if(GPSString[c]==0x0a)
                   {
                   c++;
                   GPSString[c]='\0';

				   if(GPRMC_STRING_FLAG==TRUE)
				   {
                   Ql_SendToUart(ql_uart_port1,(u8*)GPSString,Ql_strlen(GPSString));
				   }
                   
                    k1=0;
                    while(GPSString[k1]!='\0')
                    {
                    localdata[k1]=GPSString[k1];
                    k1++;
					if(k1>=250)
					break;
                    }
                    localdata[k1]='\0';
					uart2_datahandler(Ql_strlen(localdata),localdata);                   
                   }
                   else
				   {
                   c++;
				   if(c>=250)
				   break;
				   }
                   }
                     
                    //Ql_SendToUart(ql_uart_port1,"\r\n",2);
                   // Ql_memset(localdata,0,82);
                   
                   
                  /*  k1=0;
                    while(GPSString[k1]!='\0')
                    {
                    localdata[k1]=GPSString[k1];
                    k1++;
                    }
                    localdata[k1]='\0';
                    len1=GPRS_StringLength;*/
                    
                   // Ql_SendToUart(ql_uart_port1,(u8 *)localdata ,53);
                   Ql_UartClrRxBuffer(ql_uart_port2);
                   /* if(gpscount>10)
                    {
                    for(k1=0;k1<=GPRS_StringLength;k1++)
                    localdata[k1]=GPSString[k1];
                    len1=GPRS_StringLength;
                    gpscount=0;
                    }
                    else
                    gpscount++;*/
                   // uart2_datahandler(Ql_strlen(GPSString),GPSString); 
                   
                 
                    
                    }
                    
                
                }
                break;
               }            
            
            case EVENT_TIMER:
            
             //
            //s32 ret;
            
            if( tm.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
            {
            
           
            Ql_StopTimer(&tm);
            ret = Ql_GprsAPNSet(5, (u8*)Apn_Setting_Buffer, (u8*)"", (u8*)"", CallBack_GprsAPNSet);
            //  ret = Ql_GprsAPNSet(5, (u8*)"internet", (u8*)"", (u8*)"", CallBack_GprsAPNSet);
            OUT_DEBUG((char*)textBuf,(char*)"Ql_GprsAPNSet(%d)=%d\r\n",5,ret);
           
            }
            
            else if( filetimer.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
            {
            
           
            Ql_StopTimer(&filetimer);
             filesystem_function();
             Ql_StartTimer(&filetimer);
            }
            else if(timer1min.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
            {
            	  
            	Ql_StopTimer(&timer1min); 
                // Ql_sprintf(textBuf,"Ql_FileRead()=%d: readedlen=%d, strBuf=%s\r\n",0,80,GPSString);
                
                //Ql_SendToUart(ql_uart_port1,GPSString,Ql_strlen(GPSString));  
               
                //gprsupdate_flag=1;             
                //len1=GPRS_StringLength;
                //for(k1=0;k1<=GPRS_StringLength;k1++)
                //localdata[k1]=GPSString[k1];
          			
         
                 //uart2_datahandler(len1,localdata); 
                //uart2_datahandler(GPRS_StringLength,GPSString); 
                //gprsupdate_flag=1;
            	  //PrepareString(data_p);
            	  
                  //uart2_datahandler(Ql_strlen(localdata),localdata);   
            	  
            	  
            	  Not_Parameter_Update=TRUE;
            	  filesystem_function();
            	   
           
                   
                    
                    
                    
            }
            else if(versiontimer.timerId == flSignalBuffer.eventData.timer_evt.timer_id)
            {
            
            	 Send_GSMVersionString();
            	 Ql_StopTimer(&versiontimer);
            }
            else if(statustimer.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
            {
            	if(status_check=='&')
            	{
            	Ql_StartTimer(&statustimer);
            	status_check='\0';
            	}
            	else
            	{
            		Ql_Reset(0);
            	}
            }
			else if(IpSettingTimer.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
            {
            	
            	{
					Ql_StopTimer(&IpSettingTimer);
            		Ql_Reset(0);
            	}
            }

			else if(PulseTimer.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
            {
            	   Ql_StopTimer(&PulseTimer);
				   Power_Supply_Pulse();
				   Ql_StartTimer(&PulseTimer);
            	
            }

			else if( AdcTimer.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
            {
             QlADCPin adcPin;
			 adcPin = QL_PIN_ADC0;
           
             Ql_StopTimer(&AdcTimer);
             ret2 = Ql_ReadADC(adcPin, Callback_Read_ADC);
			 AdcFunction();
             Ql_StartTimer(&AdcTimer);
            }
			else if( SpeedCalTimer.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
            {
            
           
             Ql_StopTimer(&SpeedCalTimer);
			 distance_calc_TimerHandler();
             SpeedCalTimer.timeoutPeriod=Ql_SecondToTicks(3);
             Ql_StartTimer(&SpeedCalTimer);
            }
            
            //fota
            else if(fota_timer.timerId  == flSignalBuffer.eventData.timer_evt.timer_id)
            {
               
               
                Ql_SendToUart(ql_uart_port1,(u8*)"Timer expired\r\n",Ql_strlen(buffer));
                
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
                        if (filesize<=512)
                        {
                            Ql_FileRead(fd_file,(u8*)file_buffer, filesize,&temp_readlen);
                            Ql_Fota_Core_Write_Data(filesize,(s8*)file_buffer);
                            filesize=0;
                        }else
                        {
                            Ql_FileRead(fd_file,(u8*)file_buffer, 512,&temp_readlen);                    
                            Ql_Fota_Core_Write_Data(512,(s8*)file_buffer);
                            filesize-=512;                    
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
                    Ql_Fota_App_Init(FOTA_APP_COVER);            
        
                    
                    while(filesize>0)
                    {
                        if (filesize<=512)
                        {
                            Ql_FileRead(fd_file,(u8*)file_buffer, filesize,&temp_readlen);
                            Ql_Fota_App_Write_Data(filesize,(s8*)file_buffer);
                            filesize=0;
                        }else
                        {
                            Ql_FileRead(fd_file,(u8*)file_buffer, 512,&temp_readlen);                    
                            Ql_Fota_App_Write_Data(512,(s8*)file_buffer);
                            filesize-=512;                    
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
            
            break;
            
               
             
         }
		 GPIO_ReadFunction();
         
         
 }
Ql_GprsNetworkUnInitialize(1); 

}
   
  


void CallBack_GprsAPNSet(bool result, s32 error_code)
{
    OUT_DEBUG((char *)textBuf,(char *)"CallBack_GprsAPNSet(result=%d,error_code=%d)\r\n",result,error_code);
    if(result==TRUE)
    {
    			//p2="APNGet=5";
    			 //p3 = Ql_strstr(pData,"APNGet=");
                //if (p3)
                //{
                    s32 ret;
                  //  extern int atoi(const char*);
                    //p3 = Ql_strstr(pData,"=");
                    //p3 += 1;
                    ret = Ql_GprsAPNGet(5, CallBack_GprsAPNGet);
                    OUT_DEBUG((char *)textBuf,(char *)"Ql_GprsAPNGet(%d)=%d\r\n",5,ret);
       
               // }
    }
    
}

void CallBack_GprsAPNGet(u8 profileid, bool result, s32 error_code, u8 *apn, u8 *userId, u8 *password)
{
    OUT_DEBUG((char *)textBuf,(char *)"CallBack_GprsAPNGet(profileid=%d,result=%d,error_code=%d,apn=%s,userId=%s,password=%s)\r\n",profileid,result,error_code,apn,userId,password);
    
    if(result=TRUE)
    {
   					 s32 ret;
                    //extern int atoi(const char*);
                    //p = Ql_strstr(pData,"=");
                    //p += 1;
                    ret = Ql_GprsNetworkActive(1);
                    OUT_DEBUG((char *)textBuf,(char *)"Ql_GprsNetworkActive(contxtid=%d)=%d\r\n",1,ret);
    }
}

void CallBack_network_actived(u8 contexid)
{
    s8 ret;
    u8 ip_addr[4];
    
   OUT_DEBUG((char *)textBuf,(char *)"CallBack_network_actived(contexid=%d)\r\n",contexid);

    ret = Ql_GetLocalIpAddress(contexid, ip_addr);
   OUT_DEBUG((char *)textBuf,(char *)"Ql_GetLocalIpAddress(contexid=%d)=%d, ip=%d.%d.%d.%d\r\n",contexid, ret,ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
    switch(ret)
    {
    	case QL_SOC_SUCCESS:
    				//s32 ret;
                   // OpenCpuNetWorkState_e networkstate;
                   // u8 ps_status;
                   // extern int atoi(const char*);
                    
                   // p = Ql_strstr(pData,"=");
                   // p += 1;
                    ret = Ql_SocketCreate(1, 0);
                    OUT_DEBUG((char *)textBuf,(char *)"Ql_SocketCreate(contxtid=%d)=%d\r\n",1,ret);
                    if(ret < 0)
                    {
                        OUT_DEBUG((char *)textBuf,(char *)"failed to create socket\r\n");
                        break;
                    }

                   socketonly[1] = ret;

                    ret = Ql_SocketConnect(ret, address, port);
                    OUT_DEBUG((char *)textBuf,(char *)"Ql_SocketConnect(ip=%d,%d,%d,%d,port=%d)=%d\r\n",address[0],address[1],address[2],address[3],port,ret);
                    if(ret == 0)
                    {
                        OUT_DEBUG((char *)textBuf,(char *)"Ql_SocketConnect connect successed\r\n");
                    }
                    else if(ret == -2)
                    {
                        OUT_DEBUG((char *)textBuf,(char *)"please wait CallBack_socket_connect\r\n");
                    }
                    else
                    {
                        OUT_DEBUG((char *)textBuf,(char *)"Ql_SocketConnect run error\r\n");
                    }
                    
                    break;
             
    }
}

void CallBack_network_deactived(u8 contexid, s32 error_cause, s32 error)
{
    OUT_DEBUG((char *)textBuf,(char *)"CallBack_network_deactived(contexid=%d,error_cause=%d, error=%d)\r\n",contexid,error_cause,error);
    if(socketonly[0] >= 0)
    {
        Ql_SocketClose(socketonly[0]);
        socketonly[0] = 0xFF;
        send_dataLen[0]  = 0;
    }
    if(socketonly[1] >= 0)
    {
        Ql_SocketClose(socketonly[1]);
        socketonly[1] = 0xFF;
        send_dataLen[1]  = 0;
    }

    if(udponly[0] >= 0)
    {
        Ql_SocketClose(udponly[0]);
        udponly[0] = 0xFF;
        send_dataLen[0]  = 0;
    }
    if(udponly[1] >= 0)
    {
        Ql_SocketClose(udponly[1]);
        udponly[1] = 0xFF;
        send_dataLen[1]  = 0;
    }
    
    GPRS_CONNECTION=FALSE; 
    
    
    //tm.timeoutPeriod = Ql_MillisecondToTicks(7000);
    //Ql_StartTimer(&tm);
    
    Ql_StartTimer(&statustimer);

    
}


void CallBack_getipbyname(u8 contexid, bool result, s32 error, u8 num_entry, u8 *entry_address)
{
    u8 i;
    OUT_DEBUG((char *)textBuf,(char *)"CallBack_getipbyname(contexid=%d, result=%d,error=%d,num_entry=%d)\r\n",contexid, result,error,num_entry);
    for(i=0;i<num_entry;i++)
    {
        entry_address += (i*4);
      OUT_DEBUG((char *)textBuf,(char *)"entry=%d, ip=%d.%d.%d.%d\r\n",i,entry_address[0],entry_address[1],entry_address[2],entry_address[3]);
    }
}


void CallBack_socket_connect(u8 contexid, s8 sock, bool result, s32 error)
{
    s32 ret;
    OUT_DEBUG((char *)textBuf,(char *)"CallBack_socket_connect(contexid=%d,sock=%d,result=%d,error=%d)\r\n",contexid,sock,result,error);
    //now , here , you can use Ql_SocketSend to send data 
    
    		
    		
    				if(error==0)
    				{
					Global_sock=sock;
    				Ql_StopTimer(&filetimer);
    				Ql_StartTimer(&timer1min);
    				Ql_StartTimer(&statustimer);
					iret = Ql_pinWrite( QL_PINNAME_KBR4, QL_PINLEVEL_HIGH);
                    Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret, QL_PINNAME_KBR4);
                    Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
                    GPRS_CONNECTION=TRUE; 
                    Send_GSMVersionString();
                    Ql_StartTimer(&versiontimer);
				
    				// Ql_StartTimer(&smstimer);
    				}
    				else
    				{
    				 GPRS_CONNECTION=FALSE; 
    				Ql_Reset(0);
    				}
            
                  /*  if(socketonly[1]  == 0xFF)
                    {
                        OUT_DEBUG(textBuf,"socket not create\r\n");
                        return;
                    }

                    if(send_dataLen[1]  > 0)
                    {
                        OUT_DEBUG(textBuf,"socket now busy, sending\r\n");
                        return;
                    }
                    
                    
                    send_dataLen[1]  = 0;
                    dataLen[1]  = DATA_LEN;
                    do
                    {
                        ret = Ql_SocketSend(socketonly[1] , data_p + send_dataLen[1]  ,dataLen[1]  - send_dataLen[1] );
                        OUT_DEBUG(textBuf,"Ql_SocketSend(socket=%d,dataLen=%d)=%d\r\n",socketonly[1] ,dataLen[1] ,ret);
                        if(ret == (dataLen[1]  - send_dataLen[1] ))
                        {
                            //send compelete
                            send_dataLen[1]  = 0;
                            break;
                        }
                        else if((ret < 0) && (ret == -2)) 
                        {
                            //you must wait CallBack_socket_write, then send data;                                                    
                            break;
                        }
                        else if(ret < 0)
                        {
                            //error , Ql_SocketClose
                            Ql_SocketClose(socketonly[1]);
                            socketonly[1] = 0xFF;
                            send_dataLen[1]  = 0;
                            break;
                        }
                        else if(ret <= dataLen[1] )
                        {
                            send_dataLen[1]  += ret;
                            //continue send
                        }
                    }while(1);*/
                    
                    
                    
        
}
void CallBack_socket_close(u8 contexid, s8 sock, bool result, s32 error)
{
    
    
    GPRS_CONNECTION=FALSE; 
	
	OUT_DEBUG((char *)textBuf,(char *)"CallBack_socket_close(contexid=%d,sock=%d,result=%d,error=%d)\r\n",contexid,sock,result,error);
    Ql_SocketClose(sock);
    if(socketonly[0] == sock)
    {
        socketonly[0] = 0xFF;
        send_dataLen[0]  = 0;
    }
    if(socketonly[1] == sock)
    {
        socketonly[1] = 0xFF;
        send_dataLen[1]  = 0;
    }

	iret = Ql_pinWrite( QL_PINNAME_KBR4, QL_PINLEVEL_LOW);
    Ql_sprintf(buffer, "\r\nWriteHigh(%d),pin=%d\r\n",iret, QL_PINNAME_KBR4);
    Ql_SendToUart(ql_uart_port1,(u8*)buffer,Ql_strlen(buffer));
    
   
    
   // Ql_Reset(0);
}
void CallBack_socket_accept(u8 contexid, s8 sock, bool result, s32 error)
{
}
void CallBack_socket_read(u8 contexid, s8 sock, bool result, s32 error)
{
    s32 ret;
    u8 data_r[1024];
    s32 dataLen;
    u8 r1=0;
    u8 AND_RECEIVE=FALSE;
    u16 socket_data=0;
    bool istcp = TRUE;


    if(udponly[0] == sock)
    {
        istcp = FALSE;
    }
    if(udponly[1] == sock)
    {
        istcp = FALSE;
    }       
            
    
     OUT_DEBUG((char *)textBuf,(char *)"%s:CallBack_socket_read(contexid=%d,sock=%d,result=%d,error=%d)\r\n",istcp?"TCP":"UDP",contexid,sock,result,error);

    if(istcp) // tcp recv
    {
        do
        {
            ret = Ql_SocketRecv(sock, data_r, 1024);

            if((ret < 0) && (ret != -2))
            {
                OUT_DEBUG((char *)textBuf,(char *)"TCP Ql_SocketRecv()=%d, error exit\r\n",ret );
                Ql_SocketClose(sock); //you can close this socket
                if(socketonly[0] == sock)
                {
                    socketonly[0] = 0xFF;
                    send_dataLen[0]  = 0;
                }
                if(socketonly[1] == sock)
                {
                    socketonly[1] = 0xFF;
                    send_dataLen[1]  = 0;
                }            
            }
            else if(ret == -2)
            {
                //wait next CallBack_socket_read
            }
            else if(ret < 1024)
            {
                data_r[ret]='\0';
               
                if(data_r[0]=='#' && data_r[1]=='*')
                {
                	r1=0;
                	switch(data_r[2])
                	{
                	case 'O':
                	for(r1=0;r1<10;r1++)
                	owen_number[r1]=data_r[r1+3];
                	break;
                	case 'D':
                	Ql_StopTimer(&timer1min);
                	Ql_StopTimer(&statustimer);
                	Ql_SocketClose(sock);
                	ret = Ql_GprsNetworkDeactive(1);
                    OUT_DEBUG((char *)textBuf,(char *)"Ql_GprsNetworkDeactive(contxtid=%d)=%d\r\n",1,ret); 
					FTP_START_FLAG=TRUE;
                    idxCmd = 1;
                    Ql_SendToUart(ql_uart_port1,(u8*)"DOTA COMMAND RECEIVED\r\n",Ql_strlen("DOTA COMMAND RECEIVED\r\n"));
                    SendAtCmd();
                  //  data_r[2]=0; 
                	break;
                	}
                	
                }
                
                if(data_r[ret-1]=='&'||data_r[ret-1]=='@')
                status_check='&';
                OUT_DEBUG((char *)textBuf,(char *)"TCP sock(%d)len(%d):%s\r\n",sock,ret,data_r);
                socket_data=0;
                
                while(data_r[socket_data]!='\0')
                {
                	if(data_r[socket_data]=='&')
                	{
                	AND_RECEIVE=TRUE;	
                	break;
                	}
                	AND_RECEIVE=FALSE;
                	socket_data++;
                }
                
                if(AND_RECEIVE=TRUE)
                {
                	if(WritePositionStatus!=(ReadPositionStatus))
                	{
                	
                	FileSyatem_ReadFunction();
                	
                	}
                }
               
               // Ql_SocketSend(sock, data_p ,72);
                //wait next CallBack_socket_read
                
                
                
                break;
            }
            else if(ret == 1024)
            {
                data_r[ret]='\0';
                OUT_DEBUG((char *)textBuf,(char *)"TCP sock(%d)len(%d):%s\r\n",sock,ret,data_r);
                //continue loop recv data
            }
            else if(ret > 1024)
            {
                //do not here
            }
        }while(1);
    }
    else  // udp recv
    {
        u8 address_peer[4];
        u16 port_peer;

        do
        {
            ret = Ql_SocketRecvFrom(sock, data_r, 30, address_peer, &port_peer);

            if((ret < 0) && (ret == -2)) 
            {
                //you shoud wait next CallBack_socket_read
                break;
            }
            else if(ret < 0) 
            {
                OUT_DEBUG((char *)textBuf,(char *)"UDP Ql_SocketRecvFrom()=%d, error exit\r\n",ret );
                Ql_SocketClose(sock); //you can close this socket
                if(udponly[0] == sock)
                {
                    udponly[0] = 0xFF;
                    send_dataLen[0]  = 0;
                }
                if(udponly[1] == sock)
                {
                    udponly[1] = 0xFF;
                    send_dataLen[1]  = 0;
                }        
                break;
            }
            else 
            {
                OUT_DEBUG((char *)textBuf,(char *)"UDP: ip=%d.%d.%d.%d, port=%d\r\n",address_peer[0],address_peer[1],address_peer[2],address_peer[3],ret,port_peer);
                data_p[ret]='\0';
                OUT_DEBUG((char *)textBuf,(char *)"UDP: sock(%d)len(%d):%s\r\n",sock,ret,data_r);
            }
        }
        while(1);
    }

}
void CallBack_socket_write(u8 contexid, s8 sock, bool result, s32 error)
{
    s32 ret;
    s32 i;
     OUT_DEBUG((char *)textBuf,(char *)"CallBack_socket_write(contexid=%d,sock=%d,result=%d,error=%d)\r\n",contexid,sock,result,error);
    //now , here , you can continue use Ql_SocketSend to send data 

    for(i=0;i<2;i++)
    {
        if(socketonly[i] == sock)
        {
            break;
        }
    }

    if(i>=2)
    {
        OUT_DEBUG((char *)textBuf,(char *)"unknown socket\r\n");
        return;
    }
    
    while(send_dataLen[i] > 0)
    {
        ret = Ql_SocketSend(sock, data_p + send_dataLen[i] ,dataLen[i] - send_dataLen[i]);
        OUT_DEBUG((char *)textBuf,(char *)"Ql_SocketSend(socket=%d,dataLen=%d)=%d\r\n",socketonly[i],dataLen[i],ret);
        if(ret == (dataLen[i] - send_dataLen[i]))
        {
            //send compelete
            send_dataLen[i] = 0;
            break;
        }
        else if((ret < 0) && (ret == -2)) 
        {
            //you must wait next CallBack_socket_write, then send data;                                                    
            break;
        }
        else if(ret < 0)
        {
            //error , Ql_SocketClose
            Ql_SocketClose(sock); //you can close this socket
            socketonly[i] = 0xFF;
            send_dataLen[i]  = 0;
            break;
        }
        else if(ret <= dataLen[i])
        {
            send_dataLen[i] += ret;
            //continue send
        }
    }

}

void Callback_Read_ADC(QlADCPin adc_pin, u8 status, u16 adc_val)
{
			 char buffer[100];
             Ql_DebugTrace("adc: %d, status: %d, adcValue: %d\r\n", adc_pin, status, adc_val);
  			 Ql_sprintf(buffer,"adc: %d, status: %d, adcValue: %d\r\n", adc_pin, status, adc_val);
             Ql_SendToUart(ql_uart_port1, (u8*)buffer, Ql_strlen(buffer)); 
			 adc_value[3]=adc_val/1000;
			 adc_value[3]+=0x30;
			 adc_val%=1000;
			 adc_value[2]=adc_val/100;
			 adc_value[2]+=0x30;
			 adc_val%=100;
			 adc_value[1]=adc_val/10;
			 adc_value[1]+=0x30;
			 adc_val%=10;
			 adc_value[0]=adc_val;
			 adc_value[0]+=0x30;


			  
}

#endif // __EXAMPLE_TCPIP__

