/********************************************************************
	created:	2014/12/19
	filename: 	GLESContext.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESContext.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GLESContext::GLESContext()
		:mCurrent(Thread::INVALID_ID)
		,mIsInited(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GLESContext::~GLESContext()
	{
		assert( !mIsInited );
	}
	
}//namespace Blade

#endif//BLADE_USE_GLES