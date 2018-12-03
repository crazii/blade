/********************************************************************
	created:	2010/05/22
	filename: 	Pass.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Pass.h>

#define PASS_SHARED_PROPERTY	0x000000001L

namespace Blade
{
	namespace Impl
	{
		class ShaderOptionList : public Vector<HSHADEROPTION> , public Allocatable {};
	}//namespace Impl
	using namespace Impl;

	//////////////////////////////////////////////////////////////////////////
	Pass::Pass(const TString& name, Technique* parent)
		:mParent(parent)
		,mName(name)
		,mMask(PASS_SHARED_PROPERTY)
		,mActiveShadersIndex(0)
		,mShaderOptions( BLADE_NEW ShaderOptionList() )
	{
		//
		mRenderProperty = RenderPropertySet::getDefaultRenderProperty();
	}

	//////////////////////////////////////////////////////////////////////////
	Pass::Pass(const Pass& src, const TString& name, int shareMethod)
		:mShaderOptions( BLADE_NEW ShaderOptionList() )
	{
		mParent = src.mParent;
		mName = name;
		mMask = src.mMask;
		mActiveShadersIndex = src.mActiveShadersIndex;
		*mShaderOptions = *src.mShaderOptions;
		//default: share property
		mMask |= PASS_SHARED_PROPERTY;
		mRenderProperty = src.mRenderProperty;
		//not shared?
		if( (shareMethod & MS_PROPERTY) == 0 )
			this->createRenderProperty(src.mRenderProperty);

		if( (shareMethod & MS_SHADERS) != MS_SHADERS )
		{
			ShaderOptionList::const_iterator srcIter = src.mShaderOptions->begin();
			for(ShaderOptionList::iterator i = mShaderOptions->begin(); i != mShaderOptions->end(); ++i,++srcIter)
			{
				HSHADEROPTION& destOption = *i;
				const HSHADEROPTION& srcOption = *srcIter;
				destOption.bind( BLADE_NEW ShaderOption(*srcOption, shareMethod) );
			}
		}

		for(ShaderOptionList::iterator i = mShaderOptions->begin(); i != mShaderOptions->end(); ++i)
		{
			const HSHADEROPTION& option = *i;
			this->addSubState(option);
		}		
	}

	//////////////////////////////////////////////////////////////////////////
	Pass::~Pass()
	{
		this->clearSubStates();
	}

	/************************************************************************/
	/* render property                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	RenderPropertySet*		Pass::getRenderProperty() const
	{
		return mRenderProperty;
	}

	//////////////////////////////////////////////////////////////////////////
	const HRENDERPROPTYSET&	Pass::shareRenderProperty() const
	{
		mMask.raiseBits(PASS_SHARED_PROPERTY);
		return mRenderProperty;
	}

	//////////////////////////////////////////////////////////////////////////
	RenderPropertySet*		Pass::createRenderProperty(const RenderPropertySet* pToClone/* = NULL*/)
	{
		if( mRenderProperty == NULL )
		{
			BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("previous texture state empty unexpected.") );
		}

		if( !this->isUsingSharedRenderPropery() )
		{
			if(pToClone != NULL && mRenderProperty != pToClone)
				*mRenderProperty = *pToClone;
		}
		else
		{
			//
			mMask.clearBits(PASS_SHARED_PROPERTY);

			HRENDERPROPTYSET hOldProperty = mRenderProperty;
			mRenderProperty.bind( BLADE_NEW RenderPropertySet() );

			if( pToClone != NULL )
				*mRenderProperty = *pToClone;
			else
				*mRenderProperty = *hOldProperty;
		}

		return mRenderProperty;
	}

	//////////////////////////////////////////////////////////////////////////
	void					Pass::setSharedRenderProperty(const HRENDERPROPTYSET& hRenderPropty)
	{
		if( hRenderPropty == NULL )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("param render property cannot be empty."));

		mMask.raiseBits(PASS_SHARED_PROPERTY);

		if( mRenderProperty != hRenderPropty )
			mRenderProperty = hRenderPropty;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					Pass::isUsingSharedRenderPropery() const
	{
		return mMask.checkBits(PASS_SHARED_PROPERTY);
	}

	/************************************************************************/
	/* Shaders                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const index_t			Pass::addShaderOption(const HSHADEROPTION& shaderOption)
	{
		if( shaderOption == NULL )
			return INVALID_INDEX;

		mShaderOptions->push_back(shaderOption);
		index_t optionIndex = mShaderOptions->size() - 1;
		shaderOption->setOptionIndex(optionIndex);

		this->ResourceStateGroup::addSubState( shaderOption );
		return optionIndex;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					Pass::setShaderOption(index_t index, const HSHADEROPTION& shaderOption)
	{
		if (index < mShaderOptions->size())
		{
			this->ResourceStateGroup::removeSubState((*mShaderOptions)[index]);
			(*mShaderOptions)[index] = shaderOption;
			this->ResourceStateGroup::addSubState(shaderOption);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	const HSHADEROPTION&	Pass::getActiveShaderOption() const
	{
		if( mActiveShadersIndex < mShaderOptions->size() )
			return (*mShaderOptions)[mActiveShadersIndex];
		else
			return HSHADEROPTION::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	void					Pass::setActiveShaderOption(index_t optionIndex)
	{
		if( optionIndex < mShaderOptions->size() )
		{
			mActiveShadersIndex = optionIndex;
			//TODO: update render property from requirement
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range."));
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					Pass::getShaderOptionCount() const
	{
		return mShaderOptions->size();
	}

	//////////////////////////////////////////////////////////////////////////
	const HSHADEROPTION&	Pass::getShaderOption(index_t index) const
	{
		if( index < mShaderOptions->size() )
			return (*mShaderOptions)[index];
		else
			return HSHADEROPTION::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t					Pass::findShaderOption(index_t startShaderOption, index_t groupIndex) const
	{
		if( startShaderOption < mShaderOptions->size() )
		{
			for(index_t i = startShaderOption; i < mShaderOptions->size(); ++i)
			{
				const ShaderOption::GroupListLink& groupList = (*mShaderOptions)[i]->getGroupLink();
				for(size_t j = 0; j < groupList.size(); ++j)
				{
					if( groupList[j] == (uint8)groupIndex)
						return i;
				}
			}
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t					Pass::findShaderOption(const TString& name) const
	{
		for (index_t i = 0; i < mShaderOptions->size(); ++i)
		{
			if ((*mShaderOptions)[i]->getName() == name)
				return i;
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	Pass*			Pass::clone(const TString& name, int method/* = MS_ALL*/) const
	{
		return BLADE_NEW Pass(*this, name, method);
	}

	//////////////////////////////////////////////////////////////////////////
	void					Pass::resetResourceState()
	{
		this->ResourceStateGroup::clearSubStates();

		//add active shader option to sub state
		for( ShaderOptionList::iterator i = mShaderOptions->begin(); i != mShaderOptions->end(); ++i)
		{
			const HSHADEROPTION& shaderOption = *i;
			this->ResourceStateGroup::addSubState( shaderOption );
		}
	}
	
}//namespace Blade