/********************************************************************
	created:	2014/12/19
	filename: 	GLESGraphicsResourceManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESGraphicsResourceManager.h>
#include <graphics/GLES/GLESUtil.h>
#include <graphics/GLES/GLESVertexBuffer.h>
#include <graphics/GLES/GLESIndexBuffer.h>
#include <graphics/GLES/GLESVertexDeclaration.h>
#include <graphics/GLES/GLESTexture.h>
#include <graphics/GLES/GLESCubeTexture.h>
#include <graphics/GLES/GLESVolumeTexture.h>
#include <graphics/GLES/GLESShader.h>
#include <graphics/GLES/GLESShaderIntermediate.h>
#include <graphics/GLES/public/GLESAttributeNames.h>
#include <graphics/GLES/GLESContext.h>


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const TString GLESGraphicsResourceManager::TYPE = BTString("GLES_GRManager");
	GLESContext*	GLESGraphicsResourceManager::msContext;	//link
	GLESGraphicsResourceManager::ContextMap	GLESGraphicsResourceManager::msSharedContexts;	//link

	//////////////////////////////////////////////////////////////////////////
	GLESGraphicsResourceManager::GLESGraphicsResourceManager()
	{
		mConfig.TextureDir = IMGO_BOTTOM_UP;
		mConfig.DesiredOrder = PO_RGBA;

		mConfig.DefaultRTRGB = PF_R8G8B8;
		mConfig.DefaultRTRGBA = PF_R8G8B8A8;
		mConfig.DefaultRTDepth = PF_DEPTH24S8;
		mConfig.DefaultRTHDR = PF_F16X4;

		mConfig.ColorR = PF_R8;
		mConfig.ColorRG = PF_R8G8;
		mConfig.ColorRGB = PF_R8G8B8;
		mConfig.ColorRGBA = PF_R8G8B8A8;
		mConfig.ColorNormal = PF_R8G8;

		mConfig.CompressedR = PF_R_EAC;
		mConfig.CompressedRG = PF_RG_EAC;
		mConfig.CompressedRGB = PF_RGB_ETC2;
		mConfig.CompressedRGBA = PF_RGBA_ETC2EAC;
		mConfig.CompressedNormal = PF_RG_EAC;

		mConfig.DefaultR = mConfig.CompressedR;
		mConfig.DefaultRG = mConfig.CompressedRG;
		mConfig.DefaultRGB = mConfig.CompressedRGB;
		mConfig.DefaultRGBA = mConfig.CompressedRGBA;
		mConfig.DefaultNormal = PF_RG_EAC;
		mConfig.TextureExt = BTString("ktx");
		mConfig.HalfTexel2Pixel = false;

		//normal need to be linear format
		mConfig.DefaultNormal.setLinearColor(true);
		mConfig.ColorNormal.setLinearColor(true);
		mConfig.CompressedNormal.setLinearColor(true);

		//since we use GLES 3.0
		mShaderProfiles.push_back( BTString("1_0") );
		mShaderProfiles.push_back( BTString("2_0") );
		mShaderProfiles.push_back(BTString("2_a"));
		mShaderProfiles.push_back( BTString("3_0") );
		//mDefaultManager = IGraphicsResourceManager::getOtherSingletonPtr(BTString("Default"));
	}

	//////////////////////////////////////////////////////////////////////////
	GLESGraphicsResourceManager::~GLESGraphicsResourceManager()
	{
		for (ContextMap::iterator i = msSharedContexts.begin(); i != msSharedContexts.end(); ++i)
			i->second->deinitialize();
		msSharedContexts.clear();
	}

	/************************************************************************/
	/* IGraphicsResourceManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HVBUFFER	GLESGraphicsResourceManager::createVertexBuffer(const void* data, size_t vertexSize, size_t vertexCount, IGraphicsBuffer::USAGE usage)
	{
		this->validateContext(false);

		GLuint buffer = 0;
		::glGenBuffers(1, &buffer);
		assert( ::glGetError() == GL_NO_ERROR );
		return HVBUFFER(BLADE_NEW GLESVertexBuffer(buffer, vertexSize, vertexCount, usage, data));
	}

	//////////////////////////////////////////////////////////////////////////
	HIBUFFER	GLESGraphicsResourceManager::createIndexBuffer(const void* data, IIndexBuffer::EIndexType indexType,size_t indexCount,IGraphicsBuffer::USAGE usage)
	{
		this->validateContext(false);

		//GLES 2.0 doesn't support 32 bit index, but we're using GLES 3.0, which support 32bit index
		//TODO: GLES support 8 bit index format, add support?
		GLuint buffer = 0;
		::glGenBuffers(1, &buffer);
		assert( ::glGetError() == GL_NO_ERROR );
		return HIBUFFER(BLADE_NEW GLESIndexBuffer(buffer, indexType, indexCount, usage, data));
	}

	//////////////////////////////////////////////////////////////////////////
	HVDECL		GLESGraphicsResourceManager::createVertexDeclaration()
	{
		return  HVDECL(BLADE_NEW GLESVertexDeclaration());
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE	GLESGraphicsResourceManager::createDepthStencilBuffer(size_t width, size_t height, PixelFormat format, IGraphicsBuffer::ACCESS GpuAccess/* = IGraphicsBuffer::GBAF_WRITE*/)
	{
		this->validateContext(false);

		GLuint GLTexture = 0;
		::glGenTextures(1, &GLTexture);
		assert( ::glGetError() == GL_NO_ERROR );
		GLESTexture* texture = BLADE_NEW GLESTexture(GLTexture, IMGO_DEFAULT, width, height, format, IGraphicsBuffer::GBUF_RENDERTARGET, GpuAccess);
		texture->shareSampler(Texture::DEFAULT_DEPTH_RTT_SAMPLER());
		return HTEXTURE(texture);
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE	GLESGraphicsResourceManager::createTexture(size_t width, size_t height, PixelFormat format,
		int mipLevels/* = -1*/, IGraphicsBuffer::USAGE usage/* = IGraphicsBuffer::GBU_DEFAULT*/,
		ETextureType type/* = TT_2DIM*/, IMG_ORIENT layout/* = IMGO_DEFAULT*/, size_t depth/* = 1*/)
	{
		this->validateContext(false);

		if( mipLevels <= 0 )
		{
			mipLevels = Math::CalcMaxMipCount(width, height);
			if( format.isCompressed() )
				mipLevels -= 2;
		}

		if( layout == IMGO_DEFAULT )
			layout = mConfig.TextureDir;

		if (usage.isRenderTarget())
			format.setLinearColor(true);
#if 0
		GLESPixelFormat GLFormat;
		if( format == PF_UNDEFINED )
			GLFormat = GLESPixelFormatConverter::getBackBufferFormat();
		else
			GLFormat = GLESPixelFormatConverter(format);
#else
		if (format == PF_UNDEFINED)
			format = GLESPixelFormatConverter(GLESPixelFormatConverter::getBackBufferFormat());
#endif

		//GLenum GLusage = GLESBufferUsageConverter(usage);

		GLuint GLTexture = 0;
		::glGenTextures(1, &GLTexture);
		GLenum err = ::glGetError();
		assert(err == GL_NO_ERROR); BLADE_UNREFERENCED(err);
		ITexture* tex = NULL;

		if( type == TT_1DIM || type == TT_2DIM )
		{
			assert(depth == 1);
			tex = BLADE_NEW GLESTexture(GLTexture, layout, width, height, format, usage, IGraphicsBuffer::GBAF_READ_WRITE, mipLevels);
		}
		else if( type == TT_3DIM )
		{
			assert( depth == 1);
			tex = BLADE_NEW GLESVolumeTexture(GLTexture, layout, width, height, format, usage, IGraphicsBuffer::GBAF_READ_WRITE, depth, mipLevels);
		}
		else if( type == TT_CUBE )
		{
			assert( depth == 1);
			assert( width == height );
			tex = BLADE_NEW GLESCubeTexture(GLTexture, layout, width, height, format, usage, IGraphicsBuffer::GBAF_READ_WRITE, mipLevels);
		}

		if (usage.isRenderTarget())
		{
			tex->setLinearColorSpace(true);
			if (tex->getPixelFormat().isDepth())
				tex->shareSampler(Texture::DEFAULT_DEPTH_RTT_SAMPLER());
			else
				tex->shareSampler(Texture::DEFAULT_RTT_SAMPLER());
		}
		return HTEXTURE(tex);
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE	GLESGraphicsResourceManager::createTexture(const IImage& image, int mipMaps/* = IImage::MIPLOD_DEFAULT*/,  IGraphicsBuffer::USAGE usage/* = IGraphicsBuffer::GBU_STATIC*/)
	{
		//should convert to 32BBP (4byte RGBA) before create call
		if( !image.isCompressed() )
		{
			if( ( image.getBPP() != 32 && image.getBPP() != 16 )
				//should convert to the right layout (top-bottom) before create call
				//	|| image.getOrientation() != this->GLESGraphicsResourceManager::getGraphicsConfig().TextureDir
					|| (image.hasByteOrder() && image.getByteOrder() != this->GLESGraphicsResourceManager::getGraphicsConfig().DesiredOrder)
						)
			{
				assert(false);
				return HTEXTURE::EMPTY;
			}
		}

		assert(!usage.isRenderTarget() && !usage.isDepthStencil());

		this->validateContext(false);

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
	bool		GLESGraphicsResourceManager::updateTexture(const IImage& image, ITexture& texture,
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
			//|| image.getOrientation() != this->GLESGraphicsResourceManager::getGraphicsConfig().TextureDir
			|| (image.hasByteOrder() && image.getByteOrder() != this->GLESGraphicsResourceManager::getGraphicsConfig().DesiredOrder)
			)
		{
			//WHY we don't convert it here is because the conversion is time consuming and maybe called during rendering
			//so it's better for user to decide where to convert it, maybe another thread or somewhere else.
			assert(false);
			return false;
		}

		this->validateContext(false);
		return this->forceUpdateTexture(image, texture, srcPos, destPos, size);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		GLESGraphicsResourceManager::createShaders(const HSHADERINTMEDIA* interMedias, HSHADER* outShaders, size_t count/* = 1*/)
	{
		if( interMedias == NULL || outShaders == NULL || count == 0 )
			return 0;

		this->validateContext(false);
		for(size_t i = 0; i < count; ++i)
		{
			const HSHADERINTMEDIA& intmedia = interMedias[i];
			HSHADER& shader = outShaders[i];
			if( intmedia == NULL )
				return i;

			GLESShader* glesShader = BLADE_NEW GLESShader( static_cast<GLESShaderIntermediate*>(intmedia) );
			shader.bind( glesShader );
		}
		return count;
	}

	//////////////////////////////////////////////////////////////////////////
	HSHADERINTMEDIA	GLESGraphicsResourceManager::createShaderIntermediate(const TString& name, const void* data, size_t bytes,
		const TString& profile, const TString entrypoint[SHADER_TYPE_COUNT])
	{
		//create shader media in 2 format: text, precompiled binary.
		//precompile binary shader on first run, and use binary cache afterwards

		if( data != NULL )
		{
			GLESShaderIntermediate* interMedia = BLADE_NEW GLESShaderIntermediate(name);
			HSHADERINTMEDIA hShaderIntMedia(interMedia);

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
	HSHADERINTMEDIA	GLESGraphicsResourceManager::compileShader(const TString& textFileFullPath, const ParamList& option,
		const void* shaderTextBuffer, size_t shaderTextLen, const ParamList* macros/* = NULL*/) const
	{
		this->validateContext(false);
		//note: compile GLES 3.0 GLSL source code in linking phase, don't compile it here
		//becase GLESGraphicsResourceManager::compileShader is called in loading thread, and glCompileShader is not thread safe.
		//static const int VERTEX = 0;
		//static const int FRAGEMENT = 1;

		GLuint vertexShader = 0;
		GLuint fragmentShader = 0;
		vertexShader = ::glCreateShader(GL_VERTEX_SHADER);
		fragmentShader = ::glCreateShader(GL_FRAGMENT_SHADER);
		GLuint program = ::glCreateProgram();
		assert( ::glGetError() == GL_NO_ERROR );
		assert( vertexShader != 0 && fragmentShader != 0 && program != 0 );

		GLuint shaders[2] = {vertexShader, fragmentShader};
		const GLchar* texts[2] = { (const GLchar*)shaderTextBuffer, (const GLchar*)shaderTextBuffer};
		GLint lengths[2] = { (GLint)shaderTextLen, (GLint)shaderTextLen};
		//TString entry[2] = { option[ BTString("VSEntry") ], option[ BTString("FSEntry") ] };
		String prefix[2] = { BString("#define ") + GLSL_VS_SWITCH + BString("\n"), BString("#define ") + GLSL_FS_SWITCH + BString("\n") };

		//compile shaders
		for(int i = 0; i < 2; ++i)
		{
			String text = String((const char*)texts[i], lengths[i]);
			//if( entry[i] != TString::EMPTY )
			//	text = StringHelper::getReplace(text, entry[i], BString("main"), 1);
			text = BString("#version 300 es\n") + prefix[i] + text;
			const GLchar* realText = (const GLchar*)text.c_str();
			GLint realLength = (GLint)text.size();

			assert( ::glGetError() == GL_NO_ERROR );
			::glShaderSource(shaders[i], 1, &realText, &realLength);
			assert( ::glGetError() == GL_NO_ERROR );
			::glCompileShader(shaders[i]);
			assert( ::glGetError() == GL_NO_ERROR );

			GLint status = 0;
			::glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &status);
			assert( ::glGetError() == GL_NO_ERROR );
			if( status != GL_TRUE )
			{
				GLchar buffer[4096];
				GLsizei infoLength = 0;
				::glGetShaderInfoLog(shaders[i], 4096, &infoLength, buffer);
				assert( ::glGetError() == GL_NO_ERROR );
				if( infoLength != 0 )
					BLADE_LOG(Error, String(buffer, infoLength) );
				assert(false);
			}

			::glAttachShader(program, shaders[i] );
			assert( ::glGetError() == GL_NO_ERROR );
		}

		//link program
		::glLinkProgram(program);
		assert( ::glGetError() == GL_NO_ERROR );

		GLint status = 0;
		::glGetProgramiv(program, GL_LINK_STATUS, &status);
		assert( ::glGetError() == GL_NO_ERROR );
		if( status != GL_TRUE )
		{
			GLchar buffer[4096];
			GLsizei infoLength = 0;
			::glGetProgramInfoLog(program, 4096, &infoLength, buffer);
			assert( ::glGetError() == GL_NO_ERROR );
			if( infoLength != 0 )
				BLADE_LOG(Error, String(buffer, infoLength) );
			assert(false);
		}

		::glDetachShader(program, vertexShader);
		assert( ::glGetError() == GL_NO_ERROR );
		::glDetachShader(program, fragmentShader);
		assert( ::glGetError() == GL_NO_ERROR );

		::glDeleteShader(vertexShader);
		assert( ::glGetError() == GL_NO_ERROR );
		::glDeleteShader(fragmentShader);
		assert( ::glGetError() == GL_NO_ERROR );

		GLESShaderIntermediate* interMedia = BLADE_NEW GLESShaderIntermediate(textFileFullPath, program);

		return HSHADERINTMEDIA(interMedia);
	}

	//////////////////////////////////////////////////////////////////////////
	const TStringList&	GLESGraphicsResourceManager::getSupportedProfiles() const
	{
		return mShaderProfiles;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		GLESGraphicsResourceManager::getHighestProfile() const
	{
		const TStringList& profiles = mShaderProfiles;
		return profiles.size() != 0 ? profiles[profiles.size() - 1] : TString::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				GLESGraphicsResourceManager::getShaderConstantSize(SHADER_CONSTANT_TYPE type) const
	{
		switch(type)
		{
		case SCT_FLOAT1:
		case SCT_INT1:
		case SCT_UINT1:
		case SCT_BOOL1:
		case SCT_SAMPLER:
			return 4;
		case SCT_FLOAT2:
		case SCT_INT2:
		case SCT_UINT2:
		case SCT_BOOL2:
			return 2*4;
		case SCT_FLOAT3:
		case SCT_INT3:
		case SCT_UINT3:
		case SCT_BOOL3:
			return 3*4;
		case SCT_FLOAT4:
		case SCT_MATRIX22:
		case SCT_INT4:
		case SCT_UINT4:
		case SCT_BOOL4:
			return 4*4;
		case SCT_MATRIX23:
		case SCT_MATRIX32:
			return 6*4;
		case SCT_MATRIX42:
		case SCT_MATRIX24:
			return 8*4; // 2 float4s
		case SCT_MATRIX33:
			return 9*4;
		case SCT_MATRIX34:
		case SCT_MATRIX43:
			return 12*4; // 3 float4s
		case SCT_MATRIX44:
			return 16*4; // 4 float4s
		default:
			{
				ShaderConstantType t = type;
				return t.getRowCount()*t.getColCount()*4;
			}
		};
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				GLESGraphicsResourceManager::forceUpdateTexture(const IImage& image, ITexture& texture,
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

			GLESVolumeTexture& glesTex = static_cast<GLESVolumeTexture&>(texture);
			TEXREGION region( Box3i(destPos.x,destPos.y,destPos.x+size.x,destPos.y+size.y, 0, (int)imgInfo.depth) );
			for(size_t i = 0; i < mipMaps; ++i )
			{
				const IMG_MIPINFO* info = image.getMipInfo(i);
				region.mMipLevel = i;
				char* dest = (char*)glesTex.lockNoCheck(region, lockflags);
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

				glesTex.unlock();
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

			GLESCubeTexture& glesTex = static_cast<GLESCubeTexture&>(texture);
			TEXREGION region;
			for(size_t f = 0; f < imgInfo.faces; ++f)
			{
				for(size_t i = 0; i < mipMaps; ++i)
				{
					region.mFace = f;
					region.mMipLevel = i;
					const IMG_MIPINFO* info = image.getMipInfo(i, f);
					char* dest = (char*)glesTex.lockNoCheck(region, lockflags);
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

					glesTex.unlock();
				}//mipmaps
			}//faces

		}
		else	//2D textures
		{
			GLESTexture& glestex = static_cast<GLESTexture&>(texture);
			const IMG_MIPINFO* info0 = image.getMipInfo(0);
			size_t compressRate = info0->oriWidth/info0->width;
			SIZE2I destSize = size == SIZE2I::ZERO ? SIZE2I( (int)info0->oriWidth, (int)info0->oriHeight) : size;
			POINT2I levelSrc = srcPos;
			//POINT2I levelDest = destPos;
			TEXREGION region( Box3i(destPos.x,destPos.y,destPos.x+destSize.x,destPos.y+destSize.y) );

			SIZE2I levelSize = destSize;
			levelSize /= (int)compressRate;
			levelSrc /= (int)compressRate;			

			for(size_t i = 0; i < mipMaps; ++i)
			{
				const IMG_MIPINFO* info = image.getMipInfo(i);
				region.mMipLevel = i;

				char* dest = (char*)glestex.lockNoCheck(region, lockflags);
				if( dest == NULL )
					BLADE_EXCEPT(EXC_API_ERROR, BTString("unable to lock texture.") );

				const char* source = (const char*)info->data + levelSrc.y*info->rowPitch + levelSrc.x;
				const size_t sourcePitch = info->rowPitch;
				size_t pitch = region.mRowPitch;
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
				glestex.unlock();

				levelSrc /= 2;
				levelSize /= 2;
			}//for mipmaps
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

#if BLADE_GLES_SHARED_CONTEXT

	//////////////////////////////////////////////////////////////////////////
	void GLESGraphicsResourceManager::onAsyncStart(bool rendering)
	{
		this->validateContext(rendering);
	}

	//////////////////////////////////////////////////////////////////////////
	void GLESGraphicsResourceManager::onAsyncEnd()
	{
		GLESContext* ctx = msSharedContexts[Thread::getCurrentID()];
		if (ctx != NULL)
			ctx->clearCurrent();
	}

	//////////////////////////////////////////////////////////////////////////
	void GLESGraphicsResourceManager::onSyncStart()
	{
		msContext->makeCurrent(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void GLESGraphicsResourceManager::onSyncEnd()
	{
		msContext->clearCurrent();
	}

	//////////////////////////////////////////////////////////////////////////
	void GLESGraphicsResourceManager::validateContext(bool rendering)
	{
		assert(msContext != NULL);
		Thread::ID tid = Thread::INVALID_ID;
		if (msContext->checkCurrent(&tid))
			return;

		if (tid == Thread::INVALID_ID)
			tid = Thread::getCurrentID();

		GLESContext*& ctx = msSharedContexts[tid];
		if (ctx != NULL)
		{
			if (rendering || !ctx->checkCurrent(tid))
				ctx->makeCurrent(rendering, &tid);
		}
		else
		{
			ctx = GLESContext::createContext();
			ctx->initialize(msContext);
			ctx->makeCurrent(rendering, &tid);
		}
	}
#endif

}//namespace Blade

#endif//#BLADE_USE_GLES