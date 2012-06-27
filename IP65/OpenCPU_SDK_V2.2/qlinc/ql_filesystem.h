/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2010 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |      File System
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by     :   Jay XIN
 |  Coded    by     :   Jay XIN
 |  Tested   by     :   Stanley YONG
 |
 |--------------------------------------------------------------------------
 | Revision History
 | ----------------
 |  Sep. 25, 2010       Stanley Yong        Make the notes clear.
 |
 \=========================================================================*/

#ifndef __QL_FILESYSTEM_H__
#define __QL_FILESYSTEM_H__


/****************************************************************************
 * Type of file access permitted
 ***************************************************************************/
#define QL_FS_READ_WRITE            0x00000000L
#define QL_FS_READ_ONLY             0x00000100L
#define QL_FS_CREATE                0x00010000L
#define QL_FS_CREATE_ALWAYS         0x00020000L
#define QL_FS_OPEN_SHARED           0x00000200L


/****************************************************************************
 * Type of file move permitted
 ***************************************************************************/
#define QL_FS_MOVE_COPY          0x00000001     // Move file|folder by copy
#define QL_FS_MOVE_KILL          0x00000002     // Delete the moved file|folder after moving
#define QL_FS_MOVE_OVERWRITE    0x00010000      // Overwrite the existed file in destination path when move file|folder


/****************************************************************************
 * 
 ***************************************************************************/
#define QL_FS_FILE_TYPE              0x00000004     
#define QL_FS_DIR_TYPE               0x00000008     
#define QL_FS_RECURSIVE_TYPE        0x00000010


/****************************************************************************
 * Constants for File Seek
 ***************************************************************************/
typedef enum QlFsSeekPosTag
{
   QL_FS_FILE_BEGIN,    // Beginning of file
   QL_FS_FILE_CURRENT,  // Current position of file pointer
   QL_FS_FILE_END       // End of file
}QlFsSeekPos;

typedef enum tagFileSys_Storage
{
    Ql_FS_UFS = 1,
    Ql_FS_RAM = 2,
    Ql_FS_SD  = 3
} FileSys_Storage;


/******************************************************************************
* Function:     Ql_FileGetFreeSize
*  
* Description:
*               This function obtains the amount of free space on Flash.
*
* Parameters:    
*               None
* Return:  
*               The total number of free bytes on Flash.
******************************************************************************/
s32  Ql_FileGetFreeSize(void);


/******************************************************************************
* Function:     Ql_FileOpen
*  
* Description:
*               The method opens and automatically creates a named file. 
*               This function is deprecated, and a new function is available 
*               'Ql_FileOpenEx'.
*
* Parameters:    
*               asciifilename:
*                   The name of the file. 
*                   The name is limited to 252 characters. 
*
*               bcreate:
*                   If TRUE, the function will create a file and open it;
*                   If FALSE, the function opens a file.
*
*               bonlyread:
*                   TRUE or FALSE, Indicates whether this function open 
*                                  the file with read-only.
* Return:  
*               If the function succeeds, the return value specifies a file handle.
*               If the function fails, the return value is an error codes. To
*               get extended error information, see 'Error Code Definition'.
******************************************************************************/
s32  Ql_FileOpen(u8* asciifilename, bool bcreate, bool bonlyread);


/******************************************************************************
* Function:     Ql_FileOpenEx
*  
* Description:
*               The method opens and automatically creates a named file.
*
* Parameters:    
*               asciifilename:
*                   The name of the file. 
*                   The name is limited to 252 characters. 
*
*               Flag:
*                   A u32 that defines the file's opening and access mode.
*                   The possible values are shown as follow:
*                       QL_FS_READ_WRITE, can read and write
*                       QL_FS_READ_ONLY, can only read
*                       QL_FS_CREATE, opens the file, if it exists. 
*                           If the file does not exist, the function creates the file
*                       QL_FS_CREATE_ALWAYS, creates a new file. 
*                           If the file exists, the function overwrites the file 
*                           and clears the existing attributes
* Return:  
*               If the function succeeds, the return value specifies a file handle.
*               If the function fails, the return value is an error codes. To
*               get extended error information, see 'Error Code Definition'.
******************************************************************************/
s32  Ql_FileOpenEx(u8* asciifilename, u32 Flag);


/******************************************************************************
* Function:     Ql_FileRead
*  
* Description:
*               Reads data from the specified file, starting at the position 
*               indicated by the file pointer. After the read operation has been 
*               completed, the file pointer is adjusted by the number of bytes actually read.
*
* Parameters:    
*               filehandle:
*                   [in] A handle to the file to be read, which is the return value
*                        of the function Ql_FileOpen.
*
*               readbuffer:
*                   [in] Pointer to the buffer that receives the data read from the file.
*
*               readlength:
*                   [in] Number of bytes to be read from the file.
*
*               bonlyread:
*                   [out] Pointer to the number of bytes read. ReadFile sets this
*                         value to zero before doing taking action or checking errors.
* Return:  
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
******************************************************************************/
s32  Ql_FileRead(s32 filehandle, u8* readbuffer, u32 readlength, u32* readedlen);


/******************************************************************************
* Function:     Ql_FileWrite
*  
* Description:
*               This function writes data to a file. Ql_FileWrite starts writing 
*               data to the file at the position indicated by the file pointer.
*               After the write operation has been completed, the file pointer 
*               is adjusted by the number of bytes actually written. 
*
* Parameters:    
*               filehandle:
*                   [in] A handle to the file to be read, which is the return value 
*                        of the function Ql_FileOpen.
*
*               writebuffer:
*                   [in] Pointer to the buffer that receives the data read from the file.
*
*               writelength:
*                   [in] Number of bytes to be read from the file.
*
*               written:
*                   [out] Pointer to the number of bytes read. ReadFile sets this
*                        value to zero before doing taking action or checking errors.
* Return:  
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
******************************************************************************/
s32  Ql_FileWrite(s32 filehandle, u8* writebuffer, u32 writelength, u32* written);


/******************************************************************************
* Function:     Ql_FileSeek
*  
* Description:
*               Repositions the pointer in the previously opened file. 
*
* Parameters:    
*               filehandle:
*                   [in] A handle to the file to be read, which is the return value 
*                        of the function Ql_FileOpen.
*
*               Offset:
*                   [in] Number of bytes to move the file pointer.
*
*               Whence:
*                   [in] Pointer movement mode. See QlFsSeekPos.
* Return:  
*               QL_RET_OK, suceess
*               or other error codes, please see 'Error Code Definition'
******************************************************************************/
s32  Ql_FileSeek(s32 filehandle, s32 Offset, s32 Whence);


/******************************************************************************
* Function:     Ql_FileGetFilePosition
*  
* Description:
*               Gets the current value of the file pointer.
*
* Parameters:    
*               filehandle:
*                   [in] A file handle, which was returned by calling 'Ql_FileOpen'.
*
*               Position:
*                   [out] Address of u32 that will receive the current offset 
*                         from the beginning of the file.
* Return:  
*               QL_RET_OK indicates success.
*               Negative indicates failure. please see 'Error Code Definition'.
******************************************************************************/
s32  Ql_FileGetFilePosition(s32 filehandle, u32* Position);


/******************************************************************************
* Function:     Ql_FileTruncate
*  
* Description:
*               This function truncates a file to ZERO size.
*
* Parameters:    
*               filehandle: 
*                   A file handle, which was returned by calling 'Ql_FileOpen'.
* Return:  
*               QL_RET_OK, if the function succeeds
*               or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileTruncate(s32 filehandle);


/******************************************************************************
* Function:     Ql_FileFlush
*  
* Description:
*               Forces any data remaining in the file buffer to be written to the file.
*
* Parameters:    
*               filehandle: 
*                   A file handle, which was returned by calling 'Ql_FileOpen'.
* Return:  
*               None
******************************************************************************/
void  Ql_FileFlush(s32 filehandle);


/******************************************************************************
* Function:     Ql_FileClose
*  
* Description:
*               Closes the file associated with the file handle and makes 
*               the file unavailable for reading or writing.
*
* Parameters:    
*               filehandle: 
*                   A file handle, which was returned by calling 'Ql_FileOpen'.
* Return:  
*               None
******************************************************************************/
void Ql_FileClose(s32 filehandle);


/******************************************************************************
* Function:     Ql_FileGetSize
*  
* Description:
*               Retrieves the size, in bytes, of the specified file.
*
* Parameters:    
*               asciifilename:
*                   [in] The name of the file. 
*
*               filesize:
*                   [out] A pointer to the variable where the file size, 
*                         in bytes, is stored. 
* Return:  
*               QL_RET_OK, if the function succeeds
*               or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileGetSize(u8* asciifilename, u32* filesize);


/******************************************************************************
* Function:     Ql_FileDelete
*  
* Description:
*               This function deletes an existing file from a file system.
*
* Parameters:    
*               asciifilename:
*                   The name of the file. 
* Return:  
*               QL_RET_OK, if the function succeeds
*               or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileDelete(u8* asciifilename);


/******************************************************************************
* Function:     Ql_FileCheck
*  
* Description:
*               Check whether the file exists or not.
*
* Parameters:    
*               asciifilename:
*                   The name of the file. 
* Return:  
*               QL_RET_OK, if the function succeeds
*               or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileCheck(u8* asciifilename);


/******************************************************************************
* Function:     Ql_FileRename
*  
* Description:
*               Rename a file.
*
* Parameters:    
*               asciifilename:
*                   File to be renamed. 
*
*               newasciifilename:
*                   New name of file. 
* Return:  
*               QL_RET_OK, if the function succeeds
*               or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileRename(u8* asciifilename, u8* newasciifilename);


/******************************************************************************
* Function:     Ql_FileCreateDir
*  
* Description:
*               Creates a directory.
*
* Parameters:    
*               asciifilename:
*                   A pointer to a string containing the name of the directory to create. 
* Return:  
*               QL_RET_OK, if the function succeeds
*               or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileCreateDir(u8* asciidirname);


/******************************************************************************
* Function:     Ql_FileRemoveDir
*  
* Description:
*               Removes a existing directory.
*
* Parameters:    
*               asciifilename:
*                   A pointer to a string containing the name of the directory to remove. 
* Return:  
*               QL_RET_OK, if the function succeeds
*               or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileRemoveDir(u8* asciidirname);


/******************************************************************************
* Function:     Ql_FileCheckDir
*  
* Description:
*               Check whether the file exists or not.
*
* Parameters:    
*               asciifilename:
*                   The name of the directory. 
* Return:  
*               QL_RET_OK, if the function succeeds
*               or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileCheckDir(u8* asciidirname);


/******************************************************************************
* Function:     Ql_FileFindFirst
*  
* Description:
*               This function searches a directory for a file or subdirectory 
*               whose name matches the specified file name. 
*
* Parameters:    
*               asciipath:
*                   [in] Pointer to a null-terminated string that specifies a valid directory or path.
*
*               asciifilename:
*                   [in] Pointer to a null-terminated string that specifies a valid file name, 
*                        which can contain wildcard characters, such as * and ?.
*
*               filenamelength:
*                   [in] 
*
*               filesize:
*                   [out] 
*
*               isdir:
*                   [in/out] 
* Return:  
*               If the function succeeds, the return value is a search handle 
*               that can be used in a subsequent call to the 
*                   Ql_FindNextFile or Ql_FindClose functions.
*               Or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileFindFirst(u8* asciipath, u8* asciifilename, u32 filenamelength, u32* filesize, bool* isdir);


/******************************************************************************
* Function:     Ql_FileFindNext
*  
* Description:
*               Continues a file search from a previous call to the Ql_FileFindFirst 
*               function.
*
* Parameters:    
*               handle:
*                   [in] Search handle returned by a previous call to the Ql_FileFindFirst
*
*               asciifilename:
*                   [in] Pointer to a null-terminated string that specifies a valid file name, 
*                        which can contain wildcard characters, such as * and ?.
*
*               filenamelength:
*                   [in] 
*
*               filesize:
*                   [out] 
*
*               isdir:
*                   [in/out] 
* Return:  
*               If the function succeeds, the return value is a search handle 
*               that can be used in a subsequent call to the 
*                   Ql_FindNextFile or Ql_FindClose functions.
*               Or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileFindNext(s32 handle, u8* asciifilename, u32 filenamelength, u32* filesize, bool* isdir);


/******************************************************************************
* Function:     Ql_FileFindClose
*  
* Description:
*               Closes the specified search handle.
*
* Parameters:    
*               handle:
*                   Find handle, 
*                   returned by a previous call of the Ql_FileFindFirst function. 
* Return:  
*               None
******************************************************************************/
void Ql_FileFindClose(s32 handle);


/******************************************************************************
* Function:     Ql_FileXDelete
*                           
* Description:
*               Delete a file or directory.
*           
* Parameters:    
*               asciipath: 
*                       File path to be deleted
*
*               flag: 
*                   A u32 that defines the file's opening and access mode.
*                   The possible values are shown as follow:
*                       QL_FS_FILE_TYPE, 
*                       QL_FS_DIR_TYPE, 
*                       QL_FS_RECURSIVE_TYPE
*
* Return:       
*               QL_RET_OK, if the function succeeds
*               or the following Error Codes:
*                       QL_RET_ERR_PARAM
*                       QL_RET_ERR_FILENAMETOOLENGTH
*                       QL_RET_ERR_FILENOTFOUND
*                       QL_RET_ERR_PATHNOTFOUND
*                       QL_RET_ERR_GET_MEM
*                       QL_RET_ERR_GENERAL_FAILURE
******************************************************************************/
 s32  Ql_FileXDelete(u8* asciipath, u32 flag);


/******************************************************************************
* Function:     Ql_FileXMove  
*                           
* Description:
*               This function provides a facility to move/copy a file or folder
*           
* Parameters:    
*               asciisrcpath: 
*                       Source path to be moved/copied
*
*               asciidestpath:
*                       Destination path
*
*               flag:
*                   A u32 that defines the file's opening and access mode.
*                   The possible values are shown as follow:
*                       QL_FS_MOVE_COPY, 
*                       QL_FS_MOVE_KILL, 
*                       QL_FS_MOVE_OVERWRITE
*
* Return:       
*               QL_RET_OK, if the function succeeds
*               or the following Error Codes:
*                       QL_RET_ERR_PARAM
*                       QL_RET_ERR_FILENAMETOOLENGTH
*                       QL_RET_ERR_FILENOTFOUND
*                       QL_RET_ERR_PATHNOTFOUND
*                       QL_RET_ERR_GET_MEM
*                       QL_RET_ERR_FILE_EXISTS
*                       QL_RET_ERR_GENERAL_FAILURE
******************************************************************************/
s32  Ql_FileXMove(u8* asciisrcpath, u8* asciidestpath, u32 flag);


/******************************************************************************
* Function:     Ql_FileSys_GetSpaceInfo
*  
* Description:
*               Get the space information in file system area.
*
* Parameters:    
*               storage:
*                   [in] one value of 'FileSys_Storage'
*                        1 = UFS
*                        2 = RAM (only for M33)
*                        3 = SD Card (only for M33)
*
*               freeSpace:
*                   [out] Free space size, unit in bytes.
*
*               totalSpace:
*                   [out] Total space size, unit in bytes.
* Return:  
*               QL_RET_OK, if the function succeeds
*               or a negative number, please see 'Error Codes Definitions'
******************************************************************************/
s32  Ql_FileSys_GetSpaceInfo(u8 storage, u32* freeSpace, u32* totalSpace);

#endif  // End-of __QL_FILESYSTEM_H__

