#include "LPC214x.h"
#include "Types.h"
#include "Global.h"
#include "Timer.h"
#include "Uart.h"
#include <string.h>
#include "Meter.h"
#include "Dataflash.h"
#include "MiscFunctions.h"

struct MeterCommandStruct MeterCommand;


void SendCommandToMeter( uint8 MeterCmd , uint8 *MeterData, uint8 WaitForAck )
{
	uint8 DataCount = 0;
	uint8 NoOfRetries = 5;
	MeterCommand.Header = 0xFF;
	MeterCommand.Reserved = 0x00;
	MeterCommand.Checksum = 0x00;

/*	if( MeterCmd == MCT_START_SHIFT )
	{
		if( ShiftOn == TRUE )
		{
			return;
		}
	}
	else if( MeterCmd == MCT_CLOSE_SHIFT )
	{
		if( ShiftOn == FALSE )
		{
			return;
		}
	} */

	while( NoOfRetries > 0 )
	{
		if( WaitForAck != TRUE )
		{
			NoOfRetries = 0;
		}
		switch( MeterCmd )
		{
			case( MCT_REQUEST_STATUS ):
			case( MCT_ENABLE_METER ):
			case( MCT_DISABLE_METER ):			
			case( MCT_CLOSE_SHIFT ):
			case( MCT_RECORD_ON_CALL_DISTANCE ):
			case( MCT_CLEAR_MEMORY ):			
			case( MCT_PRINT_DAILY_BUSINESS_REPORT ):
			case( MCT_PRINT_MONTHLY_BUSINESS_REPORT ):
			case( MCT_PRINT_CUMULATIVE_REPORT ):		
			case( MCT_PRINT_TARIFF_STRUCTURE ):
			case( MCT_PRINT_LAST_TRIP_DATA ):
			case( MCT_PRINT_LAST_SHIFT_DATA ):		
			case( MCT_REQUEST_RTC_PARAMETERS ):		
			case( MCT_REQUEST_LAST_TRIP_DATA ):		
			case( MCT_REQUEST_LAST_SHIFT_DATA ):		
			case( MCT_QUIT_RECORDING_ON_CALL_DISTANCE ): 	
			case( MCT_CHECK_PRINTER ):
			case( MCT_POSITIVE_ACK_FRAME ):			
			case( MCT_NEGATIVE_ACK_FRAME ):
				MeterCommand.Datalen = 1;
				MeterCommand.Command = MeterCmd;			
			break;
	
			case( MCT_START_SHIFT ):				
				MeterCommand.Datalen = 4;
				MeterCommand.Command = MeterCmd;			
			break;
	
			case( MCT_REQUEST_SHIFT_DATA ):
			case( MCT_PRINT_ALL_TRIP_DATA_PER_SHIFT ):
			case( MCT_PRINT_SHIFT_DATA ):
				MeterCommand.Datalen = 5;
				MeterCommand.Command = MeterCmd;			
			break;
	
			case( MCT_CHANGE_RTC_PARAMETERS ):
				MeterCommand.Datalen = 8;
				MeterCommand.Command = MeterCmd;			
			break;
	
		    case( MCT_REQUEST_TRIP_DATA ):
				MeterCommand.Datalen = 9;
				MeterCommand.Command = MeterCmd;			
			break;
	
			case( MCT_CONF_TAXI_COMPANY_DETAILS ):
				MeterCommand.Datalen = 26;
				MeterCommand.Command = MeterCmd;			
			break;
	
			case( MCT_PRINT_CREDIT_CARD_DETAILS ):
				MeterCommand.Datalen = 56;
				MeterCommand.Command = MeterCmd;
			break;

			case( MCT_ADD_KEY_1 ):
			case( MCT_ADD_KEY_2 ):
			case( MCT_ADD_KEY_3 ):
			case( MCT_ADD_KEY_4 ):
				MeterCommand.Datalen = 22;
				MeterCommand.Command = MeterCmd;
			break;

			default:
			break;
		}
		//CALCULATE CHECKSUM
		MeterCommand.Checksum = (uint8)( ( uint8 )MeterCommand.Reserved +
		 					   			 ( uint8 )MeterCommand.Datalen +	
							   			 ( uint8 )MeterCommand.Command );
	
	   	//SEND DATA TO METER
		UART1Send( &MeterCommand.Header, 1 );
		UART1Send( &MeterCommand.Reserved, 1 );
		UART1Send( &MeterCommand.Datalen, 1 );		
		UART1Send( &MeterCommand.Command, 1 );		

	   	for( DataCount = 1; DataCount < MeterCommand.Datalen; DataCount++ )
	   	{
	   		UART1Send( ( MeterData + DataCount ), 1 );		
	   		MeterCommand.Checksum += ( uint8 )( *( MeterData + DataCount ) );		
	   	}

		UART1Send( &MeterCommand.Checksum, 1 );		
		livepulsetimer = MS_TIMER + 180000;
		AckWaitTimer = MS_TIMER + 2000;
		if( WaitForAck == TRUE )
		{			
			while( MS_TIMER < AckWaitTimer )
			{
				if( CHECKFLAG( METER_ACK_RECEIVED ))
				{
					if( MeterCmd == MCT_START_SHIFT )
					{
						ShiftOn = TRUE;
						DF_ErasePage(13);
						DF_WriteBytesToPage( 13, &ShiftOn, 0, 1 );
//						LoginError = FALSE;
//						LogoutError = FALSE;
					}
					else if( MeterCmd == MCT_CLOSE_SHIFT )
					{
//						LoginLockTimer = 0;
						KerbDuty = FALSE;
						CtrlDuty = FALSE;
						ShiftOn = FALSE;	
						DF_ErasePage(13);
						DF_WriteBytesToPage( 13, &ShiftOn, 0, 1 );
//						LoginError = FALSE;
//						LogoutError = FALSE;
					}
					RESETFLAG( METER_ACK_RECEIVED );
					NoOfRetries = 0;
					AckWaitTimer = MS_TIMER;
				}
				else if( CHECKFLAG( METER_NACK_RECEIVED ))
				{
					if( MeterCmd == MCT_START_SHIFT )
					{
						ShiftOn = TRUE;
						DF_ErasePage(13);
						DF_WriteBytesToPage( 13, &ShiftOn, 0, 1 );
						NoOfRetries = 0;
//						LoginError = FALSE;
//						LogoutError = FALSE;
					}
					else if( MeterCmd == MCT_CLOSE_SHIFT )
					{
						ShiftOn = FALSE;
						DF_ErasePage(13);
						DF_WriteBytesToPage( 13, &ShiftOn, 0, 1 );
						NoOfRetries = 0;
//						LoginError = FALSE;
//						LogoutError = FALSE;
					}
					RESETFLAG( METER_NACK_RECEIVED );										
					AckWaitTimer = MS_TIMER;
				}
			}

			if( NoOfRetries > 0)
			{
				NoOfRetries--;
			}
		}
	}		
	return;		   
}

void ParseCommandForMeter( uint8 *CommandForMeter )
{
	uint32 TempCount;
	uint32 TempCount2;
	uint8 tempdata;
	uint8 TempData[4];
	uint8 ShiftId[4];
	uint8 RTCData[8];
	uint32 TempShiftId = 0;

	for( TempCount=0; TempCount<3; TempCount++ )
	{
		TempData[ TempCount ] = *( CommandForMeter + TempCount );	
	}
	TempData[ TempCount ] = '\0';

	if( !strcmp( (char *)TempData, "LIS" ) ) 
	{		
		TempShiftId =  ( CommandForMeter[3]  - '0') * 10000000;
		TempShiftId += ( CommandForMeter[4]  - '0') * 1000000;
		TempShiftId += ( CommandForMeter[5]  - '0') * 100000;
		TempShiftId += ( CommandForMeter[6]  - '0') * 10000;
		TempShiftId += ( CommandForMeter[7]  - '0') * 1000;
		TempShiftId += ( CommandForMeter[8]  - '0') * 100;
		TempShiftId += ( CommandForMeter[9]  - '0') * 10;
		TempShiftId += ( CommandForMeter[10] - '0') * 1;

		ShiftId[3] = (uint8)(TempShiftId >>  0);
		ShiftId[2] = (uint8)(TempShiftId >>  8);
		ShiftId[1] = (uint8)(TempShiftId >> 16);
		ShiftId[0] = (uint8)(TempShiftId >> 24);

		SendCommandToMeter( MCT_START_SHIFT, &ShiftId[0] , TRUE );		
//		LoginLockTimer = 0;		
	}
	else if( !strcmp( (char *)TempData, "LOS" ) )
	{
//		LogoutLockTimer = 0;
		SendCommandToMeter( MCT_CLOSE_SHIFT, &Dummybyte, TRUE );
	}
	else if( !strcmp( (char *)TempData, "LIE" ) )
	{
//		LoginError = TRUE;
//		LogoutError = FALSE;
//		LoginLockTimer = 0;		
		if(!CHECKFLAG(SMS_RECEIVED))
		{
			DisplayWelcomeScreen();
		}
	}
	else if( !strcmp( (char *)TempData, "LOE" ) )
	{
//		LogoutError = TRUE;						   
//		LoginError = FALSE;	
//		LogoutLockTimer = 0;
		if(!CHECKFLAG(SMS_RECEIVED))
		{
			DisplayWelcomeScreen();
		}	
	}
	else if( !strcmp( (char *)TempData, "RQS" ) )
	{
		SendCommandToMeter( MCT_REQUEST_STATUS, &Dummybyte, TRUE );
	}
	else if( !strcmp( (char *)TempData, "ENM" ) )
	{
//		PickUpTimer = 0;
		SendCommandToMeter( MCT_ENABLE_METER, &Dummybyte, TRUE );
		if( CommandForMeter[3] == 'K' )
		{
			KerbDuty = TRUE;
			CtrlDuty = FALSE;
			if(!CHECKFLAG(SMS_RECEIVED))
			{
				DisplayWelcomeScreen();
			}
		}
		if( CommandForMeter[3] == 'L' )
		{
			KerbDuty = FALSE;
			CtrlDuty = TRUE;
			if(!CHECKFLAG(SMS_RECEIVED))
			{
				DisplayWelcomeScreen();
			}
		}		
	}
	else if( !strcmp( (char *)TempData, "KSE" ) )
	{
//		PickUpTimer = 0;		
	}
	else if(!strcmp( (char *)TempData, "RTC" ) )
	{
		for( TempCount=0; TempCount<7; TempCount++ )
		{
			if(*( CommandForMeter + ((TempCount * 2)+3)) >= 0x41 )
			{
				RTCData[ TempCount + 1]  = (*( CommandForMeter + ((TempCount * 2)+3) ) - 0x37) << 4;	
			}
			else
			{
				RTCData[ TempCount + 1]  = (*( CommandForMeter + ((TempCount * 2)+3) ) - 0x30) << 4;	
			}
			
			if(*( CommandForMeter + ((TempCount * 2)+4)) >= 0x41 )
			{
				RTCData[ TempCount + 1] |= (*( CommandForMeter + ((TempCount * 2)+4) ) - 0x37) << 0;	
			}
			else
			{
				RTCData[ TempCount + 1] |= (*( CommandForMeter + ((TempCount * 2)+4) ) - 0x30) << 0;	
			}
		}
		SendCommandToMeter( MCT_CHANGE_RTC_PARAMETERS, &RTCData[ 0 ] , TRUE );			
	}
	else if( TempData[0] == 'F' && TempData[1] == 'F' )
	{
		for( TempCount=0; *( CommandForMeter + TempCount ) != '\0'; TempCount++ );
		
		TempCount = TempCount/2; 
		
		for( TempCount2=0; TempCount2<TempCount; TempCount2++ )
		{
			if(*( CommandForMeter + (TempCount2 *2)+0 ) >= 0x41 )
			{
				tempdata  = (*( CommandForMeter + ((TempCount2 * 2)+0) ) - 0x37) << 4;	
			}
			else
			{
				tempdata  = (*( CommandForMeter + ((TempCount2 * 2)+0) ) - 0x30) << 4;	
			}

			if(*( CommandForMeter + (TempCount2 *2)+1 ) >= 0x41 )
			{
				tempdata  |= (*( CommandForMeter + ((TempCount2 * 2)+1) ) - 0x37) << 0;	
			}
			else
			{
				tempdata  |= (*( CommandForMeter + ((TempCount2 * 2)+1) ) - 0x30) << 0;	
			}

			UART1Send(&tempdata, 1);
		}
	}
	return;
}


void SendACKToMeter( void )
{
	uint8 Temp[5];
	
	Temp[0] = 0xFF;
	Temp[1] = 0x00;
	Temp[2] = 0x01;
	Temp[3] = 0x66;
	Temp[4] = 0x67;
	UART1Send( &Temp[0], 5);
	return;
} 

void SendNACKToMeter( void )
{
	uint8 Temp[5];
	
	Temp[0] = 0xFF;
	Temp[1] = 0x00;
	Temp[2] = 0x01;
	Temp[3] = 0x77;
	Temp[4] = 0x78;
	UART1Send( &Temp[0], 5);
	return;
}

uint32 ConvAscToHex( uint8* AscData, uint8 Length )
{
	uint32 retVal = 0;
	uint8 lclCount;
	uint8 lclTemp[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	for( lclCount=0; lclCount<Length; lclCount++ )
	{
		lclTemp[ lclCount ] = *( AscData + Length - ( lclCount + 1 )) - 0x30;
	}		
	
	retVal  = lclTemp[ 7 ] * 10000000;
	retVal += lclTemp[ 6 ] * 1000000;
	retVal += lclTemp[ 5 ] * 100000;
	retVal += lclTemp[ 4 ] * 10000;	
	retVal += lclTemp[ 3 ] * 1000;	
	retVal += lclTemp[ 2 ] * 100;	
	retVal += lclTemp[ 1 ] * 10;	
	retVal += lclTemp[ 0 ] * 1;	    
	return( retVal );
}


