/********************************************************************
	created:	2011/12/18
	filename: 	RenderStep.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IRenderTypeManager.h>
#include "RenderStep.h"
#include "RenderSchemeManager.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	RenderStep::RenderStep(const STEP_DESC* desc)
		:mDesc(desc)
		,mRenderPass(NULL)
		,mSorter(NULL)
		,mType(NULL)
		,mFillMode(GFM_DEFAULT)
		,mEnabled(desc->mEnable)
	{
		mSorter = RenderSchemeManager::getSingleton().createRenderSorter(mDesc->mSorters);
		mType = IRenderTypeManager::getSingleton().getRenderType(this->getTypeName());
	}

	//////////////////////////////////////////////////////////////////////////
	RenderStep::~RenderStep()
	{
		BLADE_DELETE mSorter;
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderStep::setShaderGroups(const Technique* tech)
	{
		assert(tech != NULL);
		if( mRenderPass != NULL || tech == NULL )
			return;

		const TStringList& groups = mDesc->mGroups;

		mShaderGroups.clear();
		mShaderGroups.reserve( groups.size() );

		for(size_t i = 0; i < groups.size(); ++i)
		{
			index_t group = tech->getShaderGroupList().findGroup( groups[i] );
			mShaderGroups.push_back(group);
		}
	}

}//namespace Blade
