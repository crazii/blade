/********************************************************************
	created:	2012/12/29
	filename: 	GraphicsEffect.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsEffect_h__
#define __Blade_GraphicsEffect_h__
#include <interface/public/graphics/IGraphicsEffect.h>
#include <math/AxisAlignedBox.h>
#include <math/Vector3.h>
#include <utility/TList.h>
#include <utility/StringList.h>
#include <interface/public/graphics/Color.h>
#include <interface/public/geometry/GeomTypes.h>
#include <interface/public/graphics/GraphicsDefs.h>

namespace Blade
{

	/************************************************************************/
	/* bounding effect: host attached will show its bounding                                                                     */
	/************************************************************************/
	class IBoundingEffect : public IGraphicsEffect
	{
	public:
		BLADE_FRAMEWORK_API static const TString TYPE;
		BLADE_FRAMEWORK_API inline IBoundingEffect() :IGraphicsEffect(TYPE)	{mColor = Color::WHITE;mShow = true;}
		BLADE_FRAMEWORK_API virtual ~IBoundingEffect() {}

		/** @brief  */
		inline const Color&	getBoundingColor() const	{return mColor;}

		/** @brief  */
		inline bool			isBoundingShown() const		{return mShow;}

		/** @brief  */
		inline void			setBoundingColor(const Color& color)
		{
			if( mColor != color )
			{
				mColor = color;
				if(mShow)
					this->setBoundingColorImpl();
			}
		}

		/** @brief  */
		inline void			setShowBounding(bool shown)
		{
			if( mShow != shown )
			{
				mShow = shown;
				this->setShowBoundingImpl();
			}
		}

	protected:
		/** @brief  */
		virtual void	setBoundingColorImpl() = 0;
		/** @brief  */
		virtual void	setShowBoundingImpl() = 0;

		Color	mColor;
		bool	mShow;
	};//class AABBEffect

	typedef Handle<IBoundingEffect> HAABBEFFECT;


	/************************************************************************/
	/* decal effect: host attached will have an decal                                                                     */
	/************************************************************************/
	class IDecalEffect : public IGraphicsEffect
	{
	public:
		BLADE_FRAMEWORK_API static const TString TYPE;

		inline IDecalEffect() :IGraphicsEffect(IDecalEffect::TYPE)
		{
			mDynamic = false;
			mPosition = POINT3::ZERO;
			mSize = 0;
			mHeight = 0;
			mAppFlag = INVALID_APPFLAG;
		}
		virtual ~IDecalEffect()	{}

		/** @brief  */
		inline const TString&	getMaterial() const	{return mMaterialName;}
		/** @brief  */
		inline scalar			getSize() const		{return mSize;}
		/** @brief  */
		inline scalar			getHeight() const	{return mHeight;}
		/** @brief  */
		inline const POINT3&	getPosition() const	{return mPosition;}
		/** @brief  */
		BLADE_ALWAYS_INLINE AppFlag	getAppFlag() const	{return mAppFlag;}
		/** @brief  */
		inline bool				isDynamic() const	{return mDynamic;}

		/** @brief  */
		inline bool initialize(bool bDynamic, const AABB& aab, const TString& material, const TStringParam& textures, const TStringParam& samplers)
		{
			mDynamic = bDynamic;
			mPosition = aab.getCenter();
			mSize = aab.getHalfSize().x;
			mMaterialName = material;
			return this->initializeImpl(textures, samplers);
		}
		/** @brief  */
		inline bool initialize(bool bDynamic, const AABB& aab, const TString& texture, const TString& sampler)
		{
			mDynamic = bDynamic;
			mPosition = aab.getCenter();
			mSize = aab.getHalfSize().x*2;
			mHeight = aab.getHalfSize().y*2;
			//use default material
			//mMaterialName
			TStringParam params;
			params.push_back(texture);
			TStringParam samplers;
			samplers.push_back(sampler);
			return this->initializeImpl(params, samplers);
		}

		/** @brief  */
		inline void		setSize(scalar size)	{mSize = size;}
		/** @brief  */
		inline void		setHeight(scalar height){mHeight = height;}
		/** @brief  */
		inline void		setPosition(const POINT3& pos)	{mPosition = pos;}
		/** @brief  */
		inline void		setAppFlag(AppFlag flag)	{mAppFlag = flag;}
		/** @brief  */
		inline bool		setMaterial(const TString& materialName)
		{
			if( this->setMaterialImpl(materialName) )
			{
				mMaterialName = materialName;
				return true;
			}
			else
				return false;
		}
		/** @brief  */
		inline AABB getAABB() const
		{
			Vector3 size(mSize/2,mHeight/2,mSize/2);
			return AABB( mPosition - size, mPosition + size );
		}

		/** @brief  */
		virtual bool setDecalImages(const TStringParam& images, const TStringParam& samplers) = 0;
	protected:
		virtual bool initializeImpl(const TStringParam& textures, const TStringParam& samplers) = 0;
		virtual bool setMaterialImpl(const TString& materialName) = 0;

		//position relative to host or abs position if host is NULL
		POINT3			mPosition;	
		TString			mMaterialName;
		scalar			mSize;
		scalar			mHeight;
		//optional filter flags
		AppFlag			mAppFlag;
		//indicates this decal is dynamic updating or static(none-movable)
		bool			mDynamic;
	};//class IDecalEffect

	typedef Handle<IDecalEffect> HDECALEFFECT;


	/************************************************************************/
	/* brush decal: no need to attach to a host to work                                                                     */
	/************************************************************************/
	class IBrushDecalEffect : public IGraphicsEffect
	{
	public:
		BLADE_FRAMEWORK_API static const TString TYPE;

		inline IBrushDecalEffect() :IGraphicsEffect(IBrushDecalEffect::TYPE)
		{
			mPosition = POINT3::ZERO;
			mSize = 0;
			mHeight = 0;
			mAppFlag = FULL_APPFLAG;
			mColor = Color::RED;mVisible = true;
		}
		virtual ~IBrushDecalEffect()	{}

		/** @brief  */
		inline bool initialize(scalar size, scalar height, const TString& texture)
		{
			mPosition = POINT3::ZERO;
			mSize = size;
			mHeight = height;
			return this->initializeImpl(texture);
		}

		/** @brief  */
		inline scalar			getSize() const		{return mSize;}
		/** @brief  */
		inline scalar			getHeight() const	{return mHeight;}
		/** @brief  */
		inline const POINT3&	getPosition() const	{return mPosition;}
		/** @brief  */
		inline AppFlag			getAppFlag() const	{return mAppFlag;}
		/** @brief  */
		inline const Color& getColor() const{return mColor;}
		/** @brief  */
		inline bool		getVisible() const	{return mVisible;}

		/** @brief  */
		AABB	getAABB() const
		{
			Vector3 size(mSize/2,mHeight/2,mSize/2);
			return AABB( mPosition - size, mPosition + size );
		}

		/** @brief  */
		inline void		setSize(scalar size)	{mSize = size;}
		/** @brief  */
		inline void		setHeight(scalar height){mHeight = height;}
		/** @brief  */
		inline void		setPosition(const POINT3& pos)	{mPosition = pos;}
		/** @brief  */
		inline void		setAppFlag(AppFlag flag)	{mAppFlag = flag;}

		/** @brief  */
		inline bool setColor(const Color& color)
		{
			if( this->setColorImpl(color) )
			{
				mColor = color;
				return true;
			}
			return false;
		}
		/** @brief  */
		inline void setVisible(bool visible)	{mVisible = visible;}

		/** @brief query elements only, no faces/triangles */
		BLADE_FRAMEWORK_API size_t queryElement(PointerParam& outList, AppFlag appFlag = INVALID_APPFLAG, const TString& spaceName = TString::EMPTY);

		/** @brief  */
		virtual bool setDecalImage(const TString& image) = 0;
	protected:
		/** @brief  */
		virtual bool initializeImpl(const TString& texture) = 0;
		/** @brief  */
		virtual bool setColorImpl(const Color& color) = 0;

		//position relative to host or abs position if host is NULL
		POINT3			mPosition;	
		scalar			mSize;
		scalar			mHeight;
		//optional filter flags
		AppFlag			mAppFlag;
		Color	mColor;
		bool	mVisible;
	};//class IBrushDecalEffect

	typedef Handle<IBrushDecalEffect> HBRUSHDECALEFFECT;


	/************************************************************************/
	/* high light effect: host attached with this effect will be high-lighted                                                                      */
	/************************************************************************/
	class IHighLightEffect : public IGraphicsEffect
	{
	public:
		BLADE_FRAMEWORK_API static const TString TYPE;
		inline IHighLightEffect() :IGraphicsEffect(TYPE)	{mColor = Color::WHITE;}

		/** @brief  */
		inline void			setHighLightColor(const Color& color)	{mColor = color;}
		/** @brief  */
		inline const Color&	getHighLightColor() const				{return mColor;}

	protected:
		Color	mColor;
	};

	/************************************************************************/
	/* transparent effect: host attached with this effect will be transparent                                                                     */
	/************************************************************************/
	class ITransparentEffect : public IGraphicsEffect
	{
	public:
		BLADE_FRAMEWORK_API static const TString TYPE;
		inline ITransparentEffect() :IGraphicsEffect(TYPE)	{mStartTransparency = mEndTransparency = 0.5f;mFadeSpeed = 0.1f;}

		/** @brief  */
		inline void	setTransparency(scalar transparency)	{mStartTransparency = mEndTransparency = transparency;}
		/** @brief  */
		inline scalar	getTransparency() const				{return mEndTransparency;}
		/** @brief  */
		inline void	setFadeTransparency(scalar start,scalar end,scalar speed)
		{
			mStartTransparency = start;mEndTransparency = end;mFadeSpeed = speed;
		}

	protected:
		scalar	mStartTransparency;
		scalar	mEndTransparency;
		scalar	mFadeSpeed;
	};


	/************************************************************************/
	/* editor gizmo effect: editor only. show gizmo for host.                                                                      */
	/************************************************************************/
	class Ray;
	class IEditGizmoEffect : public IGraphicsEffect
	{
	public:
		BLADE_FRAMEWORK_API static const TString TYPE;

		inline IEditGizmoEffect() :IGraphicsEffect(TYPE)	{}
		virtual ~IEditGizmoEffect() {}

		/** @brief  */
		virtual EGizmoType	getGizmoType() const = 0;

		/** @brief  */
		virtual bool		isVisible() const = 0;

		/** @brief  */
		virtual bool		isEnabled() const = 0;

		/** @brief parameter enable will affect the appearance */
		virtual void		show(bool visible, bool enable, EGizmoType type = GT_MOVE, GEOM_SPACE space = GS_WORLD) = 0;

		/** @brief  */
		/* @note ray is in world space */
		virtual EAxis		hitTest(const Ray& ray, scalar* dist = NULL) const = 0;

		/** @brief  */
		virtual bool		highLightAxes(EAxis axis) = 0;
	};

	/************************************************************************/
	/* HUD effect: host attached with this effect will show a HUD icon on top of it                                                                     */
	/************************************************************************/
	class IHUDEffect : public IGraphicsEffect
	{
	public:
		BLADE_FRAMEWORK_API static const TString TYPE;
		inline IHUDEffect() :IGraphicsEffect(TYPE) { mVisible = false; mHighLight = false; }
		inline ~IHUDEffect()
		{
		}

		/** @brief  */
		inline void		setVisible(bool visible)
		{
			if( mVisible != visible )
			{
				mVisible = visible;
				this->showImpl(visible);
			}
		}

		inline void		setHighLight(bool highlit)
		{
			if (mHighLight != highlit)
			{
				mHighLight = highlit;
				this->setHighLightImpl(highlit);
			}
		}

		/** @brief  */
		inline void		setIcon(index_t index, const TString& texture)
		{
			this->setIconImpl(index, texture);
		}

		/** @brief  */
		virtual void	setPixelSize(size_t width, size_t height) = 0;

	protected:
		/** @brief  */
		virtual void	showImpl(bool visible) = 0;
		/** @brief  */
		virtual void	setIconImpl(index_t index, const TString& texture) = 0;
		/** @brief  */
		virtual void	setHighLightImpl(bool highlit) = 0;

		bool			mVisible;
		bool			mHighLight;
	};
	
}//namespace Blade

#endif//__Blade_GraphicsEffect_h__