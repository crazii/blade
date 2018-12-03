/********************************************************************
	created:	2013/04/09
	filename: 	ModelType.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ModelType.h"
#include <interface/IMaterialManager.h>
#include <Material.h>
#include <BladeModel_blang.h>
#include "ModelConfigManager.h"
#include "ModelBatchCombiner.h"

namespace Blade
{
	const TString StaticModelType::TYPE = BXLang(BLANG_STATIC_MESH);

	//////////////////////////////////////////////////////////////////////////
	StaticModelType::StaticModelType(const TString& name/* = TYPE*/, bool alpha/* = false*/)
		:RenderType(name)
		,mMaterial(NULL)
		,mAlpha(alpha)
	{
		mCombiner = ModelBatchCombiner::getSingletonPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	StaticModelType::~StaticModelType()
	{

	}

	/************************************************************************/
	/*  IGraphicsType interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Material*				StaticModelType::getMaterial() const
	{
		if( mMaterial == NULL )
			mMaterial = IMaterialManager::getSingleton().getMaterial( TYPE );

		return mMaterial;
	}

	//////////////////////////////////////////////////////////////////////////
	void					StaticModelType::processRenderQueue(IRenderQueue* queue)
	{
		mCombiner->processRenderQueue(queue, mAlpha);
	}

	//////////////////////////////////////////////////////////////////////////
	void			StaticModelType::onRenderDeviceReady()
	{
		RenderType::onRenderDeviceReady();
		ModelConfigManager::getSingleton().initialize();
	}

	//////////////////////////////////////////////////////////////////////////
	void			StaticModelType::onRenderDeviceClose()
	{
		RenderType::onRenderDeviceClose();
		ModelConfigManager::getSingleton().shutdown();
	}


	const TString SkinnedModelType::TYPE = BXLang(BLANG_SKINNED_MESH);
	//////////////////////////////////////////////////////////////////////////
	SkinnedModelType::SkinnedModelType(const TString& name/* = TYPE*/)
		:RenderType(name)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	SkinnedModelType::~SkinnedModelType()
	{

	}

	/************************************************************************/
	/*  IGraphicsType interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Material*				SkinnedModelType::getMaterial() const
	{
		if( mMaterial == NULL )
			mMaterial = IMaterialManager::getSingleton().getMaterial( TYPE );
		assert(mMaterial);
		return mMaterial;
	}

	//////////////////////////////////////////////////////////////////////////
	void					SkinnedModelType::processRenderQueue(IRenderQueue* /*buffer*/)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			SkinnedModelType::onRenderDeviceReady()
	{
		RenderType::onRenderDeviceReady();
		ModelConfigManager::getSingleton().initialize();
	}

	//////////////////////////////////////////////////////////////////////////
	void			SkinnedModelType::onRenderDeviceClose()
	{
		RenderType::onRenderDeviceClose();
		ModelConfigManager::getSingleton().shutdown();
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	StaticModelAlphaType::StaticModelAlphaType()
		:StaticModelType(BTString("Static Mesh Alpha"), true)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Material*		StaticModelAlphaType::getMaterial() const
	{
		//uncomment below if need customization
		//if (mMaterial == NULL)
		//{
		//	mMaterial = StaticModelType::getMaterial();
		//	mMaterial = mMaterial->clone(mMaterial->getName() + BTString("_Alpha"));
		//}
		//return mMaterial;
		return StaticModelType::getMaterial();
	}

	//////////////////////////////////////////////////////////////////////////
	SkinnedModelAlphaType::SkinnedModelAlphaType()
		:SkinnedModelType(BTString("Skinned Mesh Alpha"))
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Material*		SkinnedModelAlphaType::getMaterial() const
	{
		//uncomment below if need customization
		//if (mMaterial == NULL)
		//{
		//	mMaterial = SkinnedModelType::getMaterial();
		//	mMaterial = mMaterial->clone(mMaterial->getName() + BTString("_Alpha"));
		//}
		//return mMaterial;
		return SkinnedModelType::getMaterial();
	}

	//////////////////////////////////////////////////////////////////////////
	MovingStaticModelType::MovingStaticModelType()
		:StaticModelType(BTString("Moving Static Mesh"))
	{

	}

}//namespace Blade