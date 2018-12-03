/********************************************************************
	created:	2011/08/31
	filename: 	GlobalLightShaderVariableUpdaters.h
	author:		Crazii
	purpose:	light variables for forward shading
*********************************************************************/
#ifndef __Blade_GlobalLightShaderVariableUpdaters_h__
#define __Blade_GlobalLightShaderVariableUpdaters_h__
#include "ShaderVariableUpdaters.h"
#include <GraphicsShaderShared.inl>
#include <interface/ILightManager.h>
#include <interface/ILight.h>
#include <math/Vector4.h>

namespace Blade
{
	class LightVariableUpdater : public GlobalVariableUpdater
	{
	public:
		LightVariableUpdater(SHADER_CONSTANT_TYPE type,size_t count)
			:GlobalVariableUpdater(type,count)		{}

	protected:
		/** @brief  */
		ILightManager*	getLightManager() const
		{
			return IShaderVariableSource::getSingleton().getLightManager();
		}
	};

	class GlobalAmbientVariableUpdater : public LightVariableUpdater
	{
	public:
		GlobalAmbientVariableUpdater()
			:LightVariableUpdater(SCT_FLOAT4,1)	{}

		/** @brief  */
		virtual const void*	updateData() const
		{
			return &( this->getLightManager()->getGlobalAmbient() );
		}
	};


	//////////////////////////////////////////////////////////////////////////
	class ForwardGlobalLightCountVariableUpdater : public LightVariableUpdater
	{
	public:
		ForwardGlobalLightCountVariableUpdater()
			:LightVariableUpdater(SCT_FLOAT4,1)	{}

		/** @brief  */
		virtual const void*	updateData() const
		{
			mLightCount[0] = (float)std::min<int>( (int)this->getLightManager()->getGlobalLightCount(), MAX_GLOBAL_LIGHT_COUNT);
			return mLightCount;
		}

		mutable float	mLightCount[4];
	};

	//////////////////////////////////////////////////////////////////////////
	//Light vector could be positions for point lights, or directions for directional lights
	class ForwardGlobalLightVectorVariableUpdater : public LightVariableUpdater
	{
	public:
		ForwardGlobalLightVectorVariableUpdater()
			:LightVariableUpdater(SCT_FLOAT4,MAX_GLOBAL_LIGHT_COUNT)	{}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const ILight* const* lights = this->getLightManager()->getGlobalLightList();
			size_t MaxCount = std::min<size_t>( this->getLightManager()->getGlobalLightCount(), MAX_GLOBAL_LIGHT_COUNT);
			for(size_t i = 0; i < MaxCount; ++i )
			{
				assert(lights[i]->getType() == LT_DIRECTIONAL);
				mVectors[i] = Vector4(-lights[i]->getDirection(), 0);
			}

			return mVectors;
		}

		mutable Vector4	mVectors[MAX_GLOBAL_LIGHT_COUNT];
	};

	//////////////////////////////////////////////////////////////////////////
	class ForwardGlobalLightDiffuseVariableUpdater : public LightVariableUpdater
	{
	public:

		ForwardGlobalLightDiffuseVariableUpdater()
			:LightVariableUpdater(SCT_FLOAT4, MAX_GLOBAL_LIGHT_COUNT)	{}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const ILight* const* lights = this->getLightManager()->getGlobalLightList();
			size_t MaxCount = std::min<size_t>( this->getLightManager()->getGlobalLightCount(), MAX_GLOBAL_LIGHT_COUNT );

			for(size_t i = 0; i < MaxCount; ++i )
			{
				mDiffuse[i] = lights[i]->getDiffuse();
			}
			return mDiffuse;
		}
		mutable Color	mDiffuse[MAX_GLOBAL_LIGHT_COUNT];
	};


	//////////////////////////////////////////////////////////////////////////
	class ForwardGlobalLightSpecularVariableUpdater : public LightVariableUpdater
	{
	public:

		ForwardGlobalLightSpecularVariableUpdater()
			:LightVariableUpdater(SCT_FLOAT4,MAX_GLOBAL_LIGHT_COUNT)	{}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const ILight* const* lights = this->getLightManager()->getGlobalLightList();
			size_t MaxCount = std::min<size_t>( this->getLightManager()->getGlobalLightCount(),MAX_GLOBAL_LIGHT_COUNT );

			for(size_t i = 0; i < MaxCount; ++i )
			{
				mSpecular[i] = lights[i]->getSpecular();
			}
			return mSpecular;
		}
		mutable Color	mSpecular[MAX_GLOBAL_LIGHT_COUNT];
	};

	//////////////////////////////////////////////////////////////////////////
	class DeferredDirectionalLightCountUpdater : public GlobalVariableUpdater
	{
	public:
		DeferredDirectionalLightCountUpdater()
			:GlobalVariableUpdater(SCT_FLOAT4, 1)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			ILightManager* manager = IShaderVariableSource::getSingleton().getLightManager();
			mLightCount[0] = 0;

			if( manager == NULL )
				return mLightCount;

			ILight** lights = manager->getGlobalLightList();
			size_t count = manager->getGlobalLightCount();

			size_t validCount = 0;
			for(size_t i = 0; i < count; ++i)
			{
				if( validCount >= BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT)
					break;

				assert(lights[i]->getType() == LT_DIRECTIONAL);
				mLightCache[ validCount++ ] = lights[i];
			}

			mLightCount[0] = float(validCount);
			return mLightCount;
		}

		/** @brief  */
		inline const ILight* const* getDirectionalLights() const
		{
			return &mLightCache[0];
		}
	protected:
		mutable float mLightCount[4];
		mutable ILight* mLightCache[BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT];
	};
	
	//////////////////////////////////////////////////////////////////////////
	class DeferredDirectionalLightDirectionUpdater : public GlobalVariableUpdater
	{
	public:
		DeferredDirectionalLightDirectionUpdater(DeferredDirectionalLightCountUpdater* updater)
			:GlobalVariableUpdater(SCT_FLOAT4, BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT)
			,mLightCountUpdater(updater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			ILightManager* manager = IShaderVariableSource::getSingleton().getLightManager();
			if( manager == NULL )
				return mDirections;
			int count = (int)*(float*)mLightCountUpdater->getVariable()->getData();
			const ILight* const* lights = mLightCountUpdater->getDirectionalLights();
			for(int i = 0; i < count; ++i)
				mDirections[i] = -lights[i]->getDirection();
			mVariable.setRealSize( (uint32)(count* mVariable.getSize()) );
			return mDirections;
		}
	protected:
		DeferredDirectionalLightCountUpdater* mLightCountUpdater;
		mutable Vector4 mDirections[BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT];
	};

	//////////////////////////////////////////////////////////////////////////
	class DeferredDirectionalLightDiffuseUpdater : public GlobalVariableUpdater
	{
	public:
		DeferredDirectionalLightDiffuseUpdater(DeferredDirectionalLightCountUpdater* updater)
			:GlobalVariableUpdater(SCT_FLOAT4, BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT)
			,mLightCountUpdater(updater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			ILightManager* manager = IShaderVariableSource::getSingleton().getLightManager();
			if( manager == NULL )
				return mDiffuses;

			int count = (int)*(float*)mLightCountUpdater->getVariable()->getData();
			const ILight* const* lights = mLightCountUpdater->getDirectionalLights();
			for(int i = 0; i < count; ++i)
				mDiffuses[i] = lights[i]->getDiffuse();
			mVariable.setRealSize((uint32)(count* mVariable.getSize()) );
			return mDiffuses;
		}
	protected:
		DeferredDirectionalLightCountUpdater* mLightCountUpdater;
		mutable Color mDiffuses[BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT];
	};

	//////////////////////////////////////////////////////////////////////////
	class DeferredDirectionalLightSpecularUpdater : public GlobalVariableUpdater
	{
	public:
		DeferredDirectionalLightSpecularUpdater(DeferredDirectionalLightCountUpdater* updater)
			:GlobalVariableUpdater(SCT_FLOAT4, BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT)
			,mLightCountUpdater(updater)
		{

		}

		/** @brief  */
		virtual const void* updateData() const
		{
			ILightManager* manager = IShaderVariableSource::getSingleton().getLightManager();
			if( manager == NULL )
				return mSpeculars;

			int count = (int)*(float*)mLightCountUpdater->getVariable()->getData();
			const ILight* const* lights = mLightCountUpdater->getDirectionalLights();
			for(int i = 0; i < count; ++i)
				mSpeculars[i] = lights[i]->getSpecular();
			mVariable.setRealSize((uint32)(count * mVariable.getSize()) );
			return mSpeculars;
		}
	protected:
		DeferredDirectionalLightCountUpdater* mLightCountUpdater;
		mutable Color mSpeculars[BLADE_MAX_DEFFERD_DIRECTIONAL_LIGHT_COUNT];
	};

}//namespace Blade



#endif // __Blade_GlobalLightShaderVariableUpdaters_h__