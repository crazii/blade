/********************************************************************
	created:	2013/04/03
	filename: 	ModelResource.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelResource_h__
#define __Blade_ModelResource_h__
#include <interface/IModelResource.h>
#include <TextureResource.h>

#include <utility/BladeContainer.h>
#include <math/AxisAlignedBox.h>
#include "SkeletonResourceState.h"

namespace Blade
{
	class IModelResourceImpl : public IModelResource
	{
	public:
		/** @brief  */
		inline virtual const ITask::Type&	getProcessTaskType() const
		{
			return GraphicsResource::getGraphicsTaskType();
		}

		/** @brief  */
		virtual SkeletonResource* getSkeletonResource() const = 0;

		/** @brief  */
		virtual const HRESOURCE&	getSkeleton() const = 0;

		/** @brief  */
		virtual const TString&	getSkeletonFile() const = 0;

		/** @brief  */
		virtual const Handle<MESHDATA>& getSharedOpaqueMesh() const = 0;

	};//class IModelResourceImpl


	class ModelResource : public IModelResourceImpl, public Allocatable
	{
	public:
		ModelResource();
		~ModelResource();
		/************************************************************************/
		/* IResource interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const;

		/************************************************************************/
		/* IModelResource interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		setNormalType(ENormalType type)		{mNormalType = type;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ENormalType	getNormalType() const				{return mNormalType;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getSubMeshCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const MESHDATA&	getSubMeshData(index_t index) const;

		/**
		@describe 
		@param
		@return
		*/
		virtual index_t		findSubMeshData(const TString& name) const;

		/*
		@describe 
		@param 
		@return the index of new added sub mesh
		*/
		virtual size_t		addSubMesh(const MESHDATA& meshData);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual index_t		getMaterialCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual index_t		addMaterial(const MATERIAL_INFO& material);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		setSkeletonRefernce(const TString& skeletonName, bool useFullPath = false);

		/************************************************************************/
		/*  run-time function (NOT available on exporting/importing)                                                                    */
		/************************************************************************/
		/*
		@describe 
		@param
		@return
		*/
		virtual bool		setSkeleton(const HRESOURCE& skeletonResource);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const MATERIAL&		getMaterial(index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const AABB&	getModelAABB() const;

		/************************************************************************/
		/* IModelResourceImpl interface                                                                     */
		/************************************************************************/
		/** @brief  */
		inline virtual SkeletonResource* getSkeletonResource() const
		{
			return mSkeletonResource.getSkeletonResource();
		}

		/** @brief  */
		inline virtual const HRESOURCE&	getSkeleton() const
		{
			return mSkeletonResource.getResource();
		}

		/** @brief  */
		inline virtual const TString&	getSkeletonFile() const
		{
			return /*this->getSkeletonResource() != NULL ? this->getSkeletonResource()->getSource() : */mSkeletonResource.getResourcePath();
		}

		/** @brief  */
		inline virtual const Handle<MESHDATA>& getSharedOpaqueMesh() const { return mSharedOpaque; }

	protected:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		void			setupMaterial();
		/**
		@describe 
		@param
		@return
		*/
		bool			checkHasBones() const;

		typedef List< MESHDATA > SubMeshList;
		typedef List<MATERIAL_INFO> MaterialInfo;
		typedef List< MATERIAL > MaterialList;

		StringTable		mMeshNames;
		MaterialInfo	mMaterialInfo;
		SubMeshList		mSubMesh;

		MaterialList	mMaterials;
		AABB			mAAB;	//local AABB
		scalar			mRadius;
		SkeletonResourceState mSkeletonResource;
		Handle<MESHDATA>	mSharedOpaque;
		Handle<MESHDATA>	mSharedAlpha;
		ENormalType			mNormalType;

		friend class ModelSerializer;
		friend class ModelSerializer_Binary;
		friend class ModelSerializer_Xml;
	};//class ModelResource

	
}//namespace Blade

#endif//__Blade_ModelResource_h__