/********************************************************************
	created:	2010/05/22
	filename: 	TextureState.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <TextureState.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	TextureState::TextureState()
	{
		mUniforms = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	TextureState::~TextureState()
	{
		this->clearSubStates();
		for (size_t i = 0; i < mStages.size(); ++i)
			BLADE_DELETE mStages[i];
	}

	//////////////////////////////////////////////////////////////////////////
	TextureState::TextureState(const TextureState& src)
		:ResourceStateGroup(src)
	{
		for(size_t i = 0; i < src.mStages.size(); ++i)
		{
			TextureStage* stage = src.mStages[i];
			TextureStage* newStage = BLADE_NEW TextureStage(stage->getSamplerUniform());
			*newStage = *stage;
			mStages.push_back(newStage);
			this->addSubState( newStage );
		}
		this->recalcLoadedCount();
	}

	//////////////////////////////////////////////////////////////////////////
	TextureState& TextureState::operator=(const TextureState& rhs)
	{
		if( this != &rhs )
		{
			this->ResourceStateGroup::operator=(rhs);

			//TODO?
			for(size_t i = 0; i < rhs.mStages.size(); ++i)
			{
				TextureStage* stage = rhs.mStages[i];
				TextureStage* newStage = BLADE_NEW TextureStage(stage->getSamplerUniform());
				*newStage = *stage;
				mStages.push_back(newStage);
				this->addSubState(newStage);
			}

			this->recalcLoadedCount();
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	TextureStage*	TextureState::addTextureStage(const TString& name, const HTEXTURE& texture/* = HTEXTURE::EMPTY*/)
	{
		TextureStage* existingStage = this->getTextureStage(name);
		if (existingStage != NULL)
		{
			existingStage->setTexture(texture);
			return existingStage;
		}
		else if (mStages.size() != mStages.capacity())
		{
			TextureStage* stage = BLADE_NEW TextureStage(name, texture);
			mStages.push_back(stage);
			this->addSubState(stage);
			return stage;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("list(texture stage) full."));
	}

	//////////////////////////////////////////////////////////////////////////
	void			TextureState::applyUniforms(const TStringList* uniforms)
	{
		if (uniforms == NULL || uniforms->size() == 0)
			return;

		if (mUniforms != uniforms)
		{
			mUniforms = uniforms;
			assert(uniforms->size() <= ITexture::MAX_TEXTURE_COUNT);

			TempVector<TextureStage*> reorder;
			TempVector<TextureStage*> backup;
			backup.insert(backup.end(), mStages.begin(), mStages.end());

			reorder.reserve(uniforms->size());
			for (size_t i = 0; i < uniforms->size(); ++i)
			{
				const TString& uniform = uniforms->at(i);
				index_t index = this->getStageIndex(uniform);

				TextureStage* stage;
				if (index == INVALID_INDEX)
					stage = BLADE_NEW TextureStage(uniforms->at(i));
				else
				{
					backup[index] = NULL;
					stage = mStages[index];
				}
				reorder.push_back(stage);
			}

			for (size_t i = 0; i < backup.size(); ++i)
			{
				if (backup[i] != NULL)	//not used - not in shader uniforms, append to tail
				{
					TextureStage* stage = mStages[i];
					BLADE_LOG(Warning, BTString("texture '") << stage->getSamplerUniform() << 
						BTString("' not used by material. make sure the stage name matches declarations in shader."));
					reorder.push_back(stage);
				}
			}

			mStages.clear();
			assert(reorder.size() <= mStages.capacity()); //need drop unused stage
			for (size_t i = 0; i < reorder.size(); ++i)
			{
				if (mStages.size() < mStages.capacity())
					mStages.push_back(reorder[i]);
				else
					BLADE_DELETE reorder[i];
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			TextureState::resetToDefault()
	{
		mStages.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	static HTEXTURESTATE mshTextureState;
	const HTEXTURESTATE&	TextureState::getDefaultTextureState()
	{
		if( mshTextureState == NULL )
		{
			mshTextureState.lock();
			mshTextureState.bind( BLADE_NEW TextureState() );
			Lock::memoryBarrier();
			mshTextureState.unlock();
		}

		return mshTextureState;
	}
	
}//namespace Blade