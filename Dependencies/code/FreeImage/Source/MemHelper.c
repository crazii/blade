#include "MemHelper.h"

void*	FreeImage_Malloc(FreeImageMemType* mem,size_t size)
{
	return mem->malloc_proc(size,mem->mem_handle);
}

void	FreeImage_Free(FreeImageMemType* mem, void* ptr)
{
	mem->free_proc(ptr,mem->mem_handle);
}

void*	FreeImage_AlignMalloc(FreeImageMemType* mem, size_t size, size_t algin)
{
	return mem->align_malloc_proc(size,algin,mem->mem_handle);
}

void	FreeImage_AlignFree(FreeImageMemType* mem, void* ptr)
{
	mem->align_free_proc(ptr, mem->mem_handle);
}

void	FreeImage_MakeTempMem(FreeImageMem* dst,FreeImageMem* src)
{
	dst->normal_mem = src->temp_mem;
	dst->temp_mem = src->temp_mem;
}