#ifndef __VMF_METRO_H__
#define __VMF_METRO_H__
#ifdef SUPPORT_METRO

using namespace Windows::Storage::Streams;
EXTERN_GUID(VMF_GUID_IRandomAccessStream, 
			0x6162b895, 0x5625, 0x4d7f, 0x9d, 0xe0, 0xf3, 0x94, 0x20, 0xb6, 0xf1, 0x5f);
typedef struct VMFIRandomAccessStream_st
{
	IRandomAccessStream^ stream; 
}VMFIRandomAccessStream;
#endif
#endif//__VMF_METRO_H__