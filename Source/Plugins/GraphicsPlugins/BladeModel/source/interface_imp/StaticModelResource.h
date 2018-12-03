/********************************************************************
	created:	2017/12/08
	filename: 	StaticModelResource.h
	author:		Crazii
	purpose:	model for static model, pre-transformed vertices (for batch combination)
*********************************************************************/
#ifndef __Blade_StaticModelResource_h__
#define __Blade_StaticModelResource_h__
#include <GraphicsResource.h>
#include <interface/IResourceManager.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/GraphicsGeometry.h>
#include "ModelResource.h"

namespace Blade
{
	class StaticModelResource : public IModelResourceImpl
	{
	protected:
		typedef List<IModelResource::MESHDATA> PreXFormedSubMeshList;
	public:
		static const TString STATIC_MODEL_RESOURCE_TYPE;
		static const TString PARAM_ORIGINAL_MODEL;
		static const TString PARAM_PRE_TRANSFORM;
	public:
		StaticModelResource()
		{
			this->setUserFlag(UF_PHONY);
		}

		~StaticModelResource()
		{
			mSubMeshes.clear();
			IResourceManager& resMan = IResourceManager::getSingleton();
			resMan.unloadResource(mModel);
		}

		/************************************************************************/
		/* IResource interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getType() const
		{
			return STATIC_MODEL_RESOURCE_TYPE;
		}
		/************************************************************************/
		/* IModelResource interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void		setNormalType(ENormalType /*type*/) { assert(false); }

		/*
		@describe
		@param
		@return
		*/
		virtual ENormalType	getNormalType() const { return static_cast<const ModelResource*>(mModel)->getNormalType(); }

		/*
		@describe
		@param
		@return
		*/
		virtual size_t		getSubMeshCount() const
		{
			assert(mSubMeshes.size() == static_cast<const ModelResource*>(mModel)->getSubMeshCount());
			return mSubMeshes.size();
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const MESHDATA&	getSubMeshData(index_t index) const
		{
			if (index < mSubMeshes.size())
			{
				PreXFormedSubMeshList::const_iterator i = mSubMeshes.begin();
				std::advance(i, index);
				return *i;
			}
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("subscript out of range."));
		}

		/**
		@describe
		@param
		@return
		*/
		virtual index_t		findSubMeshData(const TString& name) const
		{
			return static_cast<const ModelResource*>(mModel)->findSubMeshData(name);
		}

		/*
		@describe
		@param
		@return the index of new added sub mesh
		*/
		virtual size_t		addSubMesh(const MESHDATA& /*meshData*/)
		{
			assert(false);
			return INVALID_INDEX;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual index_t		getMaterialCount() const
		{
			return static_cast<const ModelResource*>(mModel)->getMaterialCount();
		}

		/*
		@describe
		@param
		@return
		*/
		virtual index_t		addMaterial(const MATERIAL_INFO& /*material*/)
		{
			assert(false);
			return 0;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual bool		setSkeletonRefernce(const TString& /*skeletonName*/, bool /*useFullPath = false*/)
		{
			assert(false);
			return false;
		}

		/************************************************************************/
		/*  run-time function (NOT available on exporting/importing)                                                                    */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual bool		setSkeleton(const HRESOURCE& /*skeletonResource*/)
		{
			assert(false);
			return false;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const MATERIAL&		getMaterial(index_t index) const
		{
			return static_cast<const ModelResource*>(mModel)->getMaterial(index);
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const AABB&	getModelAABB() const
		{
			return static_cast<const ModelResource*>(mModel)->getModelAABB();
		}

		/************************************************************************/
		/* IModelResourceImpl interface                                                                     */
		/************************************************************************/
		/** @brief  */
		inline virtual SkeletonResource* getSkeletonResource() const
		{
			return static_cast<const ModelResource*>(mModel)->getSkeletonResource();
		}

		/** @brief  */
		inline virtual const HRESOURCE&	getSkeleton() const
		{
			return static_cast<const ModelResource*>(mModel)->getSkeleton();
		}

		/** @brief  */
		inline virtual const TString&	getSkeletonFile() const
		{
			return static_cast<const ModelResource*>(mModel)->getSkeletonFile();
		}

		/** @brief  */
		inline virtual const Handle<MESHDATA>&	getSharedOpaqueMesh() const
		{
			return static_cast<const ModelResource*>(mModel)->getSharedOpaqueMesh();
		}
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline const HRESOURCE&	getResource() const
		{
			return mModel;
		}

	protected:
		HRESOURCE				mModel;
		PreXFormedSubMeshList	mSubMeshes;
		friend class StaticModelSerializer;
	};
	
}//namespace Blade
#endif//__Blade_StaticModelResource_h__