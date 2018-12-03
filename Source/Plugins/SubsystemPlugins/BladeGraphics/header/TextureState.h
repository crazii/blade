/********************************************************************
	created:	2010/05/22
	filename: 	TextureState.h
	author:		Crazii
	purpose:	texture state with list of texture stage objects
				the texture stage order in list should match declaration order in .bmt
*********************************************************************/
#ifndef __Blade_TextureState_h__
#define __Blade_TextureState_h__
#include <TextureStage.h>
#include <utility/FixedArray.h>
#include <StaticHandle.h>
#include <ResourceState.h>

namespace Blade
{
	class TextureState;
	typedef Handle<TextureState> HTEXTURESTATE;

	class BLADE_GRAPHICS_API TextureState : public ResourceStateGroup, public Allocatable
	{
	public:

	public:
		TextureState();
		~TextureState();

		TextureState(const TextureState& src);
		TextureState& operator=(const TextureState& rhs);

		/*
		@describe
		@param
		@return
		*/
		inline size_t			getTextureStageCount() const
		{
			return mStages.size();
		}

		/*
		@describe
		@param
		@return
		*/
		inline TextureStage*	getTextureStage(index_t index) const
		{
			return mStages[index];
		}

		/*
		@describe 
		@param
		@return
		*/
		BLADE_ALWAYS_INLINE TextureStage*	getTextureStage(const TString& samplerName) const
		{
			for (index_t i = 0; i < mStages.size(); ++i)
			{
				if (mStages[i]->getSamplerUniform() == samplerName)
					return mStages[i];
			}
			return NULL;
		}

		/*
		@describe 
		@param
		@return
		*/
		BLADE_ALWAYS_INLINE index_t			getStageIndex(const TString& samplerName) const
		{
			{
				for (index_t i = 0; i < mStages.size(); ++i)
				{
					if (mStages[i]->getSamplerUniform() == samplerName)
						return i;
				}
				return INVALID_INDEX;
			}
		}

		/**
		@describe add a new texture stage at the tail. if stage with the same name existing, return the existing one
		@param
		@return
		*/
		TextureStage*	addTextureStage(const TString& name, const HTEXTURE& texture = HTEXTURE::EMPTY);

		/**
		@describe
		@param
		@return
		*/
		void			applyUniforms(const TStringList* uniforms);

		/*
		@describe default: clear all stages
		@param
		@return
		*/
		void			resetToDefault();

		/*
		@describe
		@param
		@return
		*/
		static const HTEXTURESTATE&	getDefaultTextureState();
		
	private:
		typedef FixedVector<TextureStage*,ITexture::MAX_TEXTURE_COUNT>	TextureStageList;
		TextureStageList		mStages;
		const TStringList*		mUniforms;
	};//class TextureState

	struct TextureStateInstance
	{
		const TextureState*	state;
		Mask			stateMask;
		inline TextureStateInstance() :state(NULL), stateMask(0) {}
		inline TextureStateInstance(const TextureState* _state, Mask _mask) :state(_state), stateMask(_mask) {}
		inline bool operator==(const TextureStateInstance& rhs) const { return this == &rhs || (state == rhs.state && stateMask == rhs.stateMask); }
		inline bool operator!=(const TextureStateInstance& rhs) const { return !(*this == rhs); }
	};
	
}//namespace Blade


#endif //__Blade_TextureState_h__