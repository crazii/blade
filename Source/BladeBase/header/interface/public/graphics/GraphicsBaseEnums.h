/********************************************************************
	created:	2017/02/05
	filename: 	GraphicsBaseEnums.h
	author:		Crazii
	purpose:	public shared enumeration type for graphics systems
*********************************************************************/
#ifndef __Blade_GraphicsBaseEnumss_h__
#define __Blade_GraphicsBaseEnumss_h__

namespace Blade
{
	///color writing mask
	typedef enum EColorWritingChannel
	{
		CWC_NONE	= 0x00,
		CWC_RED		= 0x01,
		CWC_GREEN	= 0x02,
		CWC_BLUE	= 0x04,
		CWC_ALPHA	= 0x08,

		CWC_ALL		= CWC_RED | CWC_GREEN | CWC_BLUE | CWC_ALPHA,
	}COLOR_CHANNEL;

	///blend mode
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

	///blend operations
	typedef enum EBlendOp
	{
		BO_ADD = 1,
		BO_SUBSTRACT,
		BO_REVSUBTRACT,
		BO_MIN,
		BO_MAX,
	}BLEND_OP;

	/// test functions used for depth/stencil test etc.
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

	///stencil operations
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

	///fill mode
	typedef enum EGeometryFillMode
	{
		GFM_POINT		= 1,
		GFM_WIREFRAME	= 2,
		GFM_SOLID		= 3,
		GFM_DEFAULT,
	}FILL_MODE;

	///vertex winding
	typedef enum EHardwareCullMode
	{
		HCM_NONE = 1,
		HCM_CW,			//clockwise	(default setting)
		HCM_CCW,		//counter-clockwise
	}CULL_MODE;

	///texture addressing
	typedef enum ETextureAddressMode
	{
		TAM_WRAP =1,
		TAM_MIRROR,
		TAM_CLAMP,
	}TEXADDR_MODE;

	///texture filtering
	typedef enum ETextureFilterMode
	{
		TFM_NONE = 0,
		TFM_POINT,
		TFM_LINEAR,
		TFM_ANISOTROPIC,
	}TEXFILER_MODE;


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class Sampler : public Allocatable
	{
	public:
		Sampler()
		{
			std::memset(this, 0, sizeof(*this));
			mMaxAnisotropy = 0;
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
			mUseGlobalFilter = true;
			mUseGlobalAddressMode = true;
			mUseGlobalAnisotropy = true;
			mShadowDepthSampler = false;
		}

		Sampler(TEXADDR_MODE u, TEXADDR_MODE v, TEXADDR_MODE w, TEXFILER_MODE min, TEXFILER_MODE mag, TEXFILER_MODE mip, float minLOD = -FLT_MAX, float maxLOD = FLT_MAX)
		{
			std::memset(this, 0, sizeof(*this));
			mMaxAnisotropy = 0;
			mMipmapBias = 0;
			mMinLOD = minLOD;
			mMaxLOD = maxLOD;
			mMinFilter = min;
			mMagFilter = mag;
			mMipFilter = mip;
			mTest = TF_NEVER;
			mAddreesU = u;
			mAddreesV = v;
			mAddreesW = w;
			mUseGlobalFilter = false;
			mUseGlobalAddressMode = false;
			mUseGlobalAnisotropy = false;
			mShadowDepthSampler = false;
		}

		~Sampler()
		{

		}

		/** @brief  */
		inline bool operator==(const Sampler& rhs) const
		{
			return this == &rhs || std::memcmp(this, &rhs, sizeof(*this)) == 0;
		}

		/** @brief  */
		inline bool operator!=(const Sampler& rhs) const
		{
			return !(*this == rhs);
		}

		/** @brief  */
		inline bool			isUsingGlobalFilter() const
		{
			return mUseGlobalFilter;
		}

		/** @brief  */
		inline void			setUsingGlobalFilter(bool bGlobal)
		{
			mUseGlobalFilter = bGlobal;
		}

		/** @brief  */
		inline bool			isUsingGlobalAddressMode() const
		{
			return mUseGlobalAddressMode;
		}

		/** @brief  */
		inline void			setUsingGlobalAddressMode(bool bGlobal)
		{
			mUseGlobalAddressMode = bGlobal;
		}

		/** @brief  */
		inline bool			isUsingGlobalAnisotropy() const
		{
			return mUseGlobalAnisotropy;
		}

		/** @brief  */
		inline void			setUsingGlobalAnisotropy(bool bGlobal)
		{
			mUseGlobalAnisotropy = bGlobal;
		}

		/** @brief  */
		inline void			setAddressMode(TEXADDR_MODE u, TEXADDR_MODE v, TEXADDR_MODE w)
		{
			mAddreesU = u;	mAddreesV = v;	mAddreesW = w;
		}

		/** @brief  */
		inline TEXADDR_MODE	getAdressModeU() const
		{
			return (TEXADDR_MODE)mAddreesU;
		}
		/** @brief  */
		inline void			setAdressModeU(TEXADDR_MODE am)
		{
			mAddreesU = am;
		}

		/** @brief  */
		inline TEXADDR_MODE	getAdressModeV() const
		{
			return mAddreesV;
		}
		/** @brief  */
		inline void			setAdressModeV(TEXADDR_MODE am)
		{
			mAddreesV = am;
		}

		/** @brief  */
		inline TEXADDR_MODE	getAdressModeW() const
		{
			return mAddreesW;
		}
		/** @brief  */
		inline void			setAdressModeW(TEXADDR_MODE am)
		{
			mAddreesW = am;
		}

		/** @brief  */
		inline void			setFilterMode(TEXFILER_MODE minFilter, TEXFILER_MODE magFilter, TEXFILER_MODE mipFilter)
		{
			mMinFilter = minFilter;
			mMagFilter = magFilter;
			mMipFilter = mipFilter;
		}

		/** @brief  */
		inline TEXFILER_MODE	getMinFilter() const
		{
			return mMinFilter;
		}
		/** @brief  */
		inline void			setMinFilter(TEXFILER_MODE fm)
		{
			mMinFilter = fm;
		}

		/** @brief  */
		inline TEXFILER_MODE	getMagFilter() const
		{
			return mMagFilter;
		}
		/** @brief  */
		inline void			setMagFilter(TEXFILER_MODE fm)
		{
			mMagFilter = fm;
		}

		/** @brief  */
		inline TEXFILER_MODE	getMipFilter() const
		{
			return mMipFilter;
		}
		/** @brief  */
		inline void			setMipFilter(TEXFILER_MODE fm)
		{
			mMipFilter = fm;
		}

		/** @brief  */
		inline unsigned int	getMaxAnisotropy() const
		{
			return mMaxAnisotropy;
		}
		/** @brief  */
		inline void			setMaxAnisotropy(uint8 maxAnisotropy)
		{
			mMaxAnisotropy = maxAnisotropy;
		}

		/** @brief  */
		inline fp32			getMipmapLODBias() const
		{
			return mMipmapBias;
		}
		/** @brief  */
		inline void			setMipmapLODBias(fp32 bias)
		{
			mMipmapBias = bias;
		}

		/** @brief mipmap */
		inline fp32			getMinLOD() const
		{
			return mMinLOD;
		}
		/** @brief  */
		inline void			setMinLOD(fp32 minLOD)
		{
			mMinLOD = minLOD;
		}

		/** @brief mipmap */
		inline fp32			getMaxLOD() const
		{
			return mMaxLOD;
		}
		/** @brief  */
		inline void			setMaxLOD(fp32 maxLOD)
		{
			mMaxLOD = maxLOD;
		}

		/** @brief  */
		inline TEST_FUNC		getTestFunc() const
		{
			return mTest;
		}
		/** @brief  */
		inline void			setTestFunc(TEST_FUNC tf)
		{
			mTest = tf;
		}

		/** @brief  */
		inline bool isShadowSampler() const
		{
			return mShadowDepthSampler;
		}

		/** @brief  */
		inline void setShadowSampler(bool shadowSampler)
		{
			mShadowDepthSampler = shadowSampler;
		}

		static BLADE_BASE_API const Sampler	DEFAULT;
		static BLADE_BASE_API const Sampler DEFAULT_RTT;
		static BLADE_BASE_API const Sampler DEFAULT_RTT_DEPTH;

	protected:
		fp32		mMipmapBias;
		fp32		mMinLOD;
		fp32		mMaxLOD;
		TEXFILER_MODE	mMinFilter : 8;
		TEXFILER_MODE	mMagFilter : 8;
		TEXFILER_MODE	mMipFilter : 8;
		ETextureAddressMode		mAddreesU : 8;
		ETextureAddressMode		mAddreesV : 8;
		ETextureAddressMode		mAddreesW : 8;
		TEST_FUNC	mTest : 8;
		uint8		mMaxAnisotropy;
		bool		mUseGlobalFilter;
		bool		mUseGlobalAddressMode;
		bool		mUseGlobalAnisotropy;
		bool		mShadowDepthSampler;
	};//class Sampler

	typedef Handle<Sampler> HSAMPLER;
	
}//namespace Blade

#endif //  __Blade_GraphicsBaseEnumss_h__