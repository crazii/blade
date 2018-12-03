/********************************************************************
created:	2017/10/20
filename: 	GrassRenderType.cc
author:		Crazii
purpose:	
*********************************************************************/
#include "GrassRenderType.h"
#include "GrassConfig.h"
#include <interface/IMaterialManager.h>

namespace Blade 
{
	const TString GrassRenderType::GRASS_RENDER_TYPE = BTString("GrassRenderType");

	//////////////////////////////////////////////////////////////////////////
	GrassRenderType::GrassRenderType()
		:RenderType(GRASS_RENDER_TYPE)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GrassRenderType::~GrassRenderType()
	{

	}

	/************************************************************************/
	/*  IGraphicsType interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Material*		GrassRenderType::getMaterial() const
	{
		if (mMaterial == NULL)
			mMaterial = IMaterialManager::getSingleton().getMaterial(BTString("grass"));
		return mMaterial;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GrassRenderType::processRenderQueue(IRenderQueue* /*buffer*/)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			GrassRenderType::onRenderDeviceReady()
	{
		RenderType::onRenderDeviceReady();
		GrassConfig::getSingleton().initialize();
	}

	//////////////////////////////////////////////////////////////////////////
	void			GrassRenderType::onRenderDeviceClose()
	{
		RenderType::onRenderDeviceClose();
		GrassConfig::getSingleton().shutdown();
	}
	
}//namespace Blade 