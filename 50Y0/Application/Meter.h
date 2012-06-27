#ifndef _METER_H_
#define _METER_H_
//MCT->TFM COMMANDS
//IMPLEMENTED
#define MCT_REQUEST_STATUS					0x88	//METER SENDS ACK & THEN DATA
#define MCT_ENABLE_METER					0xEE	//METER SENDS ACK
#define MCT_START_SHIFT						0xAA	//4 bytes
#define MCT_CLOSE_SHIFT						0xBB
#define MCT_REQUEST_TRIP_DATA				0x2B    //9 bytes

//NOT IMPLEMENTED
#define MCT_DISABLE_METER					0xDD	//METER SENDS ACK
#define MCT_RECORD_ON_CALL_DISTANCE			0x54
#define MCT_CLEAR_MEMORY					0x99
#define MCT_CONF_TAXI_COMPANY_DETAILS		0xCA   //26 Bytes
#define MCT_PRINT_DAILY_BUSINESS_REPORT		0xDC
#define MCT_PRINT_MONTHLY_BUSINESS_REPORT	0xAE		 
#define MCT_PRINT_CUMULATIVE_REPORT			0xBC
#define MCT_PRINT_TARIFF_STRUCTURE			0xCB
#define MCT_PRINT_CREDIT_CARD_DETAILS		0xAC  //56 Bytes
#define MCT_PRINT_LAST_TRIP_DATA			0xA1
#define MCT_PRINT_LAST_SHIFT_DATA			0x1A
#define MCT_REQUEST_RTC_PARAMETERS			0x1B
#define MCT_CHANGE_RTC_PARAMETERS			0xB1	//8 Bytes
#define MCT_REQUEST_LAST_TRIP_DATA			0xA2
#define MCT_REQUEST_LAST_SHIFT_DATA			0x2A
#define MCT_REQUEST_SHIFT_DATA				0xB2   //5 Bytes
#define MCT_PRINT_ALL_TRIP_DATA_PER_SHIFT	0xA5   //5 Bytes
#define MCT_PRINT_SHIFT_DATA				0x2F   //5 Bytes
#define MCT_QUIT_RECORDING_ON_CALL_DISTANCE 0x3A
#define MCT_CHECK_PRINTER					0xCF
#define MCT_POSITIVE_ACK_FRAME				0x66
#define MCT_NEGATIVE_ACK_FRAME				0x77

//TFM->MCT COMMANDS
#define TFM_SEND_TRIP_START_TIME			0xAB	//ACK EXPECTED
#define TFM_SEND_CURRENT_TRIP_DETAILS		0xBA
#define TFM_SEND_LAST_TRIP_DATA				0xCA	//ACK EXPECTED
#define TFM_SEND_REQUESTED_TRIP_DETAILS		0xAC	//ACK EXPECTED
#define TFM_SEND_SHIFT_START_TIME			0xBC	//ACK EXPECTED
#define TFM_SEND_CURRENT_SHIFT_DETAILS		0xCB	//ACK EXPECTED
#define TFM_SEND_LAST_SHIFT_DETAILS			0x33	//ACK EXPECTED
#define TFM_SEND_REQUESTED_SHIFT_DETAILS	0x44	//ACK EXPECTED
#define TFM_SEND_RTC_PARAMETERS				0xBB	//METER SENDS ACK & THEN DATA
#define TFM_SEND_PRINTER_BUSY				0x88	//METER SENDS ACK & THEN DATA
#define TFM_SEND_PRINTING_OVER				0x66
#define TFM_RES_TO_QUIT_ON_CALL_DIST_CMD	0xCD	//METER SENDS ACK & THEN DATA
#define TFM_SENSOR_TAMPER					0x69
#define TFM_POSITIVE_ACK_FRAME				0x99
#define TFM_NEGATIVE_ACK_FRAME				0xEE

//TFM->CUSTOMISED COMMANDS
#define MCT_ADD_KEY_1						0x11
#define MCT_ADD_KEY_2						0x12
#define MCT_ADD_KEY_3						0x13
#define MCT_ADD_KEY_4						0x14

#define MCT_AIRPORT_ADD						0x3A
#define MCT_AIRPORT_CANCEL					0x3B
#define MCT_CCARD_ADD						0x4A
#define MCT_CCARD_CANCEL					0x4B
#define MCT_CONV_ADD						0x5A
#define MCT_CONV_CANCEL						0x5B
#define MCT_ADDNL_ADD						0x6A
#define MCT_ADDNL_CANCEL					0x6B

struct MeterCommandStruct{
	uint8 Header;
	uint8 Reserved;
	uint8 Datalen;
	uint8 Command;
	uint8 Checksum;
	uint8 Dummy[2];
};

void SendCommandToMeter( uint8 MeterCmd , uint8 *MeterData, uint8 WaitForAck );
void ParseCommandForMeter( uint8 *CommandForMeter );
void SendACKToMeter( void );
void SendNACKToMeter( void );
uint32 ConvAscToHex( uint8* AscData, uint8 Length );
#endif //_METER_H_

