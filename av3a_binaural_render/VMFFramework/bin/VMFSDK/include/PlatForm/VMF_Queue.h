/*----------------------------------------------------------------------------------------------
*
* This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and       
* confidential information. 
* 
* The information and code contained in this file is only for authorized ArcSoft employees 
* to design, create, modify, or review.
* 
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
* 
* If you are not an intended recipient of this file, you must not copy, distribute, modify, 
* or take any action in reliance on it. 
* 
* If you have received this file in error, please immediately notify ArcSoft and 
* permanently delete the original and any copy of any file and any printout thereof.
*
*-------------------------------------------------------------------------------------------------*/

/*************************************************************************************************
**      Copyright (c) 2011 by ArcSoft Inc.
**      Name        : VMF_Queue.h
**      Purpose     : the definitions for CVMFQueue
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_QUEUE_H
#define VMF_QUEUE_H

#include <VMF_Config.h>
#include <VMF_Types.h>
#include <VMF_Thread.h>

#ifdef __cplusplus
typedef struct VMF_QElement_st VMFQElement;
struct VMF_QElement_st
{
	VMFQElement *pQForw;
	VMF_VOID *pData;
};

class CVMFQueue
{
public:
	CVMFQueue();
	~CVMFQueue();

	virtual VMF_BOOL Init(VMF_S32 MaxNumber);
	virtual VMF_BOOL DeInit();
	virtual VMF_BOOL Push(VMF_VOID *data);
	virtual VMF_BOOL Delete(VMF_VOID *data);
	virtual VMF_VOID* Pop();
	virtual VMF_VOID* Get(VMF_S32 Index);
	virtual VMF_S32 GetDataLength();

private:
	VMFQElement *m_pQData; 
	VMFQElement *m_pQDataLast; 
	VMFQElement *m_pEmpty; 
	VMF_S32 m_Nelem; 
	VMF_S32 m_Number;
	VMF_Mutex m_Mutex;
};

extern "C"
{
VMF_API CVMFQueue* VMF_CreateQueue();
VMF_API VMF_VOID VMF_FreeQueue(CVMFQueue *pQueue);
}
#endif 

#endif
