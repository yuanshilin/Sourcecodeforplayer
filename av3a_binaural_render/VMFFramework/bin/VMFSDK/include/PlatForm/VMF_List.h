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
**      Name        : VMF_List.h
**      Purpose     : the definitions for VMFList
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_LIST_H
#define VMF_LIST_H

#include <VMF_Config.h>

#ifdef __cplusplus
extern "C" {
#endif 

#include <VMF_Types.h>

typedef struct VMFList_st
{
	struct VMFList_st *pPre;
	struct VMFList_st *pNext;
}VMFList;

VMF_API VMFList* VMF_CreateList(VMF_U32 Size);
VMF_API VMF_VOID VMF_DeleteList(VMFList *pList);
VMF_API VMF_VOID VMF_DeleteAllLists(VMFList *pList);
VMF_API VMF_BOOL VMF_AddListTail(VMFList *pList1, VMFList *pList2);
VMF_API VMF_BOOL VMF_AddListHead(VMFList *pList1, VMFList *pList2);

#define VMF_LIST_NEXT(x) (((VMFList*)(x))->pNext)
#define VMF_LIST_PRE(x) (((VMFList*)(x))->pPre)
#define VMF_LIST(x) ((VMFList*)(x))

#ifdef __cplusplus
}
#endif 
#endif
