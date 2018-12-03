/********************************************************************
	created:	2010/04/09
	filename: 	RenderTarget.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderTarget_h__
#define __Blade_RenderTarget_h__
#include <BladeDevice.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <utility/FixedArray.h>
#include <math/Box2i.h>

namespace Blade
{
	class RenderView;
	class IRenderDevice;

	class BLADE_DEVICE_API RenderTarget : public IRenderTarget, public Allocatable
	{
	public:
		static const size_t MAX_MRT_COUNT = 8;
	public:
		RenderTarget(const TString& name, IRenderDevice* device, size_t viewWidth, size_t viewHeight);
		virtual ~RenderTarget();

		/************************************************************************/
		/* IRenderTarget interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const HTEXTURE& getDepthBuffer() const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			setDepthBuffer(const HTEXTURE& hDethBuffer);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			setColorBuffer(index_t index, const HTEXTURE& buffer);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			setColorBufferCount(index_t count);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HTEXTURE&getColorBuffer(index_t index) const;

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t			getViewWidth() const	{return mViewWidth;}

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t			getViewHeight() const	{return mViewHeight;}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			setViewWidth(size_t width)	{mViewWidth = width; return true;}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			setViewHeight(size_t height){mViewHeight = height; return true;}

		/**
		@describe set active view rect, the rect must be with the view width & height set by setViewWidth/Height
		@param
		@return
		*/
		virtual bool			setViewRect(int32 left, int32 top, int32 width, int32 height);

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getColorBufferCount() const;

		/*
		@describe
		@param
		@return
		*/
		virtual IListener*		setListener(IListener* listener);

		/*
		@describe
		@param
		@return
		*/
		virtual IListener*		getListener() const;


		/*
		@describe
		@param
		@return
		*/
		virtual bool			isReady() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			swapBuffers();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		IRenderDevice*	getDevice() const	{return mDevice;}

		/** @brief  */
		HTEXTURE&		getDepthBuffer()	{return mDepthBuffer;}

		/*
		@describe
		@param
		@return
		*/
		void			notifySizeChange(IRenderTarget* target);

	protected:
		TString			mName;
		IRenderDevice*	mDevice;
		IListener*		mListener;
		HTEXTURE		mDepthBuffer;
		size_t			mViewWidth;
		size_t			mViewHeight;
		typedef FixedVector<HTEXTURE,MAX_MRT_COUNT> OutputBufferList;
		OutputBufferList mOutputBuffers;
	};//class RenderTarget 
	

}//namespace Blade


#endif //__Blade_RenderTarget_h__