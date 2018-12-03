/********************************************************************
	created:	2011/04/02
	filename: 	IGraphicsEffectManager.h
	author:		Crazii
	purpose:	extended helper class for graphics effects. i.e. particles, decals, etc.
				usually graphics effects are created by effect manager. effects could be attached to a IGraphicsInterface [optional].
				if the effect has a specific life time, you probably need to add it to the manager,
				otherwise adding the effect to manager is optional.
*********************************************************************/
#ifndef __Blade_IGraphicsEffectManager_h__
#define __Blade_IGraphicsEffectManager_h__
#include <utility/String.h>
#include <math/Ray.h>
#include <interface/public/graphics/IShader.h>
#include <BladeFramework.h>
#include <interface/public/graphics/IGraphicsInterface.h>
#include <interface/public/graphics/GraphicsEffect.h>
#include <interface/public/graphics/IGraphicsScene.h>

namespace Blade
{
	class IGraphicsView;
	class IGraphicsScene;
	class IElement;

	class BLADE_FRAMEWORK_API IGraphicsEffectManager
	{
	public:
		virtual ~IGraphicsEffectManager()	{}

		//////////////////////////////////////////////////////////////////////////
		//per-scene methods
		//////////////////////////////////////////////////////////////////////////
		/**
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsScene*	getScene() const = 0;

		//the following method can ONLY be called in SYNCHRONOUS mode: TS_MAIN_SYNC, or asynchronous run mode: TS_ASYNC_RUN

		/**
		@describe add effect to manage it's life time (auto delete).when time up, auto-detach from element if attached
		@param [in] lifetime : the effect's life circle. 
		<= 0 is persistent
		> 0 is a real life circle
		@return NULL if failed
		*/
		virtual bool	addEffect(const HGRAPHICSEFFECT& hEffect, scalar lifeTime) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	removeEffect(const HGRAPHICSEFFECT& hEffect) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HGRAPHICSEFFECT	createEffect(const TString& type) = 0;

		/**
		@describe controllable decal
		@param [in] aab : initial position & size of the decal
		@param [in] materialName: material of decal used
		@param [in] decalImage : the overriding image of the decal , is could be valid only lief time is not 0
		@return 
		*/
		inline bool			attachDecal(IGraphicsInterface* element, const TString& materialName, const AABB& aab,
			const TStringParam& decalImages, const TStringParam& decalSamplers)
		{
			HGRAPHICSEFFECT hEffect = this->createEffect( IDecalEffect::TYPE );
			IDecalEffect* effect = static_cast<IDecalEffect*>( hEffect );
			return effect->initialize(true, aab, materialName, decalImages, decalSamplers) && element->addEffect(hEffect);
		}

		inline bool			attachDecal(IGraphicsInterface* element, const AABB& aab, const TString& texture, const TString& sampler)
		{
			HGRAPHICSEFFECT hEffect = this->createEffect(IDecalEffect::TYPE);
			IDecalEffect* effect = static_cast<IDecalEffect*>( hEffect );
			return effect->initialize(true, aab, texture, sampler) && element->addEffect(hEffect);
		}

		inline HELEMENT	createDecalElement(const AABB& aab, 
			const TString& materialName, const TStringParam& decalImages, const TStringParam& decalSamplers,
			const TString& spaceName = TString::EMPTY)
		{
			if( decalImages.size() == 0 || materialName == TString::EMPTY )
				return HELEMENT::EMPTY;

			IGraphicsScene* scene = this->getScene();
			HELEMENT element = scene->createGraphicsElement( GraphicsConsts::EMPTY_ELEMENT_TYPE, spaceName);
			IGraphicsInterface* i = element->getInterface(IID_GRAPHICS());
			if( this->attachDecal(i, materialName, aab, decalImages, decalSamplers) )
				return element;
			else
				return HELEMENT::EMPTY;
		}

		inline HELEMENT	createDecalElement(const AABB& aab, 
			const TString& decalImage, const TString& decalSampler, 
			const TString& spaceName = TString::EMPTY)
		{
			if( decalImage == TString::EMPTY )
				return HELEMENT::EMPTY;

			IGraphicsScene* scene = this->getScene();
			HELEMENT element = scene->createGraphicsElement( GraphicsConsts::EMPTY_ELEMENT_TYPE, spaceName);
			IGraphicsInterface* i = element->getInterface(IID_GRAPHICS());
			if( this->attachDecal(i, aab, decalImage, decalSampler) )
				return element;
			else
				return HELEMENT::EMPTY;
		}

		inline HELEMENT	createBrushDecalElement(scalar size, scalar height, const TString& decalImage,
			const TString& spaceName = TString::EMPTY)
		{
			if( decalImage == TString::EMPTY )
				return HELEMENT::EMPTY;

			IGraphicsScene* scene = this->getScene();
			HELEMENT element = scene->createGraphicsElement( GraphicsConsts::EMPTY_ELEMENT_TYPE, spaceName);
			IGraphicsInterface* i = element->getInterface(IID_GRAPHICS());
			HGRAPHICSEFFECT hEffect = this->createEffect(IBrushDecalEffect::TYPE);

			IBrushDecalEffect* effect = static_cast<IBrushDecalEffect*>( hEffect );
			if( !effect->initialize(size, height, decalImage)  || !i->addEffect( hEffect ) )
				return HELEMENT::EMPTY;
			else
				return element;
		}

		/**
		@describe simple decal operation (managed & auto-deleted)
		@param [in] lifeTime : the life time of the decal
		<= 0: always living, persistent (scene life time)
		>0	: real life time
		@param [in] aab : initial position & size of the decal
		@param [in] materialName: material of decal used
		@param [in] decalImage : the overriding image of the decal , is could be valid only lief time is not 0
		@return NULL if failed
		*/
		virtual inline HGRAPHICSEFFECT createDecal(scalar lifeTime,bool bDyanmic, const AABB& aab, 
			const TString& materialName, const TStringParam& decalImages, const TStringParam& decalSamplers)
		{
			if( materialName == TString::EMPTY || decalImages.size() == 0)
				return HGRAPHICSEFFECT::EMPTY;

			HGRAPHICSEFFECT hEffect = this->createEffect(IDecalEffect::TYPE);
			IDecalEffect* effect = static_cast<IDecalEffect*>( hEffect );
			if( !effect->initialize(bDyanmic, aab, materialName, decalImages, decalSamplers) )
				return HGRAPHICSEFFECT::EMPTY;

			if( lifeTime == 0 )
				lifeTime = -1.0f;

			bool ret = this->addEffect(hEffect, lifeTime);
			assert(ret);
			if( ret )
				return hEffect;
			else
				return HGRAPHICSEFFECT::EMPTY;
		}

		/**
		@describe advanced decal operation (manually controlled)
		@param [in] aab : initial position & size of the decal
		@param [in] materialName: material of decal used
		@param [in] decalImage : the overriding image of the decal , is could be valid only lief time is not 0
		@return NULL if lifetime = 0 (immediate mode)
		*/
		virtual inline HGRAPHICSEFFECT createBrushDecal(scalar size, scalar height, const TString& decalImage, const Color& color = Color::WHITE)
		{
			if( decalImage == TString::EMPTY )
				return HGRAPHICSEFFECT::EMPTY;
			HGRAPHICSEFFECT hEffect = this->createEffect(IBrushDecalEffect::TYPE);
			IBrushDecalEffect* effect = static_cast<IBrushDecalEffect*>( hEffect );
			effect->initialize(size, height, decalImage);
			effect->setColor(color);
			return hEffect;
		}

		/**
		@describe 
		@param [in] dynamicUpdate indicates this effect will be frequently updated through updateCustomEffect()
		@return NULL if failed
		*/
		virtual HGRAPHICSEFFECT	 createCustomEffect(const Vector3* vertexdata, size_t vertexCount, bool dynamicUpdate, bool independent,
			const TString& materialName,const TStringParam& textures, const TStringParam& samplers,
			const AABB* worldAABB = NULL, const uint32* indexdata = NULL, size_t indexCount = 0) = 0;

		/**
		@describe update the custom effect. this only works if dynamicUpdate parameter used in createCustomEffect() is true.
		@param 
		@return 
		*/
		virtual bool			updateCustomEffect(const HGRAPHICSEFFECT& hCustomEffect, const Vector3* vertexdata, size_t vertexCount,
			const AABB* worldAABB = NULL, const uint32* indexdata = NULL, size_t indexCount = 0) = 0;

		//////////////////////////////////////////////////////////////////////////
		//global methods
		//////////////////////////////////////////////////////////////////////////
		/**
		@describe set per material custom variables for shader variable updating
		@param 
		@return 
		*/
		virtual bool	setCustomShaderVariable(const TString& materialName, const TString& variableName,
			SHADER_CONSTANT_TYPE type, size_t count, const void* pData) = 0;

	protected:
		/** @brief  */
		static inline void setEffectManager(IGraphicsEffect* effect, IGraphicsEffectManager* manager)
		{
			effect->setManager(manager);
		}

		/** @brief  */
		inline void manageEffect(IGraphicsEffect* effect)
		{
			assert(effect->getManager() == NULL);
			IGraphicsEffectManager::setEffectManager(effect, this);
		}

		/** @brief  */
		inline void unmanageEffect(IGraphicsEffect* effect)
		{
			assert(effect->getManager() == this);
			IGraphicsEffectManager::setEffectManager(effect, NULL);
		}
	};
	

}//namespace Blade



#endif // __Blade_IGraphicsEffectManager_h__
