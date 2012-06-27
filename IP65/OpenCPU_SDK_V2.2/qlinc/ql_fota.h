/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2010 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |      APIs for FOTA (Fireware Over The Air) Funcion.
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by     :   Jay XIN
 |  Coded    by     :   Jay XIN
 |  Tested   by     :   Stanley YONG
 |
 \=========================================================================*/

#ifndef __QL_FOTA_H__
#define __QL_FOTA_H__

#include "ql_type.h"

#define FOTA_UPDATE_FLAG_CORE		1
#define FOTA_UPDATE_FLAG_APP		2

typedef enum
{
    FOTA_MODE_NONE = 0,
    FOTA_APP_DELTA,     // FOTA差分升级
    FOTA_APP_COVER      // FOTA覆盖式升级
}QlFotaType;


/*****************************************************************
* Function:     Ql_Fota_Core_Init 
* 
* Description:
*               Initializes the FOTA-Core related functions.
*               Applications must initialize the FOTA function before they call
*               other FOTA-Core related functions.
* Parameters:
*               None
* Return:        
*               None
*****************************************************************/
void Ql_Fota_Core_Init(void);


/*****************************************************************
* Function:     Ql_Fota_Core_Write_Data 
* 
* Description:
*               FOTA write data API.
*                1. This function is used to write data to spare image pool
*                2. This API only allow sequentially writing mechanism
*                3. Authentication mechanism is executed during writing
* Parameters:
*               length: the length of writing (Unit: Bytes)
*               buffer: pointer to the start address of buffer
* Return:        
*               0:   means pass write success
*               < 0: writing action is fail
*****************************************************************/
s32  Ql_Fota_Core_Write_Data(s32 length, s8 * buffer);


/*****************************************************************
* Function:     Ql_Fota_Core_Finish 
* 
* Description:
*               FOTA finalization API.
*                1. compare calculated checksum with image checksum in the header after
*                   whole image is written
*                2. mark the status to UPDATE_NEEDED 
* Parameters:
*               None
* Return:        
*               None
*****************************************************************/
void Ql_Fota_Core_Finish(void);


/*****************************************************************
* Function:     Ql_Fota_App_Init 
* 
* Description:
*               Initializes the FOTA-Application related functions.
*                1. Initialize data structure and progress initial step
*                2. Register customized authentication function or encryption function
* Parameters:
*               update_mode: 
*                   one value of QlFotaType.
* Return:        
*               None
*****************************************************************/
void Ql_Fota_App_Init(u8 update_mode);


/*****************************************************************
* Function:     Ql_Fota_App_Write_Data 
* 
* Description:
*               FOTA write data API.
*                1. This function is used to write data to spare image pool
*                2. This API only allow sequentially writing mechanism
*                3. Authentication mechanism is executed during writing
* Parameters:
*               length: the length of writing (Unit: Bytes)
*               buffer: pointer to the start address of buffer
* Return:        
*               0:   Means pass write success
*               < 0: Writing action is fail
*****************************************************************/
s32  Ql_Fota_App_Write_Data(s32 length, s8 * buffer);


/*****************************************************************
* Function:     Ql_Fota_App_Finish 
* 
* Description:
*               FOTA finalization API.
*                1. compare calculated checksum with image checksum in the header after
*                   whole image is written
*                2. mark the status to UPDATE_NEEDED 
* Parameters:
*               None
* Return:        
*               None
*****************************************************************/
void Ql_Fota_App_Finish(void);



/*****************************************************************
* Function:     Ql_Fota_Update 
* 
* Description:
*               Starts FOTA Update.
* Parameters:
*               flag:
*                   Flag for updating,  can be:
*                       FOTA_UPDATE_FLAG_CORE 
*                    or FOTA_UPDATE_FLAG_APP
*                    or FOTA_UPDATE_FLAG_CORE | FOTA_UPDATE_FLAG_APP
* Return:        
*               0:   Means update success
*               < 0: Fail to update
*****************************************************************/
s32  Ql_Fota_Update(u32 flag);

#endif  // End-of __QL_FOTA_H__

