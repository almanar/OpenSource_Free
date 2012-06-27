#ifdef __CUSTOMER_CODE__
 
#include "ql_appinit.h"
#include "ql_interface.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_audio.h"
#include "ql_timer.h"
#include "ql_stdlib.h"

/*****************************************************
use lib
run "makelib example_customerlib new"
gen customerlib\example_customerlib.a
modify LIBS=customerlib\example_customerlib.a in makefile
run "make128 custom" or  "make custom"
*********************************************************/

//extern void Customer_Lib_Interface1(void* dest, u8 value, u32 size);
//extern u32 Customer_Lib_Interface2(void);

void ql_entry()
{
	bool           keepGoing = TRUE;
	QlEventBuffer    flSignalBuffer;

	Ql_DebugTrace("ql_entry ");
	Ql_SetDebugMode(BASIC_MODE);   

        //Customer_Lib_Interface1((void*)&flSignalBuffer, 0, sizeof(QlEventBuffer));
        //Customer_Lib_Interface2();
         
	while(keepGoing)
	{	
		Ql_GetEvent(&flSignalBuffer);
		switch(flSignalBuffer.eventType)
		{

			default:
				break;
		}
	}
}


#endif 



