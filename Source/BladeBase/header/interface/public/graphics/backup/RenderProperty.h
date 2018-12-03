/********************************************************************
	created:	2010/05/22
	filename: 	RenderState.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderState_h__
#define __Blade_RenderState_h__
#include <Handle.h>
#include <interface/public/graphics/Color.h>
#include <memory/Allocatable.h>
#include <utility/Mask.h>
#include <utility/FixedArray.h>
#include <math/Box2i.h>


namespace Blade
{
	typedef enum ERenderPropertyType
	{
		RPT_UNKNOWN		= 0,

		RPT_COLORWIRTE,
		RPT_FOG,
		RPT_ALPHACLIP,
		RPT_ALPHABLEND,
		RPT_DEPTH,
		RPT_STENCIL,
		RPT_SCISSOR,
		RPT_TEXTURE,

		RPT_COUNT,
	}RENDER_PROPERTY;


	class BLADE_BASE_API IRenderProperty
	{
	public:
		virtual ~IRenderProperty() {}

		/* @brief  */
		virtual RENDER_PROPERTY getType() const = 0;

	};//class IRenderProperty

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	typedef enum EColorChannel
	{
		CC_NONE		= 0x00,
		CC_RED		= 0x01,
		CC_GREEN	= 0x02,
		CC_BLUE		= 0x04,
		CC_ALPHA	= 0x08,

		CC_ALL		= CC_RED | CC_GREEN | CC_BLUE | CC_ALPHA,
	}COLOR_CHANNEL;

	class BLADE_BASE_API RenderColorWriteProperty : public IRenderProperty, public Allocatable<RenderColorWriteProperty>
	{
	public:
		inline RenderColorWriteProperty()	:mColorMask(CC_NONE)	{}
		inline ~RenderColorWriteProperty()							{}

		/* @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_COLORWIRTE;
		}

		/* @brief COLOR_CHANNEL,default CC_NONE  */
		inline int			getWritingColorChannel() const
		{
			return mColorMask;
		}
		/* @brief  */
		inline void		setWritingColorChannel(int mask)
		{
			mColorMask = mask;
		}

	protected:

		int	mColorMask;
	};//RenderColorWriteProperty

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API RenderFogProperty : public IRenderProperty, public Allocatable<RenderFogProperty>
	{
	public:
		inline RenderFogProperty()				{}
		inline ~RenderFogProperty()				{}


		/* @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_FOG;
		}

		//empty

	};//class RenderFogProperty


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//pure transparent
	class BLADE_BASE_API RenderAlphaClipProperty : public IRenderProperty, public Allocatable<RenderAlphaClipProperty>
	{
	public:
		inline RenderAlphaClipProperty()		{}
		inline ~RenderAlphaClipProperty()		{}


		/* @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_ALPHACLIP;
		}

		//no method/member

	};//class RenderAlphaClipProperty

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	typedef enum EBlendMode
	{
		BM_ZERO = 1,
		BM_ONE,
		BM_SRC_COLOR,
		BM_INV_SRC_COLOR,
		BM_SRC_ALPHA,
		BM_INV_SRC_ALPHA,
		BM_DEST_ALPHA,
		BM_INV_DEST_ALPHA,
		BM_DEST_COLOR,
		BM_INV_DEST_COLOR,
	}BLEND_MODE;

	typedef enum EBlendOp
	{
		BO_ADD = 1,
		BO_SUBSTRACT,
		BO_REVSUBTRACT,
		BO_MIN,
		BO_MAX,
	}BLEND_OP;

	class BLADE_BASE_API RenderAlphaBlendProperty : public IRenderProperty, public Allocatable<RenderAlphaBlendProperty>
	{
	public:
		RenderAlphaBlendProperty()
		{
			mSrcBlend = mSrcAlphaBlend = BM_ONE;
			mDestBlend = mDestAlphaBlend = BM_ZERO;
			mBlendOp = mBlendAlphaOp = BO_ADD;
		}

		~RenderAlphaBlendProperty()	{}

		/* @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_ALPHABLEND;
		}

		/* @brief  */
		inline BLEND_MODE	getSourceBlend() const
		{
			return mSrcBlend;
		}
		/* @brief  */
		inline void		setSourceBlend(BLEND_MODE bm)
		{
			mSrcBlend = bm;
		}

		/* @brief  */
		BLEND_MODE	getDestBlend() const
		{
			return mDestBlend;
		}
		/* @brief  */
		void		setDestBlend(BLEND_MODE bm)
		{
			mDestBlend = bm;
		}

		/* @brief  */
		BLEND_OP	getBlendOperation() const
		{
			return mBlendOp;
		}
		/* @brief  */
		void		setBlendOperation(BLEND_OP bo)
		{
			mBlendOp = bo;
		}

		/* @brief  */
		BLEND_MODE	getSourceAlphaBlend() const
		{
			return mSrcAlphaBlend;
		}
		/* @brief  */
		void		setSourceAlphaBlend(BLEND_MODE bm)
		{
			mSrcAlphaBlend = bm;
		}

		/* @brief  */
		BLEND_MODE	getDestAlphaBlend() const
		{
			return mDestAlphaBlend;
		}
		/* @brief  */
		void		setDestAlphaBlend(BLEND_MODE bm)
		{
			mDestAlphaBlend = bm;
		}

		/* @brief  */
		BLEND_OP	getAlphaBlendOperation() const
		{
			return mBlendAlphaOp;
		}
		/* @brief  */
		void		setAlphaBlendOperation(BLEND_OP bo)
		{
			mBlendAlphaOp = bo;
		}

	protected:
		BLEND_MODE	mSrcBlend;
		BLEND_MODE	mDestBlend;
		BLEND_OP	mBlendOp;

		BLEND_MODE	mSrcAlphaBlend;
		BLEND_MODE	mDestAlphaBlend;
		BLEND_OP	mBlendAlphaOp;
	};//class RenderAlphaBlendProperty

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//for depth test
	typedef enum ETestFunction
	{
		TF_NEVER = 1,
		TF_LESS,
		TF_EQUAL,
		TF_LESSEQUAL,
		TF_GREATER,
		TF_NOTEQUAL,
		TF_GREATEREQUAL,
		TF_ALWAYS,
	}TEST_FUNC;

	class BLADE_BASE_API RenderDepthProperty : public IRenderProperty, public Allocatable<RenderDepthProperty>
	{
	public:
		RenderDepthProperty()
		{
			mDepthTest = TF_LESS;
			mDepthBias = 0;
			mSlopeBias = 0.0f;
			mBiasClamp = 0.0f;
			mDepthWrite = true;
			mDepthClip = true;
		}
		~RenderDepthProperty()	{}

		/* @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_DEPTH;
		}

		/* @brief  */
		TEST_FUNC	getDepthTestFunc() const
		{
			return mDepthTest;
		}
		/* @brief  */
		void		setDepthTestFunc(TEST_FUNC tf)
		{
			mDepthTest = tf;
		}

		/* @brief  */
		bool		isWriteEnable() const
		{
			return mDepthWrite;
		}
		/* @brief  */
		void		setWriteEnable(bool enable)
		{
			mDepthWrite = enable;
		}

		/* @brief  */
		int			getDepthBias() const
		{
			return mDepthBias;
		}
		/* @brief  */
		void		setDepthBias(int bias)
		{
			mDepthBias = bias;
		}

		/* @brief  */
		scalar		getSlopeScaledDepthBias() const
		{
			return mSlopeBias;
		}
		/* @brief  */
		void		setSlopeScaledDepthBias(scalar SlopeScaledBias)
		{
			mSlopeBias = SlopeScaledBias;
		}

		/* @brief  */
		scalar		getDepthBiasClamp() const
		{
			return mBiasClamp;
		}
		/* @brief  */
		void		setDepthBiasClamp(scalar clamp)
		{
			mBiasClamp = clamp;
		}

		/* @brief  */
		bool		isEnableDepthClip() const
		{
			return mDepthClip;
		}
		/* @brief  */
		void		setEneableDepthClip(bool enable)
		{
			mDepthClip = enable;
		}

	protected:
		TEST_FUNC	mDepthTest;
		int			mDepthBias;
		scalar		mSlopeBias;
		scalar		mBiasClamp;
		bool		mDepthWrite;
		bool		mDepthClip;
	};//class RenderDepthProperty


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	typedef enum EStencilOperation
	{
		SOP_KEEP			= 1,
		SOP_ZERO,
		SOP_REPLACE,
		SOP_INCRESE_CLAMP,
		SOP_DECRESE_CLAMP,
		SOP_INVERT,
		SOP_INCRESE_WRAP,
		SOP_DECRESE_WRAP,
	}STENCIL_OP;

	class BLADE_BASE_API RenderStencilProperty : public IRenderProperty, public Allocatable<RenderStencilProperty>
	{
	public:
		RenderStencilProperty()
		{

		}
		~RenderStencilProperty()		{}

		/* @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_STENCIL;
		}

		/* @brief  */
		uint8		getReadMask() const
		{
			return mReadMask;
		}
		/* @brief  */
		void		setReadMask(uint8 mask)
		{
			mReadMask = mask;
		}

		/* @brief  */
		uint8		getWriteMask() const
		{
			return mWriteMask;
		}
		/* @brief  */
		void		setWriteMask(uint8 mask)
		{
			mWriteMask = mask;
		}

		/* @brief  */
		TEST_FUNC	getFrontFaceTestFunc() const
		{
			return mFrontFaceTest;
		}
		/* @brief  */
		void		setFrontFaceTestFunc(TEST_FUNC tf)
		{
			mFrontFaceTest = tf;
		}

		/* @brief stencil fail */
		STENCIL_OP	getFrontFaceFailOperation() const
		{
			return mFrontFaceFailOp;
		}
		/* @brief  */
		void		setFrontFaceFailOperation(STENCIL_OP op)
		{
			mFrontFaceFailOp = op;
		}

		/* @brief  stencil pass & depth fail */
		STENCIL_OP	getFrontFaceDepthFailOperation() const
		{
			return mFrontFaceDepthFailOp;
		}
		/* @brief  */
		void		setFrontFaceDepthFailOperation(STENCIL_OP op)
		{
			mFrontFaceDepthFailOp = op;
		}

		/* @brief  stencil pass & depth fail */
		STENCIL_OP	getFrontFacePassOperation() const
		{
			return mFrontFacePassOp;
		}
		/* @brief  */
		void		setFrontFacePassOperation(STENCIL_OP op)
		{
			mFrontFacePassOp = op;
		}

		/* @brief  */
		TEST_FUNC	getBackFaceTestFunc() const
		{
			return mBackFaceTest;
		}
		/* @brief  */
		void		setBackFaceTestFunc(TEST_FUNC tf)
		{
			mBackFaceTest = tf;
		}

		/* @brief stencil fail */
		STENCIL_OP	getBackFaceFailOperation() const
		{
			return mBackFaceFailOp;
		}
		/* @brief  */
		void		setBackFaceFailOperation(STENCIL_OP op)
		{
			mBackFaceFailOp = op;
		}

		/* @brief  stencil pass & depth fail */
		STENCIL_OP	getBackFaceDepthFailOperation() const
		{
			return mBackFaceDepthFailOp;
		}
		/* @brief  */
		void		setBackFaceDepthFailOperation(STENCIL_OP op)
		{
			mBackFaceDepthFailOp = op;
		}

		/* @brief  stencil pass & depth fail */
		STENCIL_OP	getBackFacePassOperation() const
		{
			return mBackFacePassOp;
		}
		/* @brief  */
		void		setBackFacePassOperation(STENCIL_OP op)
		{
			mBackFacePassOp = op;
		}

	protected:
		uint8		mReadMask;
		uint8		mWriteMask;
		TEST_FUNC	mFrontFaceTest;
		STENCIL_OP	mFrontFaceFailOp;
		STENCIL_OP	mFrontFaceDepthFailOp;
		STENCIL_OP	mFrontFacePassOp;

		TEST_FUNC	mBackFaceTest;
		STENCIL_OP	mBackFaceFailOp;
		STENCIL_OP	mBackFaceDepthFailOp;
		STENCIL_OP	mBackFacePassOp;
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class RenderScissorProperty : public IRenderProperty , public Allocatable<RenderScissorProperty>
	{
	public:
		enum
		{
			MAX_SCISSOR_COUNT = 16,
		};
	public:
		RenderScissorProperty();
		~RenderScissorProperty();

		/* @brief  */
		virtual RENDER_PROPERTY	getType() const	{return RPT_SCISSOR;}

		/* @brief  */
		void		setScissorBox(index_t index,const Box2i& rect)
		{
			if( index < MAX_SCISSOR_COUNT )
				mRects[index] = rect;
		}

		/* @brief  */
		void		setValidCount(size_t count)
		{
			mValidCount = count;
		}

		/* @brief  */
		size_t		getValidCount() const
		{
			return mValidCount;
		}

		/* @brief  */
		const Box2i*getScissorBoxes() const
		{
			return mRects;
		}

	protected:
		Box2i	mRects[MAX_SCISSOR_COUNT];
		size_t	mValidCount;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	typedef enum ETextureAddressMode
	{
		/// texcoord wraps at values over 1.0
		TAM_WRAP =1,
		/// texcoord mirrors (flips) at joins over 1.0
		TAM_MIRROR,
		/// texcoord clamps at 1.0
		TAM_CLAMP,
		/// texcoord outside the range [0.0, 1.0] are set to the border color
		TAM_BORDER
	}TEXADDR_MODE;

	typedef enum ETextureFilterMode
	{
		TFM_NONE = 0,
		TFM_POINT,
		TFM_LINEAR,
		TFM_ANISOTROPIC,
	}TEXFILER_MODE;

	typedef enum ETexturePropertyType
	{
		TPT_CUSTOM = -1,
		TPT_DIFFUSEMAP	= 0,
		TPT_NORMALMAP,
		TPT_DETAILMAP,
		TPT_GLOSSMAP,
		TPT_GLOWMAP,
		TPT_DARKMAP,
		TPT_LIGHTMAP,
	}TEX_PROPERTY;

	class BLADE_BASE_API RenderTextureSampler : public Allocatable<RenderTextureSampler>
	{
	public:
		RenderTextureSampler()
		{
			mBorderColor = Color::BLACK;
			mMaxAnisotropy = 16;
			mMipmapBias = 0;
			mMinLOD = -FLT_MAX;
			mMaxLOD = FLT_MAX;
			mMinFilter = TFM_LINEAR;
			mMagFilter = TFM_LINEAR;
			mMipFilter = TFM_LINEAR;
			mTest = TF_NEVER;
			mAddreesU = TAM_CLAMP;
			mAddreesV = TAM_CLAMP;
			mAddreesW = TAM_CLAMP;
			mTextureProperty = TPT_CUSTOM;
		}

		~RenderTextureSampler()
		{

		}

		/* @brief  */
		void			setAddressMode(TEXADDR_MODE u,TEXADDR_MODE v,TEXADDR_MODE w)
		{
			mAddreesU = u;	mAddreesV = v;	mAddreesW = w;
		}

		/* @brief  */
		TEXADDR_MODE	getAdressModeU() const
		{
			return mAddreesU;
		}
		/* @brief  */
		void			setAdressModeU(TEXADDR_MODE am)
		{
			mAddreesU = am;
		}

		/* @brief  */
		TEXADDR_MODE	getAdressModeV() const
		{
			return mAddreesV;
		}
		/* @brief  */
		void			setAdressModeV(TEXADDR_MODE am)
		{
			mAddreesV = am;
		}

		/* @brief  */
		TEXADDR_MODE	getAdressModeW() const
		{
			return mAddreesW;
		}
		/* @brief  */
		void			setAdressModeW(TEXADDR_MODE am)
		{
			mAddreesW = am;
		}

		/* @brief  */
		TEXFILER_MODE	getMinFilter() const
		{
			return mMinFilter;
		}
		/* @brief  */
		void			setMinFilter(TEXFILER_MODE fm)
		{
			mMinFilter = fm;
		}

		/* @brief  */
		TEXFILER_MODE	getMagFilter() const
		{
			return mMagFilter;
		}
		/* @brief  */
		void			setMagFilter(TEXFILER_MODE fm)
		{
			mMagFilter = fm;
		}

		/* @brief  */
		TEXFILER_MODE	getMipFilter() const
		{
			return mMipFilter;
		}
		/* @brief  */
		void			setMipFilter(TEXFILER_MODE fm)
		{
			mMipFilter = fm;
		}

		/* @brief  */
		const Color&	getBorderColor() const
		{
			return mBorderColor;
		}
		/* @brief  */
		void			setBorderColor(const Color& color)
		{
			mBorderColor = color;
		}

		/* @brief  */
		unsigned int	getMaxAnisotropy() const
		{
			return mMaxAnisotropy;
		}
		/* @brief  */
		void			setMaxAnisotropy(unsigned int maxAnisotropy)
		{
			mMaxAnisotropy = maxAnisotropy;
		}

		/* @brief  */
		int				getMipmapLODBias() const
		{
			return mMipmapBias;
		}
		/* @brief  */
		void			setMipmapLODBias(int bias)
		{
			mMipmapBias = bias;
		}

		/* @brief mipmap */
		fp32			getMinLOD() const
		{
			return mMinLOD;
		}
		/* @brief  */
		void			setMinLOD(fp32 minLOD)
		{
			mMinLOD = minLOD;
		}

		/* @brief mipmap */
		fp32			getMaxLOD() const
		{
			return mMaxLOD;
		}
		/* @brief  */
		void			setMaxLOD(fp32 maxLOD)
		{
			mMaxLOD = maxLOD;
		}

		/* @brief  */
		TEST_FUNC		getTestFunc() const
		{
			return mTest;
		}
		/* @brief  */
		void			setTestFunc(TEST_FUNC tf) 
		{
			mTest = tf;
		}

		/* @brief  */
		TEX_PROPERTY	getPropertyType() const
		{
			return mTextureProperty;
		}

		/* @brief  */
		void			setPropertyType(TEX_PROPERTY texproperty)
		{
			mTextureProperty = texproperty;
		}

	protected:
		Color				mBorderColor;
		unsigned int		mMaxAnisotropy;
		int					mMipmapBias;
		fp32				mMinLOD;
		fp32				mMaxLOD;
		TEXFILER_MODE		mMinFilter;
		TEXFILER_MODE		mMagFilter;
		TEXFILER_MODE		mMipFilter;
		TEST_FUNC			mTest;
		ETextureAddressMode	mAddreesU;
		ETextureAddressMode	mAddreesV;
		ETextureAddressMode	mAddreesW;
		TEX_PROPERTY		mTextureProperty;
	};//class RenderTextureSampler

	class BLADE_BASE_API RenderTextureProperty : public IRenderProperty, public Allocatable<RenderTextureProperty>
	{
	public:
		enum
		{
			MAX_TEXTURE_COUNT = 16,
		};
	public:
		RenderTextureProperty()
		{
			mPropertyMask = 0;
		}
		~RenderTextureProperty()
		{

		}

		/* @brief  */
		virtual RENDER_PROPERTY		getType() const
		{
			return RPT_TEXTURE;
		}

		/* @brief  */
		size_t						getSamplerCount() const
		{
			return mSamplers.size();
		}

		/* @brief  */
		const RenderTextureSampler*	getSampler(index_t index) const
		{
			if( index < mSamplers.size() )
				return &mSamplers[index];
			else
				return NULL;
		}

		/* @brief  */
		index_t						addSampler(const RenderTextureSampler& sampler)
		{
			if( mSamplers.size() < MAX_TEXTURE_COUNT )
			{
				mSamplers.push_back(sampler);

				if( sampler.getPropertyType() != TPT_CUSTOM )
					mPropertyMask.raiseBitAtIndex(sampler.getPropertyType());

				return mSamplers.size() - 1;
			}
			else
				return INVALID_INDEX;
		}

		/* @brief  */
		bool						setSampler(index_t index,const RenderTextureSampler& sampler)
		{
			if( index < mSamplers.size() )
			{
				RenderTextureSampler& oriSampler = mSamplers[index];
				if( oriSampler.getPropertyType() != TPT_CUSTOM )
					mPropertyMask.clearBitAtIndex( oriSampler.getPropertyType() );

				oriSampler = sampler;
				mPropertyMask.raiseBitAtIndex( sampler.getPropertyType() );
				return true;
			}
			else
				return false;
		}

		/* @brief  */
		bool						hasTextureProperty(TEX_PROPERTY tex_property)
		{
			if( tex_property == TPT_CUSTOM )
				return false;
			return mPropertyMask.checkBitAtIndex(tex_property);
		}


	protected:
		template class BLADE_BASE_API FixedVector<RenderTextureSampler,MAX_TEXTURE_COUNT>;
		typedef FixedVector<RenderTextureSampler,MAX_TEXTURE_COUNT> SamplerList;

		SamplerList		mSamplers;
		Mask			mPropertyMask;
	};//class RenderTextureProperty


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	typedef enum EGeometryFillMode
	{
		GFM_POINT		= 1,
		GFM_WIREFRAME	= 2,
		GFM_SOLID		= 3,
		GFM_DEFAULT,
	}FILL_MODE;

	typedef enum EHardwareCullMode
	{
		HCM_NONE = 1,
		//note this is inverse of d3d9 cull settings
		//because we use CCW as front face, just opposite to d3d9
		HCM_CCW,
		HCM_CW,
	}CULL_MODE;

	template class BLADE_BASE_API Handle<IRenderProperty>;
	typedef Handle<IRenderProperty>	HRenderProperty;

	class BLADE_BASE_API RenderPropertySet : public Allocatable<RenderPropertySet>
	{
	public:
		RenderPropertySet();
		~RenderPropertySet();

		/* @brief  */
		bool		isEnableLighting() const
		{
			return mEnableLighting;
		}

		/* @brief  */
		void		setEnableLighting(bool enable)
		{
			mEnableLighting = enable;
		}

		/* @brief  */
		FILL_MODE	getFillMode() const
		{
			return mFillMode;
		}

		/* @brief  */
		void		setFillMode(FILL_MODE fm)
		{
			mFillMode = fm;
		}

		/* @brief  */
		CULL_MODE	getCullMode() const
		{
			return mCullMode;
		}

		/* @brief  */
		void		setCullMode(CULL_MODE cm)
		{
			mCullMode = cm;
		}

		/* @brief  */
		bool		hasProperty(RENDER_PROPERTY eRP) const
		{
			return mPropertyMask.checkBitAtIndex( (index_t)eRP  );
		}

		/* @brief  */
		const HRenderProperty&	getRenderProperty(RENDER_PROPERTY eRP);

		/* @brief  */
		bool					addRenderProperty(const HRenderProperty& hProp);

		inline bool				addRenerProperty(IRenderProperty* prop)
		{
			if( prop )
			{
				HRenderProperty hProp;
				hProp.bind( prop );
				return this->addRenerProperty( hProp );
			}
			else
				return false;
		}

		/* @brief  */
		bool				removeRenderProperty(RENDER_PROPERTY eRP);

		/* @brief  */
		inline bool			removeRenderProperty(const HRenderProperty& hProp)
		{
			if( hProp == NULL )
				return false;
			else
			{
				return this->removeRenderProperty(hProp->getType());
			}
		}



	protected:

		FILL_MODE		mFillMode;
		CULL_MODE		mCullMode;
		bool			mEnableLighting;
		Mask			mPropertyMask;

		template class BLADE_BASE_API FixedArray<HRenderProperty,RPT_COUNT>;
		typedef FixedArray<HRenderProperty,RPT_COUNT>	PropertyList;
		PropertyList	mPropertyList;
	};//class RenderProperty
	
}//namespace Blade



#endif //__Blade_RenderState_h__