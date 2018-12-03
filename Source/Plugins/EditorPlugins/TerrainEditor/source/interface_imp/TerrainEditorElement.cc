/********************************************************************
	created:	2011/08/26
	filename: 	TerrainEditorElement.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainEditorElement.h"
#include <interface/IEditorFramework.h>
#include <interface/ITerrainConfigManager.h>
#include "TerrainToolManager.h"

namespace Blade
{
	const TString TerrainEditorElement::TERRAIN_EDITOR_ELEMENT = BTString("TerrainEditor");

	//////////////////////////////////////////////////////////////////////////
	TerrainEditorElement::TerrainEditorElement()
		:EditorElement(TerrainEditorElement::TERRAIN_EDITOR_ELEMENT, BTString("terrain.png"))
		,mInterface(NULL)
	{

	}
	
	//////////////////////////////////////////////////////////////////////////
	TerrainEditorElement::~TerrainEditorElement()
	{
		if (mEditable != NULL)
		{
			TerrainToolManager::getSingleton().removeTerrainEditable(static_cast<TerrainEditable*>(mEditable));
			if (IEditorFramework::getSingleton().getCurrentFile() != NULL)	//not closing time
			{
				if (mEditable->isModified())
					mEditable->save();
			}
		}
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	/** @brief called after loading data in main synchronous state */
	void				TerrainEditorElement::postProcess(const ProgressNotifier& notifier)
	{
		bool creatingTerrainEditable = (mEditable == NULL);

		EditorElement::postProcess(notifier);

		if( creatingTerrainEditable )
			TerrainToolManager::getSingleton().addTerrainEditable( static_cast<TerrainEditable*>(mEditable) );
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	pint16				TerrainEditorElement::getTerrainHeightBuffer() const
	{
		return mInterface->getHeightBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorElement::updateTerrainHeightBuffer(size_t x, size_t z, size_t sizeX, size_t sizeZ)
	{
		mInterface->updateHeight(x, z, sizeX, sizeZ);
	}

	//////////////////////////////////////////////////////////////////////////
	puint8				TerrainEditorElement::getTerrainNormalBuffer() const
	{
		return mInterface->getNormalBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorElement::updateTerrainNormal()
	{
		mInterface->updateNormalBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	const TStringList&	TerrainEditorElement::getTextureList() const
	{
		return mInterface->getTextureList();
	}

	//////////////////////////////////////////////////////////////////////////
	TERRAIN_LAYER*	TerrainEditorElement::getLayerBuffer() const
	{
		return mInterface->getLayerBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	uint8*				TerrainEditorElement::getBlendBuffer() const
	{
		return mInterface->getBlendBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorElement::updateBlockBlendBuffer(BLOCK_INDEX blockIndex,index_t StartX,index_t StartZ,
		index_t SizeX,size_t SizeZ,const TString& texture,bool updateTexture)
	{
		size_t x = blockIndex.mZ;
		size_t z = blockIndex.mZ;

		size_t blocks = ITerrainConfigManager::getInterface().getTerrainInfo().mBlocksPerTileSide;
		if( z >= blocks )
		{
			assert(false);
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index 'z' out of range.") );
		}
		if( x >= blocks )
		{
			assert(false);
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index 'x' out of range.") );
		}

		if(StartX + SizeX > ITerrainConfigManager::getInterface().getTerrainConfig().mBlockSize 
			|| StartZ + SizeZ > ITerrainConfigManager::getInterface().getTerrainConfig().mBlockSize)
		{
			assert(false);
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("size out of range.") );
		}

		TerrainBlockBlendData blendData;

		TString textureBase, textureExt;
		TStringHelper::getFileInfo(texture, textureBase, textureExt);
		blendData.add( SingleBlockBlendData(StartX, StartZ, SizeX, SizeZ, textureBase, updateTexture, blockIndex) );

		mInterface->updateBlockBlend(blendData);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				TerrainEditorElement::isTerrainModifed() const
	{
		return mInterface->isModified();
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorElement::initialzeInterface()
	{
		mInterface = this->getEntity()->getInterface(IID_TERRAIN());
	}

}//namespace Blade
