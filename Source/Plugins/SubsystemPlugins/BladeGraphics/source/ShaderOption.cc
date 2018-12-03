/********************************************************************
	created:	2012/02/25
	filename: 	ShaderOption.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ShaderOption.h"
#include <MaterialCloneShare.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>

namespace Blade
{
	template class FixedVector<uint8, MATERIALLOD::MAX_LOD>;

	//////////////////////////////////////////////////////////////////////////
	ShaderOption::ShaderOption(const TString& name)
		:mName(name)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ShaderOption::ShaderOption(const ShaderOption& src, int method)
		:mName(src.mName)
	{
		assert( src.mProgram != NULL );

		if( method & MS_SHADERS )
			mProgram = src.mProgram;
		else
		{
			mProgram.bind( BLADE_NEW ShaderInstance(TString::EMPTY) );
			*mProgram = *src.mProgram;
		}

		this->addSubState(mProgram);
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderOption::~ShaderOption()
	{
		this->clearSubStates();
	}

	/************************************************************************/
	/* shader                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	ShaderInstance*			ShaderOption::createShader(const TString& profile)
	{
		ShaderInstance* shaderInst = NULL;

		//cloned shader already has shader
		//if( mProgram != NULL )
		//	BLADE_EXCEPT(EXC_REDUPLICATE, BTString("shader already created.") );

		mProgram.bind( BLADE_NEW ShaderInstance(profile) );
		shaderInst = mProgram;
		this->addSubState(shaderInst);
		return shaderInst;
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderInstance*			ShaderOption::getShaderInstance() const
	{
		return mProgram;
	}

	//////////////////////////////////////////////////////////////////////////
	void					ShaderOption::setOptionIndex(index_t index)
	{
		mProgram->setOptionIndex(index);
	}
	

}//namespace Blade