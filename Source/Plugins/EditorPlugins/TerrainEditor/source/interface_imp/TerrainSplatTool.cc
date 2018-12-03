/********************************************************************
	created:	2011/05/25
	filename: 	TerrainSplatTool.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainSplatTool.h"
#include <utility/Profiling.h>
#include <ConfigTypes.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/graphics/PixelColorFormatter.h>
#include <databinding/DataSourceWriter.h>
#include <interface/IResourceManager.h>
#include <interface/ITerrainConfigManager.h>
#include <interface/IEventManager.h>
#include <interface/IMediaLibrary.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/IEditorFramework.h>

#include <BladeTerrainEditor_blang.h>

namespace Blade
{

	namespace SplatConfigString
	{
	
		static const TString SPLAT_TEXTURE = BXLang(BLANG_TERRAIN_TEXTURE);
	
	}//namespace ConfigString

	static const int MAX_I8 = 0x7F;
	static const int MAX_U8 = 0xFF;

	//////////////////////////////////////////////////////////////////////////
	void	TerrainSplatOperation::setTexture(const TString& texture)
	{
		mTextureRealPath = IResourceManager::getSingleton().schemePathToRealPath(texture);
		mTexture = texture;
	}

	//////////////////////////////////////////////////////////////////////////
	LAYER_ID	TerrainSplatOperation::getLayerID(const TStringList& textureList, bool& settingTexture)
	{
		LAYER_ID layerID = INVALID_LAYER_ID;
		settingTexture = true;
		for(size_t i = 0; i < textureList.size(); ++i )
		{
			TString path = IResourceManager::getSingleton().schemePathToRealPath(textureList[i]);
			if( path == mTextureRealPath )
			{
				layerID = (LAYER_ID)i;
				settingTexture = false;
				return layerID;
			}
		}
		
		layerID = (LAYER_ID)textureList.size();
		if( layerID > TERRAIN_LAYER::MAX_LAYER_ID )
			layerID = INVALID_LAYER_ID;
		return layerID;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	TerrainSplatOperation::getBlendChannel(LAYER_ID layer, TERRAIN_LAYER& vertexLayer)
	{
		if( layer == INVALID_LAYER_ID )
			return INVALID_INDEX;

		index_t layerIndex;
		if( (layerIndex=vertexLayer.findLayer(layer)) == INVALID_INDEX )
		{
			//layers full
			if( (layerIndex=vertexLayer.addLayer(layer)) == INVALID_INDEX)
				return INVALID_INDEX;
		}

		if( layerIndex >= TERRAIN_LAYER::MAX_LAYER_COUNT || layerIndex >= sizeof(Color::COLOR) )
		{
			assert(false);
			return INVALID_INDEX;
		}

		return layerIndex;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TerrainSplatOperation::resetLayer(LAYER_ID layer,TERRAIN_LAYER& vertexLayer)
	{
		vertexLayer.mAtlasIndex1 = vertexLayer.mAtlasIndex2 = vertexLayer.mAtlasIndex3 = layer;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TerrainBlockSplatOperation::process(const PosInfo& x,const PosInfo& z,TerrainEditable* te,scalar deltaTime,bool inverse)
	{
		assert( x.mEnd - x.mStart == x.mTerranEnd - x.mTerranStart);
		assert( z.mEnd - z.mStart == z.mTerranEnd - z.mTerranStart);

		size_t rowCount = z.mEnd - z.mStart;
		size_t colCount = x.mEnd - x.mStart;

		//terrain size in quads
		size_t terrainSize = ITerrainConfigManager::getInterface().getTerrainInfo().mTileSize;
		const size_t BlockSize = ITerrainConfigManager::getInterface().getTerrainConfig().mBlockSize;
		size_t blendMultiple = ITerrainConfigManager::getInterface().getTerrainInfo().mBlendBufferScale;
		size_t blendBytesPerPoint = sizeof(Color::COLOR);

		size_t TerrainEndX;
		size_t TerrainEndZ;
		//the brush rect generates a coordinates in vertices, not in quads, but we need it in quads
		if( z.mTerranEnd == terrainSize + 1 )
		{
			TerrainEndZ = terrainSize;
			rowCount -= 1;
		}
		else
			TerrainEndZ = z.mTerranEnd;

		if( x.mTerranEnd == terrainSize + 1 )
		{
			TerrainEndX = terrainSize;
			colCount -= 1;
		}
		else
			TerrainEndX = x.mTerranEnd;

		const TStringList& textures = te->getTextureList();
		bool updateTexture = false;
		LAYER_ID layerID = this->getLayerID(textures,updateTexture);

		puint8 buffer_pointer = te->getBlendBuffer();
		TERRAIN_LAYER* layer_pointer = te->getLayerBuffer();

		const int32* brushdata = mBrushDataRef.getBrushData();
		size_t brush_offset = (z.mStart*mBrushDataRef.getBrushSize()) + (x.mStart)*blendMultiple;
		const int32* brush_pointer = brushdata + brush_offset;

		const size_t blockStartX = x.mTerranStart/BlockSize;
		const size_t blockStartZ = z.mTerranStart/BlockSize;
		size_t blockEndX = (TerrainEndX)/BlockSize;
		size_t blockEndZ = (TerrainEndZ)/BlockSize;

		if( blockEndX == terrainSize/BlockSize )
			--blockEndX;

		if( blockEndZ == terrainSize/BlockSize )
			--blockEndZ;

#if BLADE_DEBUG
		const puint8 buffer_end = buffer_pointer + terrainSize*blendMultiple*terrainSize*blendMultiple*blendBytesPerPoint;
		const int32* brush_end = brushdata + mBrushDataRef.getBrushSize()*mBrushDataRef.getBrushSize();
		const TERRAIN_LAYER* layer_end = layer_pointer + terrainSize*terrainSize;
#endif

		for(size_t iz = blockStartZ; iz <= blockEndZ; ++iz)
		{
			size_t pointStartZ,pointEndZ;
			const size_t pointStartZTruncated = iz*BlockSize;
			pointStartZ = pointStartZTruncated;
			pointEndZ = pointStartZTruncated + BlockSize;

			if( iz == blockStartZ )
				pointStartZ = z.mTerranStart;
			if( iz == blockEndZ)
				pointEndZ = TerrainEndZ;

			assert( pointEndZ >= pointStartZ );
			if( pointEndZ == pointStartZ )
				continue;

			const int32* row_brush_pointer = brush_pointer;
			const size_t height = pointEndZ - pointStartZ;
			const size_t heightPoints = height*blendMultiple;

			for(size_t ix = blockStartX; ix <= blockEndX; ++ix)
			{
				//blend pixel points per block
				size_t pointStartX,pointEndX;

				const size_t pointStartXTruncated = ix*BlockSize;
				pointStartX = pointStartXTruncated;
				pointEndX = pointStartXTruncated+BlockSize;

				if( ix == blockStartX )
					pointStartX = x.mTerranStart;
				if( ix == blockEndX)
					pointEndX = TerrainEndX;

				assert( pointEndX >= pointStartX );
				if( pointEndX == pointStartX )
					continue;

				size_t localX = pointStartX-pointStartXTruncated;
				size_t localZ = pointStartZ-pointStartZTruncated;

				//update blend pixel
				puint8 block_buffer_pointer = buffer_pointer + (pointStartZ*terrainSize*blendMultiple+pointStartX)*blendMultiple*blendBytesPerPoint;
				TERRAIN_LAYER* block_layer_pointer = layer_pointer + (pointStartZ*terrainSize+pointStartX);
				const int32* block_brush_pointer = row_brush_pointer;
				const size_t width = pointEndX - pointStartX;
				const size_t widthPoints = width*blendMultiple;
				for(size_t pz = 0; pz < heightPoints; ++pz)
				{
					for(size_t px = 0; px < widthPoints; ++px)
					{
#if BLADE_DEBUG
						//data area overflow check
						assert( block_brush_pointer + pz*mBrushDataRef.getBrushSize() + px >= brush_pointer );
						assert( block_brush_pointer + pz*mBrushDataRef.getBrushSize() + px < brush_end );

						assert( block_buffer_pointer + pz*terrainSize*blendMultiple*blendBytesPerPoint + px*blendBytesPerPoint >= buffer_pointer );
						assert( block_buffer_pointer + pz*terrainSize*blendMultiple*blendBytesPerPoint + px*blendBytesPerPoint < buffer_end );

						assert( block_layer_pointer + pz/blendMultiple*terrainSize+px/blendMultiple >= layer_pointer );
						assert( block_layer_pointer + pz/blendMultiple*terrainSize+px/blendMultiple < layer_end );
#endif
						int val = (int)( (scalar)(*(block_brush_pointer + pz*mBrushDataRef.getBrushSize() + px)) * deltaTime + 0.5f );
						if( val == 0 )
							continue;
						TERRAIN_LAYER& layer = block_layer_pointer[pz/blendMultiple*terrainSize+px/blendMultiple];
						size_t blendChannel = this->getBlendChannel(layerID, layer);
						if( blendChannel == INVALID_INDEX )//layers full
							continue;

						uint8* pointData = block_buffer_pointer + pz*terrainSize*blendMultiple*blendBytesPerPoint + px*blendBytesPerPoint;
						int original = /*updateTexture ? 0 : */*(pointData+blendChannel);

						if (!inverse)
						{
							if (original == MAX_U8)
							{
								bool onlyOne = true;	//all other channels are zero
								for (size_t n = 0; n < blendBytesPerPoint; ++n)
								{
									if (n != blendChannel)
									{
										int result = *(pointData + n) - val / ((int)blendBytesPerPoint - 1);
										*(pointData + n) = (uint8)((result < 0) ? 0 : result);
										if (*(pointData + n) != 0)
											onlyOne = false;
									}
								}
								////if only this channel is valid, swap channel to avoid layer map/blend map interpolation issue
								////that will spare the empty channel for future use
#define ENABLE_CHANNEL_REUSE 0
#if ENABLE_CHANNEL_REUSE
								if (onlyOne && layer.getLayerCount() > 2)
								{
									this->resetLayer(layerID, layer);
									std::swap(*(pointData + blendChannel), *(pointData + this->getBlendChannel(layerID, layer)));
								}
#else
								BLADE_UNREFERENCED(onlyOne);
#endif
							}
							else
							{
								int result = original + val;
								*(pointData + blendChannel) = (uint8)((result > (int)MAX_U8) ? MAX_U8 : result);
							}
						}
						else
						{
							if (original == 0)
								layer.removeLayer(layerID);
							else
							{
								int result = original - val;
								*(pointData + blendChannel) = (uint8)((result < 0) ? 0 : result);
							}
						}
					}//for px
				}//for pz

				BLOCK_INDEX blockIndex( (uint8)ix,(uint8)iz);
				te->updateBlockBlendBuffer(blockIndex,localX,localZ,
					width,height,mTexture,updateTexture);
				row_brush_pointer += widthPoints;
			}//for ix
			brush_pointer += (heightPoints)*mBrushDataRef.getBrushSize();

		}//for iz
		return true;
	}



	//////////////////////////////////////////////////////////////////////////
	TerrainSplatTool::TerrainSplatTool(IconIndex icon,HOTKEY hotkey)
		:TerrainBrushBaseTool(BXLang(BLANG_PAINT),icon,hotkey)
		,mBlockBrushOP(mBrushData)
		,mStateRegistered(false)
	{
		DataSourceWriter<TerrainSplatTool> writer( mToolConfig );
		writer.beginLatestVersion();
		writer << SplatConfigString::SPLAT_TEXTURE << CONFIG_UIHINT(CUIH_IMAGE, TerrainConsts::TERRAIN_MEDIA_IMAGE_HINT) << &TerrainSplatTool::setSplatTexture
			<< static_cast<Bindable*>(this)	//binding target
			<< &TerrainSplatTool::mSplatTexture;
		writer.endVersion();
		mBrushColor = Color::BLUE;
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainSplatTool::~TerrainSplatTool()
	{

	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				TerrainSplatTool::onSelection()
	{
		if( this->isEnable() && this->isSelected() )
		{
			mBrushSizeMultiple = ITerrainConfigManager::getInterface().getTerrainInfo().mBlendBufferScale;
		}
		TerrainBrushBaseTool::onSelection();
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainSplatTool::onEnable(bool bEnabled)
	{
		if( bEnabled )
		{
			//BLADE_LW_PROFILING_FUNCTION();
			//note: mSplatTexture will be set by loading editor's config
			if( mSplatTexture == TString::EMPTY || IMediaLibrary::getSingleton().getMediaFile(TerrainConsts::TERRAIN_MEDIA_IMAGE_HINT, mSplatTexture) == NULL )
			{
				//init default splat texture : using any first one in terrain texture path
				const IMediaLibrary::MediaFile* mediaFile = IMediaLibrary::getSingleton().getMediaFile(TerrainConsts::TERRAIN_MEDIA_IMAGE_HINT, 0);
				if( mediaFile != NULL )
					mSplatTexture = mediaFile->mSchemePath;
			}
			mBlockBrushOP.setTexture(mSplatTexture);

			if( !mStateRegistered )
			{
				IEventManager::getSingleton().addEventHandler( OptionHighLightEvent::NAME, EventDelegate(this, &TerrainSplatTool::onOptionHighLight)  );
				mStateRegistered = true;
			}

		}
		else
		{
			if( mStateRegistered )
			{
				IEventManager::getSingleton().removeEventHandlers(OptionHighLightEvent::NAME, this);
				mStateRegistered = false;
			}
		}

		TerrainBrushBaseTool::onEnable(bEnabled);
	}

	//////////////////////////////////////////////////////////////////////////
	ITerrainBrushOperation&	TerrainSplatTool::getBrushOperation()
	{
		return mBlockBrushOP;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				TerrainSplatTool::setSplatTexture(void* data)
	{
		BLADE_UNREFERENCED(data);
		assert(data == &mSplatTexture);
		mBlockBrushOP.setTexture(mSplatTexture);
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainSplatTool::onOptionHighLight(const Event& data)
	{
		if( data.mName != OptionHighLightEvent::NAME )
		{
			assert(false);
			return;
		}

		IEditorFile* file = IEditorFramework::getSingleton().getCurrentFile();
		const IEntity* entity = (file == NULL) ? NULL : file->getSelectedEntity();

		ElementListParam elementList;
		if( entity != NULL && entity->getElementsByType(elementList, TerrainConsts::TERRAIN_ELEMENT) )
		{
			const OptionHighLightEvent& state = static_cast<const OptionHighLightEvent&>(data);
			const Bindable* bindable = state.mTarget;
			bool validSource = false;

			if( state.mUIHint == TerrainConsts::TERRAIN_MEDIA_IMAGE_HINT )
			{
				for(size_t i = 0; i < elementList.size(); ++i)
				{
					IElement* terrainElement = elementList[i];
					if( bindable == terrainElement )
					{
						validSource = true;
						break;
					}
				}
			}

			if( validSource )
			{
				const TString& optionVal = state.mOptionValue;
				const IMediaLibrary::MediaFile* mediaFile = IMediaLibrary::getSingleton().getMediaFile(TerrainConsts::TERRAIN_MEDIA_IMAGE_HINT, optionVal);
				if( mediaFile != NULL )
				{
					mSplatTexture = mediaFile->mSchemePath;
					mBlockBrushOP.setTexture(mSplatTexture);
				}
			}
		}
	}

}//namespace Blade
