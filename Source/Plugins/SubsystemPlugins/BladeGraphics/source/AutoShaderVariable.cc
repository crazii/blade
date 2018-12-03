/********************************************************************
	created:	2010/09/05
	filename: 	AutoShaderVariable.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <AutoShaderVariable.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	AutoShaderVariable::AutoShaderVariable(IUpdater* updater, SHADER_CONSTANT_TYPE type, size_t count, EShaderVariableUsage usage)
		:mUpdater(updater)
		,mData(NULL)
		,mCount((uint16)count)
		,mSize((uint16)ShaderConstantType::getConstantSize(type))
		,mUsage(usage)
		,mType(type)
		,mDataDirty(false)
	{
		mBytes = (uint32)mSize*(uint32)mCount;
	}

	//////////////////////////////////////////////////////////////////////////
	AutoShaderVariable::AutoShaderVariable(IUpdater* updater,const AutoShaderVariable& src)
		:mUpdater(updater)
		,mData(NULL)
		,mCount(src.mCount)
		,mSize(src.mSize)
		,mUsage(src.mUsage)
		,mType(src.mType)
		,mDataDirty(src.mDataDirty)
	{
		mBytes = (uint32)mSize*(uint32)mCount;
	}

}//namespace Blade