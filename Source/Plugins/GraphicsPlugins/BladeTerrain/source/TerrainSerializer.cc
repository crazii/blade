/********************************************************************
	created:	2012/09/27
	filename: 	TerrainSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/Profiling.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/graphics/PixelColorFormatter.h>
#include <interface/IResourceManager.h>
#include <interface/IGraphicsSystem.h>
#include <interface/ISpace.h>
#include <TerrainUtility.h>
#include "TerrainConfigManager.h"
#include "TerrainTextureManager.h"
#include "TerrainBufferManager.h"
#include "TerrainSerializer.h"
#include "TerrainResource.h"


namespace Blade
{
	//file format information
	struct TileResouceHeader
	{
		FourCC	magic;
		uint32	size;
		fp32	base;
		fp32	scale[3];
		uint32	reserved[25];
	};

	static const uint32 TERRAIN_MAGIC = BLADE_FCC('B','L','T','N');
	static const uint32 TEXTURE_MAGIC = BLADE_FCC('T','N','T','X');
	static const uint32 HEIGHTBLOCK_MAGIC = BLADE_FCC('T','N','H','T');
	static const uint32 NORMALBLOCK_MAGIC = BLADE_FCC('T','N','N','M');
	static const uint32 BLENDBLOCK_MAGIC = BLADE_FCC('T','N','B','W');
	static const uint32 LAYERBLOCK_MAGIC = BLADE_FCC('T','N','L','Y');
	static const uint32 PARTITIONBLOCK_MAGIC = BLADE_FCC('T', 'N', 'S', 'P');

	struct TerrainResourceSection
	{
		FourCC	magic;
	};

	struct TextureSection : public TerrainResourceSection
	{
		uint32  textureCount;

		TextureSection()
		{
			magic = TEXTURE_MAGIC;
			textureCount = 0;
		}
	};

	struct HeightDataSection : public TerrainResourceSection
	{
		uint32 bytes; 

		HeightDataSection()
		{
			magic = HEIGHTBLOCK_MAGIC;
			bytes = 0;
		}
	};

	struct NormalDataSection : public TerrainResourceSection
	{
		uint32 bytes;

		NormalDataSection()
		{
			magic = NORMALBLOCK_MAGIC;
			bytes = 0;
		}
	};

	struct BlendDataSection : public TerrainResourceSection
	{
		uint32 bytes;

		BlendDataSection()
		{
			magic = BLENDBLOCK_MAGIC;
			bytes = 0;
		}
	};

	struct LayerDataSection : public TerrainResourceSection
	{
		uint32 bytes;
		LayerDataSection()
		{
			magic = LAYERBLOCK_MAGIC;
			bytes = 0;
		}
	};

	struct PartitionMaskSection : public TerrainResourceSection
	{
		uint32 bytes;
		PartitionMaskSection()
		{
			magic = PARTITIONBLOCK_MAGIC;
			bytes = 0;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	TerrainSerializer::TerrainSerializer()
		:mHeight(0)
		,mSize(0)
		,mReload(false)
		,mLoadNormalOnly(false)
	{
		mPreConvertedLayer = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainSerializer::~TerrainSerializer()
	{
		BLADE_TMP_FREE(mPreConvertedLayer);
	}

	/************************************************************************/
	/* ISerializer interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	TerrainSerializer::loadResource(IResource* res,const HSTREAM& stream, const ParamList& /*params*/)
	{
		//BLADE_LW_PROFILING_FUNCTION();
		if( res == NULL || stream == NULL)
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource or stream data corrupted.") );
		else if( res->getType() != TerrainResource::TYPE )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource type mismatch.") );

		TerrainResource* tileRes = static_cast<TerrainResource*>(res);
		TString name;
		mReload = false;

		if( this->loadData(stream,name,tileRes->mBase,tileRes->mSize,
			&tileRes->mHeightBuffer, &tileRes->mNormalBuffer, &tileRes->mBlendBuffer, &tileRes->mLayerBuffer) )
		{
			this->copyAABBList(res);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TerrainSerializer::postProcessResource(IResource* resource)
	{
		BLADE_LW_PROFILING_FUNCTION();
		if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT() && !mLoadNormalOnly )
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("cannot post process terrain resource without graphics context."));

		TerrainResource* tileRes = static_cast<TerrainResource*>(resource);

		//reloading
		if( mReload )
		{
			BLADE_LW_PROFILING( DATA_COPY );
			size_t count = (mSize+1)*(mSize+1);

			const size_t blendMapSize = TerrainConfigManager::getSingleton().getBlendBufferSize();
			const size_t BlendSize = blendMapSize * blendMapSize;
			const size_t blockCount = TerrainConfigManager::getSingleton().getTerrainInfo().mBlocksPerTile;

			//TODO: separate size & blend size & block count check
			if( mSize != tileRes->mSize )
			{
				assert(!mLoadNormalOnly);
				TERRAIN_RES_FREE(tileRes->mHeightBuffer);
				TERRAIN_RES_FREE(tileRes->mNormalBuffer);
				TERRAIN_RES_FREE(tileRes->mBlendBuffer);
				TERRAIN_RES_FREE(tileRes->mLayerBuffer);
				tileRes->mHeightBuffer = TERRAIN_RES_ALLOCT(int16,count);
				tileRes->mNormalBuffer = TERRAIN_RES_ALLOCT(uint8, count*3);
				tileRes->mBlendBuffer = TERRAIN_RES_ALLOCT(Color::RGBA, BlendSize);
				tileRes->mLayerBuffer = TERRAIN_RES_ALLOCT(TERRAIN_LAYER, blockCount);

				tileRes->mBlendMap.clear();
				tileRes->mGlobalNormalMap.clear();
				tileRes->mLayerMap.clear();
				tileRes->mVertexPositionBuffer.clear();
				tileRes->mVertexNormalBuffer.clear();
			}

			tileRes->mSize = mSize;
			tileRes->mBase = mHeight;

			if( mLoadNormalOnly )
			{
				//resource data may be release on runtime
				if( tileRes->mNormalBuffer == NULL )
					tileRes->mNormalBuffer = TERRAIN_RES_ALLOCT(uint8, count*3);
				std::memcpy( tileRes->mNormalBuffer, mReloadingNormalCache.getData(), count*sizeof(int8)*3 );
				return;
			}

			std::memcpy( tileRes->mHeightBuffer, mReloadingHeightCache.getData(), count*sizeof(int16) );
			std::memcpy( tileRes->mNormalBuffer, mReloadingNormalCache.getData(), count*sizeof(int8)*3 );
			std::memcpy( tileRes->mBlendBuffer, mReloadingBlendCache.getData(), BlendSize*sizeof(Color::COLOR) );
			std::memcpy( tileRes->mLayerBuffer, mReloadingLayerCache.getData(), blockCount*sizeof(TERRAIN_LAYER) );
		}
		else
			assert( !mLoadNormalOnly );

		//textures
		size_t atlasSize = TerrainConfigManager::getSingleton().getTextureSize()*TerrainConfigManager::getSingleton().getTextureCount();
		if( mTextureCache.size() != 0 )
		{
			{
				//BLADE_LW_PROFILING( DIFFUSE_ATLAS );
				//normal routine
				if( tileRes->mAtlas == NULL || tileRes->mAtlas->getAtlasSize() != atlasSize )
					tileRes->mAtlas.bind( TerrainTextureManager::getSingleton().buildTerrainAtlas( mTextureCache, &mTextures[0], TERRAIN_PIXELFORMAT ) );
				else
					TerrainTextureManager::getSingleton().updateTerrainAtlas( tileRes->mAtlas, mTextureCache, &mTextures[0] );
			}

			//BLADE_LW_PROFILING( NORMAL_ATLAS );
			if( ITerrainConfigManager::getSingleton().getTextureQuality() == TerrainConsts::TEXTURE_QUALITY_HIGH )
			{
				if( tileRes->mNormalAtlas == NULL || tileRes->mNormalAtlas->getAtlasSize() != atlasSize )
					tileRes->mNormalAtlas.bind( TerrainTextureManager::getSingleton().buildTerrainAtlas( mNormalTextureCahe, &mNormalTextures[0], TERRAIN_NORMAL_FORMAT) );
				else
					TerrainTextureManager::getSingleton().updateTerrainAtlas( tileRes->mNormalAtlas, mNormalTextureCahe, &mNormalTextures[0]);
			}
			else
				assert(tileRes->mNormalAtlas == NULL);

			tileRes->mHasTexture = true;
		}
		else
		{
			//BLADE_LW_PROFILING( EMPTY_ATLAS );
			//no textures, one pre-load indeed
			assert( mTextures.size() == 1 );
			assert( mNormalTextureCahe.size() == 0 );

			mTextureCache.push_back( TerrainConsts::EMPTY_TEXTURE_FILE );
			if (tileRes->mAtlas == NULL || tileRes->mAtlas->getAtlasSize() != atlasSize)
				tileRes->mAtlas.bind( TerrainTextureManager::getSingleton().buildTerrainAtlas( mTextureCache, &mTextures[0], TERRAIN_PIXELFORMAT ) );
			else
				TerrainTextureManager::getSingleton().updateTerrainAtlas(tileRes->mAtlas, mTextureCache, &mTextures[0]);
			tileRes->mAtlas->removeLastTexture();
			//assert( tileRes->mNormalAtlas == NULL );
			//tileRes->mNormalAtlas.clear();
			tileRes->mHasTexture = false;
		}

		{
			//BLADE_LW_PROFILING( BLEND_MAP );
			if( tileRes->mBlendMap != NULL && tileRes->mBlendMap->getWidth() == TerrainConfigManager::getSingleton().getBlendBufferSize() )
				IGraphicsResourceManager::getSingleton().updateTexture( *mBlendImage, *(tileRes->mBlendMap) );
			else
				tileRes->mBlendMap = TerrainTextureManager::getSingleton().createBlendTexture(mBlendImage);
		}


		{
			//BLADE_LW_PROFILING( NORMAL_MAP );
			if( !TerrainConfigManager::getSingleton().isUseVertexNormal() )
			{
				assert( mNormalImage != HIMAGE::EMPTY );
				if( tileRes->mGlobalNormalMap != NULL )
					IGraphicsResourceManager::getSingleton().updateTexture( *mNormalImage, *(tileRes->mGlobalNormalMap) );
				else
					tileRes->mGlobalNormalMap = TerrainTextureManager::getSingleton().createNormalTexture(mNormalImage);
			}
			else
			{
				assert( mNormalImage == HIMAGE::EMPTY );
				tileRes->mGlobalNormalMap = HTEXTURE::EMPTY;
			}
		}

		{
			//BLADE_LW_PROFILING(LAYER_MAP);
			if( tileRes->mLayerMap != NULL )
				TerrainTextureManager::getSingleton().updateLayerTexture(tileRes->mLayerMap, mPreConvertedLayer, false);
			else
				tileRes->mLayerMap = TerrainTextureManager::getSingleton().createLayerTexture(mPreConvertedLayer, false);
		}

		{
			//BLADE_LW_PROFILING( VERTEX_BUFFER );
			IGraphicsResourceManager& manager = IGraphicsResourceManager::getSingleton();
			manager.cloneVertexBuffer(tileRes->mVertexPositionBuffer, *mVertexPositionBuffer);

			if( mVertexNormalBuffer != NULL )
				manager.cloneVertexBuffer(tileRes->mVertexNormalBuffer, *mVertexNormalBuffer);
			else
				tileRes->mVertexNormalBuffer = HVBUFFER::EMPTY;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TerrainSerializer::saveResource(const IResource* res,const HSTREAM& stream)
	{
		if( res == NULL || stream == NULL)
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource or stream data corrupted.") );
		else if( res->getType() != BTString("TerrainResource") )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("resource type mismatch.") );

		const TerrainResource* tileRes = static_cast<const TerrainResource*>(res);
		TileResouceHeader header;

		//header
		header.magic = TERRAIN_MAGIC;
		header.base = tileRes->getHeightBase();
		header.size = (uint32)tileRes->getSize();

		IOBuffer buffer;
		buffer.write(&header);

		size_t count = (header.size+1)*(header.size+1);

		//textures
		{
			TextureSection section;
			if( tileRes->isTextureInited() )
			{
				section.textureCount = (uint32)tileRes->mAtlas->getSubTextureCount();
				buffer.write(&section);
				for( uint32 texit = 0 ; texit < section.textureCount; ++texit)
				{
					//note: store file base(with path prefix) only, real extension(.DDS, .KTX) is decided atruntime
					const TString& file = tileRes->mAtlas->getTextureList()[texit]; 
					TString base,ext;
					TStringHelper::getFileInfo(file, base, ext);
					buffer.writeTString(base);
				}
			}
			else
				buffer.write(&section);
		}

		//height
		{
			HeightDataSection section;
			section.bytes = (uint32)(count*sizeof(uint16));
			buffer.write(&section);
			buffer.write(tileRes->getHeightBuffer(), count );
		}

		//normal
		{
			NormalDataSection section;
			section.bytes = (uint32)(count*sizeof(uint8)*3);
			buffer.write(&section);

			//normal buffer are already stored in platform independent byte order
			buffer.write(tileRes->mNormalBuffer, count*3);
		}

		//blend
		{
			BlendDataSection section;
			const size_t blendMapSize = TerrainConfigManager::getSingleton().getBlendBufferSize();
			const size_t BlendSize = blendMapSize * blendMapSize;
			section.bytes = (uint32)(BlendSize*sizeof(Color::COLOR));
			buffer.write(&section);
			buffer.write( tileRes->getBlendBuffer(), BlendSize );
		}

		//layer info
		{
			LayerDataSection section;
			section.bytes = (uint32)(sizeof(TERRAIN_LAYER)*count);
			buffer.write(&section);
			buffer.write(tileRes->getLayerBuffer(), count);
		}

		buffer.saveToStream(stream);
		stream->flush();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TerrainSerializer::createResource(IResource* res, ParamList& params)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );
		BLADE_LW_PROFILING_FUNCTION();

		const TString& path = res->getSource();
		ILog::DebugOutput << TEXT("Creating terrain resource: \"")
			<< path
			<< TEXT("\".")
			<<ILog::endLog;

		if( res == NULL || res->getType() != BTString("TerrainResource") )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid resource data/type.") );

		TerrainResource* tileRes = static_cast<TerrainResource*>(res);

		mSize = tileRes->mSize = (uint32)params[BTString("SIZE")];
		mHeight = tileRes->mBase = Vector3(params[BTString("POS")]).y;

		size_t count = (mSize+1)*(mSize+1);
		//height
		tileRes->mHeightBuffer = TERRAIN_RES_ALLOCT(int16,count);
		std::memset(tileRes->mHeightBuffer,0,count*sizeof(int16));

		//normal
		const size_t NormalBytes = count*3;
		tileRes->mNormalBuffer = TERRAIN_RES_ALLOCT(uint8,NormalBytes);
		for(size_t i = 0; i < NormalBytes; i += 3)
		{
			tileRes->mNormalBuffer[ i+0 ] = 127;	//0
			tileRes->mNormalBuffer[ i+1 ] = 255;	//1
			tileRes->mNormalBuffer[ i+2 ] = 127;	//0
		}

		//blend
		size_t blendMapSize = TerrainConfigManager::getSingleton().getBlendBufferSize();
		const size_t BlendSize = blendMapSize * blendMapSize;
		tileRes->mBlendBuffer = TERRAIN_RES_ALLOCT(Color::RGBA,BlendSize);
		Color::RGBA color(0xFF, 0, 0, 0);
		for(size_t i = 0; i < BlendSize; ++i )
			tileRes->mBlendBuffer[i] = color;

		//layer info
		{
			tileRes->mLayerBuffer = TERRAIN_RES_ALLOCT(TERRAIN_LAYER, count);
			std::memset(tileRes->mLayerBuffer, INVALID_LAYER_ID, sizeof(TERRAIN_LAYER)*count );
			tileRes->mLayerMap = TerrainTextureManager::getSingleton().createLayerTexture(tileRes->mLayerBuffer);
		}

		//atlas
		mTextureCache.push_back( TerrainConsts::EMPTY_TEXTURE_FILE );
		HIMAGE hImg = TerrainTextureManager::getSingleton().prepareSubTexture(TerrainConsts::EMPTY_TEXTURE_FILE, TERRAIN_PIXELFORMAT);
		mTextures.push_back(hImg);
		//pre-load an empty texture
		tileRes->mAtlas.bind( TerrainTextureManager::getSingleton().buildTerrainAtlas( mTextureCache, &mTextures[0], TERRAIN_PIXELFORMAT ) );
		//erase the empty texture info
		tileRes->mAtlas->removeLastTexture();
		tileRes->mHasTexture = false;
		//assert( tileRes->mNormalAtlas == NULL );
		tileRes->mNormalAtlas = HTERRAINATLAS::EMPTY;

		//blend map & normal map (textures)
		tileRes->mBlendMap = TerrainTextureManager::getSingleton().createBlendTexture( TerrainTextureManager::getSingleton().createBlendImage( (const uint8*)(tileRes->mBlendBuffer) ) );
		if( !TerrainConfigManager::getSingleton().isUseVertexNormal() )
			tileRes->mGlobalNormalMap = TerrainTextureManager::getSingleton().createNormalTexture( TerrainTextureManager::getSingleton().createNormalImage(tileRes->mNormalBuffer, mSize+1) );

		//position & normal vertex buffer
		IGraphicsResourceManager& manager = IGraphicsResourceManager::getSingleton();
		if( !TerrainConfigManager::getSingleton().isUseVertexNormal() )
		{
			HVBUFFER normal;
			TerrainBufferManager::getSingleton().createVertexBuffer(manager, tileRes->mVertexPositionBuffer, normal, 
				mSize, tileRes->mAABBList, tileRes->mHeightBuffer, NULL);
		}
		else
			TerrainBufferManager::getSingleton().createVertexBuffer(manager, tileRes->mVertexPositionBuffer, tileRes->mVertexNormalBuffer,
			mSize, tileRes->mAABBList, tileRes->mHeightBuffer, tileRes->mNormalBuffer);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TerrainSerializer::reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params)
	{
		mTextureCache.clear();
		mNormalTextureCahe.clear();
		mReloadingHeightCache.clear();
		mReloadingNormalCache.clear();
		mReloadingBlendCache.clear();
		mReloadingLayerCache.clear();
		mReloadingPartitionCache.clear();
		mReload = true;
		if(params[BTString("NORMAL_ONLY")].isValid())
			mLoadNormalOnly = params[BTString("NORMAL_ONLY")];
		bool result =  this->loadData(stream, mName, mHeight, mSize);
		if (result)
			this->copyAABBList(resource);
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TerrainSerializer::reprocessResource(IResource* resource)
	{
		assert(mReload);
		this->TerrainSerializer::postProcessResource(resource);
		return true;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	TerrainSerializer::loadData(const HSTREAM& stream,TString& name,scalar& base,size_t& size,
		int16** height/* = NULL*/, uint8** normal/* = NULL*/, Color::RGBA** blend/* = NULL*/, TERRAIN_LAYER** layer/* = NULL*/)
	{
		IOBuffer buffer(stream);

		const TileResouceHeader* header;
		buffer.readPtr(header);

		if( header->magic.fourcc != TERRAIN_MAGIC )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("stream type mismatch.") );

		mHeight =base = header->base;
		mSize = size = header->size;
		mTextureCache.clear();
		mNormalTextureCahe.clear();
		mTextures.clear();
		name = stream->getName();

		const TerrainResourceSection* resSection;
		size_t count = (size+1)*(size+1);

		while( !buffer.eof() )
		{
			buffer.readPtr(resSection);
			buffer.seekBack();
			switch( resSection->magic.fourcc )
			{
			case TEXTURE_MAGIC:
				{
					//BLADE_LW_PROFILING(TEXTURE);
					const TextureSection* section = NULL;
					buffer.readPtr(section);
					for( uint32 texit = 0 ; texit < section->textureCount; ++texit)
					{
						TString texName;
						buffer.readTString(texName);
						mTextureCache.push_back(texName);			

						if( !mLoadNormalOnly )
						{
							HIMAGE hImg = TerrainTextureManager::getSingleton().prepareSubTexture(texName, TERRAIN_PIXELFORMAT);
							mTextures.push_back(hImg);

							if( ITerrainConfigManager::getSingleton().getTextureQuality() == TerrainConsts::TEXTURE_QUALITY_HIGH )
							{
								const TString normalTexName = TerrainUtility::getNormalTextureName(texName);
								mNormalTextureCahe.push_back(normalTexName);
								hImg = TerrainTextureManager::getSingleton().prepareSubTexture( normalTexName, TERRAIN_NORMAL_FORMAT);
								mNormalTextures.push_back(hImg);
							}
						}
					}
				}
				break;
			case HEIGHTBLOCK_MAGIC:
				{
					//BLADE_LW_PROFILING(HEIGHT_BUFFER);
					const HeightDataSection* section = NULL;
					buffer.readPtr(section);
					if( mLoadNormalOnly )
					{
						buffer.seekForward(section->bytes);
						break;
					}

					assert( section->bytes == count * sizeof(int16) );
					//new data
					if( height != NULL )
						*height = TERRAIN_RES_ALLOCT(int16,count);
					else
					{
						//reload
						if( mReloadingHeightCache.reserve( section->bytes ) )
						{
							int16* int16data = (int16*)mReloadingHeightCache.getData();
							height = &int16data;
						}
						else
							BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("Buffer allocation failed.") );
					}
					buffer.readData(*height, section->bytes);
				}
				break;
			case NORMALBLOCK_MAGIC:
				{
					//BLADE_LW_PROFILING(NORMAL_BUFFER);
					const NormalDataSection* section = NULL;
					buffer.readPtr(section);
					assert( section->bytes == count*sizeof(int8)*3 );

					if( normal != NULL )
						*normal = TERRAIN_RES_ALLOCT(uint8, count*3 );
					else
					{
						//reload
						if( mReloadingNormalCache.reserve( section->bytes ) )
						{
							uint8* int8data = (uint8*)mReloadingNormalCache.getData();
							normal = &int8data;
						}
						else
							BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("Buffer allocation failed.") );
					}
					buffer.readData(*normal, section->bytes );
					if( !TerrainConfigManager::getSingleton().isUseVertexNormal() )
						mNormalImage = TerrainTextureManager::getSingleton().createNormalImage( *normal, size+1);
					else
						mNormalImage = HIMAGE::EMPTY;
				}
				break;
			case BLENDBLOCK_MAGIC:
				{
					//BLADE_LW_PROFILING(BLEND_BUFFER);
					const BlendDataSection* section = NULL;
					buffer.readPtr(section);
					const size_t blendMapSize = TerrainConfigManager::getSingleton().getBlendBufferSize();
					const size_t BlendSize = blendMapSize * blendMapSize;
					assert( section->bytes == BlendSize*sizeof(Color::RGBA)  );
					if( mLoadNormalOnly )
					{
						buffer.seekForward(section->bytes);
						break;
					}

					if(blend != NULL)
						*blend = TERRAIN_RES_ALLOCT(Color::RGBA, BlendSize );
					else
					{
						//reload
						if( mReloadingBlendCache.reserve( section->bytes ) )
						{
							Color::RGBA* cdata = (Color::RGBA*)mReloadingBlendCache.getData();
							blend = &cdata;
						}
						else
							BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("Buffer allocation failed.") );
					}

					buffer.readData(*blend, section->bytes);
					mBlendImage = TerrainTextureManager::getSingleton().createBlendImage( (uint8*)*blend );
				}
				break;
			case LAYERBLOCK_MAGIC:
				{
					//BLADE_LW_PROFILING(LAYER_BUFFER);
					const LayerDataSection* section = NULL;
					buffer.readPtr(section);
					assert( section->bytes == count*sizeof(TERRAIN_LAYER) );
					if( mLoadNormalOnly )
					{
						buffer.seekForward(section->bytes);
						break;
					}

					if (mPreConvertedLayer != NULL)
						BLADE_TMP_FREE(mPreConvertedLayer);
					mPreConvertedLayer = BLADE_TMP_ALLOCT(TERRAIN_LAYER, count);

					if( layer != NULL )
						*layer = TERRAIN_RES_ALLOCT(TERRAIN_LAYER, count);
					else
					{
						//reload
						if( mReloadingLayerCache.reserve( section->bytes ) )
						{
							TERRAIN_LAYER* layerdata = (TERRAIN_LAYER*)mReloadingLayerCache.getData();
							layer = &layerdata;
						}
						else
							BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("Buffer allocation failed.") );
					}
					buffer.readData(*layer, section->bytes );

					TerrainTextureManager::getSingleton().convertLayerBuffer(*layer, mPreConvertedLayer);
				}
				break;
			case PARTITIONBLOCK_MAGIC:
			{
				//TODO: clean up
				const PartitionMaskSection* section = NULL;
				buffer.readPtr(section);
				buffer.seekForward(section->bytes);
			}
			break;
			default:
				{
					BLADE_EXCEPT(EXC_FILE_READ,BTString("unkown data in file."));
				}
			}//switch

		}//while

		if( mLoadNormalOnly )
			return true;

		if(mTextureCache.size() == 0 ) 
		{
			HIMAGE hImg = TerrainTextureManager::getSingleton().prepareSubTexture( TerrainConsts::EMPTY_TEXTURE_FILE, TERRAIN_PIXELFORMAT);
			mTextures.push_back(hImg);
		}

		IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

		//BLADE_LW_PROFILING( VERTEX_BUFFER );
		if( !TerrainConfigManager::getSingleton().isUseVertexNormal() )
		{
			HVBUFFER normalBuffer;
			TerrainBufferManager::getSingleton().createVertexBuffer(manager, mVertexPositionBuffer, normalBuffer,
				mSize, mAABBList, *height, NULL);
		}
		else
			TerrainBufferManager::getSingleton().createVertexBuffer(manager, mVertexPositionBuffer, mVertexNormalBuffer, 
			mSize, mAABBList, *height, *normal);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainSerializer::copyAABBList(IResource* resource)
	{
		size_t count = mAABBList.size();
		TerrainResource* res = static_cast<TerrainResource*>(resource);
		res->mAABBList.resize(count);

		for (size_t i = 0; i < count; ++i)
		{
			res->mAABBList[i].resize(count);
			std::memcpy(&res->mAABBList[i][0], &mAABBList[i][0], count * sizeof(BlockAABB));
		}
	}
	
}//namespace Blade