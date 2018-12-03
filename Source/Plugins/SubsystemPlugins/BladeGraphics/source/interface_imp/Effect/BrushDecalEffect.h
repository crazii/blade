/********************************************************************
	created:	2012/12/30
	filename: 	BrushDecalEffect.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BrushDecalEffect_h__
#define __Blade_BrushDecalEffect_h__
#include "DecalEffect.h"
#include <Technique.h>
#include <Pass.h>

namespace Blade
{
	static const TString COLOR_SHADER_VARIABLE_NAME = BTString("BRUSH_COLOR");

	class BrushDecalEffect : public virtual IBrushDecalEffect, public IGraphicsUpdatable, public Allocatable
	{
	public:
		static const TString BRUSH_DECAL_MATERIAL;
		static const TString BRUSH_DECAL_SAMPLER;

		BrushDecalEffect()		{mUpdateStateMask.raiseBitAtIndex(SS_PRE_RENDER);mMaterial = NULL;}
		~BrushDecalEffect()		
		{
			//!manager's life time must be longer than this
			IEffectManager* manager = static_cast<IEffectManager*>( this->getManager() );
			if(mInternalEffect != NULL && manager != NULL)
				manager->removeEffect(mInternalEffect);

			//if( mMaterial != NULL )
			//	mMaterial->unload();
			if( mDecalImages != NULL )
				mDecalImages->unload();

			IRenderScene* scene = static_cast<IRenderScene*>(this->getManager()->getScene());
			scene->getUpdater()->removeFromUpdate(this);
		}

		/************************************************************************/
		/* IGraphicsEffect                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	onAttach()
		{
			return false;
		}
		/** @brief  */
		virtual bool	onDetach()
		{
			return false;
		}
		/** @brief  */
		virtual bool	isReady() const
		{
			return mMaterial != NULL && mMaterial->isLoaded();
		}

		/************************************************************************/
		/* IDecalEffect interface                                                                     */
		/************************************************************************/
		//////////////////////////////////////////////////////////////////////////
		virtual bool	setDecalImage(const TString& texture)
		{
			if(mMaterial == NULL || texture == TString::EMPTY )
				return false;

			IEffectManager* manager = static_cast<IEffectManager*>( this->getManager() );
			mDecalImages.clear();
			TStringParam textures;
			textures.push_back(texture);
			TStringParam samplers;
			samplers.push_back(BRUSH_DECAL_SAMPLER);
			mDecalImages = manager->createTextureState(textures, samplers);
			mDecalImages->getTextureStage(0)->setTargetPixelFormat(PF_BC4);
			mDecalImages->load();

			if( mInternalEffect != NULL )
			{
				MaterialInstance* material = this->getEffect()->getMaterialInstance();
				material->setSharedTextureState(mDecalImages);
			}
			return true;
		}

		/** @brief  */
		virtual bool initializeImpl(const TString& texture)
		{
			if( mMaterial != NULL )
			{
				assert(false);
				return false;
			}

			mMaterial = IMaterialManager::getSingleton().getMaterial( BRUSH_DECAL_MATERIAL );
			if( mMaterial == NULL )
				return false;
			this->setDecalImage(texture);
			mMaterial->load();

			IRenderScene* scene = static_cast<IRenderScene*>(this->getManager()->getScene());
			scene->getUpdater()->addForUpdate(this);
			return true;
		}

		/** @brief  */
		virtual bool setMaterialImpl(const TString& materialName)
		{
			if( mMaterial == NULL )
				return false;

			Material* material = IMaterialManager::getSingleton().getMaterial( materialName );
			if( mMaterial == NULL )
				return false;

			mMaterial = material;
			this->setColorImpl(mColor);
			return true;
		}

		/************************************************************************/
		/* IBrushDecalEffect                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	setColorImpl(const Color& color)
		{
			if( mMaterial == NULL || mMaterial->getActiveTechnique() == NULL 
				|| mMaterial->getActiveTechnique()->getPass(0) == NULL 
				|| mMaterial->getActiveTechnique()->getPass(0)->getShaderOption(0) == NULL )
				return false;
			
			if( mInternalEffect != NULL )
			{
				MaterialInstance* material = this->getEffect()->getMaterialInstance();
				//return material->setCustomInstanceVariable(COLOR_SHADER_VARIABLE_NAME,SCT_FLOAT4,1, &color);
				material->enableColor(true, color);
				return true;
			}
			else
				//keep the data and set on later init
				return true;
		}

		/************************************************************************/
		/* IGraphicsUpdatable interface                                                                    */
		/************************************************************************/
		virtual void update(SSTATE)
		{
			AABB aab = this->getAABB();
			if( mVisible )
			{
				assert( mDecalImages != NULL );

				IRenderScene* scene = static_cast<IRenderScene*>(this->getManager()->getScene());
				DecalTriangleList* triangleList = BLADE_NEW DecalTriangleList();

				AABBQuery decalVolume(aab, mAppFlag, triangleList);
				bool result = scene->queryElements(decalVolume);

				if( result && triangleList->getVertexCount() != 0)
				{
					if( mInternalEffect != NULL )
					{
						MaterialInstance* material = this->getEffect()->getMaterialInstance();
						ICamera* camera = material->getTextureProjectionCamera(0, true);
						camera->setOrthoMode(mSize,mSize);
						Vector3 eye(mPosition.x , mHeight, mPosition.z);
						Vector3 target(mPosition.x, -mHeight, mPosition.z);
						Matrix44 viewMatrix;
						//the view matrix is looking down (0,-1,0), so do not use UNIT_Y as up vector
						Matrix44::generateViewMatrix(viewMatrix, eye, target, Vector3::UNIT_Z);
						camera->setViewMatrix(viewMatrix, true);
					}

					const AABB* paab = &aab;
					IEffectManager* manager = static_cast<IEffectManager*>(this->getManager());
					if (mInternalEffect == NULL)
					{
						mInternalEffect = manager->createCustomEffect(triangleList->getVertexData(), triangleList->getVertexCount(), true, true, mMaterial, mDecalImages, paab);
						if (mInternalEffect != NULL)
						{
							CustomEffect* effect = this->getEffect();
							MaterialInstance* material = effect->getMaterialInstance();
							material->enableColor(true, mColor);
							//manager->addEffect(mInternalEffect, 0); //optional
						}
					}
					else
						manager->updateCustomEffect(mInternalEffect, triangleList->getVertexData(), triangleList->getVertexCount(), paab);
				}
				BLADE_DELETE triangleList;
			}
		}
	protected:
		/** @brief  */
		CustomEffect* getEffect() const
		{
			return static_cast<CustomEffect*>(mInternalEffect);
		}

		Material*		mMaterial;
		HTEXTURESTATE	mDecalImages;
		HGRAPHICSEFFECT	mInternalEffect;
	};
	
}//namespace Blade

#endif//__Blade_BrushDecalEffect_h__