/********************************************************************
	created:	2010/09/05
	filename: 	ShaderVariableUpdaters.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderVariableUpdaters_h__
#define __Blade_ShaderVariableUpdaters_h__
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <AutoShaderVariable.h>
#include <interface/IRenderQueue.h>
#include "../../ShaderConstantTypeBind.h"
#include <interface/IShaderVariableSource.h>

namespace Blade
{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4355)
#endif

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	class GlobalVariableUpdater : public AutoShaderVariable::IUpdater
	{
	public:
		GlobalVariableUpdater(SHADER_CONSTANT_TYPE type,size_t count)
			:mVariable(this,type,count,SVU_GLOBAL)
		{
			//array type of float1,float2,float3 (int) need special packing for some API (i.e. D3D use float4[2] for float[2], GL/GLES doesn't )
			//and need dynamic allocation on runtime, since target API can only be detected at runtime for now.
			//use of float4/int4 arrays for built-in constants is recommended, 
			//or use ShaderConstantArrayUpdater
			assert(!(type.getColCount() < 4 && count > 1));
		}

		/** @brief  */
		inline AutoShaderVariable*	getVariable()
		{
			return &mVariable;
		}

	protected:
		mutable AutoShaderVariable	mVariable;
	};


	//////////////////////////////////////////////////////////////////////////
	class InstanceVariableUpdater : public AutoShaderVariable::IUpdater
	{
	public:
		InstanceVariableUpdater(SHADER_CONSTANT_TYPE type,size_t count)
			:mVariable(this,type,count,SVU_INSTANCE)
			,mPrevData(NULL)
			,mDynamicShader(false)
		{
			assert(!(type.getColCount() < 4 && count > 1));
		}

		InstanceVariableUpdater(const InstanceVariableUpdater& src)
			:mVariable(this, src.mVariable)
		{

		}

		/** @brief  */
		inline AutoShaderVariable*	getVariable()
		{
			return &mVariable;
		}

		/** @brief  */
		inline void		onShaderSwitch(bool dynamicSwithing)
		{
			mDynamicShader = dynamicSwithing;
			mPrevData = NULL;
		}

		/** @brief  */
		inline bool needCommitData() const
		{
			const void* curData = msRootInstanceUpdater->updateData();
			const void* prevData = mPrevData;
			mPrevData = curData;
			return  mDynamicShader || prevData != curData;
		}

		/** @brief  */
		inline void setRenderOperation(const RenderOperation* renderable)
		{
			mROP = renderable;
			mVariable.markDirty();
		}

	protected:

		/** @brief  */
		inline const RenderOperation*	getRenderOperation() const
		{
			return mROP;
		}

		/** @brief  */
		inline const IRenderable* getRenderable() const
		{
			return mROP->renderable;
		}

		/** @brief  */
		inline const MaterialInstance* getMaterial() const
		{
			return mROP->material;
		}

		mutable AutoShaderVariable		mVariable;
		mutable const void*				mPrevData;
		const RenderOperation*			mROP;
		bool							mDynamicShader;
		static	InstanceVariableUpdater*msRootInstanceUpdater;
	};

	//////////////////////////////////////////////////////////////////////////
	class PropertyVariableUpdater : public AutoShaderVariable::IUpdater
	{
	public:
		PropertyVariableUpdater(SHADER_CONSTANT_TYPE type,size_t count)
			:mVariable(this,type,count,SVU_INSTANCE)
			,mPrevProperty(NULL)
		{
			assert(!(type.getColCount() < 4 && count > 1));
		}

		PropertyVariableUpdater(const PropertyVariableUpdater& src)
			:mVariable(this,src.mVariable)
			,mPrevProperty(NULL)
			,mPrevAlpha(NULL)
			,mPrevColor(NULL)
		{

		}

		~PropertyVariableUpdater()
		{

		}

		/** @brief  */
		virtual bool	needCommit() const
		{
			if( mDynamicShader )
				return true;

			bool ret = false;
			const RenderPropertySet* currentPropery = this->getPropertySet();
			if (mPrevProperty != currentPropery ||
				(currentPropery != NULL && (mPrevColor != currentPropery->getProperty(RPT_COLOR)
				|| mPrevAlpha != currentPropery->getProperty(RPT_ALPHABLEND))))
			{
				ret = true;
			}
			mPrevProperty = currentPropery;
			mPrevAlpha = currentPropery != NULL ? currentPropery->getProperty(RPT_COLOR) : HRENDERPROPERTY::EMPTY;
			mPrevColor = currentPropery != NULL ? currentPropery->getProperty(RPT_ALPHABLEND) : HRENDERPROPERTY::EMPTY;
			return ret;
		}

		/** @brief  */
		inline AutoShaderVariable*	getVariable()
		{
			return &mVariable;
		}

		/** @brief  */
		inline void		onShaderSwitch(bool dynamicSwithing)
		{
			mDynamicShader = dynamicSwithing;
			mPrevProperty = NULL;
			mPrevAlpha = NULL;
			mPrevColor = NULL;
		}

	protected:

		/** @brief  */
		inline const RenderPropertySet*	getPropertySet(bool except = false) const
		{
			const RenderPropertySet* propertySet = IShaderVariableSource::getSingleton().getRenderProperty();
			if( !except || propertySet != NULL)
				return propertySet;
			else
				BLADE_EXCEPT(EXC_NEXIST,BTString("built-in pass shader variable data's source not exist."));
		}
		AutoShaderVariable				mVariable;
		mutable const RenderPropertySet*mPrevProperty;
		mutable	IRenderProperty*		mPrevAlpha;
		mutable IRenderProperty*		mPrevColor;
		bool							mDynamicShader;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	class CameraVariableUpdater : public GlobalVariableUpdater
	{
	public:
		CameraVariableUpdater(SHADER_CONSTANT_TYPE type,size_t count)
			:GlobalVariableUpdater(type,count)
		{

		}
	protected:
		/** @brief  */
		inline const ICamera* getCamera() const
		{
			return IShaderVariableSource::getSingleton().getCamera();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class ViewVariableUpdater : public GlobalVariableUpdater
	{
	public:
		ViewVariableUpdater(SHADER_CONSTANT_TYPE type,size_t count)
			:GlobalVariableUpdater(type,count)
		{

		}

	protected:
		/** @brief  */
		inline const IRenderView* getView() const
		{
			return IShaderVariableSource::getSingleton().getView();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class ScreenSpaceUVFlipUpdater : public GlobalVariableUpdater
	{
	public:
		ScreenSpaceUVFlipUpdater()
			:GlobalVariableUpdater(SCT_FLOAT4, 1)
		{
			//dirty once
			mVariable.markDirty();
		}

		/** @brief  */
		virtual const void* updateData() const
		{
			scalar flip = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir == IMGO_TOP_DOWN ? -1.0f : 1.0f;
			mUVFlip[0] = flip;
			return &mUVFlip;
		}
	protected:
		mutable Vector4	mUVFlip;
	};

	//////////////////////////////////////////////////////////////////////////
	class ShaderConstantArrayUpdater : public AutoShaderVariable::IUpdater
	{
	public:
		ShaderConstantArrayUpdater(SHADER_CONSTANT_TYPE type, size_t count, EShaderVariableUsage usage)
			:mVariable(this, type, count, usage)
		{
			assert(type.getColCount() < 4 && count > 1);
		}

	protected:
		/** @brief  */
		template<typename T>
		T* getStorage() const
		{
			if (mStorage.getBuffer() == NULL)
			{
				mStorage.alloc(mVariable.getType(), mVariable.getCount());
				size_t size = IGraphicsResourceManager::getSingleton().getShaderConstantSize(mVariable.getType());
				mVariable.updateSizeInfo(size);
			}
			return (T*)mStorage.getBuffer();
		}

		mutable Impl::ShaderVariableData	mStorage;
		mutable AutoShaderVariable mVariable;
	};

	//this pointer as param in member class construction
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
}//namespace Blade


#endif //__Blade_ShaderVariableUpdaters_h__