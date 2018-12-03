/********************************************************************
	created:	2011/04/02
	filename: 	IEffectManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IEffectManager_h__
#define __Blade_IEffectManager_h__
#include <interface/public/graphics/IGraphicsEffectManager.h>
#include <BladeGraphics.h>
#include <interface/IMaterialManager.h>
#include <TextureState.h>

namespace Blade
{

	class IRenderTarget;
	class ITexture;

	class BLADE_GRAPHICS_API IEffectManager : public IGraphicsEffectManager
	{
	public:

		/*
		@describe 
		@param 
		@return 
		*/
		HTEXTURESTATE	createTextureState(const TStringParam& textures, const TStringParam& samplerNames)
		{
			if( textures.size() != samplerNames.size() )
				return HTEXTURESTATE::EMPTY;

			HTEXTURESTATE hTextureState;
			hTextureState.constructInstance<TextureState>();
			for(size_t i = 0; i < textures.size(); ++i)
			{
				const TString& decalImage = textures[i];
				TextureStage* tsg = hTextureState->addTextureStage(samplerNames[i]);
				tsg->setResourcePath(decalImage, TextureResource::TEXTURE_2D_SERIALIZER);
			}
			return hTextureState;
		}

		/*
		@describe
		@param [in] dynamicUpdate indicates this effect will be frequently updated through updateCustomEffect()
		@return NULL if add failed
		@return NULL if failed
		*/
		//TODO: add vertex declaration parameter
		inline virtual HGRAPHICSEFFECT	 createCustomEffect(const Vector3* vertexdata, size_t vertexCount, bool dynamicUpdate, bool independent,
			const TString& materialName, const TStringParam& textures, const TStringParam& samplers,
			const AABB* worldAABB = NULL, const uint32* indexdata = NULL, size_t indexCount = 0)
		{
			if( materialName == TString::EMPTY || vertexdata == NULL || vertexCount == 0 || textures.size() == 0)
				return HGRAPHICSEFFECT::EMPTY;

			Material* material = IMaterialManager::getSingleton().getMaterial( materialName );
			if( material == NULL )
				return HGRAPHICSEFFECT::EMPTY;

			HTEXTURESTATE hTextureState = this->createTextureState( textures, samplers);			
			return this->createCustomEffect(vertexdata, vertexCount, dynamicUpdate, independent, material, hTextureState, worldAABB, indexdata, indexCount);
		}

		/*
		@describe
		@param [in] dynamicUpdate indicates this effect will be frequently updated through updateCustomEffect()
		@return NULL if add failed
		@return NULL if failed
		*/
		//TODO: add vertex declaration parameter
		virtual HGRAPHICSEFFECT	 createCustomEffect(const Vector3* vertexdata, size_t vertexCount, bool dynamicUpdate, bool independent,
			Material* material, const HTEXTURESTATE& textureState,
			const AABB* worldAABB = NULL, const uint32* indexdata = NULL, size_t indexCount = 0) = 0;
	};

	extern template class BLADE_GRAPHICS_API Factory<IGraphicsEffect>;
	typedef Factory<IGraphicsEffect> EffectFactory;
	

}//namespace Blade



#endif // __Blade_IEffectManager_h__