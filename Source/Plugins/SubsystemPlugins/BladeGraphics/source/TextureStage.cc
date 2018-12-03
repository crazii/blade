/********************************************************************
	created:	2010/05/23
	filename: 	TextureStage.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <TextureStage.h>
#include <interface/IResourceManager.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IGraphicsSystem.h>
#include "TextureShaderHelper.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TextureStage::TextureStage(const TString& name, const HTEXTURE& texture/* = HTEXTURE::EMPTY*/)
		:mSamplerUniform(name)
	{
		mResourceType = TextureResource::TEXTURE_RESOURCE_TYPE;
		mMipmap = IImage::MIPLOD_DEFAULT;
		//TODO: add adv usage to select format. i.e. normal, color, etc
		mFormat = PF_UNDEFINED;
		mType = TT_2DIM;
		mLinearSpace = false;
		if (texture != NULL)
			this->setTexture(texture);
		else
			this->updateTextureSize();
	}

	//////////////////////////////////////////////////////////////////////////
	TextureStage::~TextureStage()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void					TextureStage::setTexture(const HTEXTURE& texture)
	{
		HTEXTURERES hRes;
		if (texture != NULL)
		{
			hRes.bind(BLADE_NEW TextureResource());
			hRes->setTexture(texture);
		}
		this->setManualResource(true, hRes);
		this->updateTextureSize();
	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&			TextureStage::getTexture() const
	{
		if( mResource == NULL )
		{
			//BLADE_EXCEPT(EXC_NEXIST,BTString("texture resource not exist.") );
			return HTEXTURE::EMPTY;
		}
		return this->getTypedResource<TextureResource>()->getTexture();
	}

	//////////////////////////////////////////////////////////////////////////
	void			TextureStage::generateParams(ParamList& params) const
	{
		params[BTString("FORMAT")] = (int)mFormat;
		params[BTString("MIPMAP")] = mMipmap;
		params[BTString("LINEAR_SPACE")] = mLinearSpace;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TextureStage::postLoad(const HRESOURCE& resource)
	{
		HTEXTURE prevTexture = this->getTexture();
		SubResourceState::postLoad(resource);

		const HTEXTURE& texture = this->getTexture();
		if (texture != NULL )
		{
			//note: same texture may change size, especially in editor mode
			//if (prevTexture != texture)
			{
				this->updateTextureSize();
			}
		}
		else
			mTexureSize = Vector4::UNIT_ALL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TextureStage::onFailed()
	{
		const IGraphicsResourceManager::GCONFIG& cfg = IGraphicsResourceManager::getSingleton().getGraphicsConfig();
		if(mFormat == cfg.CompressedNormal || mFormat == cfg.DefaultNormal)
			this->setManualResource(true, IGraphicsSystem::getSingleton().getBuiltinTextures(IGraphicsSystem::BT_UNIT_NORMAL));
		else
			this->setManualResource(true, IGraphicsSystem::getSingleton().getBuiltinTextures(IGraphicsSystem::BT_MAGENTA));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void		TextureStage::updateTextureSize()
	{
		const HTEXTURE& texture = this->getTexture();
		Impl::updateTextureSize(mTexureSize, texture);
	}
	
}//namespace Blade