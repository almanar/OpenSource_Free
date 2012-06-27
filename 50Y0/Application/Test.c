#include "LPC214x.h"
#include "Types.h"
#include "Global.h"
#include "Timer.h"
#include "Lcd.h"

void main ( void )
{
	
	init_timer0();
	enable_timer(0);
	Delay(1000);
	LCD_Init();
	LCD_Clear();
	LCD_DisplayString(1,1,"Welcome To Arya Omnitalk");
	
	
	SET_PORT1_PIN_DIRN_OUT(1<<24);
	LCD_BKLIT_OFF;
	while(1)
	{		
		LCD_BKLIT_OFF;
		Delay(10);
		LCD_BKLIT_ON;
		Delay(10);
	}
}