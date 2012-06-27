#include "LPC214x.h"
#include "Types.h"
#include "Global.h"


//uint8 LogoutError = FALSE;
//uint8 LoginError = FALSE;		
uint64 PickUpTimer;
//uint64 LoginLockTimer;
//uint64 LogoutLockTimer;
uint8 KerbDuty;
uint8 CtrlDuty;
uint8 PrevMeterHired = FALSE;
uint8 MeterHired = FALSE;
uint8 PrevShiftOn = FALSE;
uint8 ShiftOn = 3;
uint8 SMSLocation;
uint32 GlobalFlags;
uint32 GlobalFlags1;
uint64 SerialTimeoutTimer;
uint64 PowerTimeoutTimer;
uint64 BacklitTimeoutTimer;
uint64 AckWaitTimer;
uint64 MeterTimeoutTimer;
uint64 localMeterTxTimer;
uint8 MeterDisabled = FALSE;
const uint8 SendSTX = STX;
const uint8 SendETX = ETX;
uint8 SendData[4];
uint8 GPSVersion[3];
uint32 KeyStatus;
uint64 KeyRepeatTimer;
uint8 FareFromGSM[3] = {'0','0','0'};
uint8 MeterAddkeys[ 4 ] = { 1, 1, 1, 1 };
uint32 FARE;
uint8 TransactionResult = TRANSACTION_TIMEOUT;
uint8 DisplayLastMessage = FALSE;
uint32 AirportFees;
uint32 CreditCardFees;
uint32 ConvenienceFees;
uint32 AdditionalFees;
uint8 MeterAddkeysStr[ 22 ];
uint8 CurrentCheckSum = 0x00;
uint8 LastCheckSum = 0xFF;
uint64 MeterCheckTimer = 0x00000000;
uint64 MeterCommandTimer = 0x00000000;
uint8 MessageDisable = FALSE;
uint8 Dummybyte = 0x00;
//uint8 MeterCmdCount = 0;

uint8 CHECKFLAG( uint32 Flag )
{
	if ( GlobalFlags & Flag )	
	{	return 1;		}
	else						
	{	return 0;	}
}

void SETFLAG( uint32 Flag )
{
	GlobalFlags |= Flag;
	return;	
}

void RESETFLAG( uint32 Flag )
{
	GlobalFlags &= ~Flag;
	return;	
}
uint8 CHECKFLAG1( uint32 Flag )
{
	if ( GlobalFlags1 & Flag )	
	{	return 1;		}
	else						
	{	return 0;	}
}

void SETFLAG1( uint32 Flag )
{
	GlobalFlags1 |= Flag;
	return;	
}

void RESETFLAG1( uint32 Flag )
{
	GlobalFlags1 &= ~Flag;
	return;	
}
