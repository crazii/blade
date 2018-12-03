/********************************************************************
	created:	2012/04/02
	filename: 	TerrainTextureManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainTextureManager.h"

#include <utility/Profiling.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>

#include <interface/IResourceManager.h>
#include <interface/IEnvironmentManager.h>
#include <interface/public/graphics/PixelColorFormatter.h>

#include "TerrainConfigManager.h"

namespace Blade
{
	static const int UNCOMPRESSED_PIXEL_SIZE = 4;
	static const int PIXELCOUNT_PER_BLOCK = 4;	//pixel count by side (one block has 4x4 pixels)

	//////////////////////////////////////////////////////////////////////////
	TerrainTextureManager::TerrainTextureManager()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainTextureManager::~TerrainTextureManager()
	{
	}


	//////////////////////////////////////////////////////////////////////////
	TerrainAtlas*	TerrainTextureManager::buildTerrainAtlas(const TStringParam& textureList, PixelFormat format) const
	{
		TempVector<HIMAGE> textures;	
		textures.reserve( textureList.size() );
		for(size_t i = 0; i < textureList.size(); ++i)
		{
			const TString& texture = textureList[i];
			HIMAGE hImage = this->prepareSubTexture(texture, format);
			textures.push_back(hImage);
		}

		if( textureList.size() == 0 )
			return this->buildTerrainAtlas(textureList, NULL, format);
		else
			return this->buildTerrainAtlas( textureList, &textures[0], format);
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainAtlas*	TerrainTextureManager::buildTerrainAtlas(const TStringParam& textureList, const HIMAGE* images, PixelFormat format) const
	{
		const size_t AtlasTextureCount = ITerrainConfigManager::getSingleton().getTextureCount();
		const size_t textureSize = ITerrainConfigManager::getSingleton().getTextureSize();
		const size_t minMipSize = ITerrainConfigManager::getSingleton().getMinMipSize()*AtlasTextureCount;
		const size_t atlasSize = AtlasTextureCount*textureSize;
		if( AtlasTextureCount < 2 || !Math::IsPowerOf2( (uint)atlasSize) || !Math::IsPowerOf2( (uint)textureSize) )
		{
			assert(false);
			return NULL;
		}

		size_t mipCount = Math::CalcMaxMipCount( atlasSize );
		mipCount -= Math::CalcMaxMipCount(minMipSize) - 1;

		assert( mipCount >= 1 );
		HTEXTURE hTexture;
		hTexture = IGraphicsResourceManager::getSingleton().createTexture(atlasSize, atlasSize, format, (int)mipCount);
		if( hTexture == NULL )
			return NULL;

		const size_t textureCount = textureList.size();

		TerrainAtlas* atlas = BLADE_NEW TerrainAtlas(atlasSize, textureSize);
		atlas->setAtlasTexture(hTexture);
		atlas->setMaxMipLevel( mipCount-1 );	//0-mipLevel-1 totally count:mipLevel
		if( textureCount == 0 )
			return atlas;

		IMG_ORIENT layout = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir;
		HIMAGE compositeImg = IImageManager::getSingleton().createImage(atlasSize, atlasSize, IP_TEMPORARY, format, layout, (int)mipCount);

		//BLADE_LW_PROFILING( UPDATE );
		size_t textureIndex = 0;
		size_t minImageMip = mipCount;
		bool ret = false;
		for(size_t i = 0; i < atlas->getSubTextureCountPerSide(); ++i)
		{
			for(size_t j = 0; j < atlas->getSubTextureCountPerSide(); ++j)
			{
				const HIMAGE& hImage = images[textureIndex];
				const TString& texture = textureList[textureIndex];
				ret = atlas->addTexture( texture );
				assert( ret );
				minImageMip = std::min(minImageMip, (size_t)hImage->getMipmapCount() );

				//copy sub image into composite image
				size_t compressionRate = compositeImg->getImageWidth() / compositeImg->getWidth();
				POINT2I dest( int(j*textureSize/compressionRate), int(i*textureSize/compressionRate));
				for(size_t k = 0; k < minImageMip; ++k)
				{
					const IMG_MIPINFO* info = compositeImg->getMipInfo(k);
					const IMG_MIPINFO* srcInfo = hImage->getMipInfo(k);
					char* data = (char*)info->data + dest.y * info->rowPitch + dest.x * format.getSizeBytes();
					const char* src = (char*)srcInfo->data;
					for(size_t line = 0; line < srcInfo->height; ++line)
					{
						::memcpy(data, src, srcInfo->rowPitch);
						data += info->rowPitch;
						src += srcInfo->rowPitch;
					}
					dest /= 2;
				}

				if( ++textureIndex >= textureCount )
					break;
			}

			if( textureIndex >= textureCount )
				break;
		}

		IGraphicsResourceManager::getSingleton().updateTexture(*compositeImg, *hTexture);

#if 0
		if( ret && minImageMip < hTexture->getMipmapCount() )
		{
			textureIndex = 0;
			for(size_t i = textureRowIndex; i < atlas->getSubTextureCountPerSide(); ++i)
			{
				index_t colStart = i == textureRowIndex ? textureColIndex : 0;
				for(size_t j = colStart; j < atlas->getSubTextureCountPerSide(); ++j)
				{
					Box3i box( (int)(j*textureSize), int(i*textureSize), int(j*textureSize+textureSize), int(i*textureSize+textureSize) );
					hTexture->updateMipMap(box);

					if( ++textureIndex >= textureCount )
						break;
				}
				if( ++textureIndex >= textureCount )
					break;
			}
		}
#endif
		return atlas;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainTextureManager::updateTerrainAtlas(TerrainAtlas* atlas, const TStringParam& textureList, const HIMAGE* images) const
	{
		if( atlas == NULL )
		{
			assert(false);
			return false;
		}

		const size_t totalTextureSize = ITerrainConfigManager::getSingleton().getTextureSize()*ITerrainConfigManager::getSingleton().getTextureCount();
		if( atlas->getAtlasSize() != totalTextureSize )
		{
			assert(false);
			return false;
		}

		const TStringList& existingList = atlas->getTextureList();
		size_t existingCount = existingList.size();

		size_t newCount = textureList.size();
		for(size_t i = 0; i < existingCount; ++i)
		{
			if( i >= textureList.size() )
				break;

			if( textureList[i] == existingList[i] )
				continue;
/*			LAYER_ID id = atlas->findTexture( textureList[i] );
			if( id != INVALID_LAYER_ID )
			{
				assert( id > i );
				this->swapAtlasSubTexture(atlas, (LAYER_ID)i, id);
			}
			else */if( !this->setAtlasSubTexture(atlas, LAYER_ID(i), images[i], textureList[i]) )
				assert(false);
		}

		TStringParam addingList;
		for(size_t i = existingCount; i < newCount; ++i)
			addingList.push_back( textureList[i] );
		this->addTextureToAtlas(atlas, images+existingCount, addingList );

		const HTEXTURE& hTexture = atlas->getAtlasTexture();
		size_t pitch;
		uint8* atlasData = (uint8*)hTexture->lock(pitch, IGraphicsBuffer::GBLF_NO_DIRTY_UPDATE);
		for(size_t i = newCount; i < existingCount; ++i)
		{
			//clear texture content
			size_t textureSize = atlas->getSubTextureSize();
			size_t srcRowStride = 0;
			size_t height = textureSize;
#if TERRAIN_TEXTURE_COMPRESSED
			height /= PIXELCOUNT_PER_BLOCK;
#endif
			const size_t destRowStride = pitch;
			const size_t textureRowIndex = i / atlas->getSubTextureCountPerSide();
			const size_t textureColIndex = i % atlas->getSubTextureCountPerSide();

			uint8* subTexture = atlasData + height*destRowStride*textureRowIndex + textureColIndex*srcRowStride;
			for(size_t row = 0; row < height; ++row )
			{
				std::memset(subTexture,0, destRowStride);
				subTexture += destRowStride;
			}

			Box3i box( int(textureColIndex*textureSize), int(textureRowIndex*textureSize),
				int(textureColIndex*textureSize+textureSize), int(textureRowIndex*textureSize+textureSize) );
			hTexture->addDirtyRegion(box);


			atlas->removeLastTexture();
		}
		hTexture->unlock();

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE	TerrainTextureManager::prepareSubTexture(const TString& path, PixelFormat format) const
	{
		const size_t textureSize = ITerrainConfigManager::getSingleton().getTextureSize();
		//note: terrain UV is fixed as top-down (no V flip as mesh/model),
		//so image should always loaded as top-down
		IMG_ORIENT texDir = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir;
		HSTREAM stream = IResourceManager::getSingleton().loadStream(path + TEXT(".")
			+ IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureExt );
		
		HIMAGE hImage= IImageManager::getSingleton().loadImage( stream, IP_TEMPORARY, format, IImage::MIPLOD_MAX, texDir);
		if( hImage == NULL || hImage->getMipInfo(0)->oriWidth != textureSize || hImage->getMipInfo(0)->oriHeight != textureSize )
		{
			assert(false);
			return HIMAGE::EMPTY;
		}

		size_t pixelSize = format.getSizeBytes();
		if( format.isCompressed() )
			this->fixAtlasBorderPixelCompressed(hImage, textureSize, pixelSize);
		else
			this->fixAtlasBorderPixelRGBA(hImage, textureSize, pixelSize);
		return hImage;
	}

	//////////////////////////////////////////////////////////////////////////
	void*			TerrainTextureManager::getImageData(const HIMAGE& hImage, size_t& pitch, size_t& width, size_t& height, size_t mipLODLevel/* = 0*/) const
	{
		if( hImage == NULL )
		{
			assert(false);
			return NULL;
		}
		if( mipLODLevel >= hImage->getMipmapCount() )
		{
			assert(false);
			return NULL;
		}
		pitch = hImage->getMipInfo(mipLODLevel)->rowPitch;
		width = hImage->getMipInfo(mipLODLevel)->width;
		height = hImage->getMipInfo(mipLODLevel)->height;
		return hImage->getMipInfo(mipLODLevel)->data;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainTextureManager::addTextureToAtlas(TerrainAtlas* atlas, const HIMAGE* images, const TStringParam& textureList) const
	{
		if( atlas == NULL || textureList.size() == 0)
			return false;

		const HTEXTURE& hTexture = atlas->getAtlasTexture();
		if( hTexture == NULL )
			return false;

		size_t textureSize = atlas->getSubTextureSize();
		const size_t textureCount = textureList.size();
		const size_t currentCount = atlas->getSubTextureCount();
		const size_t textureRowIndex = currentCount / atlas->getSubTextureCountPerSide();
		const size_t textureColIndex = currentCount % atlas->getSubTextureCountPerSide();
		//atlasData += textureSize*destRowStride*textureRowIndex + textureColIndex*textureSize*pixelSize;

		size_t textureIndex = 0;
		bool ret = true;
		size_t minImageMip = size_t(-1);
		for(size_t i = textureRowIndex; i < atlas->getSubTextureCountPerSide(); ++i)
		{
			index_t colStart = i == textureRowIndex ? textureColIndex : 0;
			for(size_t j = colStart; j < atlas->getSubTextureCountPerSide(); ++j)
			{
				const TString& texture = textureList[textureIndex];
				const HIMAGE& hImage = images[textureIndex];
				minImageMip = std::min( minImageMip, (size_t)hImage->getMipmapCount() );

				ret = atlas->addTexture( texture );
				if( ret )
					this->updateAtlasTexture(hTexture, hImage, j ,i ,textureSize);
				else
				{
					assert(false);					
					break;
				}

				if( ++textureIndex >= textureCount )
					break;
			}
			if( ++textureIndex >= textureCount )
				break;
		}

#if 0
		if( ret && minImageMip < hTexture->getMipmapCount() )
		{
			textureIndex = 0;
			for(size_t i = textureRowIndex; i < atlas->getSubTextureCountPerSide(); ++i)
			{
				index_t colStart = i == textureRowIndex ? textureColIndex : 0;
				for(size_t j = colStart; j < atlas->getSubTextureCountPerSide(); ++j)
				{
					Box3i box( (int)(j*textureSize), int(i*textureSize), int(j*textureSize+textureSize), int(i*textureSize+textureSize) );
					hTexture->updateMipMap(box);

					if( ++textureIndex >= textureCount )
						break;
				}
				if( ++textureIndex >= textureCount )
					break;
			}
		}
#endif
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainTextureManager::addTextureToAtlas(TerrainAtlas* atlas, const TStringParam& textureList) const
	{
		TempVector<HIMAGE> imageList;
		for(size_t i = 0; i < textureList.size(); ++i)
			imageList.push_back( this->prepareSubTexture(textureList[i], atlas->getAtlasTexture()->getPixelFormat()) );
		return this->addTextureToAtlas(atlas, &imageList[0], textureList);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainTextureManager::swapAtlasSubTexture(TerrainAtlas* atlas,LAYER_ID index0, LAYER_ID index1) const
	{
		if( atlas == NULL || atlas->getSubTextureCount() <= index0 || atlas->getSubTextureCount() <= index1)
		{
			assert(false);
			return false;
		}

		const HTEXTURE& hTexture = atlas->getAtlasTexture();
		size_t textureSize = atlas->getSubTextureSize();
		size_t srcRowStride = 0;
		size_t pitch;
		uint8* atlasData = (uint8*)hTexture->lock(pitch, IGraphicsBuffer::GBLF_NO_DIRTY_UPDATE);

		size_t height = textureSize;
#if TERRAIN_TEXTURE_COMPRESSED
		height /= PIXELCOUNT_PER_BLOCK;
#endif
		const size_t destRowStride = pitch;
		const size_t textureRowIndex0 = index0 / atlas->getSubTextureCountPerSide();
		const size_t textureColIndex0 = index0 % atlas->getSubTextureCountPerSide();
		const size_t textureRowIndex1 = index1 / atlas->getSubTextureCountPerSide();
		const size_t textureColIndex1 = index1 % atlas->getSubTextureCountPerSide();

		uint8* subTexture0 = atlasData + height*destRowStride*textureRowIndex0 + textureColIndex0*srcRowStride;
		uint8* subTexture1 = atlasData + height*destRowStride*textureRowIndex1 + textureColIndex1*srcRowStride;
		TempBuffer buffer;
		buffer.reserve(destRowStride);
		for(size_t row = 0; row < height; ++row )
		{
			std::memcpy(buffer.getData(), subTexture0, destRowStride);
			std::memcpy( subTexture0, subTexture1, destRowStride );
			std::memcpy( subTexture0, buffer.getData(), destRowStride );
			subTexture0 += destRowStride;
			subTexture1 += destRowStride;
		}
		buffer.clear();

		Box3i box0( int(textureColIndex0*textureSize), int(textureRowIndex0*textureSize), int(textureColIndex0*textureSize+textureSize), int(textureRowIndex0*textureSize+textureSize) );
		Box3i box1( int(textureColIndex1*textureSize), int(textureRowIndex1*textureSize), int(textureColIndex1*textureSize+textureSize), int(textureRowIndex1*textureSize+textureSize) );
		hTexture->addDirtyRegion(box0);
		hTexture->addDirtyRegion(box1);
		hTexture->unlock();
		hTexture->updateMipMap(box0);
		hTexture->updateMipMap(box1);

		atlas->swapTexture(index0,index1);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainTextureManager::setAtlasSubTexture(TerrainAtlas* atlas,LAYER_ID index,const HIMAGE& hImage, const TString& imageName) const
	{
		if( atlas == NULL || (atlas->getSubTextureCount() <= index && index != 0) )
		{
			assert(false);
			return false;
		}

		const HTEXTURE& hTexture = atlas->getAtlasTexture();
		size_t textureSize = atlas->getSubTextureSize();
		const size_t textureColIndex = index / atlas->getSubTextureCountPerSide();
		const size_t textureRowIndex = index % atlas->getSubTextureCountPerSide();

		bool ret = atlas->setTexture(index, imageName);
		if( ret )
			this->updateAtlasTexture(hTexture, hImage, textureRowIndex, textureColIndex, textureSize);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		TerrainTextureManager::createBlendTexture(const HIMAGE& blendImage) const
	{
		size_t blendSize = blendImage->getImageWidth();
		assert( blendSize == blendImage->getImageHeight() );

		HTEXTURE hBlendTexture;
		PixelFormat format = TERRAIN_BLEND_FORMAT;
		if(	format.hasByteOrder() )
			format.setByteOrder( IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder );
		format.setLinearColor(true);

		//check the real time mode
		if( TerrainConfigManager::getSingleton().isEditingModeEnabled() )
		{
			//IGraphicsBuffer::USAGE usage = (IGraphicsBuffer::GBU_DYNAMIC);
			IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_DEFAULT;	//test shows dynamic is slower. probably because blade's d3d9 texture hold a copy of data to handle device lost
			hBlendTexture = IGraphicsResourceManager::getSingleton().createTexture(blendSize, blendSize, format, 1, usage);
		}
		else
		{
			//auto Mip-map may not supported on texture format, TODO check format support
			//IGraphicsBuffer::USAGE usage = (IGraphicsBuffer::USAGE)(IGraphicsBuffer::GBU_DEFAULT | IGraphicsBuffer::GBUF_AUTOMIPMAP);
			IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_DEFAULT;
			hBlendTexture = IGraphicsResourceManager::getSingleton().createTexture(blendSize, blendSize, format, IImage::MIPLOD_MAX, usage);
		}
		bool ret = IGraphicsResourceManager::getSingleton().updateTexture( *blendImage, *hBlendTexture );
		if(!ret)
			assert(false);
		hBlendTexture->setLinearColorSpace(true);
		return hBlendTexture;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE	TerrainTextureManager::createBlendImage(const uint8* blendBuffer) const
	{
		if( blendBuffer == NULL )
		{
			assert(false);
			return HIMAGE::EMPTY;
		}

		size_t blendSize = TerrainConfigManager::getSingleton().getBlendBufferSize();
		if( blendSize != TerrainConfigManager::getSingleton().getBlendBufferSize())
		{
			assert(false);
			return HIMAGE::EMPTY;
		}

		//note: terrain UV is fixed as top-down (no V flip as mesh/model),
		//so image should always loaded as top-down
		IMG_ORIENT imageDir = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir;
		HIMAGE hBlendImage = IImageManager::getSingleton().createImage( blendSize, blendSize, IP_TEMPORARY, PF_R8G8B8A8, imageDir, 1);

		uint8* dest = hBlendImage->getBits();
		size_t pitch = hBlendImage->getPitch();
		size_t BytesPerPixel = UNCOMPRESSED_PIXEL_SIZE;
		size_t rowBytes = blendSize*BytesPerPixel;

		uint8* rowBuffer = (uint8*)BLADE_TMP_ALLOC(rowBytes);
		for(size_t i = 0; i < blendSize; ++i )
		{
			//compress 4 channels into 3, ready for DXT1 compression if any
			std::memcpy(rowBuffer,blendBuffer, rowBytes);
			for(size_t j = 0; j < blendSize; ++j)
			{
				uint8* color = rowBuffer + j * BytesPerPixel;
				scalar sum = (scalar)color[0] + (scalar)color[1] + (scalar)color[2] + (scalar)color[3];
				color[Color::A] = (uint8)((scalar)(color[Color::A])/sum*0xFF);
				color[Color::G] = (uint8)((scalar)(color[Color::G])/sum*0xFF);
				color[Color::B] = (uint8)((scalar)(color[Color::B])/sum*0xFF);
				color[Color::R] = 0xFFu - color[Color::A] - color[Color::G] - color[Color::B];
				color[Color::A] = 0xFFu;
			}
			std::memcpy(dest, rowBuffer, rowBytes);
			blendBuffer += rowBytes;
			dest += pitch;
		}
		BLADE_TMP_FREE(rowBuffer);

		int mipMaps;
		if( TerrainConfigManager::getSingleton().isEditingModeEnabled() )
			mipMaps = 1;
		else
			mipMaps = IImage::MIPLOD_MAX;

		scalar scale = ITerrainConfigManager::getSingleton().getBlendNormalScale();
		int size = (int)(scalar(blendSize)*scale);
		if( size != (int)blendSize )
			hBlendImage = IImageManager::getSingleton().scaleImage( hBlendImage, SIZE2I(size, size), IP_TEMPORARY );

		PixelFormat destFormat = TERRAIN_BLEND_FORMAT;
		if(	destFormat.hasByteOrder() )
			destFormat.setByteOrder( IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder );
		if( destFormat != hBlendImage->getFormat() )
			return IImageManager::getSingleton().convertToFormat(hBlendImage, destFormat, IP_TEMPORARY, mipMaps);
		else
			return hBlendImage;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainTextureManager::updateBlockBlendTexture(const HTEXTURE& hBlendTexture, const TerrainBlockBlendData& blockBlendData,const uint8* tileBlendBuffer) const
	{
		const size_t BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();
		const size_t btSize = hBlendTexture->getWidth();
		const size_t BlendBytes = sizeof(Color::COLOR);
		const size_t rate = TerrainConfigManager::getSingleton().getBlendBufferSize() / TerrainConfigManager::getSingleton().getTerrainTileSize();
		size_t blendPitch = btSize*BlendBytes;
		size_t dest_pitch = 0;
		uint8* blendPixelBuffer = (uint8*)hBlendTexture->lock(dest_pitch, IGraphicsBuffer::GBLF_NO_DIRTY_UPDATE);

		for(size_t i = 0; i < blockBlendData.getCount(); ++i )
		{
			const SingleBlockBlendData& blendData = blockBlendData.getData(i);

			size_t blockX = blendData.getBlockIndex().mX;
			size_t blockZ = blendData.getBlockIndex().mZ;

			size_t StartX = blendData.getX();
			size_t StartZ = blendData.getZ();
			size_t SizeX = blendData.getSizeX();
			size_t SizeZ = blendData.getSizeZ();

			size_t blendx = (blockX*BlockSize+StartX)*rate;
			size_t blendz = (blockZ*BlockSize+StartZ)*rate;
			size_t btSizeX = SizeX*rate;
			size_t btSizeZ = SizeZ*rate;

			//because S3 Block Compress (DXTn) methods is per 4x4 texel
			//we need align the rect to 4 
			blendx = blendx/PIXELCOUNT_PER_BLOCK*PIXELCOUNT_PER_BLOCK;
			blendz = blendz/PIXELCOUNT_PER_BLOCK*PIXELCOUNT_PER_BLOCK;
			btSizeX = (blendx + btSizeX + PIXELCOUNT_PER_BLOCK-1)/PIXELCOUNT_PER_BLOCK*PIXELCOUNT_PER_BLOCK - blendx;
			btSizeZ = (blendz + btSizeZ + PIXELCOUNT_PER_BLOCK-1)/PIXELCOUNT_PER_BLOCK*PIXELCOUNT_PER_BLOCK - blendz;

			//note: terrain UV is fixed as top-down (no V flip as mesh/model),
			//so image should always loaded as top-down
			IMG_ORIENT imageDir = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir;
			HIMAGE hBlockImage = IImageManager::getSingleton().createImage(btSizeX, btSizeZ, IP_TEMPORARY, PF_R8G8B8A8, imageDir, 1);
			assert( hBlockImage->getPitch() == btSizeX*BlendBytes );

			uint8* dest = hBlockImage->getBits();

			const uint8* src = tileBlendBuffer + (blendx + blendz*btSize)*BlendBytes;
			size_t rowBytes = btSizeX*BlendBytes;

			uint8* rowBuffer = (uint8*)BLADE_TMP_ALLOC(rowBytes);
			for( size_t j = 0; j < btSizeZ; ++j )
			{
				std::memcpy(rowBuffer, src, rowBytes);
				for(size_t k = 0; k < btSizeX; ++k)
				{
					uint8* color = rowBuffer + k * BlendBytes;
					scalar sum = (scalar)color[0] + (scalar)color[1] + (scalar)color[2] + (scalar)color[3];
					color[Color::A] = (uint8)((scalar)(color[Color::A])/sum*0xFF);
					color[Color::G] = (uint8)((scalar)(color[Color::G])/sum*0xFF);
					color[Color::B] = (uint8)((scalar)(color[Color::B])/sum*0xFF);
					color[Color::R] = 0xFFu - color[Color::A] - color[Color::G] - color[Color::B];
					color[Color::A] = 0xFFu;
				}
				std::memcpy(dest, rowBuffer, rowBytes);
				dest += rowBytes;
				src += blendPitch;
			}
			BLADE_TMP_FREE(rowBuffer);

			//1 LOD level is enough
			PixelFormat destFormat = TERRAIN_BLEND_FORMAT;
			if(	destFormat.hasByteOrder() )
				destFormat.setByteOrder( IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder );
			HIMAGE hCompressedImage = IImageManager::getSingleton().convertToFormat(hBlockImage, destFormat, IP_TEMPORARY, 1);
			size_t src_pitch = 0;
			size_t width = 0,height = 0;
			src = (uint8*)this->getImageData(hCompressedImage, src_pitch, width, height);
			assert(src != NULL);
			if( destFormat.isCompressed() )
			{
				assert(height == btSizeZ/PIXELCOUNT_PER_BLOCK && src_pitch == btSizeX/PIXELCOUNT_PER_BLOCK*destFormat.getSizeBytes() );
				dest = blendPixelBuffer + (blendx/PIXELCOUNT_PER_BLOCK)*destFormat.getSizeBytes() + (blendz/PIXELCOUNT_PER_BLOCK)*dest_pitch;
			}
			else
			{
				assert(height == btSizeZ && src_pitch == btSizeX*destFormat.getSizeBytes() );
				dest = blendPixelBuffer + blendx*destFormat.getSizeBytes() + blendz*dest_pitch;
			}
			assert( blendx + width <= TerrainConfigManager::getSingleton().getBlendBufferSize() );
			assert( blendz + height <= TerrainConfigManager::getSingleton().getBlendBufferSize() );

			assert( blendx + btSizeX <= TerrainConfigManager::getSingleton().getBlendBufferSize() );
			assert( blendz + btSizeZ <= TerrainConfigManager::getSingleton().getBlendBufferSize() );

			for( size_t j = 0; j < height; ++j )
			{
				std::memcpy(dest, src, src_pitch );
				dest += dest_pitch;
				src += src_pitch;
			}

			Box3i box( (int)blendx, (int)blendz, (int)(blendx+btSizeX), (int)(blendz+btSizeZ) );
			hBlendTexture->addDirtyRegion(box);
		}
		hBlendTexture->unlock();
		return true;

		////DEBUG: full lock to assure no bugs here, so that we can judge where bugs come from.
		//const size_t BlendSize = TerrainConfigManager::getSingleton().getBlendBufferSize();
		//HIMAGE img = IImageManager::getSingleton().createImage(BlendSize, BlendSize, IP_TEMPORARY, TERRAIN_BLEND_FORMAT, 
		//	IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir, 1);

		//uint8* data = img->getBits();
		//const Color::RGBA* blendColor = (const Color::RGBA*)tileBlendBuffer;
		//for(size_t i = 0; i < BlendSize; ++i)
		//{
		//	for(size_t j = 0; j < BlendSize; ++j)
		//		ColorFormatter::writeColorLDR(data + j*TERRAIN_BLEND_FORMAT.getSizeBytes(), blendColor[i*BlendSize+j], TERRAIN_BLEND_FORMAT);
		//	data += BlendSize*TERRAIN_BLEND_FORMAT.getSizeBytes();
		//}
		//IGraphicsResourceManager::getSingleton().updateTexture(*img, *hBlendTexture);
		//return true;
	}


	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		TerrainTextureManager::createNormalTexture(const HIMAGE& normalImage) const
	{
		size_t normalSize = normalImage->getImageWidth();
		assert( normalSize == normalImage->getImageHeight() );

		PixelFormat format = TERRAIN_NORMAL_FORMAT;
		if(	format.hasByteOrder() )
			format.setByteOrder( IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder );
		format.setLinearColor(true);

		//check the real time mode
		HTEXTURE hNormalTexture;

		if( TerrainConfigManager::getSingleton().isEditingModeEnabled() )
		{
			IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_DEFAULT;
			hNormalTexture = IGraphicsResourceManager::getSingleton().createTexture(normalSize, normalSize, format, 1, usage);
		}
		else
		{
			IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_DEFAULT;
			//note: use mipmap for normal map will have problem for GLES on some android devices
			hNormalTexture = IGraphicsResourceManager::getSingleton().createTexture(normalSize, normalSize, format, IImage::MIPLOD_MAX, usage);
		}

		bool ret = IGraphicsResourceManager::getSingleton().updateTexture( *normalImage, *hNormalTexture);
		if(!ret)
			assert(false);
		return hNormalTexture;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE	TerrainTextureManager::createNormalImage(const uint8* normalBuffer, size_t normalSize) const
	{
		const size_t stride = 3;
		//note: terrain UV is fixed as top-down (no V flip as mesh/model),
		//so image should always loaded as top-down
		IMG_ORIENT imageDir = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir;
		HIMAGE hImage = IImageManager::getSingleton().createImage( normalSize, normalSize, IP_TEMPORARY, PF_R8G8B8, imageDir, 1);

		uint8* dest = hImage->getBits();
		{
			size_t pitch = hImage->getPitch();
			size_t bytesPerPixel = hImage->getBytesPerUnit();
			for(size_t i = 0; i < normalSize; ++i )
			{
				for(size_t n = 0; n < normalSize; ++n)
				{
					*(dest + n*bytesPerPixel + Color::R)	= *(normalBuffer + n*stride + 0);	//x
					//*(dest + n*bytesPerPixel + Color::G)	= *(normalBuffer + n*stride + 1);
					//*(dest + n*bytesPerPixel + Color::B)	= *(normalBuffer + n*stride + 2);
					//normal map uses R,G channel (x,y,z) swap to (x,z,y) to use x,z.
					//if store x,y and reconstruct z, z will lost sign,
					//reconstruct y because y is always positive.

					//yz channels are swizzled in shader
					//an alternative method is to always use TNB matrix instead of TBN, and always reconstruct y from tangent space normal map B channel (for all shaders)
					//in that way swizzle is not needed anymore.
					*(dest + n*bytesPerPixel + Color::B)	= *(normalBuffer + n*stride + 1);	//y
					*(dest + n*bytesPerPixel + Color::G)	= *(normalBuffer + n*stride + 2);	//z
				}
				normalBuffer += normalSize * stride;
				dest += pitch;
			}
		}

		int LOD = IImage::MIPLOD_MAX;
		if( ITerrainConfigManager::getSingleton().isEditingModeEnabled() )
			LOD = 1;
		//BLADE_LW_PROFILING(NORMAL_COMPRESS);

		scalar scale = ITerrainConfigManager::getSingleton().getBlendNormalScale();
		scalar textureSize = (scalar)TerrainConfigManager::getSingleton().getTextureSize();
		int size = (int)(textureSize*scale);
		if( size != (int)normalSize )
			hImage = IImageManager::getSingleton().scaleImage(hImage, SIZE2I(size,size), IP_TEMPORARY, IImage::SF_BILINEAR);

		return IImageManager::getSingleton().convertToFormat(hImage, TERRAIN_NORMAL_FORMAT, IP_TEMPORARY, LOD);
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		TerrainTextureManager::createLayerTexture(const TERRAIN_LAYER* layerBuffer, bool convert/* = true*/) const
	{
		size_t layerSize = TerrainConfigManager::getSingleton().getTerrainInfo().mTileSize;
		//check the real time mode
		HTEXTURE hLayerTexture;
		IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_DEFAULT;
		PixelFormat pf = PF_B4G4R4A4;
		pf.setLinearColor(true);	//layer id is not color
		hLayerTexture = IGraphicsResourceManager::getSingleton().createTexture(layerSize, layerSize, pf, 1, usage);
		if( hLayerTexture == NULL )
		{
			assert(false);
			return hLayerTexture;
		}

		bool ret = this->updateLayerTexture(hLayerTexture, layerBuffer, convert);
		assert(ret);
		BLADE_UNREFERENCED(ret);
		return hLayerTexture;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainTextureManager::updateLayerTexture(const HTEXTURE& layerMap, const TERRAIN_LAYER* layerBuffer, bool convert/* = true*/) const
	{
		PixelFormat format = PF_B4G4R4A4;
		if( format.hasByteOrder() )
			format.setByteOrder( IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder );

		size_t layerSize = TerrainConfigManager::getSingleton().getTerrainInfo().mTileSize;
		size_t pitch;
		char* data = (char*)layerMap->lock(pitch,IGraphicsBuffer::GBLF_WRITEONLY);
		assert( pitch >= layerSize*sizeof(TERRAIN_LAYER) );

		if (convert)
		{
			for (size_t i = 0; i < layerSize; ++i)
			{
				for (size_t j = 0; j < layerSize; ++j)
				{
					//convert to BGRA4444
					Color::RGBA rgba(0xFFu * layerBuffer[j].mAtlasIndex0 / 0xFu, 0xFFu * layerBuffer[j].mAtlasIndex1 / 0xFu, 0xFFu * layerBuffer[j].mAtlasIndex2 / 0xFu, 0xFFu * layerBuffer[j].mAtlasIndex3 / 0xFu);
					ColorFormatter::writeColorLDR(((TERRAIN_LAYER*)data) + j, rgba, format);
				}
				data += pitch;
				layerBuffer += layerSize;
			}
		}
		else
		{
			assert(sizeof(TERRAIN_LAYER) == format.getSizeBytes());
			if (layerSize * sizeof(TERRAIN_LAYER) == pitch)
				std::memcpy(data, layerBuffer, pitch*layerSize);
			else
			{
				for (size_t i = 0; i < layerSize; ++i)
				{
					std::memcpy(data, layerBuffer, layerSize * sizeof(TERRAIN_LAYER));
					data += pitch;
					layerBuffer += layerSize;
				}
			}
		}

		layerMap->unlock();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainTextureManager::convertLayerBuffer(const TERRAIN_LAYER* layerBuffer, TERRAIN_LAYER* layer)
	{
		PixelFormat format = PF_B4G4R4A4;
		if (format.hasByteOrder())
			format.setByteOrder(IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder);

		assert(sizeof(TERRAIN_LAYER) == format.getSizeBytes());
		size_t layerSize = TerrainConfigManager::getSingleton().getTerrainInfo().mTileSize;

		for (size_t i = 0; i < layerSize; ++i)
		{
			for (size_t j = 0; j < layerSize; ++j)
			{
				//convert to BGRA4444
				Color::RGBA rgba(0xFFu * layerBuffer[j].mAtlasIndex0 / 0xFu, 0xFFu * layerBuffer[j].mAtlasIndex1 / 0xFu, 0xFFu * layerBuffer[j].mAtlasIndex2 / 0xFu, 0xFFu * layerBuffer[j].mAtlasIndex3 / 0xFu);
				ColorFormatter::writeColorLDR(layer + j, rgba, format);
			}
			layer += layerSize;
			layerBuffer += layerSize;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainTextureManager::updateBlockLayerTexture(const HTEXTURE& layerMap, const TerrainBlockBlendData& blockBlendData, const TERRAIN_LAYER* layerData) const
	{
		const size_t TerrainSize = TerrainConfigManager::getSingleton().getTerrainTileSize();
		const size_t BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();
		assert(layerMap->getWidth() == TerrainSize);
		assert( 2 == sizeof(TERRAIN_LAYER) );//R4G4B4A4 = 16bit

		size_t dest_pitch = 0;
		TERRAIN_LAYER* layerPixelBuffer = (TERRAIN_LAYER*)layerMap->lock(dest_pitch, IGraphicsBuffer::GBLF_NO_DIRTY_UPDATE);

		for(size_t i = 0; i < blockBlendData.getCount(); ++i )
		{
			const SingleBlockBlendData& blendData = blockBlendData.getData(i);

			size_t blockX = blendData.getBlockIndex().mX;
			size_t blockZ = blendData.getBlockIndex().mZ;

			size_t StartX = blendData.getX();
			size_t StartZ = blendData.getZ();
			size_t SizeX = blendData.getSizeX();
			size_t SizeZ = blendData.getSizeZ();

			size_t layerX = (blockX*BlockSize+StartX);
			size_t layerZ = (blockZ*BlockSize+StartZ);
			size_t btSizeX = SizeX;
			size_t btSizeZ = SizeZ;

			//RGBAConverter converter = IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder;

			TERRAIN_LAYER* dest = layerPixelBuffer + (layerX) + layerZ*TerrainSize;
			const TERRAIN_LAYER* src = layerData + (layerX + layerZ*TerrainSize);
			for( size_t j = 0; j < btSizeZ; ++j )
			{
				for(size_t k = 0; k < btSizeX; ++k)
				{
					//convert to BGRA4444
					Color::RGBA rgba(0xFFu*src[k].mAtlasIndex0/0xFu, 0xFFu*src[k].mAtlasIndex1/0xFu, 0xFFu*src[k].mAtlasIndex2/0xFu, 0xFFu*src[k].mAtlasIndex3/0xFu);
					ColorFormatter::writeColorLDR(&dest[k], rgba, PF_B4G4R4A4);
				}
				dest += dest_pitch/sizeof(TERRAIN_LAYER);
				src += TerrainSize;
			}
			Box3i box( (int)layerX, (int)layerZ, (int)(layerX+btSizeX), (int)(layerZ+btSizeZ) );
			layerMap->addDirtyRegion(box);
		}
		layerMap->unlock();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TerrainTextureManager::fixAtlasBorderPixelRGBA(const HIMAGE& hSrcImage, size_t subTextureSize, size_t pixelSize, scalar /*pixelRate*//* = 1.0f/128.f*/) const
	{
		size_t pixelCount = PIXELCOUNT_PER_BLOCK*2;
		const size_t stepSize = 1;
		const size_t channelCount = hSrcImage->getFormat().getSizeBytes();

		for(size_t mip = 0; mip < hSrcImage->getMipmapCount() && subTextureSize >= pixelCount*2; ++mip, subTextureSize /= 2)
		{
			uint8* data = hSrcImage->getMipInfo(mip)->data;
			size_t pitch = hSrcImage->getMipInfo(mip)->rowPitch;

			{
				//top <=> bottom
				uint8* bottom = data + pitch*(subTextureSize-1);
				uint8* top = data;

				for(size_t i = 0; i < pixelCount; ++i )
				{
					Color::UColor* top_color = (Color::UColor*)top;
					Color::UColor* bottom_color = (Color::UColor*)bottom;
					size_t step = i / stepSize;
					scalar rate = scalar(step) * 1.0f / scalar(pixelCount/stepSize);

					for(size_t j = 0; j < subTextureSize; ++j)
					{
						Color::UColor& top_ucolor = *(top_color+j);
						Color::UColor& bottom_ucolor = *(bottom_color+j);

						for(size_t c = 0; c < channelCount; ++c)
						{
							//scalar lerp1 = top_ucolor.component[c]*rate + bottom_ucolor.component[c]*(1-rate);
							scalar lerp2 = top_ucolor.component[c]*(1-rate) + bottom_ucolor.component[c]*rate;
							//top_ucolor.component[c] = (uint8)lerp1;
							bottom_ucolor.component[c] = (uint8)lerp2;
						}					
					}

					bottom -= pitch;
					top += pitch;
				}
			}

			{
				//left <=> right
				uint8* right = data + (subTextureSize-1)*pixelSize;
				uint8* left = data;

				for(size_t row = 0; row < subTextureSize; ++row )
				{
					for(size_t col = 0; col < pixelCount; ++col)
					{
						Color::UColor& left_ucolor = *( (Color::UColor*)left+col);
						Color::UColor& right_ucolor = *( (Color::UColor*)right-col);
						size_t step = col / stepSize;
						scalar rate = scalar(step) * 1.0f / scalar(pixelCount/stepSize);

						for(size_t c = 0; c < channelCount; ++c)
						{
							//scalar lerp1 = left_ucolor.component[c]*rate + right_ucolor.component[c]*(1-rate);
							scalar lerp2 = left_ucolor.component[c]*(1-rate) + right_ucolor.component[c]*rate;
							//left_ucolor.component[c] = (uint8)lerp1;
							right_ucolor.component[c] = (uint8)lerp2;
						}
					}

					right += pitch;
					left += pitch;
				}
			}
		}//for
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTextureManager::fixAtlasBorderPixelCompressed(const HIMAGE& hSrcImage, size_t subTextureSize, size_t pixelSizeBytes, scalar /*pixelRate*//* = 1.0f/128.f*/) const
	{
#if 0
		const size_t pixelCount = PIXELCOUNT_PER_BLOCK*4;
		const size_t stepSize = PIXELCOUNT_PER_BLOCK;
#else
		const size_t pixelCount = PIXELCOUNT_PER_BLOCK*2;
		const size_t stepSize = 1;
#endif
		PixelFormat targetFormat = hSrcImage->getFormat();
		assert(targetFormat.isCompressed());

		size_t width = size_t(-1),height = size_t(-1);
		for(size_t mip = 0; mip < hSrcImage->getMipmapCount() && width >= pixelCount*2/PIXELCOUNT_PER_BLOCK; ++mip, subTextureSize /= 2)
		{
			size_t texturePitch = 0;
			uint8* subTextureData = (uint8*)this->getImageData( hSrcImage, texturePitch, width, height, mip);
			assert(width == height);

			PixelFormat format = PF_R8G8B8A8;
			format.setByteOrder( IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder );
			uint8* data = (uint8*)BLADE_TMP_ALIGN_ALLOC(subTextureSize * subTextureSize * UNCOMPRESSED_PIXEL_SIZE, IImage::IMAGE_ALIGNMENT );
			//uncompressed image pitch
			size_t pitch = subTextureSize*UNCOMPRESSED_PIXEL_SIZE;
			size_t blockSize = pixelSizeBytes;
			size_t pixelSize = UNCOMPRESSED_PIXEL_SIZE;
			size_t blockCount = subTextureSize/PIXELCOUNT_PER_BLOCK;

			{
				//BLADE_LW_PROFILING( DECOMPRESS );
				//decompress border horizontal
				for(size_t i = 0; i < blockCount; ++i)
				{
					//top
					const uint8* topComressed = subTextureData + blockSize * i;//compressed source
					uint8* topRGBA = data + pixelSize * i * PIXELCOUNT_PER_BLOCK;//uncompressed dest

					//bottom
					const uint8* bottomComressed = topComressed + texturePitch*(blockCount-1);
					uint8* bottomRGBA = topRGBA + pitch*(subTextureSize-PIXELCOUNT_PER_BLOCK);

					for(size_t j = 0; j < pixelCount/PIXELCOUNT_PER_BLOCK; ++j)
					{
						bool ret = IImageManager::getSingleton().decompressBlock(topRGBA, topComressed, subTextureSize, format, targetFormat);
						assert(ret);
						ret = IImageManager::getSingleton().decompressBlock(bottomRGBA, bottomComressed, subTextureSize, format, targetFormat);
						assert(ret);
						topComressed += texturePitch;
						bottomComressed -= texturePitch;
						topRGBA += pitch*PIXELCOUNT_PER_BLOCK;
						bottomRGBA -= pitch*PIXELCOUNT_PER_BLOCK;
					}
				}

				//decompress border vertical
				for(size_t i = 0; i < blockCount; ++i)
				{
					//left
					const uint8* leftCompressed = subTextureData + texturePitch*i;
					uint8* leftRGBA = data + pitch * i * PIXELCOUNT_PER_BLOCK;
					//right
					const uint8* rightCompressed = leftCompressed + (blockCount-1)*blockSize;
					uint8* rightRGBA = leftRGBA + (subTextureSize-PIXELCOUNT_PER_BLOCK)*pixelSize;

					for(size_t j = 0; j < pixelCount/PIXELCOUNT_PER_BLOCK; ++j)
					{
						bool ret = IImageManager::getSingleton().decompressBlock(leftRGBA, leftCompressed, subTextureSize, format, targetFormat);
						assert(ret);
						ret = IImageManager::getSingleton().decompressBlock(rightRGBA, rightCompressed, subTextureSize, format, targetFormat);
						assert(ret);

						leftCompressed += blockSize;
						rightCompressed -= blockSize;
						leftRGBA += pixelSize*PIXELCOUNT_PER_BLOCK;
						rightRGBA -= pixelSize*PIXELCOUNT_PER_BLOCK;
					}
				}
			}

			{
				//BLADE_LW_PROFILING( BLEND );
				//top <=> bottom
				uint8* bottom = data + pitch*(subTextureSize-1);
				uint8* top = data;

				for(size_t i = 0; i < pixelCount; ++i )
				{
					Color::RGBA* top_color = (Color::RGBA*)top;
					Color::RGBA* bottom_color = (Color::RGBA*)bottom;
					size_t step = i / stepSize;
					//scalar rate = 0.5f + scalar(step) * 0.5f / scalar(pixelCount/stepSize);
					scalar rate = scalar(step) * 1.0f / scalar(pixelCount/stepSize);

					for(size_t j = 0; j < subTextureSize; ++j)
					{
						Color::RGBA& top_ucolor = *(top_color+j);
						Color::RGBA& bottom_ucolor = *(bottom_color+j);

						if( targetFormat == TERRAIN_NORMAL_FORMAT )
						{
							//blend on normal vector & re-normalize normal
							Color c1(top_ucolor),c2(bottom_ucolor);
							c1 *= scalar(255.0/128.0); c1 -= 1;	//color => vector
							c2 *= scalar(255.0/128.0); c2 -= 1;

							Color c = c1*(1-rate) + c2*rate;
							//normalize
							scalar length = std::sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
							c /= length;
							c += 1;
							c /= scalar(255.0/128.0);
							bottom_ucolor = c;
						}
						else
						{
							for(size_t c = 0; c < 4; ++c)
							{
								//scalar lerp1 = top_ucolor.component[c]*rate + bottom_ucolor.component[c]*(1-rate);
								scalar lerp2 = top_ucolor.component[c]*(1-rate) + bottom_ucolor.component[c]*rate;
								//top_ucolor.component[c] = (uint8)lerp1;
								bottom_ucolor.component[c] = (uint8)lerp2;
							}
						}
					}

					bottom -= pitch;
					top += pitch;
				}

				//left <=> right
				uint8* right = data + (subTextureSize-1)*pixelSize;
				uint8* left = data;

				for(size_t row = 0; row < subTextureSize; ++row )
				{
					for(size_t col = 0; col < pixelCount; ++col)
					{
						Color::RGBA& left_ucolor = *( (Color::RGBA*)left+col);
						Color::RGBA& right_ucolor = *( (Color::RGBA*)right-col);
						size_t step = col / stepSize;
						//scalar rate = 0.5f + scalar(step) * 0.5f / scalar(pixelCount/stepSize);
						scalar rate = scalar(step) * 1.0f / scalar(pixelCount/stepSize);

						if( targetFormat == TERRAIN_NORMAL_FORMAT )
						{
							//blend on normal vector & re-normalize normal
							Color c1(left_ucolor),c2(right_ucolor);
							c1 *= scalar(255.0/128.0); c1 -= 1;	//color => vector
							c2 *= scalar(255.0/128.0); c2 -= 1;

							Color c = c1*(1-rate) + c2*rate;
							//normalize
							scalar length = std::sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
							c /= length;
							c += 1;
							c /= scalar(255.0/128.0);
							right_ucolor = c;
						}
						else
						{
							for(size_t c = 0; c < 4; ++c)
							{
								//scalar lerp1 = left_ucolor.component[c]*rate + right_ucolor.component[c]*(1-rate);
								scalar lerp2 = left_ucolor.component[c]*(1-rate) + right_ucolor.component[c]*rate;
								//left_ucolor.component[c] = (uint8)lerp1;
								right_ucolor.component[c] = (uint8)lerp2;
							}
						}
					}

					right += pitch;
					left += pitch;
				}
			}

			//compress data back
			{
				//BLADE_LW_PROFILING( RECOMPRESS );
				//re-compress border horizontal
				for(size_t i = 0; i < blockCount; ++i)
				{
					//top
					uint8* topComressed = subTextureData + blockSize * i;//compressed source
					const uint8* topRGBA = data + pixelSize * i * PIXELCOUNT_PER_BLOCK;//uncompressed dest

					//bottom
					uint8* bottomComressed = topComressed+ texturePitch*(blockCount-1);
					const uint8* bottomRGBA = topRGBA + pitch*(subTextureSize-PIXELCOUNT_PER_BLOCK);

					for(size_t j = 0; j < pixelCount/PIXELCOUNT_PER_BLOCK; ++j)
					{
						bool ret = IImageManager::getSingleton().compressBlock(topComressed, topRGBA, subTextureSize, format, targetFormat);
						assert(ret);
						ret = IImageManager::getSingleton().compressBlock(bottomComressed, bottomRGBA, subTextureSize, format, targetFormat);
						assert(ret);
						topComressed += texturePitch;
						bottomComressed -= texturePitch;
						topRGBA += pitch*PIXELCOUNT_PER_BLOCK;
						bottomRGBA -= pitch*PIXELCOUNT_PER_BLOCK;
					}
				}

				//re-compress border vertical
				for(size_t i = 0; i < blockCount; ++i)
				{
					//left
					uint8* leftCompressed = subTextureData + texturePitch*i;
					const uint8* leftRGBA = data + pitch * i * PIXELCOUNT_PER_BLOCK;
					//right
					uint8* rightCompressed = leftCompressed + (blockCount-1)*blockSize;
					const uint8* rightRGBA = leftRGBA + (subTextureSize-PIXELCOUNT_PER_BLOCK)*pixelSize;

					for(size_t j = 0; j < pixelCount/PIXELCOUNT_PER_BLOCK; ++j)
					{
						bool ret = IImageManager::getSingleton().compressBlock(leftCompressed, leftRGBA, subTextureSize, format, targetFormat);
						assert(ret);
						ret = IImageManager::getSingleton().compressBlock(rightCompressed, rightRGBA, subTextureSize, format, targetFormat);
						assert(ret);

						leftCompressed += blockSize;
						rightCompressed -= blockSize;
						leftRGBA += pixelSize*PIXELCOUNT_PER_BLOCK;
						rightRGBA -= pixelSize*PIXELCOUNT_PER_BLOCK;
					}
				}
			}
			BLADE_TMP_ALIGN_FREE(data);
		}//for
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TerrainTextureManager::updateAtlasTexture(const HTEXTURE& hTexture, const HIMAGE& hSrcImage,
		size_t subX, size_t subY, size_t subTextureSize) const
	{
		//source
		size_t srcPitch = 0;
		size_t width = 0,height = 0;
		uint8* srcData = (uint8*)this->getImageData( hSrcImage, srcPitch, width, height);
		if( !(srcData != NULL && width == height/* && width == subTextureSize*/) )
		{
			assert(false);
			return false;
		}
		const size_t pixelSize = hTexture->getPixelFormat().getSizeBytes();
		assert( srcPitch == width*pixelSize );
		BLADE_UNREFERENCED(pixelSize);

		POINT2I dest( int(subX*subTextureSize), int(subY*subTextureSize));
		SIZE2I size( (int)subTextureSize, (int)subTextureSize);
		bool ret = IGraphicsResourceManager::getSingleton().updateTexture( *hSrcImage, *hTexture, POINT2I::ZERO, dest, size);
		assert(ret);
		return ret;
	}

}//namespace Blade