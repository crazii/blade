#ifndef _MEMHELPER_H_
#define _MEMHELPER_H_

#include "FreeImage.h"

#if __cplusplus
extern "C"
{
#endif


void*	FreeImage_Malloc(FreeImageMemType* mem,size_t size);

void	FreeImage_Free(FreeImageMemType* mem, void* ptr);

void*	FreeImage_AlignMalloc(FreeImageMemType* mem, size_t size, size_t algin);

void	FreeImage_AlignFree(FreeImageMemType* mem, void* ptr);

//internal helper
void	FreeImage_MakeTempMem(FreeImageMem* dst,FreeImageMem* src);


#if __cplusplus
}
#endif





#if __cplusplus
#endif



#endif//_MEMHELPER_H_