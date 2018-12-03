/********************************************************************
	created:	2010/05/04
	filename: 	TerrainElement.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainElement_h__
#define __Blade_TerrainElement_h__
#include <ResourceState.h>
#include <Element/GraphicsElement.h>
#include <interface/ITerrainInterface.h>
#include "TerrainTile.h"
#include "TerrainResource.h"

namespace Blade
{
	class TerrainUpdater : public GraphicsElementCommand
	{
	public:
		TerrainUpdater(TerrainTile* tile, TerrainResource* resource, ParaAAB* aab, TStringList* textureList, bool* modified)
			:mTile(tile)
			,mResource(resource)
			,mAAB(aab)
			,mTextureList(textureList)
			,mModified(modified)
		{
			mUpdateNormal = false;
			mUpdateHeight = false;
			mUpdateBlend = false;
		}

		/** @brief  */
		void updateNormal() { mUpdateNormal = true; }
		void updateHeight(size_t x, size_t z, size_t sizeX, size_t sizeZ)
		{
			mUpdateHeight = true;
			mHeightX = x; mHeightZ = z; mHeightSizeX = sizeX; mHeightSizeZ = sizeZ;
		}
		void updateBlockBlend(const TerrainBlockBlendData& data)
		{
			mBlend = data;
			mUpdateBlend = true;
		}
		virtual void execute()
		{
			if (mUpdateNormal)
			{
				mTile->updateNormalBuffer(mResource->getNormalBuffer());
				*mModified = true;
			}
			if (mUpdateHeight)
			{
				mTile->updateVertexBuffer(mHeightX, mHeightZ, mHeightSizeX, mHeightSizeZ);
				*mAAB = mTile->getLocalAABB();
				*mModified = true;
			}
			if (mUpdateBlend)
			{
				mTile->updateBlockBlendBuffer(mBlend);

				//update texture record for resource (it'll be saved)
				size_t count = mBlend.getCount();
				if (count > 0)
				{
					*mModified = true;
					mResource->setTextureInited();
					*mTextureList = mTile->getTileTextureList();

					for (size_t i = 0; i < mTextureList->size(); ++i)
						(*mTextureList)[i] += BTString(".") + IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureExt;
				}
			}
		}

		TerrainTile* mTile;
		TerrainResource* mResource;
		ParaAAB*		mAAB;
		TStringList*	mTextureList;
		bool*			mModified;
		size_t	mHeightX;
		size_t	mHeightZ;
		size_t	mHeightSizeX;
		size_t	mHeightSizeZ;
		TerrainBlockBlendData mBlend;
		bool	mUpdateNormal;
		bool	mUpdateHeight;
		bool	mUpdateBlend;
	};

	class TerrainElement : public GraphicsElement, public ITerrainInterface, public Allocatable
	{
	public:
		TerrainElement();
		~TerrainElement();

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual Interface*		getInterface(InterfaceName type)
		{
			CHECK_RETURN_INTERFACE(ITerrainInterface, type, static_cast<ITerrainInterface*>(this));
			return GraphicsElement::getInterface(type);
		}

		/************************************************************************/
		/* GraphicsElement interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			onInitialize();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onResourceUnload();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			postProcess(const ProgressNotifier& notifier);

		/**
		@describe 
		@param
		@return
		*/
		virtual POINT3		getPositionHint() const
		{
			return mTile != NULL && !mTile->getWorldAABB().isNull()/* loading */ ? mTile->getWorldAABB().getCenter() : GraphicsElement::getPositionHint();
		}

		/************************************************************************/
		/* ITerrainInterface interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void	initializeTerrain(size_t x, size_t z, const TString& resourcePath)
		{
			BLADE_TS_VERIFY(TS_MAIN_SYNC);

			size_t size = TerrainConfigManager::getSingleton().getTerrainTileSize();

			mTileIndexX = (uint16)x;
			mTileIndexZ = (uint16)z;

			mPosition = Vector3((scalar)(x*size), 0, (scalar)(z*size));
			mScale = Vector3::UNIT_ALL;

			Vector3 sz((scalar)size, 1, (scalar)size);
			mBounding.getDataRef().set(Vector3::ZERO, sz);

			this->setResourcePath(resourcePath);
		}
		/** @brief  */
		virtual bool	isModified() const
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return mModified;
		}

		/** @brief  */
		virtual pint16	getHeightBuffer() const
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			if (ITerrainConfigManager::getSingleton().isEditingModeEnabled())
			{
				TerrainResource* terrainRes = static_cast<TerrainResource*>(this->getBoundResource());
				if (terrainRes != NULL)
					return terrainRes->getHeightBuffer();
			}
			return NULL;
		}

		/** @brief  */
		virtual puint8	getNormalBuffer() const
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			if (ITerrainConfigManager::getSingleton().isEditingModeEnabled())
			{
				TerrainResource* terrainRes = static_cast<TerrainResource*>(this->getBoundResource());
				if (terrainRes != NULL)
					return terrainRes->getNormalBuffer();
			}
			return NULL;
		}

		/** @brief  */
		virtual puint8	getBlendBuffer() const
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			if (ITerrainConfigManager::getSingleton().isEditingModeEnabled())
			{
				TerrainResource* terrainRes = static_cast<TerrainResource*>(this->getBoundResource());
				if (terrainRes != NULL)
					return (puint8)terrainRes->getBlendBuffer();
			}
			return NULL;
		}

		/** @brief  */
		virtual TERRAIN_LAYER* getLayerBuffer() const
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			if (ITerrainConfigManager::getSingleton().isEditingModeEnabled())
			{
				TerrainResource* terrainRes = static_cast<TerrainResource*>(this->getBoundResource());
				if (terrainRes != NULL)
					return terrainRes->getLayerBuffer();
			}
			return NULL;
		}

		/** @brief  */
		virtual const TStringList& getTextureList() const
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return mTileTextures;
		}

		/** @brief  */
		virtual void updateNormalBuffer() 
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			if (ITerrainConfigManager::getSingleton().isEditingModeEnabled())
			{
				//delay update to task rendering : this could be called at any thread so don't update immediately
				TerrainUpdater* updater = this->createUpdater();
				updater->updateNormal();
			}
		}

		/** @brief  */
		virtual void	updateHeight(size_t x, size_t z, size_t sizeX, size_t sizeZ)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			if (ITerrainConfigManager::getSingleton().isEditingModeEnabled())
			{
				//delay update to task rendering 
				TerrainUpdater* updater = this->createUpdater();
				updater->updateHeight(x, z, sizeX, sizeZ);
			}
		}

		/** @brief  */
		virtual void	updateBlockBlend(const TerrainBlockBlendData& blockBlend)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			if (ITerrainConfigManager::getSingleton().isEditingModeEnabled())
			{
				//delay update to task rendering 
				TerrainUpdater* updater = this->createUpdater();
				updater->updateBlockBlend(blockBlend);
			}
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		TerrainTile*				getTile() const		{return mTile;}

	protected:
		/** @brief  */
		TerrainUpdater*				createUpdater()
		{
			TerrainUpdater* updater = BLADE_NEW TerrainUpdater(mTile, static_cast<TerrainResource*>(this->getBoundResource()), &mBounding, &mTileTextures, &mModified);
			this->addCommand(updater);
			return updater;
		}

		/*
		@describe
		@param
		@return
		*/
		void						createTile(const Vector3& scale,size_t size);

		/*
		@describe 
		@param 
		@return 
		*/
		void						setupTile();

		/** @brief  */
		inline IStage*				getStage() const
		{
			return this->getRenderScene()->getStage();
		}

		TerrainTile*			mTile;

		//UI config notify
		void	onTextureChange(void* data);
		TStringList				mTileTextures;

		uint16					mTileIndexX;
		uint16					mTileIndexZ;
		bool					mModified;

		friend class TerrainPlugin;
	};//class TerrainElement
	
}//namespace Blade


#endif //__Blade_TerrainElement_h__
