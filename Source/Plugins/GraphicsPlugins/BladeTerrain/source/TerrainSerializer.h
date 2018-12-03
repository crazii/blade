/********************************************************************
	created:	2012/09/27
	filename: 	TerrainSerializer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainSerializer_h__
#define __Blade_TerrainSerializer_h__
#include <BladeTerrain.h>
#include <interface/public/ISerializer.h>
#include <utility/StringList.h>
#include <utility/IOBuffer.h>
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IImage.h>
#include <math/Vector3.h>
#include "TerrainTextureManager.h"

namespace Blade
{
	class IElement;

	class TerrainSerializer : public ISerializer, public TempAllocatable
	{
	public:
		TerrainSerializer();
		~TerrainSerializer();

		/************************************************************************/
		/* ISerializer interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* res,const HSTREAM& stream, const ParamList& params);

		/*
		@describe process resource.like preLoadResource, this will be called in main thread.\n
		i.e.TextureResource need to be loaded into graphics card.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveResource(const IResource* res, const HSTREAM& stream);

		/*
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* res,ParamList& params);

		/*
		@describe this method is called when resource is reloaded,
		the serializer hold responsibility to cache the loaded data for resource,
		then in main thread ISerializer::reprocessResource() is called to fill the existing resource with new data.\n
		this mechanism is used for reloading existing resource for multi-thread,
		the existing resource is updated in main thread(synchronizing state),
		to ensure that the data is changed only when it is not used in another thread.
		@param
		@return
		*/
		virtual bool	reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params);

		/*
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource);

	protected:

		/** @brief internal method for load/reload */
		/* @note when last 4 params are NULL, then to perform reloading  */
		bool	loadData(const HSTREAM& stream, TString& name, scalar& base, size_t& size,
			int16** height = NULL, uint8** normal = NULL, Color::RGBA** blend = NULL, TERRAIN_LAYER** layer = NULL);

		/** @brief  */
		void	copyAABBList(IResource* resource);
		
		TString				mName;
		scalar				mHeight;	//base height
		size_t				mSize;
		bool				mReload;

		TERRAIN_LAYER*		mPreConvertedLayer;
		TStringParam		mTextureCache;
		TStringParam		mNormalTextureCahe;

		HIMAGE				mBlendImage;
		HIMAGE				mNormalImage;

		HVBUFFER			mVertexPositionBuffer;
		HVBUFFER			mVertexNormalBuffer;	//valid if using vertex normal
		TileAABBList		mAABBList;

		//these are all for reloading cache.
		IOBuffer			mReloadingHeightCache;
		IOBuffer			mReloadingNormalCache;
		IOBuffer			mReloadingBlendCache;
		IOBuffer			mReloadingLayerCache;
		IOBuffer			mReloadingPartitionCache;

		typedef TempVector<HIMAGE>	TerrainTextureList;
		TerrainTextureList	mTextures;
		TerrainTextureList	mNormalTextures;
		bool				mLoadNormalOnly;
	};//TerrainSerializer
	
}//namespace Blade


#endif //  __Blade_TerrainSerializer_h__