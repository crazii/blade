/********************************************************************
	created:	2017/01/30
	filename: 	OutputShaderVariableUpdaters.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_OutputShaderVariableUpdaters_h__
#define __Blade_OutputShaderVariableUpdaters_h__
#include "ShaderVariableUpdaters.h"
#include "../RenderScheme/RenderOutput.h"
#include "../RenderScheme/ImageEffectOutput.h"
#include "ShaderVariableSource.h"
#include "../../TextureShaderHelper.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	class RenderBufferUpdater : public GlobalVariableUpdater
	{
	public:
		RenderBufferUpdater()
			:GlobalVariableUpdater(SCT_SAMPLER, 1)
			, mIndex(INVALID_INDEX)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const RenderOutput* output = ShaderVariableSource::getSingleton().getRenderOutput();
			if (output == NULL)
				return NULL;
			else
				return &output->getInputBuffer(mIndex);
		}

		/** @brief  */
		inline void	setIndex(index_t index)
		{
			mIndex = index;
		}
	protected:
		size_t				mIndex;
	};

	//////////////////////////////////////////////////////////////////////////
	class RenderBufferUVSPaceUpdater : public ViewVariableUpdater
	{
	public:
		RenderBufferUVSPaceUpdater()
			:ViewVariableUpdater(SCT_FLOAT4, 4)
			, mIndex(INVALID_INDEX)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const RenderOutput* output = ShaderVariableSource::getSingleton().getRenderOutput();
			if (output == NULL)
				return &Vector4::UNIT_ALL;
			else
			{
				const HTEXTURE& buffer = output->getInputBuffer(mIndex);
				return Impl::updateUVSpace(mUVSpace, buffer);
			}
		}

		/** @brief  */
		inline void	setIndex(index_t index)
		{
			mIndex = index;
		}
	protected:
		size_t				mIndex;
		mutable Vector4		mUVSpace[Impl::UVSPACE_DATA_COUNT];
	};

	//////////////////////////////////////////////////////////////////////////
	class RenderBufferSizeUpdater : public GlobalVariableUpdater
	{
	public:
		RenderBufferSizeUpdater()
			:GlobalVariableUpdater(SCT_FLOAT4, 1)
			, mIndex(INVALID_INDEX)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const RenderOutput* output = ShaderVariableSource::getSingleton().getRenderOutput();
			if (output == NULL)
				return &Vector4::UNIT_ALL;
			else
			{
				return Impl::updateTextureSize(mTextureSize, output->getInputBuffer(mIndex));
			}
		}

		/** @brief  */
		inline void	setIndex(index_t index)
		{
			mIndex = index;
		}
	protected:
		size_t				mIndex;
		mutable Vector4		mTextureSize;
	};

	//////////////////////////////////////////////////////////////////////////
	class RenderTargetSizeUpdater : public GlobalVariableUpdater
	{
	public:
		RenderTargetSizeUpdater()
			:GlobalVariableUpdater(SCT_FLOAT4, 1)
		{

		}
		/** @brief  */
		virtual const void* updateData() const
		{
			const RenderOutput* output = ShaderVariableSource::getSingleton().getRenderOutput();
			if (output == NULL || output->getTarget() == NULL || output->getTarget()->getColorBuffer(0) == NULL)
				return &Vector4::UNIT_ALL;

			return Impl::updateTextureSize(mSize, output->getTarget()->getColorBuffer(0));
		}
	protected:
		mutable Vector4	mSize;
	};

	//////////////////////////////////////////////////////////////////////////
	class PostFXInputSizeUpdater : public GlobalVariableUpdater
	{
	public:
		PostFXInputSizeUpdater()
			:GlobalVariableUpdater(SCT_FLOAT4, 1)
		{
		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const ImageEffectOutput* output = static_cast<const ImageEffectOutput*>(ShaderVariableSource::getSingleton().getImageEffectOutput());
			if (output == NULL)
				return &Vector4::UNIT_ALL;
			else
			{
				return Impl::updateTextureSize(mTextureSize, output->getIFXInput());
			}
		}
	protected:
		mutable Vector4		mTextureSize;
	};

	//////////////////////////////////////////////////////////////////////////
	class PostFXInputUpdater : public GlobalVariableUpdater
	{
	public:
		PostFXInputUpdater()
			:GlobalVariableUpdater(SCT_SAMPLER, 1)
		{
		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const ImageEffectOutput* output = static_cast<const ImageEffectOutput*>(ShaderVariableSource::getSingleton().getImageEffectOutput());
			if (output == NULL)
				return &Vector4::UNIT_ALL;
			else
			{
				return &output->getIFXInput();
			}
		}
	};


	//////////////////////////////////////////////////////////////////////////
	class PostFXInputUVSPaceUpdater : public GlobalVariableUpdater
	{
	public:
		PostFXInputUVSPaceUpdater()
			:GlobalVariableUpdater(SCT_FLOAT4, 4)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			const ImageEffectOutput* output = static_cast<const ImageEffectOutput*>(ShaderVariableSource::getSingleton().getImageEffectOutput());
			if (output == NULL)
				return &Vector4::UNIT_ALL;
			else
			{
				const HTEXTURE& buffer = output->getIFXInput();
				return Impl::updateUVSpace(mUVSpace, buffer);
			}
		}
	protected:
		mutable Vector4		mUVSpace[Impl::UVSPACE_DATA_COUNT];
	};
	
}//namespace Blade


#endif // __Blade_OutputShaderVariableUpdaters_h__