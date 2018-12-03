/********************************************************************
	created:	2010/04/27
	filename: 	View.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_View_h__
#define __Blade_View_h__
#include <BladeGraphics.h>
#include <Handle.h>
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <interface/IRenderScheme.h>

namespace Blade
{
	class ICamera;

	class View : public IGraphicsView  , public Allocatable
	{
	public:
		View(const TString& name);
		~View();

		/************************************************************************/
		/* IRenderView Interface                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual int32			getPixelLeft() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual int32			getPixelRight() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual int32			getPixelTop() const;


		/*
		@describe 
		@param 
		@return 
		*/
		virtual int32			getPixelBottom() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getRelativeLeft() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getRelativeRight() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getRelativeTop() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual scalar			getRelativeBottom() const;

		/**
		@describe 
		@param
		@return
		*/
		virtual void			getSizePosition(scalar relatives[4], size_t offsets[4]) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Color&	getBackgroundColor() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual uint32			getClearMode() const;

		/*
		@describe
		@param
		@return
		*/
		virtual scalar			getClearDepth() const;

		/*
		@describe
		@param
		@return
		*/
		virtual uint16			getClearStencil() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setSizePosition(scalar left,scalar top,scalar right,scalar bottom,
			size_t leftOffset = 0,size_t topOffset = 0,size_t rightOffset = 0, size_t bottomOffset = 0);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setBackgroundColor(const Color& color);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setClearMode(uint32 frameTypeMode,scalar depth =1.0,uint16 stencil = 0);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			calculateSizeInPixel(IRenderTarget* target);

		/**
		@describe 
		@param
		@return
		*/
		virtual void			calculateSizeInPixel(ITexture* rtt);


		/************************************************************************/
		/* IGraphicsView                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const TString&			getRenderScheme() const;

		/*
		@describe
		@param
		@return
		*/
		virtual void					setRenderScheme(const TString& schemeName);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t					getChannelCount() const;

		/*
		@describe get the render queues appended
		@param
		@return
		*/
		virtual IGraphicsChannel*		getChannel(index_t index) const;

		/*
		@describe get the render queues appended
		@param
		@return
		*/
		virtual IGraphicsChannel*		getChannel(const TString& channelRenderType) const;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t					getPhaseCount() const;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsPhase*			getPhase(index_t index) const;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsPhase*			getPhase(const TString& phaseName) const;


		/*
		@describe 
		@param 
		@return 
		*/
		virtual void					setFillMode(FILL_MODE mode);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual FILL_MODE				getFillMode() const;

		/**
		@describe
		@param
		@return
		*/
		virtual void				enableImageEffect(bool enable);

		/**
		@describe
		@param
		@return
		*/
		virtual bool				isImageEffectEnabled() const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		const TString&			getName() const;


		/*
		@describe
		@param [in] bAutoDelete: auto delete the scheme being attached on clean up.
		@param [in] deleteOld: delete the original scheme that already attached.
		@return
		*/
		IRenderScheme*			attachRenderScheme(IRenderScheme* scheme, bool bAutoDelete = false, bool deleteOld = false);

		/*
		@describe
		@param
		@return
		*/
		IRenderScheme*			getAttachedRenderScheme() const;

		/*
		@describe 
		@param 
		@return 
		*/
		IRenderPipeline*		getRenderPipeline() const;

	protected:
		IRenderScheme*	mScheme;
		IRenderPipeline*mPipeline;
		
		uint32			mClearMode;
		Color			mBackColor;
		scalar			mClearDepth;
		uint16			mClearStencil;
		bool			mAutoDeleteScheme;
		bool			mEnableImageFX;

		scalar			mRelativeLeft;
		scalar			mRelativeTop;
		scalar			mRelativeRight;
		scalar			mRelativeBottom;
		size_t			mOffsetLeft;
		size_t			mOffsetTop;
		size_t			mOffsetRight;
		size_t			mOffsetBottom;

		int32			mPixelLeft;
		int32			mPixelRight;
		int32			mPixelTop;
		int32			mPixelBottom;

		TString			mName;

		FILL_MODE		mFillMode;
		TString			mRenderSchemeName;
	};//class View
	
}//namespace Blade

#endif //__Blade_View_h__