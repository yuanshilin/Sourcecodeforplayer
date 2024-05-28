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
**      Name        : VMF_Socket.h
**      Purpose     : the definitions for marcos
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_SOCKET_H
#define VMF_SOCKET_H

#include <VMF_Config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#ifndef VMF_SOCKET
#define VMF_SOCKET
#define VMF_Socket VMF_HANDLE

#define VMF_INVALID_SOCKET  (VMF_Socket)(~0)
#define VMF_SOCKET_ERROR            (-1)

typedef struct VMF_Sockaddr_st
{
	VMF_U16 sa_family;
	VMF_S8 sa_data[14];
}VMF_SockAddr;

#define VMF_FD_SETSIZE 16
typedef struct VMF_FDSet_st
{
	VMF_U32 fd_count;               /* how many are SET? */
	VMF_Socket  fd_array[VMF_FD_SETSIZE];   /* an array of SOCKETs */
} VMF_FDSet;

typedef struct VMF_TimeVal_st
{
	VMF_S32    tv_sec;         /* seconds */
	VMF_S32    tv_usec;        /* and microseconds */
}VMF_TimeVal;

#define VMF__SS_PAD1SIZE 6
#define VMF__SS_PAD2SIZE 112
typedef struct VMF_SockAddrStorage_st
{
	VMF_S32 ss_family;      // address family
	VMF_S8 __ss_pad1[VMF__SS_PAD1SIZE];  
	VMF_S64 __ss_align;            
	VMF_S8 __ss_pad2[VMF__SS_PAD2SIZE];  
}VMF_SockAddrStorage;

typedef struct VMF_Inaddr_st
{
	union
	{
		struct { VMF_U8 s_b1,s_b2,s_b3,s_b4;} S_un_b;
		struct { VMF_U16 s_w1,s_w2; } S_un_w;
		VMF_U32 S_addr;
	};
} VMF_Inaddr;

typedef struct VMF_SSUData_st
{
	VMF_U16 wVersionRequested;
	VMF_VOID *lpWSAData;//LPWSADATA for window
}VMF_SSUData;
VMF_API VMF_S32 VMF_SocketsStartup(VMF_SSUData *pParam);
VMF_API VMF_S32 VMF_SocketsCleanUp();
VMF_API VMF_Socket VMF_CreateSocket(VMF_S32 af, VMF_S32 type, VMF_S32 protocol);
VMF_API VMF_S32 VMF_CloseSocket(VMF_Socket hSocket);
VMF_API VMF_S32 VMF_Bind(VMF_Socket hSocket, VMF_CONST VMF_SockAddr *pName, VMF_S32 NameLen);
VMF_API VMF_S32 VMF_Connect(VMF_Socket hSocket, VMF_CONST VMF_SockAddr *pName, VMF_S32 NameLen);
VMF_API VMF_S32 VMF_Listen(VMF_Socket hSocket, VMF_S32 BackLog);
VMF_API VMF_S32 VMF_Accept(VMF_Socket hSocket, VMF_SockAddr *pAddr, VMF_S32 *pAddrLen);
VMF_API VMF_S32 VMF_Getsockname(VMF_Socket hSocket, VMF_SockAddr *pName, VMF_S32 *pNameLen);
VMF_API VMF_S32 VMF_Getpeername(VMF_Socket hSocket, VMF_SockAddr *pName, VMF_S32 *pNameLen);
VMF_API VMF_S32 VMF_Socketpair(VMF_Socket hSocket, int, int, int *);
VMF_API VMF_S32 VMF_Shutdown(VMF_Socket hSocket, VMF_S32 how);
VMF_API VMF_S32 VMF_Setsockopt(VMF_Socket hSocket, VMF_S32 Level, VMF_S32 OptName, 
							   VMF_CONST VMF_VOID *OptVal, VMF_S32 OptLen);
VMF_API VMF_S32 VMF_Getsockopt(VMF_Socket hSocket, VMF_S32 Level, VMF_S32 OptName, 
							   VMF_CONST VMF_VOID *OptVal, VMF_S32 *pOptLen);
//VMF_API VMF_S32 VMF_Sendmsg(VMF_Socket hSocket, VMF_CONST struct msghdr *, unsigned int);
//VMF_API VMF_S32 VMF_Recvmsg(VMF_Socket hSocket, struct msghdr *, unsigned int);

VMF_API VMF_S64 VMF_Send(VMF_Socket hSocket, VMF_CONST VMF_VOID *pBuf, VMF_S64 Len, VMF_U32 Flags);
VMF_API VMF_S64 VMF_Recv(VMF_Socket hSocket, VMF_VOID *pBuf, VMF_S64 Len, VMF_U32 Flags,
						 VMF_S64 *pRevLen, VMF_U32 *pRevFlags);


VMF_API VMF_S64 VMF_Sendto(VMF_Socket hSocket, VMF_CONST VMF_VOID *pBuf, VMF_S64 Len, VMF_U32 Flags, 
						   VMF_CONST VMF_SockAddr *pAddrTo, VMF_S32 ToLen);
VMF_API VMF_S64 VMF_Recvfrom(VMF_Socket hSocket, VMF_VOID *pBuf, VMF_S64 Len, VMF_U32 Flags,
							 VMF_S64 *pRevLen, VMF_U32 *pRevFlags,VMF_CONST VMF_SockAddr *pAddrTo, VMF_S32 *pToLen);

VMF_API VMF_S32 VMF_Select(VMF_S32 nfds, VMF_FDSet *pReadfds, VMF_FDSet *pWritefds, VMF_FDSet *pExceptfds,
			   VMF_CONST VMF_TimeVal *pTimeOut);
VMF_API VMF_S32 VMF_Gethostname(VMF_CHAR *pName, VMF_S32 NameLen);
VMF_API VMF_U16 VMF_htons(VMF_U16 hostshort);
VMF_API VMF_U32 VMF_htonl(VMF_U32 hostlong);
VMF_API VMF_U32 VMF_inet_addr(VMF_CONST VMF_CHAR *cp);
VMF_API VMF_S8* VMF_inet_ntoa(VMF_Inaddr in);

VMF_API VMF_S32 VMF_fdset(VMF_Socket hSocket, VMF_FDSet *pfdset);
VMF_API VMF_S32 VMF_fdzero(VMF_FDSet *pfdset);
VMF_API VMF_BOOL VMF_fdisset(VMF_Socket hSocket, VMF_FDSet *pfdset);

#endif//VMF_SOCKET

#ifdef __cplusplus
}
#endif 

#endif
