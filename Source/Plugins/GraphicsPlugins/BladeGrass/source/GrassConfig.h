/********************************************************************
created:	2017/11/22
filename: 	GrassConfig.h
author:		Crazii
purpose:	
*********************************************************************/
#ifndef __Blade_GrassConfig_h__
#define __Blade_GrassConfig_h__
#include "GrassRenderType.h"
#include <utility/Bindable.h>
#include <utility/BladeContainer.h>
#include <BladeGrass.h>
#include <interface/IGrassConfig.h>
#include <math/Half.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <MaterialInstance.h>
#include <GrassShaderShared.inl>
#include <interface/IModelResource.h>

namespace Blade 
{
	enum EGrassVertexStream
	{
		GVS_POSITION = 0,		//normalized vertex position,  in mesh bounding box space
		GVS_UV,					//texture coordinate (optional)
		GVS_NORMAL,				//vertex normal
		GVS_INSTANCING,			//instancing data

		GVS_COUNT,
		GVS_BEGIN = 0,
	};

	class IRenderScene;
	class GrassCluster;

	class GrassConfig : public Bindable, public IGrassConfig, public Singleton<GrassConfig>
	{
	public:
		using Singleton<GrassConfig>::getSingleton;
		using Singleton<GrassConfig>::getSingletonPtr;
	public:
		static const EVertexElementType GRASS_POSITION_FMT = IModelResource::POSITION_FMT;
		static const EVertexElementType GRASS_TEXCOORD_FMT = IModelResource::UV_FMT;
		static const EVertexElementType GRASS_NORMAL_FMT = VET_UBYTE4N;

		typedef IModelResource::POSITION POSITION;
		typedef Vector4::PACKED INSTANCING_POSITION;
		typedef Vector4::PACKED NORMAL;
		struct TEXCOORD
		{
			fp16 u, v;
		};

		static const index_t MAX_LAYERS = 4;
		static const size_t MIN_INSTANCING_UNIT_SIZE = 4;	//4m x 4m
		static const size_t MAX_INSTANCING_UNIT_SIZE = 16;
		//max density per 1m x 1m area
		static const int MAX_DENSITY = 15;
		static const int DENSITY_BITS = 4;
		static const int DENSITY_MASK = 0xF;
	public:
		typedef TPool<GrassCluster*, TPoolDefAlloc<GrassCluster*> > ClusterPool;
		typedef TPool<HVBUFFER>	InstanceBufferPool;

		GrassConfig();
		~GrassConfig();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual IGraphicsType* getGraphicsType() const { return &this->getRenderType(); }

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		
		/** @brief  */
		void initialize();

		/** @brief  */
		void shutdown();

		/** @brief  */
		inline GrassRenderType& getRenderType() const { return mRenderType; }

		/** @brief  */
		inline const HVDECL&	getVertexDeclaration() const { return mVertexDecl; }

		/** @brief  */
		inline ClusterPool&		getClusterPool() { return mClusterPool; }

		/** @brief  */
		inline InstanceBufferPool& getInstanceBufferPool() { return mBufferPool; }

		/** @brief  */
		inline const HMATERIALINSTANCE& getLayerMaterial(index_t layer) const
		{
			assert(layer < MAX_LAYERS);
			return mLayeredMaterial[layer];
		}

		/** @brief get instancing block size in world units (meters), area size is squared. */
		inline size_t getInstancingSize() const { return mInstancingSize; }

		/** @brief partition array size (2 dim' side). real array size is squared */
		inline size_t getInstancingPartitionSize() { return mLastSpaceSize / mInstancingSize; }

		/** @brief  */
		inline uint32* getInstancingPartition() const { return mInstancingPartition; }

		/** @brief  */
		void buildInstancingPartitions(IRenderScene* scene);

		/** @brief get the target types that grass grow on. */
		//TODO: add user specified type flag for entity, so that grass can be configured on a per-entity basis
		inline AppFlag getGrassPlantingFlag() const;

	protected:
		/** @brief  */
		const uint64&		getTargetFlag(index_t) const { return mTargetFlag; }
		bool				setTargetFlag (index_t, const uint64& flag) { mTargetFlag = flag; return true;; }
		void				onConfigChange(void*);
		friend class BladeGrassPlugin;

		mutable GrassRenderType mRenderType;
		HVDECL			mVertexDecl;
		HMATERIALINSTANCE mLayeredMaterial[MAX_LAYERS];
		ClusterPool		mClusterPool;
		InstanceBufferPool mBufferPool;
		uint32*			mInstancingPartition;
		size_t			mLastSpaceSize;
		TString			mLastSpaceType;
		size_t			mInstancingSize;	//minimal instancing block in world units, ranges [MIN_INSTANCING_UNIT_SIZE, MAX_INSTANCING_UNIT_SIZE]
		uint64			mTargetFlag;		//use 64 bit for max serialization compatibility
	};
	
}//namespace Blade 

#endif // __Blade_GrassConfig_h__