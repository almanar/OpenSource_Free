#include "LPC214x.h"
#include "Types.h"
#include "Uart.h"
#include "Timer.h"
#include "MiscFunctions.h"
#include "LCD.h"
#include "Global.h"
#include "Keypad.h"
#include <string.h>
#include "Meter.h"	
#include "Dataflash.h"
#include "WDT.h"
													/* Ret Line1   Ret Line2	Ret Line3	Ret Line4 */
const uint8	KP_KeyMatrix[4][4] = /* Scan Line 1 */  {{ KEY_5,      KEY_2,		KEY_UP,   	KEY_4  		},    
							     /* Scan Line 2 */  {  KEY_6, 	   KEY_8,  	    KEY_ENTER,  KEY_0 		},    
							     /* Scan Line 3 */  {  KEY_1,      KEY_MENU, 	KEY_3,    	KEY_F1 		},   
							     /* Scan Line 4 */  {  KEY_7,      KEY_DOWN, 	KEY_9, 		KEY_PANIC	}};
										
static uint32 Keypad_IsRetLineLow( uint32 LineNo );
static void Keypad_MakeScanLineLow( uint32 LineNo );
/******************************************************************************
< Function Prototypes >
******************************************************************************/

/******************************************************************************
 *
 *	Function Name 		: 
 *
 *  Description   		: 
 *
 *  Passed Parameters 	: 
 *
 *  Modified Data     	: 
 *
 *  Return Value      	: 
 *
******************************************************************************/
void Keypad_Init( void )
{
	SET_PORT1_PIN_DIRN_IN(KP_ROW1);		//ROW1
	SET_PORT0_PIN_DIRN_IN(KP_ROW2);		//ROW2
	SET_PORT0_PIN_DIRN_IN(KP_ROW3);		//ROW3
	SET_PORT0_PIN_DIRN_IN(KP_ROW4);		//ROW4
	SET_PORT1_PIN_DIRN_OUT(KP_COL1);	//COL1
	SET_PORT0_PIN_DIRN_OUT(KP_COL2);	//COL2
	SET_PORT1_PIN_DIRN_OUT(KP_COL3);	//COL3
	SET_PORT0_PIN_DIRN_OUT(KP_COL4);	//COL4

	SET_PORT1_PIN_HIGH(KP_ROW1);		//ROW1
	SET_PORT0_PIN_HIGH(KP_ROW2);		//ROW2
	SET_PORT0_PIN_HIGH(KP_ROW3);		//ROW3
	SET_PORT0_PIN_HIGH(KP_ROW4);		//ROW4
	SET_PORT1_PIN_HIGH(KP_COL1);		//COL1
	SET_PORT0_PIN_HIGH(KP_COL2);		//COL2
	SET_PORT1_PIN_HIGH(KP_COL3);		//COL3
	SET_PORT0_PIN_HIGH(KP_COL4);		//COL4

	return;
}

/******************************************************************************
 *
 *	Function Name 		: 
 *
 *  Description   		: 
 *
 *  Passed Parameters 	: 
 *
 *  Modified Data     	: 
 *
 *  Return Value      	: 
 *
******************************************************************************/
uint32 Keypad_GetKey( void )
{
	uint32 localRetlineCount;
	uint32 localScanlineCount;
	uint32 localRetline;	
	uint32 localKeyValue;
	static uint32 localRetKeyValue;
	
	static uint32 localPrevKeyValue = KP_KEYNONE;
	static uint32 localKeyDebounceTimer;

	localRetKeyValue = 0;
	localRetline = KP_RETLINEHIGH;
	localKeyValue = KP_KEYNONE;
	localRetKeyValue = KP_KEYNONE;

	KP_MAKESCANLINESLOW;	
	
	// Check if Any Return Line is low
	for( localRetlineCount=0; localRetlineCount<KP_RETURNLINES; localRetlineCount++ )
	{
		localRetline = Keypad_IsRetLineLow( localRetlineCount );
		
		if( localRetline == KP_RETLINELOW )
		{
			break;
		}
	}

	if ( localRetline == KP_RETLINELOW )
	{
		KP_MAKERETLINESHIGH;		

		for ( localScanlineCount=0; localScanlineCount<KP_SCANLINES; localScanlineCount++ )
		{			
			KP_MAKESCANLINESHIGH;

			Keypad_MakeScanLineLow( localScanlineCount );

			localRetline = Keypad_IsRetLineLow( localRetlineCount );								 	
			 
			if ( localRetline == KP_RETLINELOW )
			{
				localKeyValue = KP_KeyMatrix[localScanlineCount][localRetlineCount]; 								
				break;		
			}
		}		
		//If Key is pressed check for debounce 
		if ( localKeyValue == localPrevKeyValue )
		{
			if( localKeyDebounceTimer == 0 )
			{
				localKeyDebounceTimer = MS_TIMER + KP_KEYDEBOUNCETIME;
			}				
		}								
	}	

	if( MS_TIMER > localKeyDebounceTimer &&  localKeyDebounceTimer != 0 && localKeyValue == KP_KEYNONE )
	{
		if( localPrevKeyValue != KP_KEYNONE )	
		{	
			localRetKeyValue = localPrevKeyValue;
			localPrevKeyValue = KP_KEYNONE;	
			localKeyDebounceTimer = 0;
			LCD_BKLIT_ON;
			SETFLAG(LCD_BACKLIT_ON);
			BacklitTimeoutTimer = MS_TIMER + 30000;
		}
		else
		{				
			localKeyDebounceTimer = 0;		
		}  	
	}
		
	localPrevKeyValue = localKeyValue;	   	
	return( localRetKeyValue );
}

/******************************************************************************
 *
 *	Function Name 		: 
 *
 *  Description   		: 
 *
 *  Passed Parameters 	: 
 *
 *  Modified Data     	: 
 *
 *  Return Value      	: 
 *
******************************************************************************/
void Process_KeyPad ( void )
{	
	uint32 Key;
	uint8 ShiftId[4] = { 0x00, 0x00 ,0x00, 0x01 };
	static uint8 PartialMessage = 0;
	static uint32 localLastKey = 0;
	static uint32 LastKeyPressed = 0;
	static uint8 LastKeyCount = 0;	
	uint8 temp[24];
	uint8 lclCount;
	

	Key = Keypad_GetKey();
	if( localLastKey != Key )
	{								   	
		if( IsKeyLocked( Key ) == TRUE )
		{
			return;			
		}
		
		switch( Key )
		{
			case KEY_UP:			
				if(CHECKFLAG(SMS_RECEIVED))
				{					
					if(strlen((char *)TempBuffer)>160)
					{						
						LCD_Clear();
						LCD_DisplayScreen(&TempBuffer[4]);
						LCD_Cursor(4,38);
						LCD_DisplayCharacter(4,'>');
						LCD_DisplayCharacter(4,'>');
						LCD_DisplayCharacter(4,'>');
					}
				}				
			break;

			case KEY_DOWN:
				if(CHECKFLAG(SMS_RECEIVED))
				{
					if(strlen((char *)TempBuffer)>160)
					{
						LCD_Clear();
						LCD_Cursor(4,38);
						LCD_DisplayCharacter(1,'<');
						LCD_DisplayCharacter(1,'<');
						LCD_DisplayCharacter(1,'<');
						LCD_DisplayString(1,4,&TempBuffer[157]);
						LCD_DisplayString(2,1,&TempBuffer[194]);
						LCD_DisplayString(3,1,&TempBuffer[234]);
						LCD_DisplayString(4,1,&TempBuffer[274]);							
					}
				}
			break;

			case KEY_0:
				/********************************/
				if( CHECKFLAG(MENU_LEVEL_TWO) )
				{		
					PartialMessage = 10;
					DisplayPartialMessage(10);
				}
				else if(CHECKFLAG(MENU_LEVEL_ONE))
				{
					SETFLAG( RESET_DELAY );
					LCD_Cursor(4,40);	LCD_DisplayCharacter(4,'*');
					Delay(500);
					LCD_Cursor(4,40);	LCD_DisplayCharacter(4,' ');
				}
				else if( CHECKFLAG(MENU_LEVEL_ZERO) && (!CHECKFLAG(SMS_RECEIVED)) )
				{
//					if( MS_TIMER > PickUpTimer )
//					{
						RESETFLAG(SMS_RECEIVED);
						RESETFLAG(MENU_LEVEL_ONE);
						RESETFLAG(MENU_LEVEL_TWO);					
						SendMessage(0);										
//					}
				}
				/********************************/		
			break;
	
			case KEY_1:
				/********************************/
				if( CHECKFLAG(MENU_LEVEL_ONE) )
				{
					RESETFLAG(MENU_LEVEL_ONE);
					SETFLAG(MENU_LEVEL_TWO);
					PartialMessage = 1;
					DisplayPartialMessage(1);
				}
				else if( CHECKFLAG(MENU_LEVEL_ZERO) )
				{					
					RESETFLAG(SMS_RECEIVED);
					RESETFLAG(MENU_LEVEL_ONE);
					RESETFLAG(MENU_LEVEL_TWO);					
					SendMessage(1);
				}
				else if( CHECKFLAG(MENU_LEVEL_TWO) )
				{
					PartialMessage = 1;
					DisplayPartialMessage(1);
				}	
				/********************************/
			break;
	
			case KEY_2:
				/********************************/
				if( CHECKFLAG(MENU_LEVEL_TWO) )
				{		
					PartialMessage = 2;
					DisplayPartialMessage(2);
				}
				else if( CHECKFLAG(MENU_LEVEL_ZERO) && (!CHECKFLAG(SMS_RECEIVED)) )
				{
					SendMessage(2);
				}	
				/********************************/
			break;
	
			case KEY_3:
				if( CHECKFLAG(MENU_LEVEL_TWO) )
				{		
					PartialMessage = 3;
					DisplayPartialMessage(3);
				}
				else if( CHECKFLAG(MENU_LEVEL_ZERO) && (!CHECKFLAG(SMS_RECEIVED)) )
				{					
					
					if(0)		 //Airport Fee is Disable
					{
							TempBuffer[ 0 ] = '-';
							DF_ReadBytesFromPage( 16, &TempBuffer[ 1 ], 7, 19 );
							TempBuffer[ 20 ] = '\0';
							strcpy( MeterAddkeysStr, &TempBuffer[ 0 ] );
							MeterAddkeysStr[ 19 ] = 0x00;	MeterAddkeysStr[ 20 ] = 0x00;	MeterAddkeysStr[ 21 ] = 0x00;
							
							if( MeterAddkeys[ 0 ] != 0 )
							{						
								DF_ReadBytesFromPage( 16, &TempBuffer[ 0 ], 1, 6 );
								AirportFees = ConvAscToHex( &TempBuffer[ 0 ], 6  );
								MeterAddkeysStr[ 21 ] = ( uint8 )( AirportFees >> 0 );
								MeterAddkeysStr[ 20 ] = ( uint8 )( AirportFees >> 8 );
								MeterAddkeysStr[ 19 ] = ( uint8 )( AirportFees >> 16 );
								
								SendMessage( MCT_AIRPORT_ADD );						
								SendCommandToMeter( MCT_ADD_KEY_1, &MeterAddkeysStr[ 0 ], TRUE );												
							}
							else
							{
								SendMessage( MCT_AIRPORT_CANCEL );
								SendCommandToMeter( MCT_ADD_KEY_1, &MeterAddkeysStr[ 0 ], TRUE );
							}
							MeterAddkeys[ 0 ] ^= 1;
					}

				}
				else if( CHECKFLAG(MENU_LEVEL_ONE))	
				{
					CopyToAckBuffer(SMS_ACK, "O1K12340F", 9);
				}
			break;
	
			case KEY_4:
				/********************************/
				if( CHECKFLAG(MENU_LEVEL_TWO) )
				{		
					PartialMessage = 4;
					DisplayPartialMessage(4);
				}
				else if( CHECKFLAG(MENU_LEVEL_ONE) )
				{		
					SETFLAG(DIAL_FIXED_NO);				
					while(CHECKFLAG(DIAL_FIXED_NO))
					{
						DialFixedNo();						
					}
					
					if(CHECKFLAG(MENU_LEVEL_ONE))
					{
						DisplayMainMenu();
					}
				}
				else if( CHECKFLAG(MENU_LEVEL_ZERO) && (!CHECKFLAG(SMS_RECEIVED)) )
				{
					SendMessage(4);					
				}
			break;
	
			case KEY_5:
				if( CHECKFLAG(MENU_LEVEL_ZERO) && (!CHECKFLAG(SMS_RECEIVED)) )
				{
					//if( MS_TIMER > LoginLockTimer )
					//{
					//	LogoutError = FALSE;
					//	LoginError = FALSE;		
						LogUserIn();
						KerbDuty = TRUE;
						CtrlDuty = FALSE;
						SendCommandToMeter( MCT_START_SHIFT, &ShiftId[0] , TRUE );
					//}
				}			
				else if( CHECKFLAG(MENU_LEVEL_TWO) )
				{		
					PartialMessage = 5;
					DisplayPartialMessage(5);
				}
			break;
	
			case KEY_6:
				if( CHECKFLAG(MENU_LEVEL_ZERO) && (!CHECKFLAG(SMS_RECEIVED)) )
				{
					//if( MS_TIMER > LogoutLockTimer )
					//{
					//	LogoutError = FALSE;
					//	LoginError = FALSE;		
						LogUserOut();
						KerbDuty = FALSE;
						CtrlDuty = FALSE;
						//SendCommandToMeter( MCT_CLOSE_SHIFT, &Dummybyte, TRUE );
					//}
				}			
				else if( CHECKFLAG(MENU_LEVEL_TWO) )
				{		
					PartialMessage = 6;
					DisplayPartialMessage(6);
				}	
 			break;
	
			case KEY_7:
				/********************************/
				if( CHECKFLAG(MENU_LEVEL_TWO) )
				{		
					PartialMessage = 7;
					DisplayPartialMessage(7);
				}
				else if( CHECKFLAG(MENU_LEVEL_ONE) )
				{		
					SETFLAG(DIAL_OTHER_NO);				
					while(CHECKFLAG(DIAL_OTHER_NO))
					{
						DialOtherNo();						
					}
					if( CHECKFLAG(MENU_LEVEL_ONE))
					{
						DisplayMainMenu();
					}
				}
				else if( CHECKFLAG(MENU_LEVEL_ZERO) )
				{
					RESETFLAG(SMS_RECEIVED);
					SendMessage(7);
				}	
				/********************************/
			break;					
	
			case KEY_8:
				/********************************/
				if( CHECKFLAG(MENU_LEVEL_TWO) )
				{		
					PartialMessage = 8;
					DisplayPartialMessage(8);
				}
				else if( CHECKFLAG(MENU_LEVEL_ZERO) && (!CHECKFLAG(SMS_RECEIVED)) )
				{
					SendMessage(8);
				}
				else if( CHECKFLAG(MENU_LEVEL_ONE))
				{
					DF_ReadBytesFromPage( 15, &PartialMessage, 0, 1 );
					if( 0 )//Removed as per instructions from Mr.S Dhru & Mr. K Shah on 20/10/10
					{
						LCD_Clear();
						LCD_DisplayStringCentered(2,"ERASING MEMORY...");
						LCD_DisplayStringCentered(3,"PLEASE WAIT...");
						LCD_BKLIT_ON;
						SETFLAG(LCD_BACKLIT_ON);
						BacklitTimeoutTimer = MS_TIMER + 180000;
						DF_ChipErase();
						LCD_Clear();
						LCD_DisplayStringCentered(2,"RESETTING UNIT...");
						Delay(500);
						SETFLAG( STOP_PULSES );
						WDTInit( );
					}
				}							 	
				/********************************/
				/********************************/
			break;
	
			case KEY_9:
				/********************************/
				if( CHECKFLAG(MENU_LEVEL_TWO) )
				{		
					PartialMessage = 9;
					DisplayPartialMessage(9);
				}
				else if( CHECKFLAG(MENU_LEVEL_ZERO) && (!CHECKFLAG(SMS_RECEIVED)) )
				{
					SendMessage(9);				
				}	
				/********************************/
			break;
			case KEY_F1:
				if ((CHECKFLAG(MENU_LEVEL_ZERO)) && (!CHECKFLAG(SMS_RECEIVED)))
				{
					LCD_Clear();//LCDOK
					LCD_DisplayString(2,16, "FUNCTION 1\0");
				  	LCD_DisplayString(4,36, "V60A0\0");
					LCD_Cursor(4,1);
					LCD_DisplayCharacter(4,'V');
					LCD_DisplayCharacter(4,GPSVersion[0]);
					LCD_DisplayCharacter(4,'.');
					LCD_DisplayCharacter(4,GPSVersion[1]);
					LCD_DisplayCharacter(4,GPSVersion[2]);
					
					//This portion has been removed since 4.0e to accomodate meter enable/disable related changes
					if(0)
					{
						CopyToAckBuffer(SMS_ACK, "O1K12340B", 9);
					}

					//This portion has been added since 4.0e to accomodate meter enable/disable related changes
					if( MeterDisabled != TRUE )
					{
						CopyToAckBuffer(SMS_ACK, "O1K12340BF", 10);
					}
					else
					{
						CopyToAckBuffer(SMS_ACK, "O1K12340B0", 10);
					}

					Delay(2000);
					DisplayWelcomeScreen();
				}
				else if(CHECKFLAG(MENU_LEVEL_ONE))
				{
					if( 0 )//Removed as per instructions from Mr.S Dhru & Mr. K Shah on 20/10/10
					{
						LCD_Clear();
						LCD_DisplayString(2,16, "TEST MODE.\0");
						Delay(1000);
						SaveSMS("1234Test Message",16,1,SAVE_MSG);
						SaveSMS("1234Test Message",16,2,SAVE_MSG);
						SaveSMS("1234Test Message",16,3,SAVE_MSG);
						SaveSMS("1234Test Message",16,4,SAVE_MSG);
						SaveSMS("1234Test Message",16,5,SAVE_MSG);
						SaveSMS("1234Test Message",16,6,SAVE_MSG);
						SaveSMS("1234Test Message",16,7,SAVE_MSG);
						SaveSMS("1234Test Message",16,8,SAVE_MSG);
						SaveSMS("1234Test Message",16,9,SAVE_MSG);
						SaveSMS("1234Test Message",16,10,SAVE_MSG);
	
						Delay(2000);
						SET_PORT0_PIN_HIGH(1<<18);
						Delay(500);
						SET_PORT0_PIN_LOW(1<<18);
						Delay(500); 
						LCD_Clear();
						LCD_DisplayString(2,16, "RESET GSM.\0");				
						Delay(2000);
						LCD_Clear();
						LCD_DisplayString(2,16, "RESET MOD.\0");									
						SETFLAG( STOP_PULSES );
						SETFLAG(TEST_MODE);
						WDTInit( );
					}	
				}
			break;
	
			case KEY_F2:
				if ((CHECKFLAG(MENU_LEVEL_ZERO)) && (!CHECKFLAG(SMS_RECEIVED)))
				{
					LCD_Clear();//LCDOK
					LCD_DisplayString(2,16, "FUNCTION 2\0");
					CopyToAckBuffer(SMS_ACK, "O1K1234F2", 9);
					Delay(2000);
					DisplayWelcomeScreen();
				}
			break;

			case KEY_MENU:
				/********************************/
				if( CHECKFLAG(MENU_LEVEL_ZERO) && (!CHECKFLAG(SMS_RECEIVED)) )
				{
					RESETFLAG(MENU_LEVEL_ZERO);
					SETFLAG(MENU_LEVEL_ONE);
					DisplayMainMenu();
				}			
				/********************************/
				else if( CHECKFLAG(MENU_LEVEL_ONE) )
				{
					RESETFLAG(MENU_LEVEL_ONE);
					SETFLAG(MENU_LEVEL_ZERO);
					DisplayWelcomeScreen();
				}	
				/********************************/
				else if( CHECKFLAG(MENU_LEVEL_TWO) )
				{
					if(CHECKFLAG(FULL_MESSAGE_DISPLAY))
					{
						RESETFLAG(FULL_MESSAGE_DISPLAY);
						DisplayPartialMessage(PartialMessage);
					}
					else
					{
						RESETFLAG(MENU_LEVEL_TWO);
						SETFLAG(MENU_LEVEL_ONE);
						DisplayMainMenu();
					}	
				}
			break;
	
			case KEY_PANIC:
				SETFLAG(MENU_LEVEL_ZERO);
				RESETFLAG(MENU_LEVEL_ONE);
				RESETFLAG(MENU_LEVEL_TWO);
				RESETFLAG(FULL_MESSAGE_DISPLAY);
				RESETFLAG(DIAL_FIXED_NO);
				RESETFLAG(DIAL_OTHER_NO);
				RESETFLAG(SMS_RECEIVED);	
				LCD_Clear();
				LCD_DisplayString(2,18, "PANIC\0");
				CopyToAckBuffer(SMS_ACK, "O1K12340A", 9);
				Delay(2000);
				DisplayWelcomeScreen();
			break;
	
			case KEY_ENTER:
				/********************************/
				if( CHECKFLAG(MENU_LEVEL_TWO) )
				{						
					SETFLAG(FULL_MESSAGE_DISPLAY);
					DisplayMessage(PartialMessage);
				}
				else if( CHECKFLAG(MENU_LEVEL_ONE))
				{
				   	if( CHECKFLAG( RESET_DELAY ) )
					{
						RESETFLAG( RESET_DELAY );
						LCD_Cursor(4, 40);
						LCD_DisplayCharacter(4,'@');
						Delay(1000);
						RESETFLAG(LIVE_PULSE);
						livepulsetimer = MS_TIMER + 300000;
					}
					
					LCD_Init();		
					Delay(1000);  
					SETFLAG(MENU_LEVEL_ZERO);
					RESETFLAG(MENU_LEVEL_ONE);
					RESETFLAG(MENU_LEVEL_TWO);
					RESETFLAG(FULL_MESSAGE_DISPLAY);
					RESETFLAG(DIAL_FIXED_NO);
					RESETFLAG(DIAL_OTHER_NO);
					DisplayWelcomeScreen();
				}
				/********************************/
			break;
		}			

		if( Key != KP_KEYNONE )
		{
			if( LastKeyPressed != Key )
			{
				LastKeyCount = 0;
				KeyRepeatTimer = MS_TIMER + 60000;
			}
			
			LastKeyPressed = Key;
			LastKeyCount++;
		}
				
		if( LastKeyCount > 30 )
		{
			if( KeyRepeatTimer > MS_TIMER )
			{				
				LockKey( Key );
				if( Key == KEY_ENTER || Key == KEY_UP || Key == KEY_DOWN || Key == KEY_MENU )
				{
					LastKeyCount = 0;
					KeyRepeatTimer = MS_TIMER + 60000;
				}
				else
				{
					temp[0] = 'O';
					temp[1] = '1';
					temp[2] = 'K';
					temp[3] = '1';
					temp[4] = '2';
					temp[5] = '3';
					temp[6] = '4';
					temp[7] = 'G';
					temp[8] = 'G';		 
					
					temp[9 ] = (KeyStatus >> 12) & 0x0F;
					temp[10] = (KeyStatus >> 8)	 & 0x0F;
					temp[11] = (KeyStatus >> 4)	 & 0x0F;
					temp[12] = (KeyStatus >> 0)	 & 0x0F;

					for( lclCount=0; lclCount<4; lclCount++ )
					{
						if( temp[ 9 + lclCount ] < 0x0A )
						{
							temp[ 9 + lclCount ] = temp[ 9 + lclCount ] + 0x30;
						}
						else
						{
							temp[ 9 + lclCount ] = temp[ 9 + lclCount ] + 0x37;
						}
					}
					CopyToAckBuffer(SMS_ACK, &temp[0], 13);
					
					temp[ 8 ] = ( KeyStatus >> 8 ) & 0xFF; 
					temp[ 9 ] = ( KeyStatus >> 0 ) & 0xFF; 
					DF_ErasePage(14);
					DF_WriteBytesToPage( 14, &temp[8], 0, 2 );
				}				
			}
			else
			{
				LastKeyCount = 0;
				KeyRepeatTimer = MS_TIMER + 60000;
			}
		}
	}	
localLastKey = Key;
}

/******************************************************************************
 *
 *	Function Name 		: 
 *
 *  Description   		: 
 *
 *  Passed Parameters 	: 
 *
 *  Modified Data     	: 
 *
 *  Return Value      	: 
 *
******************************************************************************/
static uint32 Keypad_IsRetLineLow( uint32 LineNo )
{
	uint32 localRetVal;
	
	localRetVal	= KP_RETLINEHIGH;
	
	KP_MAKERETLINESHIGH;
	
	switch( LineNo )
	{
		case 0:
			if(!( IOPIN1 & (KP_ROW1)))
			{
				localRetVal	= KP_RETLINELOW;
			}
		break;
		
		case 1:
			if(!( IOPIN0 & (KP_ROW2)))
			{
				localRetVal	= KP_RETLINELOW;
			}
		break;
		
		case 2:
			if(!( IOPIN0 & (KP_ROW3)))
			{
				localRetVal	= KP_RETLINELOW;
			}
		break;
		
		case 3:
			if(!( IOPIN0 & (KP_ROW4)))
			{
				localRetVal	= KP_RETLINELOW;
			}
		break;
		
		default:
			localRetVal	= KP_RETLINEHIGH;
		break;				
	}
	
	return( localRetVal );
}

/******************************************************************************
 *
 *	Function Name 		: 
 *
 *  Description   		: 
 *
 *  Passed Parameters 	: 
 *
 *  Modified Data     	: 
 *
 *  Return Value      	: 
 *
******************************************************************************/
static void Keypad_MakeScanLineLow( uint32 LineNo )
{
	switch( LineNo )
	{
		case 0:
			SET_PORT1_PIN_LOW(KP_COL1);//COL1
		break;
		
		case 1:
			SET_PORT0_PIN_LOW(KP_COL2);//COL2
		break;
		
		case 2:
			SET_PORT1_PIN_LOW(KP_COL3);//COL3
		break;
		
		case 3:
			SET_PORT0_PIN_LOW(KP_COL4);//COL4
		break;
		
		default:
		break;
	}
	return;	
}

void LockKey( uint32 Key )
{
	switch( Key )
	{
		case (KEY_1)://		'1'
			KeyStatus |= ( 1 << 0 );
		break;

		case (KEY_2)://		'2'
			KeyStatus |= ( 1 << 1 );
		break;

		case (KEY_3)://		'3'
			KeyStatus |= ( 1 << 2 );
		break;

		case (KEY_4)://		'4'
			KeyStatus |= ( 1 << 3 );
		break;

		case (KEY_5)://		'5'
			KeyStatus |= ( 1 << 4 );
		break;

		case (KEY_6)://		'6'
			KeyStatus |= ( 1 << 5 );
		break;

		case (KEY_7)://     '7'
			KeyStatus |= ( 1 << 6 );
		break;

		case (KEY_8)://     '8'
			KeyStatus |= ( 1 << 7 );
		break;

		case (KEY_9)://     '9'
			KeyStatus |= ( 1 << 8 );
		break;

		case (KEY_0)://     '0'
			KeyStatus |= ( 1 << 9 );
		break;

		case (KEY_F1)://	'K'
			KeyStatus |= ( 1 << 10 );
		break;

		case (KEY_F2)://    'L'
			KeyStatus |= ( 1 << 11 );
		break;

//		case (KEY_MENU)://  'M'
//			KeyStatus |= ( 1 << 12 );
//		break;
//
//		case (KEY_ENTER):// 'N'
//			KeyStatus |= ( 1 << 13 );
//		break;
//
//		case (KEY_UP)://	'O'	
//			KeyStatus |= ( 1 << 14 );
//		break;
//
//		case (KEY_DOWN)://	'P'
//			KeyStatus |= ( 1 << 15 );
//		break;

		case (KEY_PANIC)://	'Q'
			KeyStatus |= ( 1 << 12 );
		break;
	}
}

void UnLockKey( uint32 Key )
{
	switch( Key )
	{
		case (KEY_1)://		'1'
			KeyStatus &= ~( 1 << 0 );
		break;

		case (KEY_2)://		'2'
			KeyStatus &= ~( 1 << 1 );
		break;

		case (KEY_3)://		'3'
			KeyStatus &= ~( 1 << 2 );
		break;

		case (KEY_4)://		'4'
			KeyStatus &= ~( 1 << 3 );
		break;

		case (KEY_5)://		'5'
			KeyStatus &= ~( 1 << 4 );
		break;

		case (KEY_6)://		'6'
			KeyStatus &= ~( 1 << 5 );
		break;

		case (KEY_7)://     '7'
			KeyStatus &= ~( 1 << 6 );
		break;

		case (KEY_8)://     '8'
			KeyStatus &= ~( 1 << 7 );
		break;

		case (KEY_9)://     '9'
			KeyStatus &= ~( 1 << 8 );
		break;

		case (KEY_0)://     '0'
			KeyStatus &= ~( 1 << 9 );
		break;

		case (KEY_F1)://	'K'
			KeyStatus &= ~( 1 << 10 );
		break;

		case (KEY_F2)://    'L'
			KeyStatus &= ~( 1 << 11 );
		break;

//		case (KEY_MENU)://  'M'
//			KeyStatus &= ~( 1 << 12 );
//		break;
//
//		case (KEY_ENTER):// 'N'
//			KeyStatus &= ~( 1 << 13 );
//		break;
//
//		case (KEY_UP)://	'O'	
//			KeyStatus &= ~( 1 << 14 );
//		break;
//
//		case (KEY_DOWN)://	'P'
//			KeyStatus &= ~( 1 << 15 );
//		break;	 

		case (KEY_PANIC)://	'Q'
			KeyStatus &= ~( 1 << 12 );
		break;
	}
}

uint8 IsKeyLocked( uint32 Key )
{
	uint8 KeyState = 0;
	switch( Key )
	{
		case (KEY_1)://		'1'
			if( KeyStatus & ( 1 << 0 )){ KeyState = 1; }
		break;

		case (KEY_2)://		'2'
			if( KeyStatus & (1 << 1 )){ KeyState = 1; }
		break;

		case (KEY_3)://		'3'
			if( KeyStatus & (1 << 2 )){ KeyState = 1; }
		break;

		case (KEY_4)://		'4'
			if( KeyStatus & (1 << 3 )){ KeyState = 1; }
		break;

		case (KEY_5)://		'5'
			if( KeyStatus & (1 << 4 )){ KeyState = 1; }
		break;

		case (KEY_6)://		'6'
			if( KeyStatus & (1 << 5 )){ KeyState = 1; }
		break;

		case (KEY_7)://     '7'
			if( KeyStatus & (1 << 6 )){ KeyState = 1; }
		break;

		case (KEY_8)://     '8'
			if( KeyStatus & (1 << 7 )){ KeyState = 1; }
		break;

		case (KEY_9)://     '9'
			if( KeyStatus & (1 << 8 )){ KeyState = 1; }
		break;

		case (KEY_0)://     '0'
			if( KeyStatus & (1 << 9 )){ KeyState = 1; }
		break;

		case (KEY_F1)://	'K'
			if( KeyStatus & (1 << 10 )){ KeyState = 1; }
		break;

		case (KEY_F2)://    'L'
			if( KeyStatus & (1 << 11 )){ KeyState = 1; }
		break;

//		case (KEY_MENU)://  'M'
//			if( KeyStatus & (1 << 12 )){ KeyState = 1; }
//		break;
//
//		case (KEY_ENTER):// 'N'
//			if( KeyStatus & (1 << 13 )){ KeyState = 1; }
//		break;
//
//		case (KEY_UP)://	'O'	
//			if( KeyStatus & (1 << 14 )){ KeyState = 1; }
//		break;
//
//		case (KEY_DOWN)://	'P'
//			if( KeyStatus & (1 << 15 )){ KeyState = 1; }
//		break;

		case (KEY_PANIC)://	'Q'
			if( KeyStatus & (1 << 12 )){ KeyState = 1; }
		break;
	}
	
	return(KeyState);	
}
