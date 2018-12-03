/********************************************************************
	created:	2010/04/14
	filename: 	IGraphicsResourceManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IGraphicsResourceManager_h__
#define __Blade_IGraphicsResourceManager_h__
#include <interface/InterfaceSingleton.h>
#include <math/Vector2i.h>
#include "IVertexBuffer.h"
#include "IIndexBuffer.h"
#include "IVertexDeclaration.h"
#include "RenderProperty.h"
#include "ITexture.h"
#include "IShader.h"
#include "IImage.h"

namespace Blade
{
	class Matrix44;

	/** @remark these functions are mapped into device level ,so they're not added into the IGraphicsService  */
	class IGraphicsResourceManager : public InterfaceSingleton<IGraphicsResourceManager>
	{
	public:
		///factory types
		static const BLADE_BASE_API TString TYPE_SOFT;
		static const BLADE_BASE_API TString TYPE_TEMPORARY;

		///global default setting
		static const BLADE_BASE_API IMG_ORIENT DEFAULT_TEXDIR;
	public:
		///config abstraction of different API, such as OGL & MS Direct3D
		typedef struct SGraphicsConfig
		{
			IMG_ORIENT		TextureDir;			///orientation
			PIXEL_ORDER		DesiredOrder;		///desired order for RGB/RGBA

			///default render target format
			PixelFormat		DefaultRTRGB;		///default RGB format for color render target
			PixelFormat		DefaultRTRGBA;		///default RGB format for color render target
			PixelFormat		DefaultRTDepth;		///default depth format for depth/stencil render target
			PixelFormat		DefaultRTHDR;		///default format for HDR render target

			///un compressed texture format
			PixelFormat		ColorR;				///default format for un-compressed R texture
			PixelFormat		ColorRG;			///default format for un-compressed RG texture
			PixelFormat		ColorRGB;			///default format for un-compressed RGB texture
			PixelFormat		ColorRGBA;			///default format for un-compressed RGA texture
			PixelFormat		ColorNormal;		///default format for un-compressed normal map texture

			///compressed format(compressed if possible)
			PixelFormat		CompressedR;		///default format for compressed R texture
			PixelFormat		CompressedRG;		///default format for compressed RG texture
			PixelFormat		CompressedRGB;		///default format for compressed RGB texture
			PixelFormat		CompressedRGBA;		///default format for compressed RGBA texture
			PixelFormat		CompressedNormal;	///defualt format for compressed normal map texture

			///default texture format, maybe compressed
			PixelFormat		DefaultR;			///default format for RG texture, maybe compressed
			PixelFormat		DefaultRG;			///default format for RG texture, maybe compressed
			PixelFormat		DefaultRGB;			///default format for RGB texture, maybe compressed
			PixelFormat		DefaultRGBA;		///default format for RGBA texture, maybe compressed
			PixelFormat		DefaultNormal;		///default format normal map texture, maybe compressed

			TString			TextureExt;			///preferred extension for texture file
			bool			HalfTexel2Pixel;	///whether use half pixel offset to map texel to pixel
			//TODO: more to come..
		}GCONFIG;

	public:
		virtual ~IGraphicsResourceManager()	{}


		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HVBUFFER	createVertexBuffer(const void* data, size_t vertexSize,size_t vertexCount,IGraphicsBuffer::USAGE usage) = 0;

		inline bool		cloneVertexBuffer(HVBUFFER& dest, IVertexBuffer& src, IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_NONE)
		{
			size_t bytes = src.getVertexCount()*src.getVertexSize();
			if( dest != NULL )
			{
				if( dest->getVertexCount()*dest->getVertexSize() != bytes )
				{
					assert(false);
					return false;
				}

				const void* srcBuffer = src.lock(IGraphicsBuffer::GBLF_READONLY);
				void* destBuffer = dest->lock(IGraphicsBuffer::GBLF_WRITEONLY);
				std::memcpy(destBuffer, srcBuffer, bytes);
				dest->unlock();
				src.unlock();
			}
			else
			{
				const void* srcBuffer = src.lock(IGraphicsBuffer::GBLF_READONLY);
				dest = this->createVertexBuffer(srcBuffer, src.getVertexSize(), src.getVertexCount(), usage != IGraphicsBuffer::GBU_NONE ? usage : src.getUsage());
				src.unlock();
			}	
			return true;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HIBUFFER	createIndexBuffer(const void* data, IIndexBuffer::EIndexType indexType, size_t indexCount, IGraphicsBuffer::USAGE usage) = 0;

		inline bool		cloneIndexBuffer(HIBUFFER& dest, IIndexBuffer& src, IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_NONE)
		{
			size_t bytes = src.getIndexSize()*src.getIndexCount();
			if( dest != NULL )
			{
				if( dest->getIndexSize()*dest->getIndexCount() != bytes )
				{
					assert(false);
					return false;
				}

				const void* srcBuffer = src.lock(IGraphicsBuffer::GBLF_READONLY);
				void* destBuffer = dest->lock(IGraphicsBuffer::GBLF_NORMAL);
				std::memcpy(destBuffer, srcBuffer, bytes);
				dest->unlock();
				src.unlock();
			}
			else
			{
				const void* srcBuffer = src.lock(IGraphicsBuffer::GBLF_READONLY);
				dest = this->createIndexBuffer(srcBuffer, src.getIndexType(), src.getIndexCount(), usage != IGraphicsBuffer::GBU_NONE ? usage : src.getUsage());
				src.unlock();
			}		
			return true;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HVDECL		createVertexDeclaration() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual HTEXTURE	createDepthStencilBuffer(size_t width, size_t height, PixelFormat format,IGraphicsBuffer::ACCESS GpuAccess = IGraphicsBuffer::GBAF_WRITE) = 0;

		/**
		@describe create texture with empty data
		@param
		@return
		*/
		virtual HTEXTURE	createTexture(size_t width, size_t height, PixelFormat format, int mipLevels = -1, IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_DEFAULT,
			ETextureType type = TT_2DIM, IMG_ORIENT layout = IMGO_DEFAULT, size_t depth = 1) = 0;

		/**
		@describe for 2D texture loading from file,\n
		using GBU_STATIC (to create D3D11_USAGE_DEFAULT or D3D11_USAGE_IMMUTABLE texture
		@param 
		@return 
		*/
		virtual HTEXTURE	createTexture(const IImage& image, int mipMaps = IImage::MIPLOD_DEFAULT, IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_STATIC) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		updateTexture(const IImage& image, ITexture& texture,
			const POINT2I& srcPos = POINT2I::ZERO, const POINT2I& destPos = POINT2I::ZERO, const SIZE2I& size = SIZE2I::ZERO) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t		createShaders(const HSHADERINTMEDIA* interMedias, HSHADER* outShaders, size_t count = 1) = 0;

		/**
		@describe create shader media from a compiled data
		@param
		@return
		*/
		virtual HSHADERINTMEDIA	createShaderIntermediate(const TString& name, const void* data, size_t bytes, 
			const TString& profile, const TString entrypoint[SHADER_TYPE_COUNT]) = 0;

		/**
		@describe this compiling process could be called multi-threaded.
		@param
		@return
		*/
		virtual HSHADERINTMEDIA	compileShader(const TString& textFileFullPath, const ParamList& option,
			const void* shaderTextBuffer, size_t shaderTextLen, const ParamList* macros = NULL) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const TStringList&	getSupportedProfiles() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const TString&		getHighestProfile() const = 0;

		/**
		@describe get the size in bytes of the shader constant type
		@note the size is API related
		@param 
		@return 
		*/
		virtual size_t				getShaderConstantSize(SHADER_CONSTANT_TYPE type) const = 0;

		/**
		@describe convert projection matrix into platform/API matrix
		@param 
		@return 
		*/
		virtual Matrix44&		convertProjectionMatrix(Matrix44& projectMatrix) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const GCONFIG&	getGraphicsConfig() const = 0;


	};//class IGraphicsResourceManager

	extern template class BLADE_BASE_API Factory<IGraphicsResourceManager>;
	
}//namespace Blade


#endif //__Blade_IGraphicsResourceManager_h__