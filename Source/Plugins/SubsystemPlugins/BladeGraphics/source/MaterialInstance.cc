/********************************************************************
	created:	2012/03/22
	filename: 	MaterialInstance.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <MaterialInstance.h>
#include <Material.h>
#include <Technique.h>
#include <Pass.h>
#include <ShaderOption.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include "Camera.h"

#define SHARED_TEXTURE	0x00000001L

namespace Blade
{
	template class FixedArray<HCAMERA,Impl::TEX_PROJECTION_LIMIT>;

	//////////////////////////////////////////////////////////////////////////
	MaterialInstance::MaterialInstance(Material* pTemplate)
		:mTemplate(pTemplate)
		,mDynamicPass(NULL)
		,mMask(0)
		,mTextureMask(uint32(-1))
		,mShaderGroup(0)
		,mForceGroup(0xFF)
		,mDynamicShaderIndex(0xFF)
		,mLODIndex( MATERIALLOD::INVALID_LOD )
	{
		mTemplate->addListener(this);
		this->ResourceStateGroup::addSubState( pTemplate );

		mTextureState.bind(BLADE_NEW TextureState());
		this->ResourceStateGroup::addSubState(mTextureState);
	}

	//////////////////////////////////////////////////////////////////////////
	MaterialInstance::MaterialInstance(Material* pTemplate, const HTEXTURESTATE& sharedTexture)
		:mTemplate(pTemplate)
		,mDynamicPass(NULL)
		,mMask(0)
		,mTextureMask(uint32(-1))
		,mShaderGroup(0)
		,mForceGroup(0xFF)
		,mDynamicShaderIndex(0xFF)
		,mLODIndex( MATERIALLOD::INVALID_LOD )
	{
		mTemplate->addListener(this);
		mTextureState = sharedTexture;
		this->ResourceStateGroup::addSubState( pTemplate );
		this->ResourceStateGroup::addSubState(mTextureState);
	}

	//////////////////////////////////////////////////////////////////////////
	MaterialInstance::~MaterialInstance()
	{
		if( mTemplate != NULL )
			mTemplate->removeListener(this);
		this->ResourceStateGroup::clearSubStates();
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			MaterialInstance::onActivateTechnique(Technique* old, Technique* now)
	{
		mShaderGroup = 0;
		if (now != NULL && now->getLODSetting().size() > 0)
		{
			mLODIndex = (MATERIALLOD::LOD_INDEX)Math::Clamp<size_t>(mLODIndex, 0, now->getLODSetting().size() - 1);
			mShaderGroup = now->getLODSetting()[mLODIndex].mTargetGroupIndex;
		}
		else
			mLODIndex = 0;

		if( mForceGroup != 0xFF )
		{
			const TString& name = old->getShaderGroupList().getAt(mForceGroup).getName();
			mForceGroup = (uint8)now->getShaderGroupList().findGroup(name);
		}
		this->updateDynamicPass();
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&			MaterialInstance::getName() const
	{
		return mTemplate->getName();
	}

	//////////////////////////////////////////////////////////////////////////
	Technique*				MaterialInstance::getActiveTechnique() const
	{
		return mTemplate->getActiveTechnique();
	}

	//////////////////////////////////////////////////////////////////////////
	void					MaterialInstance::setTemplate(Material* pTemplate)
	{
		if(mTemplate == pTemplate)
			return;
		mShaderVariables.clear();

		bool prevLoaded = this->isLoaded() && this->getSubStateCount() > 0;

		if( mTemplate != NULL )
		{
			this->ResourceStateGroup::removeSubState(mTemplate);
			mTemplate->removeListener(this);
		}

		mTemplate = pTemplate;

		if( mTemplate != NULL )
		{
			mTemplate->addListener(this);
			this->ResourceStateGroup::addSubState( pTemplate );
			if( prevLoaded)
			{
				if (!pTemplate->isLoaded())
				{
					this->refreshLoadState();
					this->load();
				}
				//update LOD index
				mShaderGroup = 0;
				mLODIndex = 0;
				mTextureState->applyUniforms(&pTemplate->getTextureUniforms());
				this->updateDynamicPass();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	index_t					MaterialInstance::setShaderGroup(const TString& groupName)
	{
		Technique* tech = this->getActiveTechnique();
		if( tech == NULL)
			return INVALID_INDEX;

		index_t index = tech->getShaderGroupList().findGroup( groupName );
		if( index != INVALID_INDEX )
			mForceGroup = (uint8)index;
		else
			mForceGroup = 0xFF;
		return index;
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderOption*			MaterialInstance::getDynamicShader() const
	{
		if( mDynamicPass == NULL || mDynamicShaderIndex == (uint8)0xFF )
			return NULL;
		assert( mDynamicShaderIndex < mDynamicPass->getShaderOptionCount() );
		return mDynamicPass->getShaderOption(mDynamicShaderIndex);
	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURESTATE&	MaterialInstance::shareTextureState() const
	{
		mMask.raiseBits(SHARED_TEXTURE);
		return mTextureState;
	}

	//////////////////////////////////////////////////////////////////////////
	TextureState*			MaterialInstance::createTextureState(const TextureState* pToClone/* = NULL*/,bool reset/* = true*/)
	{
		if( mTextureState == NULL )
			BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("previous texture state empty unexpected.") );

		if( !this->isUsingSharedTexture() )
		{
			if( pToClone == NULL )
			{
				if( reset )
					//clear to empty/default?
					mTextureState->resetToDefault();
			}
			else if( mTextureState != pToClone)
				*mTextureState = *pToClone;
			else
			{
				//HTEXTURESTATE hOldState = mTextureState;

				//mTextureState.bind( BLADE_NEW TextureState() );
				//*mTextureState = *pToClone;

				//this->removeSubState(hOldState);
				//this->addSubState(mTextureState);
			}
		}
		else
		{
			//
			mMask.clearBits(SHARED_TEXTURE);

			HTEXTURESTATE hOldState = mTextureState;

			mTextureState.bind( BLADE_NEW TextureState() );
			if( pToClone != NULL )
				*mTextureState = *pToClone;
			else
				*mTextureState = *hOldState;

			this->removeSubState(hOldState);
			this->addSubState(mTextureState);
		}

		return mTextureState;
	}

	//////////////////////////////////////////////////////////////////////////
	void					MaterialInstance::setSharedTextureState(const HTEXTURESTATE& hSharedTextureState)
	{
		if( hSharedTextureState != NULL )
		{
			if( mTextureState != NULL )
			{
				mMask.raiseBits(SHARED_TEXTURE);
				if( mTextureState != hSharedTextureState )
				{
					this->removeSubState(mTextureState);
					this->addSubState(hSharedTextureState);

					mTextureState = hSharedTextureState;
				}
			}
			else
				BLADE_EXCEPT(EXCE_STATE_ERROR,BTString("previous texture state empty unexpected.") );
		}
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("param texture states cannot be empty."));

		mTextureMask = 0;
		for (size_t i = 0; i < mTextureState->getTextureStageCount(); ++i)
			mTextureMask.raiseBitAtIndex(i);
	}

	//////////////////////////////////////////////////////////////////////////
	bool					MaterialInstance::isUsingSharedTexture() const
	{
		return mMask.checkBits(SHARED_TEXTURE);
	}

	/************************************************************************/
	/* LODs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const LODSetting*	MaterialInstance::getLODSetting() const
	{
		if( this->getActiveTechnique() == NULL )
			return NULL;
		else
			return &(this->getActiveTechnique()->getLODSetting());
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool					MaterialInstance::setLOD(MATERIALLOD::LOD_INDEX LODIndex)
	{
		if( mLODIndex == LODIndex || this->getLODSetting() == NULL )
			return true;

		const LODSetting& LODSettings = *(this->getLODSetting());

		if( LODIndex >= LODSettings.size() )
			return false;

		Technique* tech = this->getActiveTechnique();
		if( tech == NULL )
			return false;

		const MATERIALLOD& LOD = LODSettings[LODIndex];
		uint8 groupIndex = LOD.mTargetGroupIndex;

		const ShaderGroupList& groupList = tech->getShaderGroupList();
		if( groupIndex > groupList.size() )
			return false;

		uint16 textureStart;
		uint16 textureEnd;
		const ShaderGroup& sortGroup = groupList[groupIndex];;
		ShaderGroup::SRequireValue val = sortGroup.getRequirement( ShaderGroup::SR_TEXTURE );
		if( val.isEnabled() )
		{
			textureStart = val.mIndexStart;
			textureEnd = val.mIndexEnd;
		}
		else
		{
			mLODIndex = LODIndex;
			mShaderGroup = groupIndex;
			this->updateDynamicPass();
			return true;
		}

		const TStringList& textures = mTemplate->getTextureUniforms();
		uint16 minTexIndex = std::min(textureStart, (uint16)textures.size() );
		uint16 maxTexIndex = std::min(textureEnd, (uint16)(textures.size()-1) );

		bool bTextureOK = true;
		for(uint16 i = minTexIndex; i <= maxTexIndex; ++i)
		{
			if( i >= textures.size() )
			{
				assert(false);
				return true;
			}

			const TString& uniform = textures[i];
			TextureStage* stage = mTextureState->getTextureStage(uniform);
			if( stage == NULL )
				return true;
			if( !stage->isLoaded() || stage->getResourcePath() == TString::EMPTY )
			{
				if( stage->getTexture() != NULL )
					assert( stage->isManualResource() );
				else
					bTextureOK = false;
			}
		}

		//invalid texture found,cancel
		if( !bTextureOK )
			return true;

		const bool sharedTexture = (mMask&SHARED_TEXTURE) != 0;
		mTextureMask = 0;

		bool bLoadTexture = false;
		bool bUnloadTexture = false;
		for(uint16 i = 0; i < minTexIndex; ++i)
		{
			TextureStage* stage = mTextureState->getTextureStage(textures[i]);
			if( stage == NULL )
				continue;

			if (!stage->isManualResource() && !sharedTexture)
			{
				bUnloadTexture = true;
				stage->setEnable(false);
			}
			mTextureMask.clearBitAtIndex(i);
		}

		for(uint16 i = minTexIndex; i <= maxTexIndex; ++i)
		{
			TextureStage* stage = mTextureState->getTextureStage(textures[i]);
			if( !stage->isEnabled() && !stage->isManualResource())
				bLoadTexture = true;
			stage->setEnable(true);
			mTextureMask.raiseBitAtIndex(i);
		}

		for(uint16 i = maxTexIndex+1u; i < textures.size(); ++i)
		{
			TextureStage* stage = mTextureState->getTextureStage(textures[i]);
			if( stage == NULL )
				continue;
			if (!stage->isManualResource() && stage->isEnabled() && !sharedTexture)
			{
				bUnloadTexture = true;
				stage->setEnable(false);
			}
			mTextureMask.clearBitAtIndex(i);
		}

		if(bUnloadTexture || bLoadTexture)
		{
			this->refreshLoadState();
			this->load();
		}

		mLODIndex = LODIndex;
		if( !bLoadTexture || this->isLoaded() )
		{
			mShaderGroup = groupIndex;
			this->updateDynamicPass();
		}
		//else waiting callback
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	MATERIALLOD::LOD_INDEX	MaterialInstance::calculateMaxLOD() const
	{
		Technique* tech = this->getActiveTechnique();
		if( tech == NULL )
			return 0;

		const LODSetting& LODSettings = *( this->getLODSetting());
		if( LODSettings.size() == 0 )
			return 0;

		for(index_t i = 0; i < LODSettings.size(); ++i)
		{
			const MATERIALLOD& LOD = LODSettings[i];
			uint16 groupIndex = LOD.mTargetGroupIndex;

			const ShaderGroupList& groupList = tech->getShaderGroupList();
			if( groupIndex > groupList.size() )
				continue;

			uint16 textureStart;
			uint16 textureEnd;
			const ShaderGroup& sortGroup = groupList[groupIndex];;
			ShaderGroup::SRequireValue val = sortGroup.getRequirement( ShaderGroup::SR_TEXTURE );
			if( !val.isEnabled() )
				return (MATERIALLOD::LOD_INDEX)i;

			textureStart = val.mIndexStart;
			textureEnd = val.mIndexEnd;

			BLADE_UNREFERENCED(textureStart);
			if( mTextureState->getTextureStageCount() > textureEnd )
				return (MATERIALLOD::LOD_INDEX)i;
		}
		//fall back: return the lowest
		return (MATERIALLOD::LOD_INDEX)( LODSettings.size()-1 );
	}

	/************************************************************************/
	/* effects                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void					MaterialInstance::setTextureProjectionCamera(index_t index,ICamera* cam)
	{
		if( index >= TEX_PROJECTION_LIMIT )
		{
			assert(false);
			return;
		}
		mTextureProjection[index].bind( cam );
	}

	//////////////////////////////////////////////////////////////////////////
	ICamera*				MaterialInstance::createTexutreProjectionCamera(index_t index)
	{
		if( index >= TEX_PROJECTION_LIMIT )
		{
			assert(false);
			return NULL;
		}

		if( mTextureProjection[index]  == NULL )
			mTextureProjection[index].bind( BLADE_NEW Camera() );

		return mTextureProjection[index];
	}

	//////////////////////////////////////////////////////////////////////////
	const HRENDERPROPTYSET&MaterialInstance::getRenderProperty() const
	{
		return mInstanceProperty;
	}

	//////////////////////////////////////////////////////////////////////////
	ICamera*				MaterialInstance::getTextureProjectionCamera(index_t index, bool create/* = false*/) const
	{
		if( index >= TEX_PROJECTION_LIMIT )
		{
			assert(false);
			return NULL;
		}
		else if( mTextureProjection[index] == NULL && create)
			mTextureProjection[index].bind( BLADE_NEW Camera() );

		return mTextureProjection[index];
	}

	//////////////////////////////////////////////////////////////////////////
	void					MaterialInstance::enableColor(bool bEnable, const Color& diffuse,
		const Color& specular/* = Color::WHITE*/, const Color& emissive/* = Color::WHITE*/)
	{
		if( bEnable )
		{
			if( mInstanceProperty == NULL )
				mInstanceProperty.constructInstance<RenderPropertySet>();

			ColorProperty* color = BLADE_NEW ColorProperty();
			color->setDiffuse(diffuse);
			color->setSpecular(specular);
			color->setEmissive(emissive);
			mInstanceProperty->setProperty(color);
		}
		else if( mInstanceProperty != NULL )
			mInstanceProperty->removeProperty(RPT_COLOR);
	}

	//////////////////////////////////////////////////////////////////////////
	void					MaterialInstance::enableAlphaBlend(bool enable, scalar opacity)
	{
		if( enable )
		{
			if( mInstanceProperty == NULL )
				mInstanceProperty.constructInstance<RenderPropertySet>();

			AlphaBlendProperty* prop = mInstanceProperty->getProperty(RPT_ALPHABLEND);
			if( prop == NULL )
			{
				prop = BLADE_NEW AlphaBlendProperty();
				mInstanceProperty->setProperty(prop);
			}		
			prop->setSourceBlend(BM_SRC_ALPHA);
			prop->setDestBlend(BM_INV_SRC_ALPHA);
			prop->setBlendOperation(BO_ADD);
			prop->setBlendFactor(opacity);
		}
		else if( mInstanceProperty != NULL )
			mInstanceProperty->removeProperty(RPT_ALPHABLEND);
	}

	/************************************************************************/
	/* clone & copy                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HMATERIALINSTANCE MaterialInstance::clone(int shareMask/* = MIS_ALL*/) const
	{
		MaterialInstance* instance;

		bool shareTexture = (shareMask&MIS_TEXTURE) != 0;
		bool shareProjCamera = (shareMask&MIS_PROJCAM) != 0;

		if( shareTexture )
			instance = BLADE_NEW MaterialInstance( mTemplate, this->shareTextureState() );
		else
		{
			instance = BLADE_NEW MaterialInstance( mTemplate );
			*instance->mTextureState = *mTextureState;
		}

		instance->mLODIndex = mLODIndex;
		instance->mShaderGroup = mShaderGroup;

		if( shareProjCamera )
			//share all projection camera
			instance->mTextureProjection = mTextureProjection;
		else
		{
			index_t i = 0;
			while( mTextureProjection[i] != NULL)
			{
				instance->setTextureProjectionCamera(i, mTextureProjection[i]->clone() );
				++i;
			}
		}

		if( mInstanceProperty != NULL )
		{
			instance->mInstanceProperty.constructInstance<RenderPropertySet>();
			for(int i = RPT_BEGIN; i < RPT_COUNT; ++i)
			{
				RENDER_PROPERTY eProp = RENDER_PROPERTY(i);
				if( !mInstanceProperty->hasProperty(eProp) )
					continue;
				int shareBit = 1 << i;
				if( (shareMask&shareBit) )
					instance->mInstanceProperty->setProperty( mInstanceProperty->getProperty(eProp) );
				else
					instance->mInstanceProperty->setProperty( IRenderProperty::cloneProperty(mInstanceProperty->getProperty(eProp)) );
			}
		}

		instance->recalcLoadedCount();
		return HMATERIALINSTANCE(instance);
	}

	/************************************************************************/
	/* ResourceStateGroup overrides                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			MaterialInstance::onAllReady()
	{
		assert( this->isLoaded() );

		mTextureState->applyUniforms(&mTemplate->getTextureUniforms());
		this->switchLOD();

		if(mShaderVariables.size() == 0 && mTemplate->hasCustomInstanceShaderVariables())
		{
			//auto-create per-instance variable data storage
			//and link sampler index to texture stages.
			for(size_t i = 0; i < mTemplate->getTechniqueCount(); ++i)
			{
				const Technique* tech = mTemplate->getTechnique(i);
				for(size_t j = 0; j < tech->getPassCount(); ++j)
				{
					const Pass* pass = tech->getPass(j);
					for(size_t k = 0; k < pass->getShaderOptionCount(); ++k)
					{
						const ShaderOption* option = pass->getShaderOption(k);
						const ShaderInstance* shaderInst = option->getShaderInstance();
						shaderInst->createAndLinkCustomInstanceVariable(mShaderVariables);
					}
				}
			}//for each technique
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			MaterialInstance::switchLOD()
	{
		const LODSetting* LODSettings = this->getLODSetting();

		if( LODSettings != NULL && LODSettings->size() != 0)
		{
			//get the lowest material LOD level
			if( mLODIndex == MATERIALLOD::INVALID_LOD )
				mLODIndex = LODSettings->back().mLODIndex;
		}

		if( mLODIndex != MATERIALLOD::INVALID_LOD && mLODIndex < LODSettings->size() )
			mShaderGroup = (*LODSettings)[mLODIndex].mTargetGroupIndex;

		this->updateDynamicPass();
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialInstance::updateDynamicPass()
	{
		Technique* tech = this->getActiveTechnique();
		if( tech == NULL)
		{
			mDynamicPass = NULL;
			mDynamicShaderIndex = 0xFF;
			return;
		}

		if( !mDynamicPassName.empty() )
			mDynamicPass = this->getActiveTechnique()->getPass(mDynamicPassName);
		else
			mDynamicPass = this->getActiveTechnique()->getPass(0);

		if( mDynamicPass != NULL )
			mDynamicShaderIndex = (uint8)mDynamicPass->findShaderOption(0, this->getActiveShaderGroup());
		else
			mDynamicShaderIndex = 0xFF;
	}

}//namespace Blade