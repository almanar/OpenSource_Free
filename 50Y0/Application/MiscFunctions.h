#ifndef _MISCFUNCTIONS_H
#define _MISCFUNCTIONS_H

	#define METER_BUFFER	1
	#define SMS_BUFFER		2
	#define COMMAND_BUFFER	3
	#define ACK_BUFFER		4

	#define METER_BUF_INDEX	8
	#define METER_BUFSIZE	512
	#define SMS_BUF_INDEX	8
	#define SMS_BUFSIZE		328
	#define CMD_BUF_INDEX	8
	#define CMD_BUFSIZE		8

	#define SAVE_MSG		1
	#define DISCARD_MSG		2
   	
	#define OUTPUT_1	( 1<<17	)
	#define OUTPUT_2	( 1<<16	)
	#define OUTPUT_3	( 1<<15	)
	#define OUTPUT_4	( 1<<21	)
	#define OUTPUT_5	( 1<<22 )

void DisplayWelcomeScreen( void );
void ProcessUartRxBuffers( void );
void ProcessBuffer( uint8 CopyBuffer, uint8 *SrcBuffer, uint8 Action, uint8 MeterSend );
void ProcessUartTxBuffers( void );
void SaveSMS( uint8 *SMSData, uint32 SMSDataCount, uint8 Location, uint8 Action );
void ProcessIOCommand( uint8 *IOCMDBuffer );
void SendDataToModem( void );
void SendMessage (uint8 Message );
void DisplayMainMenu( void );
void DisplayPartialMessage( uint8 MessageNo );
void DisplayMessage( uint8 MessageIndex );
void DialFixedNo( void );
void DialOtherNo( void );
void LogUserIn( void );
void LogUserOut( void );
void InitIOS( void );
void CheckIOS( void );
void CopyToAckBuffer(uint8 ack_type, uint8 *ack_data, uint8 ack_len);
void GetCreditCardAmount( uint8 *CardData, uint32 Length );
void WaitForAcceptance( void );
uint8 CheckDateTime( uint8 *DateTime );
#endif
