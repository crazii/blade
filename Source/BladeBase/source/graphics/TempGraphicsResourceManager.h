/********************************************************************
	created:	2013/05/20
	filename: 	TempGraphicsResourceManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <BladeException.h>
#include "DefaultVertexBuffer.h"
#include "DefaultIndexBuffer.h"
#include "DefaultVertexDeclaration.h"

namespace Blade
{
	/* @remark the default GraphicsBufferManager create system memory buffers */

	class TempGraphicsResourceManager : public IGraphicsResourceManager,public Singleton<TempGraphicsResourceManager>
	{
	public:
		TempGraphicsResourceManager(){}
		~TempGraphicsResourceManager(){}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const
		{
			return IGraphicsResourceManager::TYPE_TEMPORARY;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HVBUFFER	createVertexBuffer(const void* data, size_t vertexSize,size_t vertexCount,IGraphicsBuffer::USAGE usage)
		{
			return HVBUFFER(BLADE_NEW DefaultVertexBuffer(data, vertexSize, vertexCount, usage, Memory::getTemporaryPool()));
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HIBUFFER	createIndexBuffer(const void* data, IIndexBuffer::EIndexType indexType,size_t indexCount,IGraphicsBuffer::USAGE usage)
		{
			return HIBUFFER(BLADE_NEW DefaultIndexBuffer(data, indexType, indexCount, usage, Memory::getTemporaryPool()));
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HVDECL		createVertexDeclaration()
		{
			return HVDECL(BLADE_NEW DefaultVertexDeclaration());
		}

		/*
		@describe
		@param
		@return
		*/
		virtual HTEXTURE	createDepthStencilBuffer(size_t width, size_t height, PixelFormat format,IGraphicsBuffer::ACCESS GpuAccess = IGraphicsBuffer::GBAF_WRITE)
		{
			BLADE_UNREFERENCED(width);
			BLADE_UNREFERENCED(height);
			BLADE_UNREFERENCED(format);
			BLADE_UNREFERENCED(GpuAccess);

			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual HTEXTURE	createTexture(size_t width, size_t height, PixelFormat format,
			int mipLevels = -1, IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_DEFAULT,
			ETextureType type = TT_2DIM, IMG_ORIENT layout = IMGO_DEFAULT, size_t depth = 1)
		{
			BLADE_UNREFERENCED(width);
			BLADE_UNREFERENCED(height);
			BLADE_UNREFERENCED(mipLevels);

			BLADE_UNREFERENCED(layout);
			BLADE_UNREFERENCED(type);
			BLADE_UNREFERENCED(depth);

			BLADE_UNREFERENCED(format);
			BLADE_UNREFERENCED(usage);
			
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe for 2D texture loading from file,\n
		using GBU_STATIC (to create D3D11_USAGE_DEFAULT or D3D11_USAGE_IMMUTABLE texture
		@param 
		@return 
		*/
		virtual HTEXTURE	createTexture(const IImage& image, int mipMaps = IImage::MIPLOD_DEFAULT, IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_STATIC)
		{
			BLADE_UNREFERENCED(image);
			BLADE_UNREFERENCED(mipMaps);
			BLADE_UNREFERENCED(usage);
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		updateTexture(const IImage& image, ITexture& texture,
			const POINT2I& srcPos = POINT2I::ZERO, const POINT2I& destPos = POINT2I::ZERO, const SIZE2I& size = SIZE2I::ZERO) const
		{
			BLADE_UNREFERENCED(image);
			BLADE_UNREFERENCED(texture);
			BLADE_UNREFERENCED(srcPos);
			BLADE_UNREFERENCED(destPos);
			BLADE_UNREFERENCED(size);
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual size_t		createShaders(const HSHADERINTMEDIA* interMedias, HSHADER* outShaders, size_t count = 1)
		{
			BLADE_UNREFERENCED(interMedias);
			BLADE_UNREFERENCED(outShaders);
			BLADE_UNREFERENCED(count);
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual HSHADERINTMEDIA	createShaderIntermediate(const TString& name, const void* data, size_t bytes,
			const TString& profile, const TString entrypoint[SHADER_TYPE_COUNT])
		{
			BLADE_UNREFERENCED(name);
			BLADE_UNREFERENCED(data);
			BLADE_UNREFERENCED(bytes);
			BLADE_UNREFERENCED(profile);
			BLADE_UNREFERENCED(entrypoint);
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual HSHADERINTMEDIA	compileShader(const TString& textFileFullPath, const ParamList& option,
			const void* shaderTextBuffer, size_t shaderTextLen, const ParamList* macros = NULL) const
		{
			BLADE_UNREFERENCED(textFileFullPath);
			BLADE_UNREFERENCED(option);
			BLADE_UNREFERENCED(shaderTextBuffer);
			BLADE_UNREFERENCED(shaderTextLen);
			BLADE_UNREFERENCED(macros);
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const TStringList&	getSupportedProfiles() const
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const TString&		getHighestProfile() const
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe get the size in bytes of the shader constant type
		@note the size is API related
		@param 
		@return 
		*/
		virtual size_t				getShaderConstantSize(SHADER_CONSTANT_TYPE type) const
		{
			BLADE_UNREFERENCED(type);
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe convert projection matrix into platform/API matrix
		@param 
		@return 
		*/
		virtual Matrix44&		convertProjectionMatrix(Matrix44& projectMatrix) const
		{
			BLADE_UNREFERENCED(projectMatrix);
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const GCONFIG&	getGraphicsConfig() const
		{
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
		}

	};//class TempGraphicsResourceManager
	
}//namespace Blade