/********************************************************************
	created:	2014/12/19
	filename: 	GLESGraphicsResourceManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESGraphicsResourceManager_h__
#define __Blade_GLESGraphicsResourceManager_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <Singleton.h>
#include <utility/StringList.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <graphics/GLES/GLESContext.h>

namespace Blade
{
	class GLESGraphicsResourceManager : public IGraphicsResourceManager, public Singleton<GLESGraphicsResourceManager>
	{
	public:
		using Singleton<GLESGraphicsResourceManager>::getSingleton;
		using Singleton<GLESGraphicsResourceManager>::getSingletonPtr;
	public:
		static const TString TYPE;
	public:
		GLESGraphicsResourceManager();
		~GLESGraphicsResourceManager();

		/************************************************************************/
		/* IGraphicsResourceManager interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const
		{
			static TString TYPE = BTString("GLES");
			return TYPE;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HVBUFFER	createVertexBuffer(const void* data, size_t vertexSize,size_t vertexCount,IGraphicsBuffer::USAGE usage);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HIBUFFER	createIndexBuffer(const void* data, IIndexBuffer::EIndexType indexType,size_t indexCount,IGraphicsBuffer::USAGE usage);


		/*
		@describe 
		@param 
		@return 
		*/
		virtual HVDECL		createVertexDeclaration();

		/*
		@describe
		@param
		@return
		*/
		virtual HTEXTURE	createDepthStencilBuffer(size_t width, size_t height, PixelFormat format,IGraphicsBuffer::ACCESS GpuAccess = IGraphicsBuffer::GBAF_WRITE);

		/*
		@describe
		@param
		@return
		*/
		virtual HTEXTURE	createTexture(size_t width, size_t height, PixelFormat format,
			int mipLevels = -1, IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_DEFAULT,
			ETextureType type = TT_2DIM, IMG_ORIENT layout = IMGO_DEFAULT, size_t depth = 1);

		/*
		@describe for 2D texture loading from file,\n
		using GBU_STATIC (to create D3D11_USAGE_DEFAULT or D3D11_USAGE_IMMUTABLE texture
		@param 
		@return 
		*/
		virtual HTEXTURE	createTexture(const IImage& image, int mipMaps = IImage::MIPLOD_DEFAULT,  IGraphicsBuffer::USAGE usage = IGraphicsBuffer::GBU_STATIC);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		updateTexture(const IImage& image, ITexture& texture,
			const POINT2I& srcPos = POINT2I::ZERO, const POINT2I& destPos = POINT2I::ZERO, const SIZE2I& size = SIZE2I::ZERO) const;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t		createShaders(const HSHADERINTMEDIA* interMedias, HSHADER* outShaders, size_t count = 1);

		/*
		@describe create shader media from a compiled data
		@param
		@return
		*/
		virtual HSHADERINTMEDIA	createShaderIntermediate(const TString& name, const void* data, size_t bytes,
			const TString& profile, const TString entrypoint[SHADER_TYPE_COUNT]);

		/*
		@describe compile shader from a loaded text buffer
		@param
		@return
		*/
		virtual HSHADERINTMEDIA	compileShader(const TString& textFileFullPath, const ParamList& option,
			const void* shaderTextBuffer, size_t shaderTextLen, const ParamList* macros = NULL) const;

		/*
		@describe
		@param
		@return
		*/
		virtual const TStringList&	getSupportedProfiles() const;

		/*
		@describe get the size in floats of the specific shader constant type
		@param
		@return
		*/
		virtual const TString&		getHighestProfile() const;

		/*
		@describe get the size in bytes of the shader constant type
		@note the size is API related
		@param 
		@return 
		*/
		virtual size_t				getShaderConstantSize(SHADER_CONSTANT_TYPE type) const;

		/*
		@describe convert projection matrix into platform/API matrix
		@param 
		@return 
		*/
		virtual Matrix44&			convertProjectionMatrix(Matrix44& projectMatrix) const
		{
			return projectMatrix;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const GCONFIG&	getGraphicsConfig() const
		{
			return mConfig;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool				forceUpdateTexture(const IImage& image, ITexture& texture,
			const POINT2I& srcPos = POINT2I::ZERO, const POINT2I& destPos = POINT2I::ZERO, const SIZE2I& size = SIZE2I::ZERO) const;

		/** @brief  */
		static inline void setContex(GLESContext* context) { msContext = context; }

#if BLADE_GLES_SHARED_CONTEXT
		/** @brief  */
		void onAsyncStart(bool rendering);

		/** @brief  */
		void onAsyncEnd();

		/** @brief  */
		void onSyncStart();

		/** @brief  */
		void onSyncEnd();

		/** @brief  */
		static void validateContext(bool rendering);
#else
		static BLADE_ALWAYS_INLINE void validateContext(bool) { }
#endif
	protected:
		GCONFIG				mConfig;
		TStringList			mShaderProfiles;

		static GLESContext*	msContext;	//link

		typedef StaticMap<Thread::ID, GLESContext*> ContextMap;
		static ContextMap msSharedContexts;
	};//class GLESGraphicsResourceManager

}//namespace Blade


#endif//BLADE_USE_GLES

#endif//__Blade_GLESGraphicsResourceManager_h__