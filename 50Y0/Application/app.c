#include "LPC214x.h"
#include "Types.h"
#include "Target.h"
#include "Irq.h"
#include "Timer.h"
#include "Uart.h"
#include "Lcd.h"
#include "Spi.h"
#include "ExtInt.h"
#include "WDT.h"
#include "Dataflash.h"
#include "MiscFunctions.h"
#include "CardReader.h"
#include "Keypad.h"
#include "Global.h"
#include "Meter.h"
#include "WDT.h"
#include <string.h>

uint8 Status;
uint8 LastStatus = 0;
int main( void )
{		
	static uint8 process_switch = 0;	
	uint8 temp;
	uint8 KeyStatusByte[2];
	uint8 MBFailStatus = 0;
	
	PINSEL0 = 0x00000000;	PINSEL1 = 0x00000000;	PINSEL2 = 0x00000000;
	IODIR0 = 0xFFFFFFFF;	IODIR1 = 0xFFFFFFFF;
	
	init_VIC();
	//WDTInit();
	//Reset Test P0.18
	PINSEL1 &= ~(0x0000C030);
	
	SET_PORT1_PIN_DIRN_OUT(1<<20);
	SET_PORT0_PIN_DIRN_OUT(1<<23);		
	SET_PORT0_PIN_DIRN_OUT(1<<18);

	SET_PORT0_PIN_HIGH(1<<23);
	SET_PORT1_PIN_HIGH(1<<20);
	
	SET_PORT0_PIN_LOW(1<<18);

	RESETFLAG(TEST_MODE);	

	//LIVEPULSE
	RESETFLAG(LIVE_PULSE);				
	livepulsetimer = MS_TIMER + 180000;
	KerbDuty = FALSE;
	CtrlDuty = FALSE;
	
	init_timer0();	enable_timer(0);	//Start Timer 0
	init_timer1();	enable_timer(1);	//Start Timer 1	

	LCD_POWER_OFF;	
	Delay(500);	
	LCD_POWER_ON;	//TURN LCD ON
	Delay(500);	
	for(temp = 0; temp < 8; temp++)
	{
		SET_PORT1_PIN_HIGH( LCD_DATA );
		LCD_CLK_LOW;	LCD_CLK_HIGH;
	}

	LCD_RS_LOW;		LCD_RW_LOW;		LCD_EN1_LOW;	LCD_EN2_LOW;	
	
	LCD_Init();		//LCD Init

	
	Keypad_Init();
	EINTInit();

	
	SETFLAG(MENU_LEVEL_ZERO);		
	//InitIOS();
	SPIInit();		//SPI Init
	DF_DisableBlkProt( );

	DF_ReadBytesFromPage( 0, &temp, 0,1 );
	if( temp != 3 )
	{
		temp = 3;
		DF_ErasePage( 0 );
		DF_WriteBytesToPage( 0, &temp, 0,1 );	
	}

	DF_ReadBytesFromPage( 13, &temp, 0,1 );
	if( temp != TRUE  && temp != FALSE )
	{
		ShiftOn = 3;
	}
	else
	{
		ShiftOn = temp;
	}
	/**********************************************************************/
	//READ GPS VERSION
	/**********************************************************************/	
	DF_ReadBytesFromPage( 12, &GPSVersion[0], 0,3 );
	if( GPSVersion[0] == 0xFF )
	{
		GPSVersion[0] = '0';	GPSVersion[1] = '0';	GPSVersion[2] = '0';
	}
	/**********************************************************************/
	   
	/**********************************************************************/
	//READ SMS LOCATION
	/**********************************************************************/		
	DF_ReadBytesFromPage( 11, &SMSLocation, 0,1 );
	if( SMSLocation > 10 )
    {
        DF_ErasePage(11);
        SMSLocation = 0;
        DF_WriteBytesToPage( 11, &SMSLocation, 0, 1 );
    }
	/**********************************************************************/
	
	/**********************************************************************/
	//READ Key Status
	/**********************************************************************/	
	DF_ReadBytesFromPage( 14, &KeyStatusByte[0], 0,2 );
	if( KeyStatusByte[0] == 0xFF && KeyStatusByte[1] == 0xFF )
	{
		KeyStatus = 0x00000000;
	}
	else
	{
		KeyStatus = ( uint32 )( KeyStatusByte[0] << 8 ) | ( uint32 )( KeyStatusByte[1] << 0 ); 
	}
	/**********************************************************************/

	/**********************************************************************/
	//READ Meter Status
	/**********************************************************************/	
	DF_ReadBytesFromPage( 15, &MeterDisabled, 0,1 );
	if( MeterDisabled == '0')
	{
		MeterDisabled = TRUE;
	}
	else
	{
		MeterDisabled = FALSE; 
	}
	/**********************************************************************/
	DF_ReadBytesFromPage( 16, &temp, 0,1 );
	if( temp != '1' )
	{
		DF_WriteBytesToPage( 16, "1000000AIRPORT FEES     :", 0,  26 );		
		DF_WriteBytesToPage( 16, "2008000CREDIT CARD FEES :", 28, 26 );		
		DF_WriteBytesToPage( 16, "3008000CONVENIENCE FEES :", 56, 26 );		
	}
	DF_ReadBytesFromPage( 60,&LastStatus, 0,1 );
	//senddelaytimer = 0;
	KeyRepeatTimer = 0;
	UART0Init(9600);
	UART1Init(57600);	
	Delay(3000);	
	temp = ETX;
	Delay(2000);
	//Power Up string commented currently	
	UART0Send("#*U*#",5);		UART0Send(&temp,1);
	Delay(2000);	
	
	if( 0 )
	{		
		//This Portion has been replaced by CopyToAckBuffer to send Version 3 times after dota
		UART0Send("#*V3.01*#",9);	UART0Send(&temp,1);	
	}
	
	CopyToAckBuffer( SMS_ACK, "V60A0", 5 );
	
	UART0Send("|FE000000ARM RESETY0000", 23 ); 
	UART0Send(&temp,1);

	SETFLAG(METER_STATUS);
	DisplayWelcomeScreen();
	RESETFLAG(WELCOME_SCREEN);
	RESETFLAG(FIRST_TIME);
	SETFLAG1(POWERSTATUS);
	RESETFLAG1(MONITORSTATUS);
	RESETFLAG1(CURRENTSTATUS);
	
	while(0)
	{
	DF_WriteBytesToPage( 16, "1008000AIRPORT FEES     :", 0,  26 );
		DF_ReadBytesFromPage( 16,(uint8*)&TempBuffer[0], 0, 26 );
					TempBuffer[ 26 ] = '\0';
					UART1Send(&TempBuffer[0],26);
					UART1Send("\n",1);		
		DF_WriteBytesToPage( 16, "2008000CREDIT CARD FEES :", 28, 26 );	
		DF_ReadBytesFromPage( 16,&TempBuffer[0], 28, 26 );
					TempBuffer[ 26 ] = '\0';
					UART1Send(&TempBuffer[0],26);
					UART1Send("\n",1);
		DF_WriteBytesToPage( 16, "3008000CONVENIENCE FEES :", 56, 26 );
			DF_ReadBytesFromPage( 16,&TempBuffer[0], 56, 26 );
					TempBuffer[ 26 ] = '\0';
					UART1Send(&TempBuffer[0],26);
					UART1Send("\n",1);
		
	}
	while( 1 )												   
	{
		switch( process_switch )
		{
			case( 1 ):
			case( 2 ):
			case( 3 ): 
				if( MeterCheckTimer < MS_TIMER )
				{
					MessageDisable = FALSE;
				}
				else
				{
					MessageDisable = FALSE;
				}
						
				Process_Card();				
				ProcessUartRxBuffers();	
		   		if( CHECKFLAG( SEND_ACK_TO_METER ) )
				{
					RESETFLAG( SEND_ACK_TO_METER );					
					SendACKToMeter();					
				}

				if( CHECKFLAG( SEND_NACK_TO_METER ) )
				{
					RESETFLAG( SEND_NACK_TO_METER );					
					//SendNACKToMeter();					
				}

				ProcessUartTxBuffers();
				/************************************************************************************************/
				if(0)
				{	//This Portion has been removed in Ver 2.21 to remove the compatibility between Autotrax CA &
					//CA1 assuming Autotrax CA hardware is outdated 7 will not be used in the production model
					//Henceforth no further Inputs will be detected & will not be sent to GSM
					CheckIOS();
				}
				/************************************************************************************************/
				if(!CHECKFLAG(SERIAL0_BUSY) && !CHECKFLAG(SERIAL1_BUSY))
				{
					SendDataToModem();
					Process_KeyPad();					
				}
				
				//LIVEPULSE
				if(CHECKFLAG(LIVE_PULSE))
				{
					RESETFLAG(LIVE_PULSE);
					//livepulsetimer = MS_TIMER + 180000;
				}
	
				if(MS_TIMER > livepulsetimer)
				{					
					
					SET_PORT0_PIN_HIGH(1<<18);
					Delay(500);
					SET_PORT0_PIN_LOW(1<<18);
					Delay(500);
					LCD_Cursor(4,1);
					LCD_DisplayCharacter(4,'*');
					livepulsetimer = MS_TIMER + 180000;
				}				
				if(CHECKFLAG(WELCOME_SCREEN))
				{					
					if(!CHECKFLAG(FIRST_TIME))
					{						
						SETFLAG(FIRST_TIME);
						DisplayWelcomeScreen();
					}
				}

				if( (PrevMeterHired != MeterHired) && (!CHECKFLAG(SMS_RECEIVED)))
				{
					PrevMeterHired = MeterHired;
					DisplayWelcomeScreen();	
				}
				
				if( ( PrevShiftOn != ShiftOn ) && (!CHECKFLAG(SMS_RECEIVED)))
				{
					PrevShiftOn = ShiftOn;
					DisplayWelcomeScreen();	
				}
				if(CHECKFLAG1(POWERSTATUS))
				{
				if( IOPIN1 & MB_FAIL )
				{	
					Status = 1;
					
				}
				else
				{
					Status = 0;	
					
				}
				SETFLAG1(MONITORSTATUS);
				RESETFLAG1(POWERSTATUS);
				}
				if(CHECKFLAG1(CURRENTSTATUS))
				{
					if( LastStatus != Status )
					{
					if( Status == 1 ){	CopyToAckBuffer(SMS_ACK, "J    ", 5);	}
					else{				CopyToAckBuffer(SMS_ACK, "I    ", 5);	}
					}
					LastStatus = Status;
					DF_WriteBytesToPage( 60, &LastStatus, 0, 1 );
					RESETFLAG1(CURRENTSTATUS);
				}
				/************************************************************************************************/
				if(0)
				{	//This Portion has been removed in Ver 4.0c	& Ver4.0d on request by Mr. Kunjal Shah
					//& also has been confirmed with Mr. Saumil Dhru
					if( IOPIN1 & MB_FAIL )
					{					
						if( MBFailStatus == 1 )
						{					
							LCD_BKLIT_ON;
							SETFLAG(LCD_BACKLIT_ON);
							BacklitTimeoutTimer = MS_TIMER + 30000;
							CopyToAckBuffer(SMS_ACK, "J    ", 5);						
							MBFailStatus = 0;
						}
					}
					else
					{					
						if( MBFailStatus == 0 )
						{					
							CopyToAckBuffer(SMS_ACK, "I    ", 5);
							MBFailStatus = 1;
							LCD_BKLIT_ON;
							SETFLAG(LCD_BACKLIT_ON);
							BacklitTimeoutTimer = MS_TIMER; //For timer switch off
						}
					}
				}
				/************************************************************************************************/
			break;

			default:
				process_switch = 0;
			break;
		}
		process_switch++;		
	}		
}
