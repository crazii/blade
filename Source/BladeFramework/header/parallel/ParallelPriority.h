/********************************************************************
	created:	2013/01/15
	filename: 	ParallelPriority.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ParallelPriority_h__
#define __Blade_ParallelPriority_h__

namespace Blade
{
	///pre-defined priority sections
	enum EParaPriority
	{
		PP_HIGHEST	= 0x0C00,
		PP_HIGHER	= 0x0A00,
		PP_HIGH		= 0x0800,
		PP_MIDDLE	= 0x0600,
		PP_LOW		= 0x0400,
		PP_LOWER	= 0x0200,
		PP_LOWEST	= 0x0000,
		PP_INVALID  = -1,
	};
	
}//namespace Blade

#endif//__Blade_ParallelPriority_h__