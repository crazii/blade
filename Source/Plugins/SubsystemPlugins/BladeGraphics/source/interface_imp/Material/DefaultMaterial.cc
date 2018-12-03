/********************************************************************
	created:	2010/05/23
	filename: 	DefaultMaterial.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "DefaultMaterial.h"
#include "MaterialManager.h"
#include <Technique.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	DefaultMaterial::DefaultMaterial()
		:Material(TString::EMPTY)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DefaultMaterial::DefaultMaterial(const TString& name)
		:Material(name)
		,mCloned(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DefaultMaterial::~DefaultMaterial()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DefaultMaterial::DefaultMaterial(const DefaultMaterial& src,const TString& name,int method)
		:Material( name )
		,mCloned(true)
	{
		for( size_t i = 0; i < src.getTechniqueCount(); ++i)
		{
			Technique* tech = src.getTechnique(i)->clone(method);
			tech->setParent(NULL);
			this->addTechnique( tech );
		}
		mTextureUniforms = src.mTextureUniforms;
		this->recalcLoadedCount();
	}

	//////////////////////////////////////////////////////////////////////////
	HMATERIAL	DefaultMaterial::clone(const TString& name,int method/* = MCSM_EXCEPT_TEXTURE*/) const
	{
		assert( mName != name );

		//check whether the clone name is available
		Material* existed = MaterialManager::getSingleton().getMaterial(name);
		if( existed != NULL )
			return HMATERIAL::EMPTY;

		HMATERIAL hMaterial( BLADE_NEW DefaultMaterial(*this,name,method) );
		MaterialManager::getSingleton().addMaterial(hMaterial);
		return hMaterial;
	}
}//namespace Blade