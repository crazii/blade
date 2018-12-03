/********************************************************************
	created:	2011/09/03
	filename: 	SkySphere.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "SkySphere.h"
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IMaterialManager.h>
#include <RenderUtility.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	SkySphere::SkySphere()
		:mRadius(0)
		,mDensity(0)
	{
		mSpaceFlags = CSF_INDEPENDENT;
		mUpdateFlags = CUF_DEFAULT_VISIBLE;
		mWorldAABB.setInfinite();
		mLocalAABB.setInfinite();
	}

	//////////////////////////////////////////////////////////////////////////
	SkySphere::~SkySphere()
	{
		if( mMaterial != NULL )
			mMaterial->unload();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		SkySphere::initialize(scalar radius,size_t density,const TString& texture/* = TString::EMPTY*/)
	{
		if( mMaterial == NULL )
		{
			Material* material = this->getRenderType().getMaterial();
			mMaterial.bind( BLADE_NEW MaterialInstance( material ) );
			assert(mMaterial != NULL );
			mMaterial->setShaderGroup(BTString("sky_sphere"));
		}

		if( density == mDensity && radius == mRadius && this->getTexture() == texture )
			return;

		if( texture != TString::EMPTY )
		{
			TString base, ext;
			TStringHelper::getFileInfo(texture, base, ext);

			if( mMaterial->getTextureState()->getTextureStageCount() == 0 || this->getTexture() != texture )
			{
				TextureStage* ts = mMaterial->getTextureState()->addTextureStage(BTString("skyDiffuse"));
				ts->setResourcePath(base + BTString(".") + IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureExt);
				//because sky sphere keeps constant distance from camera, no mip-level change
				ts->setTargetMipMap(0);
				mMaterial->load();
			}
		}

		mDensity = density;
		mRadius = radius;

		RenderUtility::GEOMETRY geom(mGeometry, mIndexBuffer, mVertexSource);
		bool ret = RenderUtility::createSphere(geom, mRadius, mDensity, RenderUtility::BE_UV, RenderUtility::FF_INSIDE, Math::PI*4/5 );
		if( !ret )
			assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	SkySphere::getTexture() const
	{
		return mMaterial == NULL ? TString::EMPTY 
					: mMaterial->getTextureState()->getTextureStage(0)->getResourcePath();
	}
	
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	SkyBox::SkyBox()
	{
		mSpaceFlags = CSF_INDEPENDENT;
		mUpdateFlags = CUF_DEFAULT_VISIBLE;
		mWorldAABB.setInfinite();
		mLocalAABB.setInfinite();
		mGeometry = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	SkyBox::~SkyBox()
	{
		if (mMaterial != NULL)
			mMaterial->unload();
	}

	//////////////////////////////////////////////////////////////////////////
	const GraphicsGeometry&	SkyBox::getGeometry() const
	{
		return *mGeometry;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		SkyBox::initialize(const TString& texture)
	{
		if (mMaterial == NULL)
		{
			Material* material = this->getRenderType().getMaterial();
			mMaterial.bind(BLADE_NEW MaterialInstance(material));
			assert(mMaterial != NULL);
			mMaterial->setShaderGroup(BTString("sky_box"));
		}
		//TODO: support 6 textures for each cube face?

		TString base, ext;
		TStringHelper::getFileInfo(texture, base, ext);

		if (mMaterial->getTextureState()->getTextureStageCount() == 0 || this->getTexture() != texture)
		{
			TextureStage* ts = mMaterial->getTextureState()->addTextureStage(BTString("skyDiffuse"));
			ts->setTextureType(TT_CUBE);
			ts->setResourcePath(base + BTString(".") + IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureExt);
			//because sky sphere keeps constant distance from camera, no mip-level change
			ts->setTargetMipMap(0);
			mMaterial->load();
		}
		//pre-create graphics resource, since getGeometry may be called out of graphics context(multi threaded culling)
		mGeometry = &RenderUtility::getUnitQuad();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString& SkyBox::getTexture() const
	{
		return mMaterial == NULL ? TString::EMPTY
			: mMaterial->getTextureState()->getTextureStage(0)->getResourcePath();
	}

}//namespace Blade
