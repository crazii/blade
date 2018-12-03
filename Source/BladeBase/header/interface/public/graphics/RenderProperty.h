/********************************************************************
	created:	2010/05/22
	filename: 	RenderProperty.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderProperty_h__
#define __Blade_RenderProperty_h__
#include <Handle.h>
#include <utility/Mask.h>
#include <utility/FixedArray.h>
#include <utility/String.h>
#include <utility/TList.h>
#include <math/Box2i.h>
#include "GraphicsBaseEnums.h"
#include "Color.h"


namespace Blade
{
	typedef enum ERenderPropertyType
	{
		RPT_UNKNOWN		= 0,

		RPT_COLOR,
		RPT_COLORWIRTE,
		RPT_FOG,
		RPT_ALPHABLEND,
		RPT_DEPTH,
		RPT_STENCIL,
		RPT_SCISSOR,

		RPT_COUNT,
		RPT_BEGIN = RPT_UNKNOWN + 1,
	}RENDER_PROPERTY;


	class BLADE_BASE_API IRenderProperty
	{
	public:
		virtual ~IRenderProperty() {}

		/** @brief  */
		virtual RENDER_PROPERTY getType() const = 0;

		static IRenderProperty* createProperty(RENDER_PROPERTY eProp);
		static IRenderProperty* cloneProperty(IRenderProperty* prop);
	};//class IRenderProperty

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT ColorProperty : public IRenderProperty, public Allocatable
	{
	public:
		inline ColorProperty()	:mDiffuse(Color::WHITE),mSpecular(Color::WHITE),mEmissive(Color::WHITE)	{}
		inline ~ColorProperty()	{}

		/** @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_COLOR;
		}

		/** @brief  */
		const Color&	getDiffuse() const
		{
			return mDiffuse;
		}

		/** @brief  */
		void			setDiffuse(const Color& diffuse)
		{
			mDiffuse = diffuse;
		}

		/** @brief  */
		const Color&	getSpecular() const
		{
			return mSpecular;
		}

		/** @brief  */
		void			setSpecular(const Color& specular)
		{
			mSpecular = specular;
		}

		/** @brief  */
		const Color&	getEmissive() const
		{
			return mEmissive;
		}

		/** @brief  */
		void			setEmissive(const Color& emissive)
		{
			mEmissive = emissive;
		}

	protected:
		Color	mDiffuse;
		Color	mSpecular;
		Color	mEmissive;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT ColorWriteProperty : public IRenderProperty, public Allocatable
	{
	public:
		inline ColorWriteProperty()	:mColorMask(CWC_NONE)	{}
		inline ~ColorWriteProperty()							{}

		/** @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_COLORWIRTE;
		}

		/** @brief COLOR_CHANNEL,default CC_NONE  */
		inline int			getWritingColorChannel() const
		{
			return mColorMask;
		}
		/** @brief  */
		inline void		setWritingColorChannel(int mask)
		{
			mColorMask = mask;
		}

	protected:

		int	mColorMask;
	};//ColorWriteProperty

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT FogProperty : public IRenderProperty, public Allocatable
	{
	public:
		inline FogProperty()				{}
		inline ~FogProperty()				{}


		/** @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_FOG;
		}

		//empty

	};//class FogProperty

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT AlphaBlendProperty : public IRenderProperty, public Allocatable
	{
	public:
		AlphaBlendProperty()
		{
			mBlendFactor = 1.0f;
			mSrcBlend = mSrcAlphaBlend = BM_ONE;
			mDestBlend = mDestAlphaBlend = BM_ZERO;
			mBlendOp = mBlendAlphaOp = BO_ADD;
			mIndependent = false;
		}

		~AlphaBlendProperty()	{}

		/** @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_ALPHABLEND;
		}

		/** @brief  */
		inline BLEND_MODE	getSourceBlend() const
		{
			return mSrcBlend;
		}
		/** @brief  */
		inline void		setSourceBlend(BLEND_MODE bm)
		{
			mSrcBlend = bm;
		}

		/** @brief  */
		BLEND_MODE	getDestBlend() const
		{
			return mDestBlend;
		}
		/** @brief  */
		void		setDestBlend(BLEND_MODE bm)
		{
			mDestBlend = bm;
		}

		/** @brief  */
		BLEND_OP	getBlendOperation() const
		{
			return mBlendOp;
		}
		/** @brief  */
		void		setBlendOperation(BLEND_OP bo)
		{
			mBlendOp = bo;
		}

		/** @brief  */
		BLEND_MODE	getSourceAlphaBlend() const
		{
			return mSrcAlphaBlend;
		}
		/** @brief  */
		void		setSourceAlphaBlend(BLEND_MODE bm)
		{
			mSrcAlphaBlend = bm;
		}

		/** @brief  */
		BLEND_MODE	getDestAlphaBlend() const
		{
			return mDestAlphaBlend;
		}
		/** @brief  */
		void		setDestAlphaBlend(BLEND_MODE bm)
		{
			mDestAlphaBlend = bm;
		}

		/** @brief  */
		BLEND_OP	getAlphaBlendOperation() const
		{
			return mBlendAlphaOp;
		}
		/** @brief  */
		void		setAlphaBlendOperation(BLEND_OP bo)
		{
			mBlendAlphaOp = bo;
		}

		/** @brief blend factor used in shader to control transparency */
		scalar		getBlendFactor() const
		{
			return mBlendFactor;
		}

		/** @brief  */
		void		setBlendFactor(scalar opaticy)
		{
			mBlendFactor = opaticy;
		}

		/** @brief  */
		bool		getIndependentBlend() const
		{
			return mIndependent;
		}

		/** @brief  */
		void		setIndependentBlend(bool independent)
		{
			mIndependent = independent;
		}

	protected:
		scalar		mBlendFactor;
		BLEND_MODE	mSrcBlend		: 8;
		BLEND_MODE	mDestBlend		: 8;
		BLEND_MODE	mSrcAlphaBlend	: 8;
		BLEND_MODE	mDestAlphaBlend	: 8;
		BLEND_OP	mBlendOp		: 8;
		BLEND_OP	mBlendAlphaOp	: 8;
		bool		mIndependent;
	};//class AlphaBlendProperty

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT DepthProperty : public IRenderProperty, public Allocatable
	{
	public:
		DepthProperty()
		{
			mDepthTestFunc = TF_LESS;
			mDepthBias = 0;
			mSlopeBias = 0.0f;
			mBiasClamp = 0.0f;
			mDepthTest = true;
			mDepthWrite = true;
			mDepthClip = true;
		}
		~DepthProperty()	{}

		/** @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_DEPTH;
		}

		/** @brief  */
		TEST_FUNC	getDepthTestFunc() const
		{
			return mDepthTestFunc;
		}
		/** @brief  */
		void		setDepthTestFunc(TEST_FUNC tf)
		{
			mDepthTestFunc = tf;
		}

		/** @brief  */
		bool		isTestEnable() const
		{
			return mDepthTest;
		}

		/** @brief  */
		void		setTestEnable(bool enable)
		{
			mDepthTest = enable;
		}

		/** @brief  */
		bool		isWriteEnable() const
		{
			return mDepthWrite;
		}
		/** @brief  */
		void		setWriteEnable(bool enable)
		{
			mDepthWrite = enable;
		}

		/** @brief  */
		int			getDepthBias() const
		{
			return mDepthBias;
		}
		/** @brief  */
		void		setDepthBias(int bias)
		{
			mDepthBias = bias;
		}

		/** @brief  */
		scalar		getSlopeScaledDepthBias() const
		{
			return mSlopeBias;
		}
		/** @brief  */
		void		setSlopeScaledDepthBias(scalar SlopeScaledBias)
		{
			mSlopeBias = SlopeScaledBias;
		}

		/** @brief  */
		scalar		getDepthBiasClamp() const
		{
			return mBiasClamp;
		}
		/** @brief  */
		void		setDepthBiasClamp(scalar clamp)
		{
			mBiasClamp = clamp;
		}

		/** @brief  */
		bool		isEnableDepthClip() const
		{
			return mDepthClip;
		}
		/** @brief  */
		void		setEneableDepthClip(bool enable)
		{
			mDepthClip = enable;
		}

	protected:
		int			mDepthBias;
		scalar		mSlopeBias;
		scalar		mBiasClamp;
		TEST_FUNC	mDepthTestFunc : 8;
		bool		mDepthTest;
		bool		mDepthWrite;
		bool		mDepthClip;
	};//class DepthProperty


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT StencilProperty : public IRenderProperty, public Allocatable
	{
	public:
		StencilProperty()
		{
			mEnable = false;
		}
		~StencilProperty()		{}

		/** @brief  */
		virtual RENDER_PROPERTY getType() const
		{
			return RPT_STENCIL;
		}

		/** @brief  */
		bool		isEnable() const
		{
			return mEnable;
		}

		/** @brief  */
		void		setEnable(bool enable)
		{
			mEnable = enable;
		}

		/** @brief  */
		uint8		getReadMask() const
		{
			return mReadMask;
		}
		/** @brief  */
		void		setReadMask(uint8 mask)
		{
			mReadMask = mask;
		}

		/** @brief  */
		uint8		getWriteMask() const
		{
			return mWriteMask;
		}
		/** @brief  */
		void		setWriteMask(uint8 mask)
		{
			mWriteMask = mask;
		}

		/** @brief  */
		uint		getRef() const
		{
			return mRef;
		}

		/** @brief  */
		void		setRef(uint _ref)
		{
			mRef = _ref;
		}

		/** @brief  */
		TEST_FUNC	getFrontFaceTestFunc() const
		{
			return mFrontFaceTest;
		}
		/** @brief  */
		void		setFrontFaceTestFunc(TEST_FUNC tf)
		{
			mFrontFaceTest = tf;
		}

		/** @brief stencil fail */
		STENCIL_OP	getFrontFaceFailOperation() const
		{
			return mFrontFaceFailOp;
		}
		/** @brief  */
		void		setFrontFaceFailOperation(STENCIL_OP op)
		{
			mFrontFaceFailOp = op;
		}

		/** @brief  stencil pass & depth fail */
		STENCIL_OP	getFrontFaceDepthFailOperation() const
		{
			return mFrontFaceDepthFailOp;
		}
		/** @brief  */
		void		setFrontFaceDepthFailOperation(STENCIL_OP op)
		{
			mFrontFaceDepthFailOp = op;
		}

		/** @brief  stencil pass & depth fail */
		STENCIL_OP	getFrontFacePassOperation() const
		{
			return mFrontFacePassOp;
		}
		/** @brief  */
		void		setFrontFacePassOperation(STENCIL_OP op)
		{
			mFrontFacePassOp = op;
		}

		/** @brief  */
		TEST_FUNC	getBackFaceTestFunc() const
		{
			return mBackFaceTest;
		}
		/** @brief  */
		void		setBackFaceTestFunc(TEST_FUNC tf)
		{
			mBackFaceTest = tf;
		}

		/** @brief stencil fail */
		STENCIL_OP	getBackFaceFailOperation() const
		{
			return mBackFaceFailOp;
		}
		/** @brief  */
		void		setBackFaceFailOperation(STENCIL_OP op)
		{
			mBackFaceFailOp = op;
		}

		/** @brief  stencil pass & depth fail */
		STENCIL_OP	getBackFaceDepthFailOperation() const
		{
			return mBackFaceDepthFailOp;
		}
		/** @brief  */
		void		setBackFaceDepthFailOperation(STENCIL_OP op)
		{
			mBackFaceDepthFailOp = op;
		}

		/** @brief  stencil pass & depth fail */
		STENCIL_OP	getBackFacePassOperation() const
		{
			return mBackFacePassOp;
		}
		/** @brief  */
		void		setBackFacePassOperation(STENCIL_OP op)
		{
			mBackFacePassOp = op;
		}

	protected:
		uint		mRef;
		TEST_FUNC	mFrontFaceTest			: 8;
		STENCIL_OP	mFrontFaceFailOp		: 8;
		STENCIL_OP	mFrontFaceDepthFailOp	: 8;
		STENCIL_OP	mFrontFacePassOp		: 8;

		TEST_FUNC	mBackFaceTest			: 8;
		STENCIL_OP	mBackFaceFailOp			: 8;
		STENCIL_OP	mBackFaceDepthFailOp	: 8;
		STENCIL_OP	mBackFacePassOp			: 8;
		uint8		mReadMask;
		uint8		mWriteMask;
		bool		mEnable;
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT ScissorProperty : public IRenderProperty , public Allocatable
	{
	public:
		enum
		{
			MAX_SCISSOR_COUNT = 8,
		};
	public:
		ScissorProperty()		{}
		~ScissorProperty()	{}

		/** @brief  */
		virtual RENDER_PROPERTY	getType() const	{return RPT_SCISSOR;}

		/** @brief  */
		void		setScissorBox(index_t index,const Box2i& rect)
		{
			if( index < MAX_SCISSOR_COUNT )
				mRects[index] = rect;
		}

		/** @brief  */
		void		setValidCount(size_t count)
		{
			mValidCount = count;
		}

		/** @brief  */
		size_t		getValidCount() const
		{
			return mValidCount;
		}

		/** @brief  */
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
	typedef Handle<IRenderProperty>	HRENDERPROPERTY;

	class RenderPropertySet : public Allocatable
	{
	public:
		RenderPropertySet()
			:mPropertyMask(0)
			, mFillMode(GFM_SOLID)
			, mCullMode(HCM_CCW)
		{}

		~RenderPropertySet() {}

		/** @brief  */
		FILL_MODE	getFillMode() const
		{
			return mFillMode;
		}

		/** @brief  */
		void		setFillMode(FILL_MODE fm) const
		{
			mFillMode = fm;
		}

		/** @brief  */
		CULL_MODE	getCullMode() const
		{
			return mCullMode;
		}

		/** @brief  */
		void		setCullMode(CULL_MODE cm)
		{
			mCullMode = cm;
		}

		/** @brief  */
		bool		hasProperty(RENDER_PROPERTY eRP) const
		{
			return mPropertyMask.checkBitAtIndex((index_t)eRP);
		}

		/** @brief  */
		const HRENDERPROPERTY&	getProperty(RENDER_PROPERTY eRP) const
		{
			if (eRP < RPT_COUNT)
				return mPropertyList[eRP];
			else
				return HRENDERPROPERTY::EMPTY;
		}

		/** @brief  */
		BLADE_BASE_API bool		addProperty(const HRENDERPROPERTY& hProp);

		inline bool				addProperty(IRenderProperty* prop)
		{
			if( prop )
			{
				assert(this->getProperty(prop->getType()) != prop);
				return this->addProperty(HRENDERPROPERTY(prop));
			}
			else
				return false;
		}

		/** @brief  */
		BLADE_BASE_API bool		setProperty(const HRENDERPROPERTY& hProp);

		inline bool				setProperty(IRenderProperty* prop)
		{
			if( prop )
			{
				if (this->getProperty(prop->getType()) != prop)
					return this->setProperty(HRENDERPROPERTY(prop));
				return true;
			}
			else
				return false;
		}

		/** @brief  */
		BLADE_BASE_API bool				removeProperty(RENDER_PROPERTY eRP);

		/** @brief  */
		inline bool			removeProperty(const HRENDERPROPERTY& hProp)
		{
			if( hProp == NULL )
				return false;
			else if(this->getProperty(hProp->getType()) == hProp)
				return this->removeProperty(hProp->getType());
			assert(false);
			return false;
		}

		/** @brief merge source into this, if a property already exist, replace it in this and use source(shadowing) */
		BLADE_BASE_API void				mergeProperties(const RenderPropertySet& mergeSource);

		/** @brief  */
		BLADE_BASE_API static const Handle<RenderPropertySet>&	getDefaultRenderProperty();

	protected:
		HRENDERPROPERTY	mPropertyList[RPT_COUNT];
		Mask		mPropertyMask;
		mutable FILL_MODE	mFillMode : 8;
		CULL_MODE	mCullMode : 8;
	};//class RenderPropertySet

	typedef Handle<RenderPropertySet> HRENDERPROPTYSET;


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT SamplerState : public Allocatable
	{
	public:
		SamplerState() {}
		~SamplerState() {}

		/** @brief  */
		size_t				getSamplerCount() const
		{
			return mSamplers.size();
		}

		/** @brief  */
		const HSAMPLER&		getSampler(index_t index) const
		{
			return index < mSamplers.size() ? mSamplers[index] : HSAMPLER::EMPTY;
		}

		/** @brief  */
		index_t				getSamplerIndex(const TString& samplerName) const
		{
			return mSamplers.find(samplerName);
		}

		/** @brief  */
		void				addSampler(const TString& name, const Sampler& sampler)
		{
			mSamplers[name] = HSAMPLER(BLADE_NEW Sampler(sampler));
		}

		/** @brief  */
		bool				setSampler(index_t index, const Sampler& sampler)
		{
			if (index < mSamplers.size())
			{
				mSamplers.at(index).mValue = HSAMPLER(BLADE_NEW Sampler(sampler));
				return true;
			}
			return false;
		}

	private:
		typedef TNamedList<HSAMPLER>	SamplerList;

		SamplerList	mSamplers;
	};//class TextureProperty
	
}//namespace Blade



#endif //__Blade_RenderProperty_h__