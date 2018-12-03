/********************************************************************
	created:	2010/04/14
	filename: 	D3D9GraphicsResourceManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9Header.h>
#include <graphics/windows/D3D9/D3D9TypeConverter.h>
#include <graphics/windows/D3DShaderIntermediate.h>

#include <graphics/windows/D3D9/D3D9GraphicsResourceManager.h>
#include <graphics/windows/D3D9/D3D9IndexBuffer.h>
#include <graphics/windows/D3D9/D3D9VertexBuffer.h>
#include <graphics/windows/D3D9/D3D9PixelBuffer.h>
#include <graphics/windows/D3D9/D3D9Texture.h>
#include <graphics/windows/D3D9/D3D9VolumeTexture.h>
#include <graphics/windows/D3D9/D3D9CubeTexture.h>
#include <graphics/windows/D3D9/D3D9RenderSurface.h>
#include <graphics/windows/D3D9/D3D9Shader.h>
#include <graphics/windows/SoftwareShader.h>
#include <graphics/windows/D3D9/ID3D9Helper.h>
#include <graphics/windows/D3D9/D3DXIncludeLoader.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)		//while(true)
#endif

namespace Blade
{
	const TString D3D9GraphicsResourceManager::TYPE = BTString("D3D9");

#define VALIDATE_DEVICE()	do{ if(mDevice==NULL){BLADE_EXCEPT(EXC_NEXIST,BTString("render device not present."));} }while(false)

	D3D9GraphicsResourceManager::D3D9GraphicsResourceManager()
		:mDevice(NULL)
		,mRenderResourceReleased(false)
	{
		mConfig.TextureDir = IMGO_TOP_DOWN;
		mConfig.DesiredOrder = PO_BGRA;

		mConfig.DefaultRTRGB = PF_RGB;
		mConfig.DefaultRTRGBA = PF_XRGB;
		mConfig.DefaultRTDepth = PF_DEPTH24S8;
		mConfig.DefaultRTHDR = PF_F16X4;

		mConfig.ColorR = PF_R8;
		mConfig.ColorRG = PF_R8G8;
		mConfig.ColorRGB = PF_ARGB;
		mConfig.ColorRGBA = PF_ARGB;
		mConfig.ColorNormal = PF_ARGB;
		//mConfig.ColorNormal = PF_R8G8;	//dx9 doesn't suppport r8g8

		mConfig.CompressedR = PF_BC4;
		mConfig.CompressedRG = PF_BC5;
		mConfig.CompressedRGB = PF_BC1;
		mConfig.CompressedRGBA = PF_BC3;
		mConfig.CompressedNormal = PF_BC5;

		mConfig.DefaultR = mConfig.CompressedR;
		mConfig.DefaultRG = mConfig.CompressedRG;
		mConfig.DefaultRGB = mConfig.CompressedRGB;
		mConfig.DefaultRGBA = mConfig.CompressedRGBA;
		mConfig.DefaultNormal = PF_BC5;
		mConfig.TextureExt = BTString("dds");

		//normal need to be linear format
		mConfig.DefaultNormal.setLinearColor(true);
		mConfig.ColorNormal.setLinearColor(true);
		mConfig.CompressedNormal.setLinearColor(true);

		mConfig.HalfTexel2Pixel = true;
	}

	D3D9GraphicsResourceManager::~D3D9GraphicsResourceManager()
	{
		this->releaseDefaultResource();
	}

	/************************************************************************/
	/* IGraphicsResourceManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HVBUFFER	D3D9GraphicsResourceManager::createVertexBuffer(const void* data, size_t vertexSize,size_t vertexCount,IGraphicsBuffer::USAGE usage)
	{
		VALIDATE_DEVICE();

		D3DPOOL pool =(usage & (IGraphicsBuffer::GBUF_DYNAMIC | IGraphicsBuffer::GBUF_CPU_READ)) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
		IDirect3DVertexBuffer9* VB = NULL;
		size_t bytes = vertexSize*vertexCount;
		HRESULT result = mDevice->CreateVertexBuffer((UINT)bytes,D3DUsageConverter(usage),0,pool,&VB,NULL);
		if( FAILED(result) )
			BLADE_EXCEPT(EXC_UNKNOWN, BTString("unable to create vertex buffer."));

		return HVBUFFER(BLADE_NEW D3D9VertexBuffer(VB, data, vertexSize, vertexCount, usage));
	}

	//////////////////////////////////////////////////////////////////////////
	HIBUFFER	D3D9GraphicsResourceManager::createIndexBuffer(const void* data, IIndexBuffer::EIndexType indexType,size_t indexCount,IGraphicsBuffer::USAGE usage)
	{
		VALIDATE_DEVICE();

		D3DPOOL pool = (usage & (IGraphicsBuffer::GBUF_DYNAMIC | IGraphicsBuffer::GBUF_CPU_READ))? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
		size_t indexSize = indexType == IIndexBuffer::IT_16BIT?2u:4u;
		IDirect3DIndexBuffer9* IB = NULL;
		size_t bytes = indexCount*indexSize;
		HRESULT result = mDevice->CreateIndexBuffer((UINT)bytes,D3DUsageConverter(usage),D3DFormatConverter(indexType),pool,&IB,NULL);
		if( FAILED(result) )
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to create indexbuffer.") );
		return HIBUFFER( BLADE_NEW D3D9IndexBuffer(IB, data, indexType, indexCount, usage) );
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE	D3D9GraphicsResourceManager::createDepthStencilBuffer(size_t width, size_t height, PixelFormat format,IGraphicsBuffer::ACCESS GpuAccess/* = IGraphicsBuffer::GBAF_WRITE*/)
	{
		VALIDATE_DEVICE();
		//create depth stencil
		D3DFORMAT D3DFormat = D3DFormatConverter(format);

		if( !format.isDepth() || D3DFormat == D3DFMT_UNKNOWN )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid depth format.") );
		else
		{
			//check format support?
		}

		if( GpuAccess & IGraphicsBuffer::GBAF_READ )
		{
			LPDIRECT3DTEXTURE9 pTexture = NULL;
			D3DFORMAT fmt;

			//note: D24S8 / D16 can only be used as shadow map in d3d9
			if( format.isStencil() )	//D24S8 shadow map. TODO: add explicit shadow format? cannot distinguish INTZ from D24X8 shadow map, if input is D24S8 with read access
										//update: shadow map has no stencil, use INTZ if stencil exist
				fmt = (D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z');
			else if( format == PF_DEPTH16 )
				fmt = D3DFormatConverter(format);	//D16 is OK
			else if (format.getSizeBytes() == 4 /*&& !format.isFloat()*/)
			{
				assert(format == PF_DEPTH32);
				fmt = D3DFMT_D24X8;	//most possible accepted fourcc,both by NV and ATI
			}
			else if( format.getSizeBytes() == 3 )
				fmt = (D3DFORMAT)MAKEFOURCC('D','F','2','4');	//try ATI fourcc
			else if( format.getSizeBytes() == 2 )
				fmt = (D3DFORMAT)MAKEFOURCC('D','F','1','6');	//try ATI fourcc
			else
				BLADE_EXCEPT(EXC_API_ERROR,BTString("invalid depth stencil format.") );

			if( FAILED(mDevice->CreateTexture((UINT)width,(UINT)height,1,D3DUSAGE_DEPTHSTENCIL,fmt,D3DPOOL_DEFAULT,&pTexture,NULL)) )
				BLADE_EXCEPT(EXC_API_ERROR,BTString("falied to create depth stencil texture.") );

			D3D9Texture* tex = BLADE_NEW D3D9Texture(pTexture, IGraphicsBuffer::GBAF_NONE, mConfig.TextureDir);
			tex->shareSampler(Texture::DEFAULT_DEPTH_RTT_SAMPLER());
			return HTEXTURE(tex);
		}
		else
		{
			D3DFORMAT fmt = D3DFormatConverter(format);
			LPDIRECT3DSURFACE9 pSurface = NULL;
			if( FAILED(mDevice->CreateDepthStencilSurface( (UINT)width, (UINT)height, fmt, D3DMULTISAMPLE_NONE, 0, FALSE, &pSurface, NULL) ) )
				BLADE_EXCEPT(EXC_API_ERROR,BTString("falied to create depth stencil texture.") );
			D3D9PixelBuffer* pxBuffer = BLADE_NEW D3D9PixelBuffer(pSurface, D3D9PixelBuffer::PBL_RENDERTARGET);
			return HTEXTURE(BLADE_NEW D3D9RenderSurface(pxBuffer));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE	D3D9GraphicsResourceManager::createTexture(size_t width, size_t height, PixelFormat format,
		int mipLevels/* = -1*/, IGraphicsBuffer::USAGE usage/* = IGraphicsBuffer::GBU_DEFAULT*/,
		ETextureType type/* = TT_2DIM*/, IMG_ORIENT layout/* = IMGO_DEFAULT*/, size_t depth/* = 1*/)
	{
		VALIDATE_DEVICE();
		if( mipLevels < 0 )
			mipLevels = 0;

		if( layout == IMGO_DEFAULT )
			layout = mConfig.TextureDir;

		D3DFORMAT d3dformat;
		if( format == PF_UNDEFINED )
			d3dformat = D3DFormatConverter::getBackBufferFormat();
		else
			d3dformat = D3DFormatConverter(format);

		DWORD d3dusage = D3DUsageConverter(usage);

		D3DPOOL pool = (usage & (IGraphicsBuffer::GBUF_DYNAMIC|IGraphicsBuffer::GBUF_RENDERTARGET|IGraphicsBuffer::GBUF_DEPTHSTENCIL))? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

		ITexture* tex = NULL;

		if( type == TT_1DIM || type == TT_2DIM )
		{
			assert(depth == 1);
			IDirect3DTexture9* pTexture = NULL;
			//D3DUSAGE_WRITEONLY only applicable to vertex buffer
			d3dusage &= ~D3DUSAGE_WRITEONLY;

			HRESULT result = mDevice->CreateTexture((UINT)width, (UINT)height, (UINT)mipLevels, d3dusage,d3dformat,pool,&pTexture,NULL);
			if( FAILED(result) )
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to create texture.") );

			tex = BLADE_NEW D3D9Texture(pTexture, usage.getCPUAccess(), layout);
		}
		else if( type == TT_3DIM )
		{
			IDirect3DVolumeTexture9* pTexture = NULL;
			HRESULT result = mDevice->CreateVolumeTexture( (UINT)width, (UINT)height, (UINT)depth, (UINT)mipLevels, d3dusage, d3dformat, pool, &pTexture, NULL );
			if( FAILED(result) )
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to create volume texture.") );

			tex = BLADE_NEW D3D9VolumeTexture(pTexture, usage.getCPUAccess(), layout);
		}
		else if( type == TT_CUBE )
		{
			assert( depth == 1);
			assert( width == height );
			
			IDirect3DCubeTexture9* pTexture = NULL;
			HRESULT result = mDevice->CreateCubeTexture( (UINT)width, (UINT)mipLevels, d3dusage, d3dformat, pool, &pTexture, NULL );
			if( FAILED(result) )
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("unable to create cube texture.") );
			tex = BLADE_NEW D3D9CubeTexture(pTexture, usage.getCPUAccess(), layout);
		}

		if (usage.isRenderTarget())
		{
			tex->setLinearColorSpace(true);
			if (tex->getPixelFormat().isDepth())
				tex->shareSampler(Texture::DEFAULT_DEPTH_RTT_SAMPLER());
			else
				tex->shareSampler( Texture::DEFAULT_RTT_SAMPLER() );
		}
		return HTEXTURE(tex);
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE	D3D9GraphicsResourceManager::createTexture(const IImage& image, int mipMaps/* = IImage::MIPLOD_DEFAULT*/,  IGraphicsBuffer::USAGE usage/* = IGraphicsBuffer::GBU_STATIC*/)
	{
		//should convert to 32BBP (4byte RGBA) before create call
		if( !image.isCompressed() )
		{
			if( ( image.getBPP() != 32 && image.getBPP() != 16 && image.getBPP() != 8)
				//should convert to the right layout (top-bottom) before create call
				//	|| image.getOrientation() != this->D3D9GraphicsResourceManager::getGraphicsConfig().TextureDir
					|| image.hasByteOrder() && image.getByteOrder() != this->D3D9GraphicsResourceManager::getGraphicsConfig().DesiredOrder
					)
			{
				assert(false);
				return HTEXTURE::EMPTY;
			}
		}
		assert(!usage.isRenderTarget() && !usage.isDepthStencil());

		const IMG_INFO& infoHeader = image.getInfo();
		ETextureType type;
		if( infoHeader.depth != 1)
			type = TT_3DIM;
		else if (infoHeader.faces == 6)
			type = TT_CUBE;
		else
		{
			type = TT_2DIM;
			assert(infoHeader.faces == 1);
		}

		if( mipMaps == IImage::MIPLOD_DEFAULT )
			mipMaps = infoHeader.mipmap;
		else if( mipMaps < 0 )
			usage = usage | IGraphicsBuffer::GBUF_AUTOMIPMAP;

		HTEXTURE texture = this->createTexture(image.getImageWidth(), image.getImageHeight(), infoHeader.format, 
			mipMaps, usage, type, image.getOrientation(), infoHeader.depth);
		
		//update texture ignoring the write access
		if( texture == NULL || !this->forceUpdateTexture(image, *texture) )
		{
			assert(false);
			BLADE_EXCEPT(EXC_API_ERROR,BTString("load texture failed."));
		}
		return texture;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		D3D9GraphicsResourceManager::updateTexture(const IImage& image, ITexture& texture,
		const POINT2I& srcPos/* = POINT2I::ZERO*/, const POINT2I& destPos/* = POINT2I::ZERO*/, const SIZE2I& size/* = SIZE2I::ZERO*/) const
	{
		const ITexture::DESC& desc = texture.getDesc();
		const IMG_INFO& infoHeader = image.getInfo();
		if( !desc.mUsage.isWriteable() )
		{
			assert(false);
			return false;
		}

		//format must match
		if( desc.mPixelFormat != infoHeader.format
			//should convert to the right layout (top-bottom) before create call
			|| image.getOrientation() != this->D3D9GraphicsResourceManager::getGraphicsConfig().TextureDir
			|| image.hasByteOrder() && image.getByteOrder() != this->D3D9GraphicsResourceManager::getGraphicsConfig().DesiredOrder
			)
		{
			//WHY we don't convert it here is because the conversion is time consuming and maybe called during rendering
			//so it's better for user to decide where to convert it, maybe another thread or somewhere else.
			assert(false);
			return false;
		}

		return this->forceUpdateTexture(image, texture, srcPos, destPos, size);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		D3D9GraphicsResourceManager::createShaders(const HSHADERINTMEDIA* interMedias, HSHADER* outShaders, size_t count/* = 1*/)
	{
		if( interMedias == NULL || outShaders == NULL || count == 0)
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid input params.") );

		if( mDevice == NULL )
		{
			for(size_t i = 0; i < count; ++i)
			{
				D3DShaderIntermediate* pIntMedia = static_cast<D3DShaderIntermediate*>( interMedias[i] );
				outShaders[i].bind( BLADE_NEW SoftwareShader(pIntMedia) );
			}
			return count;
		}

		size_t created = 0;
		for(size_t i = 0; i < count; ++i )
		{
			const HSHADERINTMEDIA& interMedia = interMedias[i];
			if( interMedia == NULL || interMedia->getShaderID() != D3DShaderIntermediate::getD3DShaderID() )
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid shader intermediate.") );
			D3DShaderIntermediate* pIntMedia = static_cast<D3DShaderIntermediate*>( interMedia );

			HRESULT hr;
			IDirect3DVertexShader9* pVertexShader = NULL;
			IDirect3DPixelShader9* pPixelShader = NULL;

			TempBuffer& buffer = pIntMedia->getBuffer();
			void* vsBuffer;
			void* psBuffer;
			size_t vsSize,psSize;
			if( !D3D9Shader::readShaderBinary(buffer.getData(), buffer.getSize(), vsBuffer, vsSize, psBuffer, psSize) )
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid shader intermediate content.") );

			//shader shader objects if binary is the same
			for(size_t j = 0; j < created; ++j )
			{
				void* previousVSBuffer;
				void* previousPSBuffer;
				size_t previousVSSize, previousPSSize;

				const TempBuffer& previousBuffer = static_cast<D3DShaderIntermediate*>(interMedias[j])->getBuffer();
				bool ret = D3D9Shader::readShaderBinary( previousBuffer.getData(), previousBuffer.getSize(),
					previousVSBuffer, previousVSSize, previousPSBuffer, previousPSSize);
				assert(ret);
				BLADE_UNREFERENCED(ret);

				if( pVertexShader != NULL && vsSize == previousVSSize &&
					::memcmp(vsBuffer, previousVSBuffer, vsSize) == 0 )
				{
					pVertexShader = static_cast<D3D9Shader*>(outShaders[j])->getVertexShader();
					pVertexShader->AddRef();
				}

				if( pPixelShader != NULL && psSize == previousPSSize &&
					::memcmp(psBuffer, previousPSBuffer, psSize) == 0 )
				{
					pPixelShader = static_cast<D3D9Shader*>(outShaders[j])->getPixelShader();
					pPixelShader->AddRef();
				}

				if( pVertexShader != NULL && pPixelShader != NULL )
					break;
			}

			if( pVertexShader == NULL )
			{
				hr = mDevice->CreateVertexShader( static_cast<const DWORD*>(vsBuffer), &pVertexShader);
				if (FAILED(hr))
					BLADE_EXCEPT(EXC_API_ERROR,BTString("create vertex shader failed - \"") + pIntMedia->getName() + BTString("\".") );
			}

			if( pPixelShader == NULL )
			{
				hr = mDevice->CreatePixelShader( static_cast<const DWORD*>(psBuffer), &pPixelShader);
				if (FAILED(hr))
					BLADE_EXCEPT(EXC_API_ERROR,BTString("create pixel shader failed - \"") + pIntMedia->getName() + BTString("\".") );
			}

			outShaders[i].bind( BLADE_NEW D3D9Shader(pVertexShader, pPixelShader, pIntMedia) );
			++created;
		}
		return created;
	}

	//////////////////////////////////////////////////////////////////////////
	HSHADERINTMEDIA	D3D9GraphicsResourceManager::createShaderIntermediate(const TString& name, const void* data,
		size_t bytes, const TString& profile, const TString entrypoint[SHADER_TYPE_COUNT])
	{
		if( data != NULL )
		{
			D3DShaderIntermediate* interMedia = BLADE_NEW D3DShaderIntermediate(name);
			//bind first,in case of exception
			HSHADERINTMEDIA hShaderIntMedia(interMedia);

			assert( bytes != 0 );
			void* vsData;
			void* psData;
			size_t vsSize, psSize;

			if( !D3D9Shader::readShaderBinary(data, bytes, vsData, vsSize, psData, psSize) )
				return HSHADERINTMEDIA::EMPTY;

			LPD3DXCONSTANTTABLE pTable = NULL;

			if( FAILED( ::D3DXGetShaderConstantTable((const DWORD*)vsData, &pTable) ) || pTable == NULL )
				BLADE_EXCEPT(EXC_API_ERROR, BTString("get constant table failed - \"") );

			this->getShaderConstantInfo(SHADER_VERTEX, pTable, interMedia->getConstantTable() );
			pTable->Release();
			pTable = NULL;

			if( FAILED( ::D3DXGetShaderConstantTable((const DWORD*)psData, &pTable) ) || pTable == NULL )
				BLADE_EXCEPT(EXC_API_ERROR, BTString("get constant table failed - \"") );

			this->getShaderConstantInfo(SHADER_FRAGMENT, pTable, interMedia->getConstantTable() );
			pTable->Release();

			interMedia->setBuffer(data, bytes);

			for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i)
			{
				EShaderType type = EShaderType(i);
				interMedia->setProfile(type, profile);
				interMedia->setEntry(type, entrypoint[i]);
			}
			return hShaderIntMedia;
		}
		else
		{
			assert( bytes == 0 );
			return HSHADERINTMEDIA::EMPTY;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	static TString SHADER_TYPE_PREFIX[SHADER_TYPE_COUNT] = 
	{
		BTString("VS"), BTString("FS"), BTString("GS"),
	};

	static String SHADER_PROFILE_PREFIX[SHADER_TYPE_COUNT] = 
	{
		BString("vs_"), BString("ps_"), BString("gs_"),
	};
	//////////////////////////////////////////////////////////////////////////
	HSHADERINTMEDIA	D3D9GraphicsResourceManager::compileShader(const TString& textFileFullPath, const ParamList& option,
		const void* shaderTextBuffer, size_t shaderTextLen, const ParamList* macros/* = NULL*/) const
	{
		D3DXIncludeLoader incLoader;
		{
			TString path, file;
			TStringHelper::getFilePathInfo(textFileFullPath, path, file);
			incLoader.setPath(path);
		}

		D3DShaderIntermediate* interMedia = BLADE_NEW D3DShaderIntermediate(textFileFullPath);
		//bind first,in case of exception
		HSHADERINTMEDIA hShaderIntMedia(interMedia);

		const Variant& profile = option[BTString("BLADE_PROFILE")];
		if( !profile.isValid())
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("shader profile not set.") );

		typedef TempVector<LPD3DXBUFFER> ShaderBufferList;
		ShaderBufferList shaderBuffers;

		for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i )
		{
			//read the profile option
			EShaderType type = EShaderType(i);
			if( type == SHADER_GEOMETRY || type == SHADER_ANY )
				continue;

			LPD3DXBUFFER shaderbuffer = NULL;
			LPD3DXBUFFER errormsg = NULL;
			LPD3DXCONSTANTTABLE	constTable = NULL;

			const TString& profileString = profile;
			interMedia->setProfile(type, profileString);
			String targetprofile = SHADER_PROFILE_PREFIX[i] + StringConverter::TStringToString(profileString.c_str(), profileString.size());

			//read the code entry
			Variant val = option[ SHADER_TYPE_PREFIX[i] + BTString("Entry") ];
			if(!val.isValid())
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("shader entry not defined.") );
			const TString& entryString = val;
			String entry = StringConverter::TStringToString(entryString.c_str(), entryString.size());
			interMedia->setEntry(type, entryString);

			//read the compiling flags
			DWORD flags = D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY;
			val = option[BTString("ColomnMajor")];
			if(!val.isValid() || val == false )
				flags |= D3DXSHADER_PACKMATRIX_ROWMAJOR;
			else
				flags |= D3DXSHADER_PACKMATRIX_COLUMNMAJOR;

			//well, NOT suitable for DX9,but add it.
			val = option[BTString("OptimizationLevel")];
			if(!val.isValid())
#if BLADE_DEBUG
				flags |= D3DXSHADER_OPTIMIZATION_LEVEL1;
#else
				flags |= D3DXSHADER_OPTIMIZATION_LEVEL3;
#endif
			else if( val == -1 )
				flags |= D3DXSHADER_SKIPOPTIMIZATION;
			else
			{
				static const int ofs[4] = 
				{
					D3DXSHADER_OPTIMIZATION_LEVEL0,D3DXSHADER_OPTIMIZATION_LEVEL1,
					D3DXSHADER_OPTIMIZATION_LEVEL2,D3DXSHADER_OPTIMIZATION_LEVEL3
				};
				uint ol = (uint)(int)val;
				flags |= ol <= 3 ? ofs[ol] : D3DXSHADER_OPTIMIZATION_LEVEL3;
			}

			typedef TempVector<D3DXMACRO> MacroList;
			MacroList d3dMacros;

			TList< std::pair<String, String> > macroCache;

			if( macros != NULL && macros->size() > 0 )
			{
				d3dMacros.reserve( macros->size() + 1 );

				macroCache.reserve(macros->size());
				for(size_t j = 0; j < macros->size(); ++j)
				{
					D3DXMACRO m;
					macroCache.push_back(std::pair<String, String>());
					macroCache.back().first = StringConverter::TStringToString(macros->at(j).getName());
					macroCache.back().second = StringConverter::TStringToString(macros->at(j).getValue().toTString());
					m.Name = macroCache.back().first.c_str();
					m.Definition = macroCache.back().second.c_str();
					d3dMacros.push_back(m);
				}
			}

			D3DXMACRO d3d9api = { "BLADE_D3D9", "1" };
			d3dMacros.push_back(d3d9api);
			D3DXMACRO d3dapi = { "BLADE_D3D", "1" };
			d3dMacros.push_back(d3dapi);

			//end of macro list
			D3DXMACRO end = {NULL, NULL};
			d3dMacros.push_back(end);

			HRESULT hr = ::D3DXCompileShader(
				(LPCSTR)shaderTextBuffer,
				(UINT)shaderTextLen,
				d3dMacros.size() > 0 ? &d3dMacros[0] : NULL,
				&incLoader,
				entry.c_str(),
				targetprofile.c_str(),
				flags,
				&shaderbuffer,
				&errormsg,
				&constTable
				);

			if( errormsg != NULL )
			{
				const TString textFilePath = TStringHelper::standardizePath(textFileFullPath);
				TString textFileDir, textFileName;
				TStringHelper::getFilePathInfo(textFilePath, textFileDir, textFileName);

				TString info = StringConverter::StringToTString(static_cast<const char*>(errormsg->GetBufferPointer()),errormsg->GetBufferSize());
				TStringTokenizer lines;
				lines.tokenize(info, TEXT("\n\r"));
				for (size_t j = 0; j < lines.size(); ++j)
				{
					index_t rpos = lines[j].find(TEXT("\\memory("));
					TString output;
					if (rpos != TString::npos)
					{
						TString path = lines[j].substr_nocopy(0, rpos);
						output = TStringHelper::getReplace(lines[j], path + TEXT("\\memory"), textFilePath, 1);
					}
					else
						output = textFileDir + BTString("/") + lines[j];
					
					bool isWarning = lines[j].find(TEXT(": warning")) != TString::npos;
					ID3D9Helper::getSingleton().outputString(output, isWarning ? ILog::LL_CAREFUL : ILog::LL_CRITICAL);
				}
				ULONG count = errormsg->Release();
				assert(count == 0);
				BLADE_UNREFERENCED(count);
			}

			if( constTable != NULL || !FAILED(hr) )
			{
				this->getShaderConstantInfo(type, constTable, interMedia->getConstantTable() );
				ULONG count = constTable->Release();
				assert(count == 0);
				BLADE_UNREFERENCED(count);
				interMedia->setCompilingFlags(type, flags);
			}
			else
			{
				//error
				hShaderIntMedia.clear();
				break;
			}

			if( shaderbuffer != NULL )
				shaderBuffers.push_back(shaderbuffer);
		}

		if( shaderBuffers.size() == 2 )
		{
			TempBuffer buffer;
			D3D9Shader::writeShaderBinary(buffer, 
				shaderBuffers[SHADER_VERTEX]->GetBufferPointer(), shaderBuffers[SHADER_VERTEX]->GetBufferSize(), 
				shaderBuffers[SHADER_FRAGMENT]->GetBufferPointer(), shaderBuffers[SHADER_FRAGMENT]->GetBufferSize() );

			interMedia->setBuffer(buffer.getData(), buffer.getSize() );
		}

		for(size_t i = 0; i < shaderBuffers.size(); ++i)
			shaderBuffers[i]->Release();

		return hShaderIntMedia;
	}

	//////////////////////////////////////////////////////////////////////////
	const TStringList&	D3D9GraphicsResourceManager::getSupportedProfiles() const
	{
		return mProfiles;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		D3D9GraphicsResourceManager::getHighestProfile() const
	{
		return mProfiles.size() != 0 ? mProfiles[mProfiles.size() - 1] : TString::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				D3D9GraphicsResourceManager::getShaderConstantSize(SHADER_CONSTANT_TYPE type) const
	{
		//note: D3D align constant size to on register size (float4)
		switch(type)
		{
		case SCT_BOOL1:
			return 4;
		case SCT_SAMPLER:
		case SCT_FLOAT2:
		case SCT_INT2:
		case SCT_UINT2:
		case SCT_FLOAT3:
		case SCT_INT3:
		case SCT_UINT3:
		case SCT_FLOAT4:
		case SCT_INT4:
		case SCT_UINT4:
			return 4*4;
		case SCT_MATRIX22:
		case SCT_MATRIX23:
		case SCT_MATRIX24:
			return 8*4; // 2 float4s
		case SCT_MATRIX32:
		case SCT_MATRIX33:
		case SCT_MATRIX34:
			return 12*4; // 3 float4s
		case SCT_MATRIX42:
		case SCT_MATRIX43:
		case SCT_MATRIX44:
			return 16*4; // 4 float4s
		default:
			{
				ShaderConstantType t = type;
				if( t.getAtomType() == SCTF_BOOL )
				{
					assert( t.getRowCount() == 1 );
					return t.getColCount()*4;
				}
				else
				{
					size_t col = (t.getColCount()+3)&(~0x3);
					return col*t.getRowCount()*4;
				}
			}
		};
	}

	//////////////////////////////////////////////////////////////////////////
	Matrix44&		D3D9GraphicsResourceManager::convertProjectionMatrix(Matrix44& projectMatrix) const
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif
		//input: standard [-1,1]
		//output: D3D depth range [0,1]

		//use general conversion: the simple method may be incorrect for some non-standard projection. i.e. LiSPSM projection matrix
		static const Matrix44 scaleoffset = Matrix44::generateTranslation(POINT3(0, 0, 1))*Matrix44::generateScale(Vector3(1, 1, 0.5f));
		projectMatrix *= scaleoffset;

		//projectMatrix[2][2] = (projectMatrix[2][2] + projectMatrix[2][3])/2;
		//projectMatrix[3][2] = (projectMatrix[3][2] + projectMatrix[3][3])/2;
		return projectMatrix;

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
	}

	/************************************************************************/
	/* ID3D9LostResourceManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	D3D9GraphicsResourceManager::registerDefaultResource(ID3D9Resource* resource)
	{
		if(resource != NULL)
			mDefaultResources.insert(resource);
	}

	//////////////////////////////////////////////////////////////////////////
	void	D3D9GraphicsResourceManager::unRegisterDefaultResource(ID3D9Resource* resource)
	{
		if( resource != NULL )
			mDefaultResources.erase(resource);
	}

	//////////////////////////////////////////////////////////////////////////
	void	D3D9GraphicsResourceManager::releaseDefaultResource()
	{
		if( mRenderResourceReleased )
			return;

		for(DefualtResourceList::iterator i = mDefaultResources.begin(); i != mDefaultResources.end(); ++i )
		{
			(*i)->release();
		}
		mRenderResourceReleased = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	D3D9GraphicsResourceManager::rebuildDefaultResource()
	{
		for(DefualtResourceList::iterator i = mDefaultResources.begin(); i != mDefaultResources.end(); ++i )
		{
			(*i)->rebuild(mDevice);
		}
		mRenderResourceReleased = false;
	}


	/************************************************************************/
	/* custom interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		D3D9GraphicsResourceManager::setD3D9Device(IDirect3DDevice9* device, IDirect3D9* d3d9, UINT adapter)
	{
		D3DDISPLAYMODE mode;
		d3d9->GetAdapterDisplayMode(adapter, &mode);
		if (FAILED(d3d9->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('N', 'U', 'L', 'L'))))
		{
		
		}

		mDevice = device;
		LPCSTR pps = ::D3DXGetPixelShaderProfile(device);

		if( pps < BString("ps_2_0") )
			mProfiles.push_back( BTString("1_1") );
		else if( pps == BString("ps_2_0") )
		{
			mProfiles.push_back( BTString("1_1") );
			mProfiles.push_back( BTString("2_0") );
		}
		else if( pps == BString("ps_2_a") )
		{
			mProfiles.push_back( BTString("1_1") );
			mProfiles.push_back( BTString("2_0") );
			mProfiles.push_back( BTString("2_a") );
		}
		else if( pps == BString("ps_2_b") )
		{
			mProfiles.push_back( BTString("1_1") );
			mProfiles.push_back( BTString("2_0") );
			mProfiles.push_back( BTString("2_b") );
		}
		else
		{
			mProfiles.push_back( BTString("1_1") );
			mProfiles.push_back( BTString("2_0") );
			mProfiles.push_back( BTString("2_a") );
			mProfiles.push_back( BTString("2_b") );

			DWORD maxMRT = 1;
			D3DCAPS9 dev_caps;
			if (!FAILED(d3d9->GetDeviceCaps(adapter, D3DDEVTYPE_HAL, &dev_caps)))
				maxMRT = dev_caps.NumSimultaneousRTs;

			//deferred shading need profile 3_0
			//and also need INTZ depth stencil
			bool hasINTZ = !FAILED(d3d9->CheckDeviceFormat(adapter,
				D3DDEVTYPE_HAL,
				mode.Format,
				D3DUSAGE_DEPTHSTENCIL,
				D3DRTYPE_SURFACE,
				(D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z')));
			
			if(hasINTZ && maxMRT > 2 )
			{
				mProfiles.push_back(BTString("3_0"));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		D3D9GraphicsResourceManager::getShaderConstantInfo(EShaderType type, LPD3DXCONSTANTTABLE table, ShaderInfoTable& infoTable) const
	{
		D3DXCONSTANTTABLE_DESC desc;
		HRESULT hr = table->GetDesc(&desc);
		if( FAILED(hr) )
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("reading shader constant table info error.") );

		for (unsigned int i = 0; i < desc.Constants; ++i)
		{
			// Recursive for the structure params : from Ogre
			this->processConstantEntry(type, table, NULL, TString::EMPTY, i, infoTable);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		D3D9GraphicsResourceManager::processConstantEntry(EShaderType type, LPD3DXCONSTANTTABLE table,D3DXHANDLE parent,const TString& prefix,UINT index, ShaderInfoTable& infoTable) const
	{
		D3DXHANDLE hConstant = table->GetConstant(parent, index);

		D3DXCONSTANT_DESC desc;
		UINT numParams = 1;
		HRESULT hr = table->GetConstantDesc(hConstant, &desc, &numParams);
		if (FAILED(hr))
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("Cannot get constant description") );

		TString paramName = StringConverter::StringToTString(desc.Name);
		// trim the odd '$' which appears at the start of the function constant names
		if (paramName[0] == tchar('$') )
			paramName = paramName.substr(1);

		// trim the '[0]' suffix if it exists, we will add our own indexing later
		if (TStringHelper::isEndWith(paramName, BTString("[0]")) )
			paramName = paramName.substr(0, paramName.size() - 3 );

		if (desc.Class == D3DXPC_STRUCT && desc.Elements == 1 )
		{
			//generate a prefix for structure member
			TString stPrefix = prefix + paramName + BTString(".");
			//recursively get member info
			for (unsigned int i = 0; i < desc.StructMembers; ++i)
				this->processConstantEntry(type, table, hConstant, stPrefix, i, infoTable);
		}
		else
		{
			IShader::CONSTANT_INFO info;
			info.mShaderType = type;
			info.mName = prefix + paramName;
			info.mElementCount = (uint16)desc.Elements;
			info.mIndex = (uint16)desc.RegisterIndex;
			//info.mBytes = desc.Bytes;
			assert(desc.Columns >= 1);
			switch(desc.RegisterSet)
			{
			case D3DXRS_INT4:
				assert(desc.Rows == 1);
				assert(desc.Columns <= 4);
				info.mType = EShaderConstantType(SCT_INT1 + desc.Columns - 1);
				break;
			case D3DXRS_FLOAT4:
				switch(desc.Class)
				{
				case D3DXPC_MATRIX_COLUMNS:
				case D3DXPC_MATRIX_ROWS:
					{
						UINT firstDim, secondDim;
						//firstDim = desc.RegisterCount / desc.Elements;
						if (desc.Class == D3DXPC_MATRIX_ROWS)
						{
							firstDim = desc.Rows;
							secondDim = desc.Columns;
						}
						else
						{
							firstDim = desc.Columns;
							secondDim = desc.Rows;
						}
						assert( firstDim >= 1 && firstDim <= 4 && secondDim >= 1 && secondDim <= 4);
						info.mType = EShaderConstantType( SCTF_FLOAT|((firstDim-1)<<2)|(secondDim-1) );
					}
					break;
				case D3DXPC_SCALAR:
				case D3DXPC_VECTOR:
				default:
					assert(desc.Rows == 1);
					assert(desc.Columns <= 4);
					info.mType = EShaderConstantType(SCT_FLOAT1 + desc.Columns - 1);
				}
				break;
			case D3DXRS_BOOL:
				assert(desc.Rows == 1);
				assert(desc.Columns <= 4);
				info.mType = EShaderConstantType(SCT_BOOL1 + desc.Columns - 1);
				break;
			case D3DXRS_SAMPLER:
				assert(desc.Rows == 1);
				info.mType = SCT_SAMPLER;
				break;
			default:
				assert(false);
				break;
			};
			info.mRegisterCount = (uint16)desc.RegisterCount;
			info.mBytes = (size_t)info.mRegisterCount * 16u;
			info.mElementSize = (uint16)this->getShaderConstantSize(info.mType);
			infoTable.addConstantInfo(info);

			//access each element in array: sampler only for now.
			if( info.mElementCount > 1 && info.mElementCount <= 16 && info.mType == SCT_SAMPLER)
			{
				for(size_t i = 0; i < info.mElementCount; ++i)
				{
					IShader::CONSTANT_INFO elementInfo = info;
					elementInfo.mName += BTString("[") + TStringHelper::fromUInt((uint)i) + BTString("]");
					elementInfo.mElementCount = 1;
					elementInfo.mIndex = (uint16)(info.mIndex + desc.RegisterCount/info.mElementCount * i );
					elementInfo.mRegisterCount = (uint16)( desc.RegisterCount/info.mElementCount );
					elementInfo.mBytes = elementInfo.mRegisterCount * 16u;
					infoTable.addConstantInfo(elementInfo);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool				D3D9GraphicsResourceManager::forceUpdateTexture(const IImage& image, ITexture& texture,
		const POINT2I& srcPos/* = POINT2I::ZERO*/, const POINT2I& destPos/* = POINT2I::ZERO*/, const SIZE2I& size/* = SIZE2I::ZERO*/) const
	{
		const IMG_INFO& imgInfo = image.getInfo();
		const ITexture::DESC& desc = texture.getDesc();

		size_t mipMaps = std::min<size_t>(imgInfo.mipmap, desc.mMipmapCount);
		IGraphicsBuffer::LOCKFLAGS lockflags = IGraphicsBuffer::GBLF_WRITEONLY;

		if( desc.mTextureType == TT_3DIM )
		{
			if( imgInfo.depth == 0 || imgInfo.faces > 1)
			{
				assert(false);
				return false;
			}

			D3D9VolumeTexture& d3dtex = static_cast<D3D9VolumeTexture&>(texture);
			TEXREGION region( Box3i(destPos.x,destPos.y,destPos.x+size.x,destPos.y+size.y, 0, (int)imgInfo.depth) );
			for(size_t i = 0; i < mipMaps; ++i )
			{
				const IMG_MIPINFO* info = image.getMipInfo(i);
				region.mMipLevel = i;
				char* dest = (char*)d3dtex.lockNoCheck(region, lockflags);
				if( dest == NULL )
					BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to lock texture.") );

				const char* source = (const char*)info->data;
				
				const size_t sourcePitch = info->rowPitch;
				const size_t sourceSlicePitch = info->slicePitch;
				const size_t rowPitch = region.mRowPitch;
				const size_t slicePitch = region.mSlicePitch;

				size_t bytes = info->width*desc.mPixelFormat.getSizeBytes();
				const size_t height = info->height;

				for(size_t depth = 0; depth < desc.mDepth; ++depth )
				{
					char* destSlice = dest;
					const char* sourceSlice = source;

					for(size_t n = 0; n < height; ++n )
					{
						std::memcpy(destSlice, sourceSlice, bytes);
						destSlice += rowPitch;
						sourceSlice += sourcePitch;
					}

					dest += slicePitch;
					source += sourceSlicePitch;
					region.mBox.mMaxPoint /= 2;
					region.mBox.mMinPoint /= 2;
				}

				d3dtex.unlock();
			}
			return true;
		}
		else if( desc.mTextureType == TT_CUBE )
		{
			if( imgInfo.faces != 6)
			{
				assert(false);
				return false;
			}

			D3D9CubeTexture& d3dtex = static_cast<D3D9CubeTexture&>(texture);
			TEXREGION region;
			for(size_t f = 0; f < imgInfo.faces; ++f)
			{
				for(size_t i = 0; i < mipMaps; ++i)
				{
					region.mFace = f;
					region.mMipLevel = i;
					const IMG_MIPINFO* info = image.getMipInfo(i, f);
					char* dest = (char*)d3dtex.lockNoCheck(region, lockflags);
					if( dest == NULL )
						BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to lock texture.") );

					const char* source = (const char*)info->data;
					const size_t sourcePitch = info->rowPitch;
					const size_t pitch = region.mRowPitch;

					size_t bytes = info->width*desc.mPixelFormat.getSizeBytes();
					const size_t height = info->height;
					for(size_t n = 0; n < height; ++n )
					{
						std::memcpy(dest, source, bytes);
						dest += pitch;
						source += sourcePitch;
					}

					d3dtex.unlock();
				}//mipmaps
			}//faces

		}
		else	//2D textures
		{
			D3D9Texture& d3dtex = static_cast<D3D9Texture&>(texture);
			const IMG_MIPINFO* info0 = image.getMipInfo(0);
			size_t compressRate = info0->oriWidth/info0->width;
			SIZE2I destSize = size == SIZE2I::ZERO ? SIZE2I( (int)info0->oriWidth, (int)info0->oriHeight) : size;
			POINT2I levelSrc = srcPos;
			POINT2I levelDest = destPos;
			TEXREGION region( Box3i(destPos.x,destPos.y,destPos.x+destSize.x,destPos.y+destSize.y) );
			
			destSize /= (int)compressRate;
			levelSrc /= (int)compressRate;
			levelDest /= (int)compressRate;
			SIZE2I levelSize = destSize;

			for(size_t i = 0; i < mipMaps; ++i)
			{
				const IMG_MIPINFO* info = image.getMipInfo(i);
				region.mMipLevel = i;

				//note: dest offset already made// + levelDest.y*rect.Pitch + levelDest.x;
				char* dest = (char*)d3dtex.lockNoCheck(region, lockflags);
				if( dest == NULL )
					BLADE_EXCEPT(EXC_API_ERROR, BTString("unable to lock texture.") );

				const char* source = (const char*)info->data + levelSrc.y*info->rowPitch + levelSrc.x;
				const size_t sourcePitch = info->rowPitch;
				size_t pitch = region.mRowPitch;

				////fix DX9 hacking for 3Dc('ATI2'), aka BC5
				//the D3D9 runtime assume the extension format is 8 BPP (1 byte per pixel)
				//but actually it is 16/8 bytes per block. we need to fix the pitch & sub rect offset
				if( desc.mPixelFormat == PF_BC5 || desc.mPixelFormat == PF_BC4 )
				{
					size_t correctPitch = pitch * desc.mPixelFormat.getSizeBytes()/4;
					int left = (region.mBox.getLeft()+3)/4;
					int top = (region.mBox.getTop()+3)/4;
					dest += left*desc.mPixelFormat.getSizeBytes() - region.mBox.getLeft();
					dest += top*correctPitch - region.mBox.getTop()*pitch;
					pitch = correctPitch;
				}
				assert( pitch >= sourcePitch );

				const size_t height = info->height;
				if( pitch == sourcePitch )
					std::memcpy(dest, source, sourcePitch*height);
				else
				{
					size_t bytes = levelSize.x * desc.mPixelFormat.getSizeBytes();
					assert( pitch >= bytes && sourcePitch >= bytes );
					for(size_t n = 0; n < height; ++n )
					{
						std::memcpy(dest, source, bytes);
						dest += pitch;
						source += sourcePitch;
					}
				}
				d3dtex.unlock();

				region.mBox.mMaxPoint /= 2;
				region.mBox.mMinPoint /= 2;
				levelSrc /= 2;
				levelSize /= 2;
				levelDest /= 2;
			}//for mipmaps

			if( false )
				::D3DXSaveTextureToFile(TEXT("Z:\\watch.dds"), D3DXIFF_DDS, d3dtex.getD3D9Resource(),NULL);
		}//2D

		if( imgInfo.mipmap < desc.mMipmapCount )
		{
			Box3i box = Box3i::EMPTY;
			if( desc.mTextureType == TT_2DIM )
				box = Box3i(destPos.x,destPos.y,destPos.x+size.x,destPos.y+size.y);

			for(size_t i = imgInfo.mipmap; i < desc.mMipmapCount; ++i)
				texture.updateMipMap(i, box);
		}
		return true;
	}
	
}//namespace Blade


#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
