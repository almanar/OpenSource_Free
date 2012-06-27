#ifndef _GLOBAL_H
	#define _GLOBAL_H

	//Controller Port0 Defines
	#define SET_PORT0_PIN_DIRN_IN( PIN )	{	IODIR0  &= ~PIN;	}
	#define SET_PORT0_PIN_DIRN_OUT( PIN )	{	IODIR0  |= PIN;		}	
	#define SET_PORT0_PIN_HIGH( PIN )		{	IOSET0  |= PIN;		}
	#define SET_PORT0_PIN_LOW( PIN )		{	IOCLR0  |= PIN;		}

	//Controller Port1 Defines
	#define SET_PORT1_PIN_DIRN_IN( PIN )	{	IODIR1  &= ~PIN;	}	
	#define SET_PORT1_PIN_DIRN_OUT( PIN )	{	IODIR1  |= PIN;		}	
	#define SET_PORT1_PIN_HIGH( PIN )		{	IOSET1  |= PIN;		}	
	#define SET_PORT1_PIN_LOW( PIN )		{	IOCLR1  |= PIN;		}	

	#define EXT_IP1 ( 1<<11 )
	#define EXT_IP2 ( 1<<23 )
	#define EXT_IP3 ( 1<<10 )
	#define EXT_IP4 ( 1<<13 )
	#define EXT_IP5 ( 1<<12 )
	#define MB_FAIL ( 1<<30 )
	#define SB_LOW
	
	#define SET 	1
	#define RESET	2

	#define STX 			0x02
	#define ETX 			0x03
	#define METER_HEADER	0xFE

	#define LCD_POWER_TIMEOUT		600000
	#define	SERIAL_TIMEOUT_TIME		10
	#define METER_TIMEOUT_TIME		10000
	#define METER_TIMEOUT			500
	#define METER_TX_RETRIES		3
	#define METER_ETX				0xEF

	#define SEND_NACK_TO_METER		( 1<<0 )
	#define SEND_ACK_TO_METER		( 1<<1 )
	#define METER_NACK_RECEIVED		( 1<<2 )
	#define METER_ACK_RECEIVED		( 1<<3 )
	#define FIRST_TIME				( 1<<4 )
	#define WELCOME_SCREEN			( 1<<5 )
	#define METER_STATUS			( 1<<6 )
	#define STOP_PULSES				( 1<<7 )
	#define WAIT_FOR_METER_DELAY	( 1<<8 )
	#define RESET_DELAY				( 1<<9 )
	#define TEST_MODE				( 1<<11 )
	#define LIVE_PULSE				( 1<<12 )
	#define LCD_ON					( 1<<13	)
	#define START_EXT1_TIMER		( 1<<14	)
	#define LCD_BACKLIT_ON			( 1<<15	)
	#define SENTINEL_START			( 1<<16	)
	#define SMS_RECEIVED			( 1<<17	)
	#define ETX_RECEIVED			( 1<<18	)
	#define START_SERIAL_TIMEOUT	( 1<<19	)
	#define SERIAL_TIMEOUT_OVER		( 1<<20	)
	#define STX_RECEIVED			( 1<<21	)
	#define SERIAL0_BUSY			( 1<<22	)
	#define START_METER_TIMEOUT		( 1<<23	)
	#define SERIAL1_BUSY			( 1<<24	)
	#define METER_HDR_RECEIVED		( 1<<25 )
	#define DIAL_OTHER_NO			( 1<<26 )
	#define DIAL_FIXED_NO			( 1<<27	)
	#define FULL_MESSAGE_DISPLAY	( 1<<28	)
	#define MENU_LEVEL_TWO  		( 1<<29	)//Message Screen
	#define MENU_LEVEL_ONE  		( 1<<30	)//Main Screen
	#define MENU_LEVEL_ZERO			( 1UL<<31 )//Welcome Screen

#define POWERSTATUS                 ( 1<< 14) 
#define MONITORSTATUS   			( 1<< 15)
#define CURRENTSTATUS				( 1<< 16)
	#define SMS_ACK					1
	#define METER_ACK				2
	#define CMD_ACK					3

	#define ACK_BUF_TYPE			3
	#define ACK_BUF_INDEX 			20
	#define ACK_BUF_SIZE 			42
	#define INPUT_BUF_INDEX			8
	#define INPUT_BUFSIZE			9
	#define INTERBYTE_DELAY			50

	#define TRANSACTION_TIMEOUT		'T'
	#define TRANSACTION_COMPLETE	'D'
	#define TRANSACTION_IN_PROCESS	'P'

//	extern uint8 LogoutError;
//	extern uint8 LoginError;
//  extern uint64 PickUpTimer;
//	extern uint64 LoginLockTimer;
//	extern uint64 LogoutLockTimer;
	extern uint8 KerbDuty;
	extern uint8 CtrlDuty;
	extern uint8 PrevMeterHired;
	extern uint8 MeterHired;
	extern uint8 PrevShiftOn;
	extern uint8 ShiftOn;
//	extern uint8 MeterCmdCount;
	extern uint8 SMSLocation;
	extern uint8 GPSVersion[3];
	extern uint32 GlobalFlags;
	extern uint64 SerialTimeoutTimer;
	extern uint64 PowerTimeoutTimer;
	extern uint64 BacklitTimeoutTimer;
	extern uint64 AckWaitTimer;
	extern uint64 MeterTimeoutTimer;
	extern uint64 KeyRepeatTimer;
	extern uint64 localMeterTxTimer;
	extern uint32 MeterSendDelay;
	extern uint8 MeterDisabled;
	extern const uint8 SendSTX;
	extern const uint8 SendETX;
	extern uint8 TempBuffer[ 528 ];
	extern uint8 SendData[4];
	extern uint8 AckBuffer[ ACK_BUF_TYPE ][ ACK_BUF_INDEX ][ ACK_BUF_SIZE ];
	extern uint8 InputBuffer[INPUT_BUF_INDEX][INPUT_BUFSIZE];
	extern uint32 KeyStatus;
	extern uint8 FareFromGSM[3];
	extern uint8 MeterAddkeys[ 4 ];
	extern uint32 AirportFees;
	extern uint32 CreditCardFees;
	extern uint32 ConvenienceFees;
	extern uint32 AdditionalFees;
	extern uint8 MeterAddkeysStr[ 22 ];
	extern uint8 CurrentCheckSum;
	extern uint8 LastCheckSum;
	extern uint8 LastStatus;
	extern uint64 MeterCheckTimer;
	extern uint64 MeterCommandTimer;
	extern uint8 MessageDisable;
	extern uint8 Dummybyte;
	extern uint32 FARE;
	extern uint8 TransactionResult;
	extern uint8 DisplayLastMessage;
	uint8 CHECKFLAG( uint32 Flag );
	void SETFLAG( uint32 Flag );
	void RESETFLAG( uint32 Flag );

#endif
/* 	
	//Timeouts
	#define SERIAL_TIMEOUT_TIME		2000
	#define BACKLIT_TIMEOUT_TIME	10000
	#define LCD_TIMEOUT_TIME		10000

	//Global bitmap flags for operations in process
	//Card Reader Flags
	#define	SENTINEL_START		(1<<0)
	#define	START_EXT1_TIMER	(1<<1)

	//#define	(1<<2)
	//#define	(1<<3)
	#define	SERIAL_TIMEOUT_OVER  (1<<4)
	#define	START_SERIAL_TIMEOUT (1<<5)
	//#define	(1<<6)
	//#define	(1<<7)
	//#define	(1<<8)
	//#define	(1<<9)
	//#define	(1<<11)
	//#define	(1<<12)
	//#define	(1<<13)
	//#define	(1<<14)
	//#define	(1<<15)
	//#define	(1<<16)
	//#define	(1<<17)
	//#define	(1<<18)
	//#define	(1<<19)
	//#define	(1<<20)
	#define SWITCH_BUZZER_ON		1<<23
	#define SWITCH_BACKLIT_ON		1<<22
	#define SMS_RECEIVED			1<<21   
	#define MESSAGE_START			1<<19


//Externs from Uart Module


//Global Arrays
extern uint8 MeterBuffer[METER_BUF_INDEX][ METER_BUFSIZE ];
extern uint8 SMSBuffer[SMS_BUF_INDEX][ SMS_BUFSIZE ];
extern uint8 CMDBuffer[CMD_BUF_INDEX][ CMD_BUFSIZE ];

extern const uint8 SendSTX;
extern const uint8 SendETX;

//Externs from Timer Module
extern uint64 MS_TIMER;


extern uint64 SerialTimeoutTimer;
extern uint64 BacklitTimeoutTimer;
extern uint64 LCDTimeoutTimer;

uint8 CHECKFLAG( uint32 Flag );
void SETFLAG( uint32 Flag );
void RESETFLAG( uint32 Flag );*/
