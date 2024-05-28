/*----------------------------------------------------------------------------------------------
*
* This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and 		
* confidential information. 
* 
* The information and code contained in this file is only for authorized ArcSoft employemp4 
* to design, create, modify, or review.
* 
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
* 
* If you are not an INTended recipient of this file, you must not copy, distribute, modify, 
* or take any action in reliance on it. 
* 
* If you have received this file in error, please immediately notify ArcSoft and 
* permanently delete the original and any copy of any file and any printout thereof.
*
*-------------------------------------------------------------------------------------------------*/

/*************************************************************************************************
**      Copyright (c) 2011 by ArcSoft Inc.
**      Name        : VMF_File.h
**      Purpose     : the definitions for files
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_FILE_H
#define VMF_FILE_H

#include <VMF_Config.h>
#include <VMF_Types.h>

#ifdef __cplusplus

#define FILE_IO_TIMEOUT			0x80000001
#define FILE_IO_OPENFAILED		0x80000002
#define FILE_IO_ALREADY_FAIL	0x80000003
class CVMFFile
{
public:
	CVMFFile();
	~CVMFFile();
	virtual VMF_BOOL Open(VMF_CONST VMF_STRING lpszFileName, VMF_STRING nOpenFlags);
	virtual VMF_BOOL IsOpen();
	virtual VMF_S32 Close();
	virtual VMF_S32 Flush();
	virtual VMF_VOID* Gets(VMF_VOID *lpBuf, VMF_U32 nCount);
	virtual VMF_S32 Puts( const VMF_CHAR *str);
	virtual VMF_S32 Scanf(const VMF_CHAR *format, VMF_VOID **ppOut, VMF_U32 nCount);
	virtual VMF_S32 Printf(const VMF_CHAR *format, ... );
	virtual VMF_U32 Read(VMF_VOID *lpBuf, VMF_U32 nCount);
	virtual VMF_S32 Write(VMF_CONST VMF_VOID *lpBuf, VMF_U32 nCount);
	virtual VMF_U64 GetLength();
	virtual VMF_BOOL SeekFromCurrent(VMF_S64 lOff);
	virtual VMF_BOOL SeekFromBegin(VMF_S64 lOff);
	virtual VMF_BOOL SeekFromEnd(VMF_S64 lOff);
	virtual VMF_STRING GetFilePath();
	virtual VMF_U64 GetPosition();

public:
	VMF_VOID *m_pHandle;
	VMF_STRING m_pFileName;
	VMF_BOOL m_bIsOSSFile;
	VMF_U64 m_nBytesNum;
	VMF_U64 m_nFilePos;
	VMF_U32 m_nCacheSize;
        VMF_U32 m_nSyncData;
};

class CVMFFileEx : public CVMFFile
{
public:
	CVMFFileEx();
	~CVMFFileEx();
	//inherite from base class
	virtual VMF_S32 Write(VMF_CONST VMF_VOID *lpBuf, VMF_U32 nCount);
	virtual VMF_S32 Close();
	virtual VMF_BOOL Open(VMF_CONST VMF_STRING lpszFileName, VMF_STRING nOpenFlags);
	virtual VMF_BOOL SeekFromCurrent(VMF_S64 lOff);
	virtual VMF_BOOL SeekFromBegin(VMF_S64 lOff);
	virtual VMF_BOOL SeekFromEnd(VMF_S64 lOff);
	virtual VMF_S32 Flush();
	virtual VMF_U64 GetPosition();


	//new methods
	virtual VMF_S32 SetCacheSize(VMF_U32 nCacheSize); //unit in block,ie,4096Bytes. the total cachesize is nCacheSize*4K byte
	virtual VMF_U32 GetCacheSize();
	virtual VMF_U64 GetLength();
	virtual VMF_U64 Clean();
	virtual VMF_S32 SetThread(VMF_BOOL bThreaded, VMF_U32 blockSize); //write blockSize Byte every time in thread, 0 means write whenever there is data
	virtual VMF_S32 SetTimeOut(VMF_U32 timeOutMs, VMF_U32 timeOutOpenMs); //be valid in thread mode timeOutOpenMs:timeout for open file, timeOutMs: tiemout for other operation
	VMF_U32 *reserved[10];
	static VMF_U32 write_thread_entry(VMF_VOID* lpParameter);
	VMF_VOID    start_write_thread();
	VMF_S32     WriteToCache(VMF_CONST VMF_VOID *lpBuf, VMF_U32 nCount);
	VMF_U32     WriteFileFromCache(VMF_U32 nToWrite, VMF_BOOL writeAnyway);
	VMF_BOOL	OpenInternal();
private:
	VMF_U8 *m_pCache;
	VMF_U32 m_nCurrLen;
	VMF_BOOL m_bThreaded;
	VMF_HANDLE m_hThread;
	VMF_U32 m_nReadOffset;
	VMF_U32 m_nWriteOffset;
	VMF_U32 m_nBlockSize;
	VMF_BOOL m_bFlush; //1:begin flush, 0-end flush. flush: write all buffer data to file
	VMF_BOOL m_bExitThread; //1:to exit, 0-init
	VMF_BOOL m_bClean;
	VMF_U32 m_timeOutMs;
	VMF_U32 m_timeOutOpenMs;
	VMF_STRING m_pFileFlags;
	VMF_U32 m_control;
	VMF_BOOL m_OpenFileFailed;
	VMF_BOOL m_bThreadExited;
	VMF_U64 m_tStartOpenFile;
	VMF_U64 m_nFileSize;
};

extern "C"
{
VMF_API CVMFFile* VMF_CreateFile();
VMF_API VMF_VOID VMF_FreeFile(CVMFFile *pFile);
VMF_API CVMFFileEx* VMF_CreateFileEx();
VMF_API VMF_VOID VMF_FreeFileEx(CVMFFileEx *pFile);
}
#endif 

#endif





