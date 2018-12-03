/********************************************************************
	created:	2011/04/02
	filename: 	RenderType.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <RenderType.h>
#include <Material.h>
#include <Technique.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	RenderType::RenderType(const TString& name)
		:mAppFlag(INVALID_APPFLAG)
		,mName(name)
		,mRenderBuffer(NULL)
		,mLastMat(NULL)
		,mFixedFillMode(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	RenderType::~RenderType()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Material*	RenderType::updateProfile(const TString& profile, bool& updated)
	{
		Material* mat = this->getMaterial();
		updated = mLastMat != mat;
		mLastMat = mat;
		if( mat == NULL )
			return mat;

		Technique* tech = mat->getActiveTechnique();
		if( mat->getTechniqueCount() <= 1 
			|| tech->getProfile() == profile )
			return mat;

		index_t matching = mat->getTechniqueByProfile(profile);
		if( matching == INVALID_INDEX )
			return mat;
		
		Technique* newTech = mat->getTechnique(matching);
		if( tech == NULL || newTech->getProfile() > tech->getProfile() )
		{
			mat->setActiveTechnique(matching);
			updated = true;
		}
		return mat;
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderType::onRenderDeviceReady()
	{
		Material* material = this->getMaterial();
		if( material != NULL )
			material->loadSync();
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderType::onRenderDeviceClose()
	{
		Material* material = this->getMaterial();
		if( material != NULL )
			material->unload();
	}

}//namespace Blade
