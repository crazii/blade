/********************************************************************
	created:	2010/04/28
	filename: 	View.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "View.h"
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/ICamera.h>
#include <interface/IRenderSchemeManager.h>


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	View::View(const TString& name)
		:mScheme(NULL)
		,mPipeline(NULL)
		,mClearMode(FBT_ALL)
		,mBackColor(Color::BLACK_ALPHA)
		,mClearDepth(1.0f)
		,mClearStencil(0)
		,mAutoDeleteScheme(false)
		,mEnableImageFX(true)
		,mRelativeLeft(0.0f)
		,mRelativeTop(0.0f)
		,mRelativeRight(1.0f)
		,mRelativeBottom(1.0f)
		,mOffsetLeft(0)
		,mOffsetTop(0)
		,mOffsetRight(0)
		,mOffsetBottom(0)
		,mPixelLeft(0)
		,mPixelRight(0)
		,mPixelTop(0)
		,mPixelBottom(0)
		,mName(name)
		,mFillMode(GFM_DEFAULT)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	View::~View()
	{
		BLADE_DELETE mPipeline;
		if( mAutoDeleteScheme )
			IRenderSchemeManager::getSingleton().destroyRenderScheme(mScheme);
	}

	/************************************************************************/
	/* IRenderView Interface                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	int32			View::getPixelLeft() const
	{
		return mPixelLeft;
	}

	//////////////////////////////////////////////////////////////////////////
	int32			View::getPixelRight() const
	{
		return mPixelRight;
	}

	//////////////////////////////////////////////////////////////////////////
	int32			View::getPixelTop() const
	{
		return mPixelTop;
	}

	//////////////////////////////////////////////////////////////////////////
	int32			View::getPixelBottom() const
	{
		return mPixelBottom;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			View::getRelativeLeft() const
	{
		return mRelativeLeft;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			View::getRelativeRight() const
	{
		return mRelativeRight;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			View::getRelativeTop() const
	{
		return mRelativeTop;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			View::getRelativeBottom() const
	{
		return mRelativeBottom;
	}

	//////////////////////////////////////////////////////////////////////////
	void			View::getSizePosition(scalar relatives[4], size_t offsets[4]) const
	{
		relatives[0] = mRelativeLeft;
		relatives[1] = mRelativeTop;
		relatives[2] = mRelativeRight;
		relatives[3] = mRelativeBottom;
		offsets[0] = mOffsetLeft;
		offsets[1] = mOffsetTop;
		offsets[2] = mOffsetRight;
		offsets[3] = mOffsetBottom;
	}

	//////////////////////////////////////////////////////////////////////////
	const Color&	View::getBackgroundColor() const
	{
		return mBackColor;
	}

	//////////////////////////////////////////////////////////////////////////
	uint32			View::getClearMode() const
	{
		return mClearMode;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			View::getClearDepth() const
	{
		return mClearDepth;
	}

	//////////////////////////////////////////////////////////////////////////
	uint16			View::getClearStencil() const
	{
		return mClearStencil;
	}

	//////////////////////////////////////////////////////////////////////////
	void			View::setSizePosition(scalar left,scalar top,scalar right,scalar bottom,
		size_t leftOffset/* = 0*/,size_t topOffset/* = 0*/,size_t rightOffset/* = 0*/, size_t bottomOffset/* = 0*/)
	{
		mRelativeLeft = left;
		mRelativeTop = top;
		mRelativeRight = right;
		mRelativeBottom = bottom;

		mOffsetLeft = leftOffset;
		mOffsetTop = topOffset;
		mOffsetRight = rightOffset;
		mOffsetBottom = bottomOffset;
	}

	//////////////////////////////////////////////////////////////////////////
	void			View::setBackgroundColor(const Color& color)
	{
		mBackColor = color;
	}

	//////////////////////////////////////////////////////////////////////////
	void			View::setClearMode(uint32 frameTypeMode,scalar depth/* =1.0*/,uint16 stencil/* = 0*/)
	{
		mClearMode = frameTypeMode;
		mClearDepth = depth;
		mClearStencil = stencil;
	}

	//////////////////////////////////////////////////////////////////////////
	void					View::calculateSizeInPixel(IRenderTarget* target)
	{
		if( target != NULL )
		{
			scalar width = (scalar)target->getViewWidth();
			scalar height = (scalar)target->getViewHeight();

			mPixelLeft	= (int32)(width*mRelativeLeft) + (int32)mOffsetLeft;
			mPixelTop	= (int32)(height*mRelativeTop) + (int32)mOffsetTop;
			mPixelRight	= (int32)(width*mRelativeRight) + (int32)mOffsetRight;
			mPixelBottom= (int32)(height*mRelativeBottom) + (int32)mOffsetBottom;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			View::calculateSizeInPixel(ITexture* rtt)
	{
		if(rtt != NULL)
		{
			assert(rtt->getTextureUsage().isRenderTarget() || rtt->getTextureUsage().isDepthStencil());
			scalar width = (scalar)rtt->getWidth();
			scalar height = (scalar)rtt->getHeight();

			mPixelLeft	= (int32)(width*mRelativeLeft) + (int32)mOffsetLeft;
			mPixelTop	= (int32)(height*mRelativeTop) + (int32)mOffsetTop;
			mPixelRight	= (int32)(width*mRelativeRight) + (int32)mOffsetRight;
			mPixelBottom= (int32)(height*mRelativeBottom) + (int32)mOffsetBottom;
		}
	}

	/************************************************************************/
	/* IGraphicsView                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&			View::getRenderScheme() const
	{
		return mRenderSchemeName;
	}

	//////////////////////////////////////////////////////////////////////////
	void					View::setRenderScheme(const TString& schemeName)
	{
		mRenderSchemeName = schemeName;
		assert( mScheme == NULL );
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					View::getChannelCount() const
	{
		if( mPipeline == NULL )
			return 0;
		else
			return mPipeline->getChannelCount();
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsChannel*		View::getChannel(index_t index) const
	{
		if( mPipeline == NULL )
			return NULL;
		else
			return mPipeline->getChannel(index);
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsChannel*		View::getChannel(const TString& channelRenderType) const
	{
		if( mPipeline == NULL )
			return NULL;
		else
			return mPipeline->getChannel(channelRenderType);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					View::getPhaseCount() const
	{
		if( mPipeline == NULL )
			return 0;
		else
			return mPipeline->getPhaseCount();
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsPhase*			View::getPhase(index_t index) const
	{
		if( mPipeline == NULL )
			return NULL;
		else
			return mPipeline->getPhase(index);
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsPhase*			View::getPhase(const TString& phaseName) const
	{
		if( mPipeline == NULL )
			return NULL;
		else
			return mPipeline->getPhase(phaseName);
	}

	//////////////////////////////////////////////////////////////////////////
	void				View::setFillMode(FILL_MODE mode)
	{
		assert( mode >= GFM_POINT && mode <= GFM_DEFAULT );
		mFillMode = mode;
	}

	//////////////////////////////////////////////////////////////////////////
	FILL_MODE			View::getFillMode() const
	{
		return mFillMode;
	}

	//////////////////////////////////////////////////////////////////////////
	void				View::enableImageEffect(bool enable)
	{
		mEnableImageFX = enable;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				View::isImageEffectEnabled() const
	{
		return mEnableImageFX;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&		View::getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderScheme*		View::attachRenderScheme(IRenderScheme* scheme, bool bAutoDelete/* = false*/, bool deleteOld/* = false*/)
	{
		if( mScheme == scheme )
			return mScheme;

		IRenderScheme* oldScheme = mScheme;

		//note: do not delete pipeline so frequently
		//delete it only on the next attach if a different scheme attached.
		if( mScheme != NULL )
		{
			if( mAutoDeleteScheme && deleteOld )
			{
				BLADE_DELETE mPipeline;
				mPipeline = NULL;
				IRenderSchemeManager::getSingleton().destroyRenderScheme(mScheme);
				oldScheme = NULL;
			}
		}

		if( scheme != NULL )
		{
			if( mPipeline == NULL || mPipeline->getPrototype() != scheme )
			{
				BLADE_DELETE mPipeline;
				mPipeline = scheme->createPipeline();
			}
		}
		assert(mPipeline != NULL);
		
		if( scheme != NULL )
			mRenderSchemeName = scheme->getName();
		else
			mRenderSchemeName = TString::EMPTY;

		mScheme = scheme;
		mAutoDeleteScheme = bAutoDelete;
		return oldScheme;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderScheme*		View::getAttachedRenderScheme() const
	{
		return mScheme;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderPipeline*	View::getRenderPipeline() const
	{
		return mPipeline;
	}

}//namespace Blade