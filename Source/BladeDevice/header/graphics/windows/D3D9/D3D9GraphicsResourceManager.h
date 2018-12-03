/********************************************************************
	created:	2010/04/14
	filename: 	D3D9GraphicsResourceManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9GraphicsResourceManager_h__
#define __Blade_D3D9GraphicsResourceManager_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include "D3D9Resource.h"
#include <Singleton.h>
#include <utility/StringList.h>
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/graphics/ShaderInfoTable.h>

#include <graphics/windows/D3D9/D3D9VertexDeclaration.h>

namespace Blade
{
	class D3D9GraphicsResourceManager : public IGraphicsResourceManager ,public ID3D9LostResourceManager,public Singleton<D3D9GraphicsResourceManager>
	{
	public:
		static const TString TYPE;
	public:
		using Singleton<D3D9GraphicsResourceManager>::getSingleton;
		using Singleton<D3D9GraphicsResourceManager>::getSingletonPtr;
		D3D9GraphicsResourceManager();
		~D3D9GraphicsResourceManager();

		/************************************************************************/
		/* IGraphicsResourceManager interface                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const	{return TYPE;}

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
		virtual HVDECL		createVertexDeclaration()
		{
			HVDECL decl;
			decl.bind( BLADE_NEW D3D9VertexDeclaration() );
			return decl;
		}

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
			const POINT2I& srcPos = POINT2I::ZERO, const POINT2I& destPos = POINT2I::ZERO, const SIZE2I& size = SIZE2I::ZERO)const;

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
			const void* shaderTextBuffer, size_t shaderTextLen, const ParamList* macros = NULL) const ;

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
		virtual Matrix44&		convertProjectionMatrix(Matrix44& projectMatrix) const;

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
		/* ID3D9LostResourceManager interface                                                                     */
		/************************************************************************/
		/** @brief  */
		void				registerDefaultResource(ID3D9Resource* resource);

		/** @brief  */
		void				unRegisterDefaultResource(ID3D9Resource* resource);

		/** @brief */
		void				releaseDefaultResource();

		/** @brief  */
		void				rebuildDefaultResource();

	protected:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void		setD3D9Device(IDirect3DDevice9* device, IDirect3D9* d3d9, UINT adapter);

		/** @brief  */
		void				getShaderConstantInfo(EShaderType type, LPD3DXCONSTANTTABLE table, ShaderInfoTable& infoTable) const;

		/** @brief  */
		void				processConstantEntry(EShaderType type, LPD3DXCONSTANTTABLE table,D3DXHANDLE parent,const TString& prefix,UINT index, ShaderInfoTable& infoTable) const;

		/** @brief  */
		bool				forceUpdateTexture(const IImage& image, ITexture& texture,
			const POINT2I& srcPos = POINT2I::ZERO, const POINT2I& destPos = POINT2I::ZERO, const SIZE2I& size = SIZE2I::ZERO) const;

		/************************************************************************/
		/* statics                                                                     */
		/************************************************************************/
		/** @brief  */
		//static size_t		getElementSize(SHADER_CONSTANT_TYPE type);

		typedef Set<ID3D9Resource*>	DefualtResourceList;

		IDirect3DDevice9*	mDevice;
		DefualtResourceList	mDefaultResources;
		bool				mRenderResourceReleased;

		TStringList			mProfiles;
		GCONFIG				mConfig;
	};//class D3D9GraphicsResourceManager
	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif //__Blade_D3D9GraphicsResourceManager_h__