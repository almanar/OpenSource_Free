#ifndef _KEYPAD_H
	#define _KEYPAD_H		
	
	#define KEY_1		'1'
	#define KEY_2		'2'
	#define KEY_3		'3'
	#define KEY_4		'4'
	#define KEY_5		'5'
	#define KEY_6		'6'
	#define KEY_7       '7'
	#define KEY_8       '8'
	#define KEY_9       '9'
	#define KEY_0       '0'
	#define KEY_F1		'K'
	#define KEY_F2      'L'
	#define KEY_MENU    'M'
	#define KEY_ENTER   'N'
	#define KEY_UP		'O'	
	#define KEY_DOWN	'P'
	#define KEY_PANIC	'Q'
	
	#define KP_RETLINEHIGH 		(uint32)0
	#define KP_RETLINELOW		(uint32)1

	#define KP_KEYNONE			(uint32)0
	#define KP_KEYDEBOUNCETIME	(uint32)5

	#define KP_ROW1	( 1<<17 )
	#define KP_ROW2	( 1<<28 )
	#define KP_ROW3	( 1<<29 )
	#define KP_ROW4	( 1<<30 )

	#define KP_COL1 ( 1<<16 )
	#define KP_COL2	( 1UL<<31 )
	#define KP_COL3	( 1UL<<31 )
	#define KP_COL4	( 1<<2  )
	
	#define KP_RETURNLINES			4 //0.10 11 12 13
	#define KP_SCANLINES			4 //0.26 27 28 29

	#define KP_MAKESCANLINESHIGH	{	SET_PORT1_PIN_HIGH(KP_COL1); SET_PORT0_PIN_HIGH(KP_COL2); SET_PORT1_PIN_HIGH(KP_COL3); SET_PORT0_PIN_HIGH(KP_COL4); }

	#define KP_MAKESCANLINESLOW		{	SET_PORT1_PIN_LOW(KP_COL1); SET_PORT0_PIN_LOW(KP_COL2); SET_PORT1_PIN_LOW(KP_COL3); SET_PORT0_PIN_LOW(KP_COL4); }

	#define KP_MAKERETLINESHIGH		{	SET_PORT1_PIN_HIGH(KP_ROW1); SET_PORT0_PIN_HIGH(KP_ROW2); SET_PORT0_PIN_HIGH(KP_ROW3); SET_PORT0_PIN_HIGH(KP_ROW4); }

	#define KP_MAKERETLINESLOW	    {	SET_PORT1_PIN_LOW(KP_ROW1);	SET_PORT0_PIN_LOW(KP_ROW2);	SET_PORT0_PIN_LOW(KP_ROW3);	SET_PORT0_PIN_LOW(KP_ROW4);	}

	void Keypad_Init( void );
	uint32 Keypad_GetKey( void );
	void Process_KeyPad ( void );
	
	void Keypad_ProcessKey( void );

	void DisplayPartialMessage( uint8 MessageNo );
	void DisplayMessage( uint8 MessageIndex );
	void ResetAllPartialMessages( void );
	uint32 GetNumberFromUser(uint8 *Buffer, uint8 Count);
	void LockKey( uint32 Key );
	void UnLockKey( uint32 Key );
	uint8 IsKeyLocked( uint32 Key );

#endif //_KEYPADPRV_H
