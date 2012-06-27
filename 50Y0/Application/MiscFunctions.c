#include "LPC214x.h"
#include "Types.h"
#include "Lcd.h"
#include "Uart.h"
#include "DataFlash.h"
#include "LCD.h"
#include "Timer.h"
#include "Keypad.h"
#include "Global.h"
#include "Meter.h"
#include "MiscFunctions.h"
#include "Dataflash.h"
#include "WDT.h"
#include <string.h>

uint8 MeterBuffer[METER_BUF_INDEX][ METER_BUFSIZE ];
uint8 SMSBuffer[SMS_BUF_INDEX][ SMS_BUFSIZE ];
uint8 CMDBuffer[CMD_BUF_INDEX][ CMD_BUFSIZE ];
uint8 TempBuffer[ DF_BYTESPERPAGE ];
uint8 AckBuffer[ ACK_BUF_TYPE ][ ACK_BUF_INDEX ][ ACK_BUF_SIZE ];
uint8 InputBuffer[INPUT_BUF_INDEX][INPUT_BUFSIZE];
extern uint8 SMSLocation;
uint8 power_status=0;


/************************************************************************************/
void DisplayWelcomeScreen( void )
{
	if(!CHECKFLAG( SMS_RECEIVED ))
	{
		LCD_Clear( );
	 	LCD_DisplayString( 1,1, "TRICITY\0" );
		
		LCD_DisplayString( 4,25, "    Meter Status\0" );		
		//if( CHECKFLAG( METER_NOT_OK ) )	{	LCD_DisplayString( 4,25, "    Meter Not OK\0" );	}
		//else								
		//{	  LCD_DisplayString( 4,25, "        Meter OK\0" );		
			
		if( ShiftOn == TRUE )
		{ 	
			LCD_DisplayString( 4,25, "      Meter Free\0" );		
			
			if( KerbDuty == TRUE )
			{					
				LCD_DisplayString( 4,25, "     Meter Ready\0" );
			}
			else if( CtrlDuty == TRUE )
			{
				LCD_DisplayString( 4,25, "     Meter Ready\0" );
			}

			if( MeterHired == TRUE )		{	LCD_DisplayString( 4,25, "     Meter Hired\0" );		}
			//else if( MeterHired == FALSE )	{	LCD_DisplayString( 4,25, "      Meter Free\0" );		}				
/*				if( LogoutError == TRUE )
			{
				LCD_DisplayString( 4,25, "    Logout Error\0" );
			}*/
		}
		else if( ShiftOn == FALSE )	
		{ 	
			LCD_DisplayString( 4,25, "    Meter Logout\0" );						
		}						
		//}								

	
	/*	if( LoginError == TRUE )
		{
			LCD_DisplayString( 4,25, "     Login Error\0" );
		}*/
		if( DisplayLastMessage == TRUE )
		{
			DisplayMessage(1);
			DisplayLastMessage = FALSE;	
		}
	}
	return;
}
/************************************************************************************/

/************************************************************************************/
void ProcessUartRxBuffers( void )
{
	uint8 localCount;
	uint32 Data;
	uint8 lclTemp;	
	uint8 temp[ 13 ];
	uint8 lclCount;

	if( CHECKFLAG( SERIAL_TIMEOUT_OVER ) && !CHECKFLAG( SERIAL0_BUSY ))
	{
		for( localCount=0; localCount<UART_BUF_INDEX; localCount++ )
		{
			if( UART0Buffer[ localCount ][ 0 ] != '\0' )
			{
				switch( UART0Buffer[ localCount ][ 0 ] )	
				{
					case( 'M' ):
						if( TransactionResult != TRANSACTION_IN_PROCESS )
						{
							if( UART0Buffer[ localCount ][ 1 ] == 'Q' )
							{
								//FILL DATA TO SEND BACK
								temp[ 0 ] = 'O';	temp[ 1 ] = '1';	temp[ 2 ] = 'K';	
								temp[ 3 ] = '1';	temp[ 4 ] = '2';	temp[ 5 ] = '3';	
								temp[ 6 ] = '4';	temp[ 7 ] = 'G';	temp[ 8 ] = 'S';
								temp[ 9  ] = UART0Buffer[ localCount ][ 2 ];
								temp[ 10 ] = UART0Buffer[ localCount ][ 3 ];
								temp[ 11 ] = UART0Buffer[ localCount ][ 4 ];
								temp[ 12 ] = UART0Buffer[ localCount ][ 5 ];
								CopyToAckBuffer(SMS_ACK, &temp[ 0 ], 13);

								//SAVE DATA IN FLASH
								for( lclCount=0; lclCount<4; lclCount++ )
								{
									if( temp[ lclCount + 9 ] > 0x39 )
									{
										temp[ lclCount + 9 ] = temp[ lclCount + 9 ] - 0x37;
									}
									else
									{
										temp[ lclCount + 9 ] = temp[ lclCount + 9 ] - 0x30;
									}
								}

								KeyStatus = 0x00000000;
								KeyStatus |= temp[ 9  ];		KeyStatus <<= 4;
								KeyStatus |= temp[ 10 ];		KeyStatus <<= 4;
								KeyStatus |= temp[ 11 ];		KeyStatus <<= 4;
								KeyStatus |= temp[ 12 ];
					
								temp[ 0 ] = ( KeyStatus >> 8 ) & 0xFF; 
								temp[ 1 ] = ( KeyStatus >> 0 ) & 0xFF; 																
								
								DF_ErasePage(14);
								DF_WriteBytesToPage( 14, &temp[ 0 ], 0, 2 );
							}
							else
							{
								if( UART0Buffer[ localCount ][ 1 ] != 'G' )
								{
									ProcessBuffer( METER_BUFFER, &UART0Buffer[ localCount ][ 1 ], 0, 0 );
									UART0Buffer[ localCount ][ 0 ] = '\0';
								}
							}
						}
					break;

					case( 'F' ):
						Data = ( UART0Buffer[ localCount ][ 1 ] );
						if( Data > '9' ){	Data = Data - 0x37;		}
						else{				Data = Data - 0x30;		}
						FARE  = (uint32)(Data<<20);

						Data = ( UART0Buffer[ localCount ][ 2 ] );
						if( Data > '9' ){	Data = Data - 0x37;		}
						else{				Data = Data - 0x30;		}
						FARE  |= (uint32)(Data<<16);

						Data = ( UART0Buffer[ localCount ][ 3 ] );
						if( Data > '9' ){	Data = Data - 0x37;		}
						else{				Data = Data - 0x30;		}
						FARE  |= (uint32)(Data<<12);

						Data = ( UART0Buffer[ localCount ][ 4 ] );
						if( Data > '9' ){	Data = Data - 0x37;		}
						else{				Data = Data - 0x30;		}
						FARE  |= (uint32)(Data<<8);

						Data = ( UART0Buffer[ localCount ][ 5 ] );
						if( Data > '9' ){	Data = Data - 0x37;		}
						else{				Data = Data - 0x30;		}
						FARE  |= (uint32)(Data<<4);

						Data = ( UART0Buffer[ localCount ][ 6 ] );
						if( Data > '9' ){	Data = Data - 0x37;		}
						else{				Data = Data - 0x30;		}
						FARE  |= (uint32)(Data<<0);
												
						UART0Buffer[ localCount ][ 0 ] = '\0';
					break;

					case( 'A' )://Store
						//SendCommandToMeter( MCT_CHECK_PRINTER, &Dummybyte, TRUE );
						if( TransactionResult != TRANSACTION_IN_PROCESS )
						{
							ProcessBuffer( SMS_BUFFER, &UART0Buffer[ localCount ][ 1 ],SAVE_MSG, 0 );
						}
						UART0Buffer[ localCount ][ 0 ] = '\0';
					break;

					case( 'B' )://METER ENABLE/DISABLE
						if( UART0Buffer[ localCount ][ 3 ] == '0' )
						{
							DF_WriteBytesToPage( 15, &UART0Buffer[ localCount ][ 3 ], 0, 1 );
							MeterDisabled = TRUE;
							//CopyToAckBuffer(SMS_ACK, "O1K12340B0", 10);
						}
						else if( UART0Buffer[ localCount ][ 3 ] == 'F' )
						{
							DF_WriteBytesToPage( 15, &UART0Buffer[ localCount ][ 3 ], 0, 1 );
							MeterDisabled = FALSE;
							//CopyToAckBuffer(SMS_ACK, "O1K12340BF", 10);
						}
					break;

					case( 'Y' )://Store
						//SendCommandToMeter( MCT_CHECK_PRINTER, &Dummybyte, TRUE );
						ProcessBuffer( SMS_BUFFER, &UART0Buffer[ localCount ][ 1 ], SAVE_MSG, 0 );						
						UART0Buffer[ localCount ][ 0 ] = '\0';
					break;

					case( 'T' )://Transaction Messages						
						TransactionResult = TRANSACTION_COMPLETE;
						ProcessBuffer( SMS_BUFFER, &UART0Buffer[ localCount ][ 1 ], DISCARD_MSG, 0 );						
						UART0Buffer[ localCount ][ 0 ] = '\0';						
					break;

					case( '*' ):
						if( TransactionResult != TRANSACTION_IN_PROCESS )
						{
							ProcessBuffer( COMMAND_BUFFER, &UART0Buffer[ localCount ][ 1 ], 0, 0 );
						}
						UART0Buffer[ localCount ][ 0 ] = '\0';
					break;

					case( '$' ):
						ProcessBuffer( ACK_BUFFER, &UART0Buffer[ localCount ][ 1 ], 0, 0 );
						UART0Buffer[ localCount ][ 0 ] = '\0';
					break;

					case( '@' ):
						if( TransactionResult != TRANSACTION_IN_PROCESS )
						{
							GPSVersion[ 0 ] = UART0Buffer[ localCount ][ 1 ];
							GPSVersion[ 1 ] = UART0Buffer[ localCount ][ 2 ];
							GPSVersion[ 2 ] = UART0Buffer[ localCount ][ 3 ];
							DF_ErasePage( 12 );        				
        					DF_WriteBytesToPage( 12, &GPSVersion[ 0 ], 0, 3 );
						}
						UART0Buffer[ localCount ][ 0 ] = '\0';
					break;
					
					case( 'D' ):
						if( UART0Buffer[ localCount ][ 1 ] == 'T' )
						{
							UART0Buffer[ localCount ][8]='\0';
							if(strcmp(&UART0Buffer[ localCount ][ 0 ],"DTAstart")==0)
							{
							localCount = 0x00;
							DF_WriteBytesToPage( 0, &localCount, 0, 1 );
							SETFLAG( STOP_PULSES );
							WDTInit( );
							}
						}
					break;

					case( 'V' ):
						SETFLAG( STOP_PULSES );
						WDTInit( );
					break;

					case( 'E' ):
						LCD_Clear( );
						LCD_DisplayStringCentered( 2, "ERASING MEMORY...." );
						LCD_DisplayStringCentered( 3, "PLEASE WAIT...." );
						DF_ReadBytesFromPage( 60,&LastStatus, 0,1 );
						power_status=LastStatus;
						DF_ChipErase( );
						LastStatus = power_status;
						DF_WriteBytesToPage( 60, &LastStatus, 0, 1 );
						SETFLAG( STOP_PULSES );
						WDTInit( );
					break;

					case( 'C' ):	//Store for Meter					
						if( UART0Buffer[ localCount ][ 6 ] == '0' )
						{
							//Send Stored Data
							strcpy( &TempBuffer[ 0 ] ,"|FECHARGE\0" );
							DF_ReadBytesFromPage( 16, &TempBuffer[ 9 ], 0, 28 );
							CopyToAckBuffer( SMS_ACK, &TempBuffer[ 0 ], 37 );
														
							strcpy( &TempBuffer[ 0 ] ,"|FECHARGE\0" );
							DF_ReadBytesFromPage( 16, &TempBuffer[ 9 ], 28, 28 );
							CopyToAckBuffer( SMS_ACK, &TempBuffer[ 0 ], 37 );
							
							strcpy( &TempBuffer[ 0 ] ,"|FECHARGE\0" );
							DF_ReadBytesFromPage( 16, &TempBuffer[ 9 ], 56, 28 );
							CopyToAckBuffer( SMS_ACK, &TempBuffer[ 0 ], 37 );							
						}
						else
						{
							//Save Data
							lclTemp = UART0Buffer[ localCount ][ 6 ] - 0x30;							
							DF_WriteBytesToPage( 16, &UART0Buffer[ localCount ][ 6 ], 28*( lclTemp - 1 ), 28 );
						}	
					break;

					default:
						UART0Buffer[ localCount ][ 0 ] = '\0';
					break;
				}
			}	
		}
		
		Uart0BufferIndex = 0;
		RESETFLAG( SERIAL_TIMEOUT_OVER );
	}
	return;
}
/************************************************************************************/


void ProcessBuffer( uint8 CopyBuffer, uint8 *SrcBuffer, uint8 Action , uint8 MeterSend )
{
	uint8 temp;
	uint32 tempcount = 0;
	uint32 localCount = 0;
	uint32 localCharCount=0;
	static uint8 localMeterBufIdx = 0;
	static uint8 localSMSBufIdx = 0;
	static uint8 localCMDBufIdx = 0;

	switch( CopyBuffer )
	{
		case( METER_BUFFER ):
			while( *SrcBuffer != '\0' )
			{
				MeterBuffer[ localMeterBufIdx ][ localCount++ ]	= *SrcBuffer;
				SrcBuffer++;
			}			
			MeterBuffer[ localMeterBufIdx ][ localCount ] = '\0';	
			
			//FIXME CHECK OF DATE AND TIME
			if( MeterBuffer[ localMeterBufIdx ][ 1 ] == 'C' &&
				MeterBuffer[ localMeterBufIdx ][ 2 ] == 'C' )
			{
				if( strlen((char *)MeterBuffer[ localMeterBufIdx ]) > 16 )
				{
					for( tempcount=0; tempcount<localCount; tempcount++ )
					{
						if( CheckDateTime( &MeterBuffer[ localMeterBufIdx ][3] ) == TRUE )
						{
							UART1Send( &MeterBuffer[ localMeterBufIdx ][ tempcount ], ( 1 ));
							Delay(INTERBYTE_DELAY);
						}
					}
				}
			}
			else
			{
				//Sending Abbreviated Command from server to meter
				ParseCommandForMeter(&MeterBuffer[ localMeterBufIdx ][0]);				
				
				//Sending Direct Command from server to meter
				//UART1Send( &MeterBuffer[ localMeterBufIdx ][ 0 ], tempcount);
			}
//			MeterCmdCount++;
			//UART0SendBuffer("#$M#", 4);
			localMeterBufIdx++;
			
			if( localMeterBufIdx == METER_BUF_INDEX )
			{
				localMeterBufIdx = 0;	
			}
		break;

		case( SMS_BUFFER ):						
			ConvenienceFees = 0x00000000;
			if( *( SrcBuffer + 4 ) != 'C' )
			{
				ConvenienceFees = ConvAscToHex( SrcBuffer + 4, 6 );
			}

			//Extract Message ID
			for( localCount=0; localCount<4; localCount++ )
			{
				SMSBuffer[ localSMSBufIdx ][ localCount ] = *( SrcBuffer + localCount );
			}									
			
			//Extract Message & Combine
			while( *SrcBuffer != '\0' )
			{
				//Separate Convenience from Message *#J1234XXXXXX<Message>*#
				SMSBuffer[ localSMSBufIdx ][ localCount++ ]	= *( SrcBuffer + 10 );
				SrcBuffer++;
			}			
			SMSBuffer[ localSMSBufIdx ][ localCount ] = '\0';
			
			if( localCount > 4 )//If Full Message Received
			{
				if( MessageDisable != TRUE )
				{
					DF_ReadBytesFromPage( 11, &SMSLocation, 0,1 );			
					SMSLocation++;
					if(SMSLocation > 10)
					{
						SMSLocation = 1;	
					}			
				
					SaveSMS( &SMSBuffer[localSMSBufIdx][0], localCount, SMSLocation, Action );			
					localSMSBufIdx++;
				}
			}
			else
			{				
				CopyToAckBuffer(SMS_ACK, "O1K12340D", 9);
			}
			
			if( localSMSBufIdx == SMS_BUF_INDEX )
			{
				localSMSBufIdx = 0;	
			}

			//Send Convenience Fees To Meter
			//if String is "CANCEL" after ID Send 0 to Meter - Checking only 'C' of "CANCEL"
			if( Action != DISCARD_MSG && SMSBuffer[localSMSBufIdx-1][0] != 'X' )
			{ 				
				if( MessageDisable != TRUE )
				{
					if( MeterSend == 1 )
					{
						strcpy( MeterAddkeysStr, " CONVENIENCE FEES :");
						MeterAddkeysStr[ 21 ] = ( uint8 )( ConvenienceFees >> 0 );
						MeterAddkeysStr[ 20 ] = ( uint8 )( ConvenienceFees >> 8 );
						MeterAddkeysStr[ 19 ] = ( uint8 )( ConvenienceFees >> 16 );															
						SendCommandToMeter( MCT_ADD_KEY_3, &MeterAddkeysStr[ 0 ], TRUE );
					}
				}
			}
		break;

		case( COMMAND_BUFFER ):
			while( *SrcBuffer != '\0' )
			{
				CMDBuffer[ localCMDBufIdx ][ localCount++ ]	= *SrcBuffer;
				SrcBuffer++;
			}
			CMDBuffer[ localCMDBufIdx ][ localCount ] = '\0';
			ProcessIOCommand(&CMDBuffer[ localCMDBufIdx ][0]);			
			localCMDBufIdx++;
			
			if( localCMDBufIdx == CMD_BUF_INDEX )
			{
				localCMDBufIdx = 0;	
			}		
		break;

		case( ACK_BUFFER ):
		{
			switch( *(SrcBuffer + 0) )
			{
				case('D')://MDT ACK				
				livepulsetimer = MS_TIMER + 180000;
				//FIXME
//					LCD_Cursor(2,1);
//					LCD_DisplayCharacter(1,'D');
				for( localCount=0; localCount<(ACK_BUF_INDEX-1); localCount++ )	
				{
					for(localCharCount=0; localCharCount<ACK_BUF_SIZE; localCharCount++)
					{
						AckBuffer[ SMS_ACK ][ localCount ][localCharCount] = AckBuffer[ SMS_ACK ][localCount + 1][localCharCount];
					}
				}
				AckBuffer[ SMS_ACK ][ ACK_BUF_INDEX-1 ][ 0 ] = '\0';

				break;
					
				case('M')://Meter ACK	
				livepulsetimer = MS_TIMER + 180000;
					//FIXME
//					LCD_Cursor(2,1);
//					LCD_DisplayCharacter(1,'M');										
					localMeterTxTimer = 0x00;
					for( localCount=0; localCount<(UART_BUF_INDEX-1); localCount++ )	
					{						
						if( UART1Buffer[ localCount ][0] != '\0')
						{
							UART1Buffer[ localCount ][0] = '\0';
							UART1Buffer[ localCount ][UART_BUFSIZE - 1] = '\0';
							break;
						}
					}										
				break;

				case('I')://IO ACK
					livepulsetimer = MS_TIMER + 180000;
					//FIXME
//					LCD_Cursor(2,1);
//					LCD_DisplayCharacter(1,'I');					
					for( localCount=0; localCount<(INPUT_BUF_INDEX-1); localCount++ )	
					{
						for(localCharCount=0; localCharCount<ACK_BUF_SIZE; localCharCount++)
						{
							InputBuffer[ localCount ][localCharCount] = InputBuffer[localCount + 1][localCharCount];
						}
					}
					InputBuffer[ INPUT_BUF_INDEX-1 ][ 0 ] = '\0';
				break;

				case('L'):
					//FIXME
//					LCD_Cursor(2,1);
//					LCD_DisplayCharacter(1,'L');					
					//LIVEPULSE
					SETFLAG( LIVE_PULSE );
					livepulsetimer = MS_TIMER + 180000;
					temp = 0x03;
					UART0Send("#*K*#",5);
					UART0Send(&temp,1);					
				break;
			}		
		}
		break;

		default:
		break;
	}
return;
}

void ProcessUartTxBuffers( void )
{
	uint8 localMeterCount;
	uint8 localCount;
	uint8 localCharCount;
	uint8 localTemp;
	uint8 localSendCount[3];
	uint8 Count;
	static uint8 localMeterTxCount = 0;
	extern uint64 localMeterTxTimer;
	static uint8 localInputTxCount = 0;
	static uint32 localInputTxTimer = 0;
	static uint8 localSMSTxCount = 0;
	static uint64 localSMSTxTimer = 0;
	static uint8 lclMeterCount = 0;
	uint8 lclDataCount;

	if( !CHECKFLAG(SERIAL0_BUSY) )
	{								
		//Process Meter Tx Buffer
		if( Uart1BufferIndex != 0 )
		{		   
			for( localMeterCount=0; localMeterCount<UART_BUF_INDEX; localMeterCount++ )
			{
				if( UART1Buffer[localMeterCount][0] != '\0' && 
					UART1Buffer[localMeterCount][UART_BUFSIZE -1] != 'A')
				{
					UART1Buffer[localMeterCount][UART_BUFSIZE -1] = 'A';
					localMeterTxCount = 0;
					//localMeterTxTimer = 0;	
				} 
				
				if( UART1Buffer[localMeterCount][0] != '\0' && 
				    localMeterTxCount < METER_TX_RETRIES && 
				    UART1Buffer[localMeterCount][UART_BUFSIZE -1] == 'A')
				{
					
					if( localMeterTxTimer < MS_TIMER )
					{			
						//for(localCharCount=0; UART1Buffer[localMeterCount][localCharCount] != METER_ETX; localCharCount++);				
//						localCharCount=0;
//						lclDataCount = 1;
//						while( lclDataCount )
//						{
//							if( UART1Buffer[localMeterCount][localCharCount + 0 ] == METER_ETX &&
//								UART1Buffer[localMeterCount][localCharCount + 1 ] == 0x00 )
//							{
//								lclDataCount = 0;
//							}
//							else
//							{
//								localCharCount++;
//							}
//						}
						localCharCount = UART1Buffer[localMeterCount][localCharCount + 2 ] + 4;

						//FIXMEUART0SendBuffer( &UART1Buffer[localMeterCount][0], localCharCount );
						localTemp = '|';
						UART0Send(&localTemp, 1); 						
						for( localTemp=0; localTemp<localCharCount; localTemp++ )
						{
							localSendCount[0] = UART1Buffer[localMeterCount][localTemp]>>4;
							localSendCount[1] = UART1Buffer[localMeterCount][localTemp] & 0x0F;
							if( localSendCount[0] > 9 )
							{
								localSendCount[0] = localSendCount[0] + 0x37;	
							}
							else
							{
								localSendCount[0] = localSendCount[0] + 0x30;	
							}

							if( localSendCount[1] > 9 )
							{
								localSendCount[1] = localSendCount[1] + 0x37;	
							}
							else
							{
								localSendCount[1] = localSendCount[1] + 0x30;	
							}
							UART0Send(&localSendCount[0],1);
							UART0Send(&localSendCount[1],1);
						}
	
						localSendCount[ 0 ] = 'Y';
						localSendCount[ 1 ] = lclMeterCount + 0x30;

						UART0Send(&localSendCount[0],1);
						UART0Send(&localSendCount[1],1);
					
						lclDataCount = localCharCount * 2;
						if( lclDataCount > 99 )
						{
							localSendCount[ 2 ] = ( lclDataCount%10 ) + 0x30;	lclDataCount = lclDataCount/10;
							localSendCount[ 1 ] = ( lclDataCount%10 ) + 0x30;	lclDataCount = lclDataCount/10;						
							localSendCount[ 0 ] = ( lclDataCount ) + 0x30;
							UART0Send(&localSendCount[0],3);
						}
						else
						{
							localSendCount[ 2 ] = ( lclDataCount%10 ) + 0x30;	lclDataCount = lclDataCount/10;						
							localSendCount[ 1 ] = ( lclDataCount ) + 0x30;
							localSendCount[ 0 ] = '0';
							UART0Send(&localSendCount[0],3);
						}
																								
						localSendCount[ 0 ] = 'Z';
						UART0Send(&localSendCount[0],1);
																			
						lclMeterCount++;
						if( lclMeterCount > 9 )
						{
							lclMeterCount = 0;	
						}
						
						localTemp = ETX;
						UART0Send(&localTemp, 1); 
						localMeterTxCount++;
						localMeterTxTimer = MS_TIMER + METER_TIMEOUT_TIME;
						Delay(1000); 
						
						if( localMeterTxCount == METER_TX_RETRIES  )
						{
							localMeterTxCount = 0;
							//localMeterTxTimer = 0;
							UART1Buffer[localMeterCount][0] = '\0';
							UART1Buffer[localMeterCount][UART_BUFSIZE - 1] = '\0';	
						}						
					}
					break;
				}				
			}
			if( localMeterCount == UART_BUF_INDEX )
			{				
				Uart1BufferIndex = 0;
			}
		}

		//Process Input Buffer
		if( InputBuffer[0][0] != '\0' && localInputTxCount < METER_TX_RETRIES && InputBuffer[0][INPUT_BUFSIZE-1] == 'A')
		{
			if(localInputTxTimer < MS_TIMER )
			{
				UART0SendBuffer( &InputBuffer[0][0], 8 );
				localInputTxCount++;
				localInputTxTimer = MS_TIMER + METER_TIMEOUT_TIME;
			}

			if( localInputTxCount == METER_TX_RETRIES  )
			{
				localInputTxCount = 0;
				localInputTxTimer = 0;
				InputBuffer[0][0] = '\0';

				for( localCount=0; localCount<(INPUT_BUF_INDEX-1); localCount++ )	
				{
					for(localCharCount=0; localCharCount<INPUT_BUFSIZE; localCharCount++)
					{ 
						InputBuffer[ localCount ][localCharCount] = InputBuffer[localCount + 1][localCharCount];
					}
				}
				InputBuffer[ INPUT_BUF_INDEX-1 ][ 0 ] = '\0';
			}				
		}
		else if( InputBuffer[0][0] != '\0' && InputBuffer[0][INPUT_BUFSIZE-1] != 'A')
		{
			InputBuffer[0][INPUT_BUFSIZE-1] = 'A';
			localInputTxCount = 0;
			localInputTxTimer = 0;
		}

		//Process SMS Ack Buffer
		if( AckBuffer[ SMS_ACK ][0][0] != '\0' && localSMSTxCount < METER_TX_RETRIES && AckBuffer[ SMS_ACK ][0][ACK_BUF_SIZE-1] == 'A')
		{			
			if(localSMSTxTimer < MS_TIMER )
			{
				for(Count=0; AckBuffer[ SMS_ACK ][0][Count] != ETX; Count++);
				Count++;
				UART0Send( &AckBuffer[ SMS_ACK ][0][0], Count );
				localSMSTxCount++;
				localSMSTxTimer = MS_TIMER + METER_TIMEOUT_TIME;
			}

			if( localSMSTxCount == METER_TX_RETRIES  )
			{
				localSMSTxCount = 0;
				localSMSTxTimer = 0;
				AckBuffer[ SMS_ACK ][0][0] = '\0';

				for( localCount=0; localCount<(ACK_BUF_INDEX-1); localCount++ )	
				{
					for( localCharCount=0; localCharCount<ACK_BUF_SIZE; localCharCount++ )	
					{					
						AckBuffer[ SMS_ACK ][ localCount ][localCharCount] = AckBuffer[ SMS_ACK ][localCount + 1][localCharCount];
					}
				}
				AckBuffer[ SMS_ACK ][ ACK_BUF_INDEX-1 ][ 0 ] = '\0';
			}				
		}
		else if( AckBuffer[ SMS_ACK ][0][0] != '\0' && AckBuffer[ SMS_ACK ][0][ACK_BUF_SIZE-1] != 'A')
		{
			AckBuffer[ SMS_ACK ][0][ACK_BUF_SIZE-1] = 'A';
			localSMSTxCount = 0;
			localSMSTxTimer = 0;
		}
	}
	return;
}

void SaveSMS( uint8 *SMSData, uint32 SMSDataCount, uint8 Location, uint8 Action )
{	
	uint32 Count;
	extern uint8 SendData[4];
	uint8 SaveStatus;	

	SaveStatus = Action;
	DF_DisableBlkProt( );
	for( Count=0; Count<328; Count++ )
	{
		TempBuffer[ Count ] = '\0';
	}

	for( Count=0; Count<SMSDataCount; Count++ )
	{
		TempBuffer[ Count ] = *(SMSData + Count);
	}
	TempBuffer[ Count ] = '\0';
	
	for( Count=0; Count<4; Count++ )
	{
		if(*(SMSData + Count) != 'X')
		{			
			SendData[ Count ] = *(SMSData + Count);
		}
		else
		{
			SaveStatus = DISCARD_MSG;
			SendData[ Count ] = '\0';
		}
	}

	if( SaveStatus == SAVE_MSG )
	{
		DF_ErasePage(11);	
		DF_WriteBytesToPage( 11, &Location, 0, 1 );

		DF_ErasePage( Location );
		DF_WriteDataIntoBuffer( TempBuffer );
		DF_WriteBufferToPage( Location );
	}
	//FIXME 
	if( TransactionResult != TRANSACTION_IN_PROCESS )
	{		
		if(!CHECKFLAG(SERIAL0_BUSY))
		{					 
			LCD_Clear();	//FIXMELCD
			LCD_DisplayScreen(&TempBuffer[4]);
			if(strlen((char *)TempBuffer) > 160)
			{
				LCD_Cursor(4,38);
				LCD_DisplayCharacter(4,'>');
				LCD_DisplayCharacter(4,'>');
				LCD_DisplayCharacter(4,'>');
			}
		}
	}
	else
	{
		DisplayLastMessage = TRUE;
	}
	
	if( SendData[0] != '\0' )		
	{
		CopyToAckBuffer(SMS_ACK, &SendData[0], 4);
	}

	SETFLAG(SMS_RECEIVED);
	SETFLAG(MENU_LEVEL_ZERO);
	RESETFLAG(MENU_LEVEL_ONE);
	RESETFLAG(MENU_LEVEL_TWO);
	RESETFLAG(FULL_MESSAGE_DISPLAY);
	RESETFLAG(DIAL_FIXED_NO);
	RESETFLAG(DIAL_OTHER_NO);
	return;	
}

void ProcessIOCommand( uint8 *IOCMDBuffer )
{
	if( *( IOCMDBuffer+1 ) == '1' && *( IOCMDBuffer+2 ) == '1')
	{
		SET_PORT0_PIN_LOW( OUTPUT_1 );
	}
	else if( *( IOCMDBuffer+1 ) == '1' && *( IOCMDBuffer+2 ) == '0')
	{
		SET_PORT0_PIN_HIGH( OUTPUT_1 );
	}

	if( *( IOCMDBuffer+1 ) == '2' && *( IOCMDBuffer+2 ) == '1')
	{
		SET_PORT0_PIN_LOW( OUTPUT_2 );
	}
	else if( *( IOCMDBuffer+1 ) == '2' && *( IOCMDBuffer+2 ) == '0')
	{
		SET_PORT0_PIN_HIGH( OUTPUT_2 );
	}

	if( *( IOCMDBuffer+1 ) == '3' && *( IOCMDBuffer+2 ) == '1')
	{
		SET_PORT0_PIN_LOW( OUTPUT_3 );
	}
	else if( *( IOCMDBuffer+1 ) == '3' && *( IOCMDBuffer+2 ) == '0')
	{
		SET_PORT0_PIN_HIGH( OUTPUT_3 );
	}

	if( *( IOCMDBuffer+1 ) == '4' && *( IOCMDBuffer+2 ) == '1')
	{
		SET_PORT1_PIN_LOW( OUTPUT_4 );
	}
	else if( *( IOCMDBuffer+1 ) == '4' && *( IOCMDBuffer+2 ) == '0')
	{
		SET_PORT1_PIN_HIGH( OUTPUT_4 );
	}

	if( *( IOCMDBuffer+1 ) == '5' && *( IOCMDBuffer+2 ) == '1')
	{
		SET_PORT1_PIN_LOW( OUTPUT_5 );
	}
	else if( *( IOCMDBuffer+1 ) == '5' && *( IOCMDBuffer+2 ) == '0')
	{
		SET_PORT1_PIN_HIGH( OUTPUT_5 );
	}

	return;
}

void SendDataToModem( void )
{
	uint32 localCount;

	if( !CHECKFLAG( SERIAL0_BUSY ) && UARTTxBufferCount > 0 )
	{
		for( localCount=0; localCount<UARTTxBufferCount; localCount++ )
		{
			while( !(U1LSR & LSR_THRE) );	/* THRE status, contain valid data */		
			U1THR = UART0TxBuffer[localCount];						
		}
		UARTTxBufferCount = 0;		
	}

	return;
}

void SendMessage (uint8 Message )
{
	extern uint8 SendData[4];
	uint8 Dummybyte;
	uint8 Temp[14];	
	uint8 lclTemp;

	Temp[0] = STX;		Temp[1] = 'O';
	Temp[2] = '1';  	Temp[3] = 'K';
	Temp[4] = '1';  	Temp[5] = '2';
	Temp[6] = '3';  	Temp[7] = '4';
	Temp[8] = '0';  	Temp[9] = '0';
	Temp[10] = ETX;
	LCD_Init();
	LCD_Clear();
	switch(Message)
	{		
		case 0:		
			LCD_DisplayString(2,15, "CALL PICKED\0");		
			CopyToAckBuffer(SMS_ACK, "O1K12340C", 9);			
			if( MeterDisabled != TRUE )
			{
				SendCommandToMeter( MCT_ENABLE_METER, &Dummybyte, TRUE );
			}
//			PickUpTimer = MS_TIMER + 300000;
		break;

		case 1:															
			LCD_DisplayString(2,15, "I ACCEPT\0");
			Temp[9] = '1'; 									
			if(SendData[0] != 0x00)
			{
				Temp[10] = SendData[0];
				Temp[11] = SendData[1];
				Temp[12] = SendData[2];
				Temp[13] = SendData[3];				
				CopyToAckBuffer(SMS_ACK, &Temp[1], 13);
				SendData[0] = 0x00;			
			}
			else
			{				
				CopyToAckBuffer(SMS_ACK, &Temp[1], 9);
			}			
		break;

		case 2:			
			LCD_DisplayString(2,15, "STARTING\0");			
			CopyToAckBuffer(SMS_ACK, "O1K123402", 9);			
		break;

		case 3:
			LCD_DisplayString(2,15, "VEHICLE FAILED\0");
			CopyToAckBuffer(SMS_ACK, "O1K123404", 9);			
		break;

		case 4:
			LCD_DisplayString(2,15, "CALL ME BACK\0");		
			CopyToAckBuffer(SMS_ACK, "O1K123405", 9);			
		break;

		case 5:
		break;

		case 6:
		break;

		case 7:
			LCD_DisplayString(2,15, "I REJECT\0");
			Temp[9] = '6'; 									
			if(SendData[0] != 0x00)
			{
				Temp[10] = SendData[0];
				Temp[11] = SendData[1];
				Temp[12] = SendData[2];
				Temp[13] = SendData[3];				
				CopyToAckBuffer(SMS_ACK, &Temp[1], 13);
				SendData[0] = 0x00;			
			}
			else
			{				
				CopyToAckBuffer(SMS_ACK, &Temp[1], 9);
			}			
		break;

		case 8:			
			LCD_DisplayString(2,15, "VEHICLE PARKED\0");
			CopyToAckBuffer(SMS_ACK, "O1K123407", 9);
			TempBuffer[0] = 0xDF;
			DF_WriteBytesToPage( 15,&TempBuffer[0], 0, 1 );			
		break;

		case 9:
			LCD_DisplayString(2,12, "IN TRAFFIC JAM\0");
			CopyToAckBuffer(SMS_ACK, "O1K123409", 9);									
		break;

		case ( MCT_AIRPORT_ADD ):
			for( lclTemp=0; lclTemp<24; lclTemp++ )
			{
				TempBuffer[ lclTemp ] = ' ';
			}
			
			AirportFees = AirportFees/100;

			DF_ReadBytesFromPage( 16, &TempBuffer[ 1 ], 7, 19 );
			TempBuffer[ 19 ] = ' ';
			TempBuffer[ 20 ] = ( AirportFees/100 );
			TempBuffer[ 21 ] = ( AirportFees - ( TempBuffer[ 20 ] * 100 ))/10;
			TempBuffer[ 22 ] = ( AirportFees - (( TempBuffer[ 20 ] * 100 ) + ( TempBuffer[ 21 ] * 10 )));

			if( TempBuffer[ 20 ] != 0 )
			{
				TempBuffer[ 20 ] = TempBuffer[ 20 ] + 0x30;
			}
			else
			{
				TempBuffer[ 20 ] = ' ';
			}
			TempBuffer[ 21 ] = TempBuffer[ 21 ] + 0x30;
			TempBuffer[ 22 ] = TempBuffer[ 22 ] + 0x30;
			TempBuffer[ 23 ] = '\0';
		
			LCD_DisplayStringCentered( 2, &TempBuffer[ 1 ] );
			Delay(1000);
		break;

		case ( MCT_AIRPORT_CANCEL ):
			LCD_DisplayString(2,10, "AIRPORT FEE REMOVED\0");
			Delay(1000);
		break;

		case ( MCT_CCARD_ADD ):
			LCD_DisplayString(2,1, "CREDIT CARD FEE ADDED TO METER - Rs. 10\0");
			Delay(1000);
		break;

		case ( MCT_CCARD_CANCEL ):
			LCD_DisplayString(2,8, "CREDIT CARD FEE REMOVED\0");
			Delay(1000);
		break;

		case ( MCT_CONV_ADD ):
			LCD_DisplayString(2,1, "CONVENIENCE FEE ADDED TO METER - Rs. 50\0");
			Delay(1000);
		break;

		case ( MCT_CONV_CANCEL ):
			LCD_DisplayString(2,8, "CONVENIENCE FEE REMOVED\0");
			Delay(1000);
		break;

		case ( MCT_ADDNL_ADD ):	
			LCD_DisplayString(2,2, "ADDITIONAL FEE ADDED TO METER - Rs. 10\0");
			Delay(1000);
		break;

		case ( MCT_ADDNL_CANCEL ):
			LCD_DisplayString(2,8, "ADDITIONAL FEE REMOVED\0");
			Delay(1000);
		break;
	}
	Delay(1000);
	DisplayWelcomeScreen();
	return;
}

void DisplayMainMenu( void )
{
	LCD_Clear();

	LCD_DisplayString(1,1,"1 :TO READ OLD MSG.\0");
	LCD_DisplayString(2,1,"4 :TO DIAL FIXED NO.\0");
	LCD_DisplayString(3,1,"7 :TO ENTER NO.\0");
	return;
}

void DisplayPartialMessage( uint8 MessageNo )
{
	LCD_Clear();

	LCD_DisplayString(1,1,"READ MESSAGE\0");
	
	switch( MessageNo )
	{
		
		case 1:			
			LCD_DisplayString(2,1,"MESSAGE 1/10\0");
		break;
		
		case 2:
			LCD_DisplayString(2,1,"MESSAGE 2/10\0");
		break;
		
		case 3:
			LCD_DisplayString(2,1,"MESSAGE 3/10\0");
		break;

		case 4:
			LCD_DisplayString(2,1,"MESSAGE 4/10\0");
		break;

		case 5:
			LCD_DisplayString(2,1,"MESSAGE 5/10\0");
		break;

		case 6:
			LCD_DisplayString(2,1,"MESSAGE 6/10\0");
		break;

		case 7:
			LCD_DisplayString(2,1,"MESSAGE 7/10\0");
		break;

		case 8:
			LCD_DisplayString(2,1,"MESSAGE 8/10\0");
		break;

		case 9:
			LCD_DisplayString(2,1,"MESSAGE 9/10\0");
		break;

		case 10:
			LCD_DisplayString(2,1,"MESSAGE 10/10\0");
		break;

		default:
		break;
	}
	
	if( MessageNo>0 && MessageNo<11)
	{ 				
		DF_ReadBytesFromPage( 11, &SMSLocation, 0,1 );
		if( SMSLocation >= MessageNo  )
		{
			MessageNo = SMSLocation - MessageNo + 1;		
		}
		else
		{
			MessageNo =  SMSLocation + ( 10 - MessageNo) + 1;
		}
		DF_ReadPageToBuffer(MessageNo);
		DF_ReadDataFromBuffer(TempBuffer);
		if(TempBuffer[0] != 0xFF)
		{
			LCD_DisplayString(3,1,&TempBuffer[4]);	
			LCD_DisplayString(4,1,&TempBuffer[44]);						
		}
		else
		{
			LCD_DisplayString(3,1,"NO MESSAGE\0");
		}
	}
}

void DisplayMessage( uint8 MessageIndex )
{	
	uint32 MessageLength;
	uint8 Key = KP_KEYNONE;
	
	if( MessageIndex>0 && MessageIndex<11)
	{		
		DF_ReadBytesFromPage( 11, &SMSLocation, 0,1 );
		LCD_Clear();
		if( SMSLocation >= MessageIndex  )
		{
			MessageIndex = SMSLocation - MessageIndex + 1;		
		}
		else
		{
			MessageIndex =  SMSLocation + ( 10 - MessageIndex) + 1;
		}
		
		DF_ReadPageToBuffer(MessageIndex);
		DF_ReadDataFromBuffer(TempBuffer);

		if(TempBuffer[0] != 0xFF)
		{
			MessageLength = strlen((char *)TempBuffer);
			if( MessageLength > 164 )
			{
				LCD_Clear();
				SendData[0] = TempBuffer[0];
				SendData[1] = TempBuffer[1];
				SendData[2] = TempBuffer[2];
				SendData[3] = TempBuffer[3];

				LCD_DisplayString(1,1,&TempBuffer[4]);			
				LCD_DisplayString(2,1,&TempBuffer[44]);
				LCD_DisplayString(3,1,&TempBuffer[84]);			
				LCD_DisplayString(4,1,&TempBuffer[124]);
				LCD_Cursor(4,38);
				LCD_DisplayCharacter(4,'>');
				LCD_DisplayCharacter(4,'>');
				LCD_DisplayCharacter(4,'>');
				
				while( Key == KEY_UP || Key == KEY_DOWN || Key == KP_KEYNONE )
				{
					Key = Keypad_GetKey();
					if( Key == KEY_UP )
					{
						LCD_Clear();
						LCD_DisplayString(1,1,&TempBuffer[4]);			
						LCD_DisplayString(2,1,&TempBuffer[44]);
						LCD_DisplayString(3,1,&TempBuffer[84]);			
						LCD_DisplayString(4,1,&TempBuffer[124]);
						LCD_Cursor(4,38);
						LCD_DisplayCharacter(4,'>');
						LCD_DisplayCharacter(4,'>');
						LCD_DisplayCharacter(4,'>');
					}
					else if( Key == KEY_DOWN )
					{
						LCD_Clear();
						LCD_Cursor(1,1);
						LCD_DisplayCharacter(1,'<');
						LCD_DisplayCharacter(1,'<');
						LCD_DisplayCharacter(1,'<');
						LCD_DisplayString(1,4,&TempBuffer[161]);			
						LCD_DisplayString(2,1,&TempBuffer[198]);
						LCD_DisplayString(3,1,&TempBuffer[238]);			
						LCD_DisplayString(4,1,&TempBuffer[278]);	
					}	
				}
		  	}
			else
			{
				SendData[0] = TempBuffer[0];
				SendData[1] = TempBuffer[1];
				SendData[2] = TempBuffer[2];
				SendData[3] = TempBuffer[3];
				LCD_DisplayString(1,1,&TempBuffer[4]);			
				LCD_DisplayString(2,1,&TempBuffer[44]);
				LCD_DisplayString(3,1,&TempBuffer[84]);			
				LCD_DisplayString(4,1,&TempBuffer[124]);
			}
		}
		else
		{
			LCD_DisplayString(1,1,"NO MESSAGE\0");
		}
	}
return;
}

void DialFixedNo( void )
{
	uint32 KeyCount;
	uint32 Key;
	uint8 SendKey;
	KeyCount = 0;	

	LCD_Clear();

	LCD_DisplayString(1,1,"DIAL FIXED NO");
	LCD_Cursor(2,1);

	while(KeyCount != 1)
	{
		Key = Keypad_GetKey();
		if(( Key >= KEY_0 ) && ( Key <= KEY_9))
		{
			LCD_Clear();

			LCD_DisplayString(1,1, "Dialling Fixed No.\0");
			LCD_Cursor(1,19);
			LCD_DisplayCharacter(1,(uint8)Key);
			KeyCount++;			
		}
		if( (Key == KEY_MENU) )
		{
			RESETFLAG(DIAL_FIXED_NO);	
			return ;	
		}
		
		if( Key == KEY_PANIC )
		{
			SETFLAG(MENU_LEVEL_ZERO);
			RESETFLAG(MENU_LEVEL_ONE);
			RESETFLAG(MENU_LEVEL_TWO);
			RESETFLAG(FULL_MESSAGE_DISPLAY);
			RESETFLAG(DIAL_FIXED_NO);
			RESETFLAG(DIAL_OTHER_NO);
			RESETFLAG(SMS_RECEIVED);
			LCD_Clear();

			LCD_DisplayString(2,18, "PANIC\0");
			UART0Send((uint8 *)&SendSTX,1);
			UART0Send("O1K12340A",9);
			UART0Send((uint8 *)&SendETX,1);
			Delay(2000);
			DisplayWelcomeScreen();
			RESETFLAG(DIAL_FIXED_NO);
			return;
		}
		if( CHECKFLAG( SERIAL_TIMEOUT_OVER ))
		{
			RESETFLAG(DIAL_FIXED_NO);
			return;
		}
	}
	UART0Send((uint8 *)&SendSTX,1);
	UART0Send("AD",2);
	SendKey = (uint8)Key;
	UART0Send(&SendKey, 1);
	UART0Send((uint8 *)&SendETX,1);
	Delay(2000);
	//LCD_Clear();


	DisplayWelcomeScreen();
	return;
}

void DialOtherNo( void )
{
//	uint32 KeyCount;
//	uint32 Key;
//	uint8 KeyString[10];

//	KeyCount = 0;
	//LCD_Clear();
	
	DisplayWelcomeScreen();
	RESETFLAG(DIAL_OTHER_NO);
	return;
	
	/*
	//Done on request from Kunjal
	LCD_DisplayString(1,1,"DIAL OTHER NO");
	LCD_Cursor(2,1);
	//Done on request from Kunjal

	while(KeyCount != 11)
	{
		Key = Keypad_GetKey();
		if( (Key == KEY_MENU) )
		{
			RESETFLAG(DIAL_OTHER_NO);
			return;
		}
		if( Key == KEY_PANIC )
		{
			SETFLAG(MENU_LEVEL_ZERO);
			RESETFLAG(MENU_LEVEL_ONE);
			RESETFLAG(MENU_LEVEL_TWO);
			RESETFLAG(FULL_MESSAGE_DISPLAY);
			RESETFLAG(DIAL_FIXED_NO);
			RESETFLAG(DIAL_OTHER_NO);
			RESETFLAG(SMS_RECEIVED);
			LCD_Clear();

			LCD_DisplayString(2,18, "PANIC\0");
			UART0Send((uint8 *)&SendSTX,1);
			UART0Send("O1K12340A",9);
			UART0Send((uint8 *)&SendETX,1);
			Delay(2000);
			DisplayWelcomeScreen();
			RESETFLAG(DIAL_OTHER_NO);
			return;
		}
		
		if( CHECKFLAG( SERIAL_TIMEOUT_OVER ))
		{
			RESETFLAG(DIAL_OTHER_NO);
			return;
		}

		if(( Key >= KEY_0 ) && ( Key <= KEY_9) && KeyCount<11 )
		{
			LCD_DisplayCharacter(2, Key);
			KeyString[KeyCount] = Key;
			KeyCount++;
		}
		if( KeyCount == 10 )
		{
			Key = Keypad_GetKey();
			while((Key != KEY_ENTER) && (Key != KEY_MENU) && (Key != KEY_PANIC))
			{
				Key = Keypad_GetKey();
			}
			if(Key == KEY_ENTER)
			{
				LCD_Clear();
			
				LCD_DisplayString(1,1, "Dialling Other No...\0");
				KeyCount++;
			}
			
			if( (Key == KEY_MENU) )
			{
				RESETFLAG(DIAL_OTHER_NO);
				return;
			}
			
			if( Key == KEY_PANIC )
			{
				SETFLAG(MENU_LEVEL_ZERO);
				RESETFLAG(MENU_LEVEL_ONE);
				RESETFLAG(MENU_LEVEL_TWO);
				RESETFLAG(FULL_MESSAGE_DISPLAY);
				RESETFLAG(DIAL_FIXED_NO);
				RESETFLAG(DIAL_OTHER_NO);
				RESETFLAG(SMS_RECEIVED);
				LCD_Clear();

				LCD_DisplayString(2,18, "PANIC\0");
				UART0Send((uint8 *)&SendSTX,1);
				UART0Send("O1K12340A",9);
				UART0Send((uint8 *)&SendETX,1);
				Delay(2000);
				DisplayWelcomeScreen();
				RESETFLAG(DIAL_OTHER_NO);
				return;
			}
		}																		
	}
	UART0Send((uint8 *)&SendSTX,1);
	UART0Send("AC",2);
	UART0Send(KeyString, 10);
	UART0Send((uint8 *)&SendETX,1);
	Delay(2000);
	//LCD_Clear();	

	DisplayWelcomeScreen();
	return; */
}

void LogUserIn( void )
{
	uint32 KeyCount;
	uint32 Key;
	uint8 KeyString[15] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
	uint8 ShiftId[4];	
	uint64 TempShiftId = 0;
	uint8 Temp[2];

	KeyCount = 0;
	LCD_Clear();

	LCD_DisplayString(1,1,"Log In");
	LCD_Cursor(2,1);

	while( (KeyCount != 16) && (CHECKFLAG( SMS_RECEIVED ) != 1))//&& (Key != KEY_MENU) && (Key != KEY_ENTER) )
	{
		Key = Keypad_GetKey();

		if( (Key >= KEY_0) && (Key <= KEY_9) && (KeyCount<=14) )
		{
			LCD_DisplayCharacter(2, Key);
			KeyString[0] = KeyString[1];
			KeyString[1] = KeyString[2];
			KeyString[2] = KeyString[3];
			KeyString[3] = KeyString[4];
			KeyString[4] = KeyString[5];
			KeyString[5] = KeyString[6];
			KeyString[6] = KeyString[7];
			KeyString[7] = KeyString[8];
			KeyString[8] = KeyString[9];
			KeyString[9] = KeyString[10];
			KeyString[10] = KeyString[11];
			KeyString[11] = KeyString[12];
			KeyString[12] = KeyString[13];
			KeyString[13] = KeyString[14];
			KeyString[14] = Key;
			KeyCount++;
		}
		
		if( Key == KEY_MENU || Key == KEY_ENTER )
		{
			KeyCount = 16;
		}

		if( Key == KEY_PANIC )
		{
			SETFLAG(MENU_LEVEL_ZERO);
			RESETFLAG(MENU_LEVEL_ONE);
			RESETFLAG(MENU_LEVEL_TWO);
			RESETFLAG(FULL_MESSAGE_DISPLAY);
			RESETFLAG(DIAL_FIXED_NO);
			RESETFLAG(DIAL_OTHER_NO);
			RESETFLAG(SMS_RECEIVED);	
			LCD_Clear();

			LCD_DisplayString(2,18, "PANIC\0");
			UART0Send((uint8 *)&SendSTX,1);
			UART0Send("O1K12340A",9);
			UART0Send((uint8 *)&SendETX,1);
			Delay(2000);
			DisplayWelcomeScreen();
			return;
		}
    }

	if( CHECKFLAG( SMS_RECEIVED ) == 1 )
	{									
		DisplayWelcomeScreen();		
		return;
	}

	if(Key == KEY_ENTER)
	{
		//SEND LOGIN TO METER
		
		TempShiftId += ( KeyString[ 6  ]  - '0') * 100000000;
		TempShiftId += ( KeyString[ 7  ]  - '0') * 10000000;
		TempShiftId += ( KeyString[ 8  ]  - '0') * 1000000;
		TempShiftId += ( KeyString[ 9  ]  - '0') * 100000;
		TempShiftId += ( KeyString[ 10 ]  - '0') * 10000;
		TempShiftId += ( KeyString[ 11 ]  - '0') * 1000;
		TempShiftId += ( KeyString[ 12 ]  - '0') * 100;
		TempShiftId += ( KeyString[ 13 ]  - '0') * 10;
		TempShiftId += ( KeyString[ 14 ]  - '0') * 1;

		ShiftId[ 3 ] = 0x00;
		ShiftId[ 2 ] = 0x00;
		ShiftId[ 1 ] = 0x00;
		ShiftId[ 0 ] = 0x00;

		if( TempShiftId == 111111111 )
		{
			Temp[ 0 ] = 0x00;
			DF_WriteBytesToPage( 0, &Temp[ 0 ], 0, 1 );
			SETFLAG( STOP_PULSES );
			WDTInit( );
		}
		else if( TempShiftId == 325460005 )
		{
			KeyStatus = 0x00000000;
			Temp[ 0 ] = 0x00;
			Temp[ 1 ] = 0x00;
			DF_ErasePage(14);
			DF_WriteBytesToPage( 14, &Temp[ 0 ], 0, 2 );
		}
		else if( TempShiftId == 325460006 )
		{
			Temp[ 0 ] = 'F';
			DF_WriteBytesToPage( 15, &Temp[ 0 ], 0, 1 );
			MeterDisabled = FALSE;
		}
		else
		{
			ShiftId[ 3 ] = (uint8)(TempShiftId >>  0);
			ShiftId[ 2 ] = (uint8)(TempShiftId >>  8);
			ShiftId[ 1 ] = (uint8)(TempShiftId >> 16);
			ShiftId[ 0 ] = (uint8)(TempShiftId >> 24);
	
			//SendCommandToMeter( MCT_START_SHIFT, &ShiftId[ 0 ], TRUE );
	
			UART0Send((uint8 *)&SendSTX,1);
			UART0Send("O1K123403",9);
			UART0Send(KeyString, 15);
			UART0Send((uint8 *)&SendETX,1);
			Delay(1000);		
		}		
//		LoginLockTimer = MS_TIMER + 300000;
	}
	//LCD_Clear();
	DisplayWelcomeScreen();
	return;
}

void LogUserOut( void )
{
	uint32 KeyCount;
	uint32 Key;
	uint8 KeyString[15] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
	uint8 Dummybyte;
	uint8 ShiftId[4];	
	uint64 TempShiftId = 0;
	uint8 Temp[2];

	KeyCount = 0;
	LCD_Clear();

	LCD_DisplayString(1,1,"Log Out\0");
	LCD_Cursor(2,1);

	while( (KeyCount != 16) && (CHECKFLAG( SMS_RECEIVED ) != 1 ))
	{
		Key = Keypad_GetKey();

		if( (Key >= KEY_0) && (Key <= KEY_9) && (KeyCount<=14) )
		{
			LCD_DisplayCharacter(2, Key);
			KeyString[0] = KeyString[1];
			KeyString[1] = KeyString[2];
			KeyString[2] = KeyString[3];
			KeyString[3] = KeyString[4];
			KeyString[4] = KeyString[5];
			KeyString[5] = KeyString[6];
			KeyString[6] = KeyString[7];
			KeyString[7] = KeyString[8];
			KeyString[8] = KeyString[9];
			KeyString[9] = KeyString[10];
			KeyString[10] = KeyString[11];
			KeyString[11] = KeyString[12];
			KeyString[12] = KeyString[13];
			KeyString[13] = KeyString[14];
			KeyString[14] = Key;
			KeyCount++;
		}
		
		if( Key == KEY_MENU || Key == KEY_ENTER )
		{
			KeyCount = 16;
		}

		if( Key == KEY_PANIC )
		{
			SETFLAG(MENU_LEVEL_ZERO);
			RESETFLAG(MENU_LEVEL_ONE);
			RESETFLAG(MENU_LEVEL_TWO);
			RESETFLAG(FULL_MESSAGE_DISPLAY);
			RESETFLAG(DIAL_FIXED_NO);
			RESETFLAG(DIAL_OTHER_NO);
			RESETFLAG(SMS_RECEIVED);
			LCD_Clear();

			LCD_DisplayString(2,18, "PANIC\0");
			UART0Send((uint8 *)&SendSTX,1);
			UART0Send("O1K12340A",9);
			UART0Send((uint8 *)&SendETX,1);
			Delay(2000);
			DisplayWelcomeScreen();
			return;
		}
	}
	if( CHECKFLAG( SMS_RECEIVED ) == 1 )
	{											
		DisplayWelcomeScreen();
 		return;
	}

	if(Key == KEY_ENTER)
	{
		TempShiftId += ( KeyString[ 6  ]  - '0') * 100000000;
		TempShiftId += ( KeyString[ 7  ]  - '0') * 10000000;
		TempShiftId += ( KeyString[ 8  ]  - '0') * 1000000;
		TempShiftId += ( KeyString[ 9  ]  - '0') * 100000;
		TempShiftId += ( KeyString[ 10 ]  - '0') * 10000;
		TempShiftId += ( KeyString[ 11 ]  - '0') * 1000;
		TempShiftId += ( KeyString[ 12 ]  - '0') * 100;
		TempShiftId += ( KeyString[ 13 ]  - '0') * 10;
		TempShiftId += ( KeyString[ 14 ]  - '0') * 1;
		
		ShiftId[ 3 ] = 0x00;
		ShiftId[ 2 ] = 0x00;
		ShiftId[ 1 ] = 0x00;
		ShiftId[ 0 ] = 0x00;

		if( TempShiftId == 111111111 )
		{
			Temp[ 0 ] = 0x00;
			DF_WriteBytesToPage( 0, &Temp[ 0 ], 0, 1 );
			SETFLAG( STOP_PULSES );
			WDTInit( );
		}
		else if( TempShiftId == 325460005 )
		{
			KeyStatus = 0x00000000;
			Temp[ 0 ] = 0x00;
			Temp[ 1 ] = 0x00;
			DF_ErasePage(14);
			DF_WriteBytesToPage( 14, &Temp[ 0 ], 0, 2 );
		}
		else if( TempShiftId == 325460006 )
		{
			Temp[ 0 ] = 'F';
			DF_WriteBytesToPage( 15, &Temp[ 0 ], 0, 1 );
			MeterDisabled = FALSE;
		}
		else
		{
			SendCommandToMeter( MCT_CLOSE_SHIFT, &Dummybyte, TRUE );

			UART0Send((uint8 *)&SendSTX,1);
			UART0Send("O1K123408",9);
			UART0Send(KeyString, 15);
			UART0Send((uint8 *)&SendETX,1);
			SendCommandToMeter( MCT_CLOSE_SHIFT, &Dummybyte, TRUE );
			Delay(1000);
		}		
//		LogoutLockTimer = MS_TIMER + 180000;		
	}
	//LCD_Clear();
	DisplayWelcomeScreen();
	return;
}

void InitIOS( void )
{	
	PINSEL0 &= ~(0xC0000000);
	PINSEL1 &= ~(0x0000000F);


	SET_PORT0_PIN_DIRN_IN( EXT_IP1 );
	SET_PORT1_PIN_DIRN_IN( EXT_IP2 );
	SET_PORT0_PIN_DIRN_IN( EXT_IP3 );
	SET_PORT0_PIN_DIRN_IN( EXT_IP4 );
	SET_PORT0_PIN_DIRN_IN( EXT_IP5 );
	
	SET_PORT1_PIN_DIRN_OUT(1<<30);
	SET_PORT1_PIN_DIRN_IN(MB_FAIL);

	SET_PORT0_PIN_DIRN_OUT(OUTPUT_1);
	SET_PORT0_PIN_DIRN_OUT(OUTPUT_2);
	SET_PORT0_PIN_DIRN_OUT(OUTPUT_3);
	SET_PORT1_PIN_DIRN_OUT(OUTPUT_4);
	SET_PORT1_PIN_DIRN_OUT(OUTPUT_5);
	
	//SET_PORT0_PIN_HIGH(1<<17);
	//SET_PORT1_PIN_HIGH(1<<20);

	SET_PORT0_PIN_LOW(OUTPUT_1);
	SET_PORT0_PIN_LOW(OUTPUT_2);
	SET_PORT0_PIN_LOW(OUTPUT_3);
	SET_PORT1_PIN_LOW(OUTPUT_4);
	SET_PORT1_PIN_LOW(OUTPUT_5);
	
	SET_PORT0_PIN_HIGH( EXT_IP1 );
	SET_PORT1_PIN_HIGH( EXT_IP2 );
	SET_PORT0_PIN_HIGH( EXT_IP3 );
	SET_PORT0_PIN_HIGH( EXT_IP4 );
	SET_PORT0_PIN_HIGH( EXT_IP5 );

	return;
}

void CheckIOS( void )
{
	uint32 IOState;
	//uint8 MBFailStatus;
	//static uint8 LastMBFailStatus = 0;
	static uint8 LastIOState = 0;
	uint8 InputBufferIndex = 0;

	IOState = 0xFF;
	
/*	if( IOPIN1 & MB_FAIL )	{	MBFailStatus = 1;	}
	else					{	MBFailStatus = 0;	}

	if( LastMBFailStatus != MBFailStatus )
	{				
		if(!MBFailStatus)
		{
			if(livepulsetimer > MS_TIMER)
			{
				UART0Send( "#*U*#",5 );
				UART0Send( &SendETX,1 );				
				LCD_Init();
				DisplayWelcomeScreen();
			}
		}
		else
		{
			if(livepulsetimer > MS_TIMER)
			{
				UART0Send( "#*B*#",5 );
				UART0Send( &SendETX,1 );
			}
		}
	}*/

	if(!( IOPIN0 & EXT_IP1 ))
	{
		IOState &= ~(1<<0);
	}

	if(!( IOPIN1 & EXT_IP2 ))
	{
		IOState &= ~(1<<1);
	}

	if(!( IOPIN0 & EXT_IP3 ))
	{
		IOState &= ~(1<<2);
	}
	if(!( IOPIN0 & EXT_IP4 ))
	{
		IOState &= ~(1<<3);
	}
	if(!( IOPIN0 & EXT_IP5 ))
	{
		IOState &= ~(1<<4);
	}

	if( IOState != 0xFF && IOState != LastIOState )
	{
		Delay(100);
		for(InputBufferIndex=0; (InputBufferIndex<INPUT_BUF_INDEX && InputBuffer[InputBufferIndex][0] != '\0'); InputBufferIndex++);
		if(InputBufferIndex == INPUT_BUF_INDEX)
		{
			InputBufferIndex = 0;
		}

		InputBuffer[InputBufferIndex][0]= '#';
		InputBuffer[InputBufferIndex][1]= '*';
		InputBuffer[InputBufferIndex][2]= 'I';
		if( (IOState >> 4) > 9 )
		{
			InputBuffer[InputBufferIndex][3]= (IOState >> 4) + 0x37;
		}
		else
		{
			InputBuffer[InputBufferIndex][3]= (IOState >> 4) + 0x30;
		}
						  
		if ((IOState & 0x0F) > 9 )
		{
			InputBuffer[InputBufferIndex][4]= (IOState & 0x0F) + 0x37;
		}
		else
		{
			InputBuffer[InputBufferIndex][4]= (IOState & 0x0F) + 0x30;
		}
		InputBuffer[InputBufferIndex][5]= '*';
		InputBuffer[InputBufferIndex][6]= '#';
		InputBuffer[InputBufferIndex][7]= ETX;
	}
	LastIOState = IOState;	
	//LastMBFailStatus = MBFailStatus;
}

void CopyToAckBuffer(uint8 ack_type, uint8 *ack_data, uint8 ack_len)
{
	uint8 localCount = 0;
	uint8 Count = 0;
	for(localCount=0; (AckBuffer[ SMS_ACK ][ localCount ][0] !='\0' && localCount != ACK_BUF_INDEX); localCount++);
	 	
	if(localCount < ACK_BUF_INDEX )
	{
		if( ack_len > 4 )
		{
			AckBuffer[ SMS_ACK ][ localCount ][0] = STX;
			for(Count=0; Count<ack_len; Count++)
			{
				AckBuffer[ SMS_ACK ][ localCount ][Count + 1] = *(ack_data + Count);
			}
			AckBuffer[ SMS_ACK ][ localCount ][Count + 1] = ETX;
		}
		else
		{
			AckBuffer[ SMS_ACK ][ localCount ][0] = STX;
			AckBuffer[ SMS_ACK ][ localCount ][1] =	'O';
			AckBuffer[ SMS_ACK ][ localCount ][2] =	'1';
			AckBuffer[ SMS_ACK ][ localCount ][3] =	'K';
			AckBuffer[ SMS_ACK ][ localCount ][4] =	'1';
			AckBuffer[ SMS_ACK ][ localCount ][5] =	'2';
			AckBuffer[ SMS_ACK ][ localCount ][6] =	'3';
			AckBuffer[ SMS_ACK ][ localCount ][7] =	'4';
			AckBuffer[ SMS_ACK ][ localCount ][8] =	'0';
			AckBuffer[ SMS_ACK ][ localCount ][9] =	'0';
			AckBuffer[ SMS_ACK ][ localCount ][10] = *(ack_data + 0);
			AckBuffer[ SMS_ACK ][ localCount ][11] = *(ack_data + 1);
			AckBuffer[ SMS_ACK ][ localCount ][12] = *(ack_data + 2);
			AckBuffer[ SMS_ACK ][ localCount ][13] = *(ack_data + 3);
			AckBuffer[ SMS_ACK ][ localCount ][14] = ETX;
			AckBuffer[ SMS_ACK ][ localCount ][15] = '\0';
		}
	}
}

void GetCreditCardAmount( uint8 *CardData, uint32 Length )
{
	uint32 KeyCount;
	uint32 Key;
	uint8 temp;
	uint8 localTempChar;
	uint8 KeyString[4] = {'0','0','0','0'};
	uint8 DisplayFare[7] = {'0','0','0','0','.','0','0', };
	uint32 TestFare;												
	uint32 InputTimerTimeout;

	TransactionResult = TRANSACTION_IN_PROCESS;
	TestFare = FARE;

	DisplayFare[6] = TestFare%10 + '0';	TestFare /= 10;	 
	DisplayFare[5] = TestFare%10 + '0';	TestFare /= 10;	 
	DisplayFare[3] = TestFare%10 + '0';	TestFare /= 10;	 
	DisplayFare[2] = TestFare%10 + '0';	TestFare /= 10;	 
	DisplayFare[1] = TestFare%10 + '0';	TestFare /= 10;	 
	DisplayFare[0] = TestFare + '0';	

	KeyCount = 0;
	LCD_Clear();
	LCD_DisplayString(1,1, "THE CURRENT FARE IS: ");
	LCD_DisplayString(1,21, DisplayFare );
	LCD_DisplayString(2,1,"TO MODIFY PROVIDE AMOUNT OR PRESS ENTER");
	LCD_DisplayString(3,1,"PLEASE ENTER AMOUNT\0");
	LCD_Cursor(4,1);
	InputTimerTimeout = MS_TIMER + 30000;
	//&& !CHECKFLAG(SERIAL0_BUSY)
	while(( KeyCount != 5 ) && ( InputTimerTimeout > MS_TIMER ))//&& (Key != KEY_MENU) && (Key != KEY_ENTER) )
	{
		Key = Keypad_GetKey();

		if( (Key >= KEY_0) && (Key <= KEY_9) && (KeyCount<=3) )
		{
			LCD_DisplayCharacter(4, Key);
			KeyString[0] = KeyString[1];
			KeyString[1] = KeyString[2];
			KeyString[2] = KeyString[3];
			KeyString[3] = Key;
			KeyCount++;
			InputTimerTimeout = MS_TIMER + 30000; //Timeout reset on every Keypress
		}

		if( Key == KEY_MENU || Key == KEY_ENTER || Key == KEY_PANIC )
		{
			if( Key == KEY_MENU )
			{
				if ((KeyCount > 0) && (KeyCount <= 4))
				{
					KeyCount = 0;
					LCD_Clear();
					LCD_DisplayString(1,1, "THE CURRENT FARE IS: ");
					LCD_DisplayString(1,21, DisplayFare );
					LCD_DisplayString(2,1,"TO MODIFY PROVIDE AMOUNT OR PRESS ENTER");
					LCD_DisplayString(3,1,"PLEASE ENTER AMOUNT\0");
					LCD_Cursor(4,1);
				}
				else
				{
					KeyCount = 5;
				}	
			}
			else
			{
				KeyCount = 5;
			}			
		}

/*		if( KeyCount == 3 )
		{			
			Key = Keypad_GetKey();
			while( Key != KEY_ENTER || Key != KEY_MENU )
			{
				Key = Keypad_GetKey();	
			}
			KeyCount++;
		}
*/
	}
	
	if(Key == KEY_ENTER)
	{
		temp = 0x02;
		UART0Send(&temp,1);
		UART0Send("R1C",3);
		
		//Card Data Length
		temp = Length/10 + 0x30;	UART0Send(&temp,1);
		temp = Length%10 + 0x30;	UART0Send(&temp,1);

		if( KeyString[0] == '0' && KeyString[1] == '0' &&
			KeyString[2] == '0' && KeyString[3] == '0' )
		{			
			UART0Send(&DisplayFare[0], 4);
			UART0Send(&DisplayFare[5], 2);
		}
		else
		{
			UART0Send(KeyString, 4);
			UART0Send("00", 2);
		}				

		UART0Send( CardData, Length );
		temp = 0x03;
		UART0Send(&temp,1);	
		WaitForAcceptance();			
	}
	if( Key == KEY_PANIC )
	{
		SETFLAG(MENU_LEVEL_ZERO);
		RESETFLAG(MENU_LEVEL_ONE);
		RESETFLAG(MENU_LEVEL_TWO);
		RESETFLAG(FULL_MESSAGE_DISPLAY);
		RESETFLAG(DIAL_FIXED_NO);
		RESETFLAG(DIAL_OTHER_NO);
		RESETFLAG(SMS_RECEIVED);
		LCD_Clear();
		LCD_DisplayString(2,18, "PANIC\0");

		localTempChar = 0x02;
		UART0Send(&localTempChar,1);
		UART0Send("O1K12340A",9);
		localTempChar = 0x03;
		UART0Send(&localTempChar,1);
		Delay(2000);
		DisplayWelcomeScreen();															 
		RESETFLAG(DIAL_FIXED_NO);
	}

	if( InputTimerTimeout <= MS_TIMER && 
		TransactionResult != TRANSACTION_COMPLETE && 
		TransactionResult != TRANSACTION_TIMEOUT )
	{
		LCD_Clear();
		LCD_DisplayStringCentered(2,"AMOUNT NOT ENTERED - TRANSACTION CANCELLED");
		LCD_DisplayStringCentered(3,"PLS SWIPE AGAIN");
		Delay(2000);
	}
	//LCD_Clear();
	DisplayWelcomeScreen();
	return;
}

void WaitForAcceptance( void )
{
	uint32 Key;
	uint8 localTempChar;
	uint32 TransactionTimeoutTimer;
	LCD_Clear();
	LCD_DisplayStringCentered(2,"TRANSACTION IN PROCESS, PLS WAIT...\0");		
	LCD_BKLIT_ON;
	SETFLAG(LCD_BACKLIT_ON);
	BacklitTimeoutTimer = MS_TIMER + 150000;
	TransactionTimeoutTimer = MS_TIMER + 120000;	
	
	Key = Keypad_GetKey();
	while(	( Key != KEY_MENU) && 
			( Key != KEY_PANIC ) && 
			( TransactionResult != TRANSACTION_COMPLETE ) &&
			( TransactionResult != TRANSACTION_TIMEOUT ))	
	{		
		ProcessUartRxBuffers();
		Key = Keypad_GetKey();
		if( TransactionTimeoutTimer <= MS_TIMER )
		{
			TransactionResult = TRANSACTION_TIMEOUT;
		}
	}

	if( TransactionResult == TRANSACTION_TIMEOUT )
	{
		LCD_Clear();
		LCD_DisplayStringCentered(2,"TRANSACTION TIMED OUT...");
		Delay(2000);
		RESETFLAG( SMS_RECEIVED );		
	}
	else if( TransactionResult == TRANSACTION_COMPLETE )
	{
		Delay(2000);
	}

	else if( Key == KEY_PANIC )
	{
		SETFLAG(MENU_LEVEL_ZERO);
		RESETFLAG(MENU_LEVEL_ONE);
		RESETFLAG(MENU_LEVEL_TWO);
		RESETFLAG(FULL_MESSAGE_DISPLAY);
		RESETFLAG(DIAL_FIXED_NO);
		RESETFLAG(DIAL_OTHER_NO);
		RESETFLAG(SMS_RECEIVED);
		LCD_Clear();
		LCD_DisplayString(2,18, "PANIC\0");		  
		
		localTempChar = 0x02;
		UART0Send(&localTempChar,1);
		UART0Send("O1K12340A",9);
		localTempChar = 0x03;
		UART0Send(&localTempChar,1);
		Delay(2000);
		DisplayWelcomeScreen();
		RESETFLAG(DIAL_FIXED_NO);
	}
	return; 
}

uint8 CheckDateTime( uint8 *DateTime )
{
	uint8 seconds, hour, min, day, month, year;
	uint8 Retval = FALSE;
	
	seconds  = (*(DateTime + 0) - '0') * 10;
	seconds += (*(DateTime + 1) - '0');

	min      = (*(DateTime + 2) - '0') * 10;
	min     += (*(DateTime + 3) - '0');

	hour  	 = (*(DateTime + 4) - '0') * 10;
	hour    += (*(DateTime + 5) - '0');

	day  	 = (*(DateTime + 8) - '0') * 10;
	day 	+= (*(DateTime + 9) - '0');

	month  	 = (*(DateTime + 10) - '0') * 10;
	month 	+= (*(DateTime + 11) - '0');

	year  	 = (*(DateTime + 12) - '0') * 10;
	year 	+= (*(DateTime + 13) - '0');

	if( seconds < 60 )
		if( min < 60 )
			if( hour < 24 )
				if( day < 32 )
					if( month < 13 )
						if( year < 47 )
							Retval = TRUE;
	return(Retval);
}
