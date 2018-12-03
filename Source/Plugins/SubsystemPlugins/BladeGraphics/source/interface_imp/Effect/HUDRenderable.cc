/********************************************************************
	created:	2015/12/30
	filename: 	HUDRenderable.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "HUDRenderable.h"
#include "../RenderHelper/HelperRenderType.h"
#include <RenderUtility.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <GraphicsShaderShared.inl>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	HUDRenderable::HUDRenderable()
	{
		mSpaceFlags = CSF_ONSCREEN | CSF_INDEPENDENT;	//enable 2d pick
		mUpdateFlags |= CUF_DEFAULT_VISIBLE;

		mMaterial.bind( BLADE_NEW MaterialInstance( IMaterialManager::getSingleton().getMaterial(BTString("HUD"))) );
		mWorldTransform = Matrix44::IDENTITY;
		mVisible = false;

		TextureState* textures = mMaterial->createTextureState();
		for(int i = 0; i < BLADE_MAX_DYNAMIC_TEXTURE_COUNT ; ++i)
		{
			TextureStage* ts = textures->addTextureStage(BTString("hudDiffuse[") + TStringHelper::fromInt(i) + TEXT(']'));
			ts->setTargetPixelFormat( IGraphicsResourceManager::getSingleton().getGraphicsConfig().CompressedRGBA );
			ts->setTargetMipMap(1);
			ts->setEnable(false);
		}
		mMaterial->loadSync();
		assert(mMaterial->isLoaded());
		this->setPixelSize(32, 32);
		this->setHighLight(false);
		mQuadGeometry = &RenderUtility::getUnitQuad();
	}

	//////////////////////////////////////////////////////////////////////////
	HUDRenderable::~HUDRenderable()
	{

	}

	/************************************************************************/
	/* IRenderable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	RenderType&				HUDRenderable::getRenderType() const
	{
		return HelperRenderType::getSingleton();
	}

	//////////////////////////////////////////////////////////////////////////
	const GraphicsGeometry&	HUDRenderable::getGeometry() const
	{
		return *mQuadGeometry;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			HUDRenderable::setPixelSize(size_t x, size_t y)
	{
		Vector4 size((scalar)x, (scalar)y, 0, 0);
		bool ret = mMaterial->setShaderVariable(BTString("pixelSize"), SCT_FLOAT4, 1, &size);
		assert(ret); BLADE_UNREFERENCED(ret);
		size /= 2;
		mLocalAABB.set(-size, size);
	}

	//////////////////////////////////////////////////////////////////////////
	void			HUDRenderable::setTexture(index_t index, const TString& texture)
	{
		if( index < mMaterial->getTextureState()->getTextureStageCount() )
		{
			TextureStage* ts = mMaterial->getTextureState()->getTextureStage(index);
			assert( ts != NULL );
			ts->setEnable( !texture.empty() );
			ts->setResourcePath(texture);
		}
		else
			assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void			HUDRenderable::setHighLight(bool highlit)
	{
		Color c = highlit ? Color::WHITE : Color::GRAY;
		bool ret = mMaterial->setShaderVariable(BTString("color"), SCT_FLOAT4, 1, &c);
		assert(ret); BLADE_UNREFERENCED(ret);
	}
	
}//namespace Blade