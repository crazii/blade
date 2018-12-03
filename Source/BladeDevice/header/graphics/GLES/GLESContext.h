/********************************************************************
	created:	2014/12/19
	filename: 	GLESContext.h
	author:		Crazii
	purpose:	OS dependent initialization functions for GLES (equals to EGL/EAGL)
*********************************************************************/
#ifndef __Blade_GLESContext_h__
#define __Blade_GLESContext_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <interface/public/graphics/PixelFormat.h>
#include <threading/Thread.h>

namespace Blade
{
	class IWindow;
	class RenderWindow;

	class GLESContext
	{
	public:
		GLESContext();
		virtual ~GLESContext();

		/** @brief  */
		bool	initialize(IWindow* window, uint version = 3)
		{
			if (!mIsInited)
			{
				assert(mCurrent == Thread::INVALID_ID);
				mIsInited = this->initializeImpl(window, version);
			}
			else
				assert(false);
			return mIsInited;
		}

		/** @brief  */
		bool	initialize(GLESContext* sharedContext, uint version = 3)
		{
			if (!mIsInited)
			{
				assert(sharedContext != NULL);
				assert(mCurrent == Thread::INVALID_ID);
				mIsInited = this->initializeImpl(sharedContext, version);
			}
			else
				assert(false);
			return mIsInited;
		}

		/** @brief  */
		bool	deinitialize()
		{
			if( mIsInited )
				mIsInited = !this->deinitializeImpl();
			return mIsInited;
		}

		/** @brief  */
		bool	makeCurrent(bool rendering, const Thread::ID* ptid = NULL)
		{
			assert( mIsInited );

			Thread::ID tid = (ptid != NULL ? *ptid : Thread::getCurrentID());
			if (rendering ||mCurrent == Thread::INVALID_ID || mCurrent != tid)
			{
				if (this->makeCurrentImpl(rendering))
					mCurrent = tid;
			}
			return mCurrent != Thread::INVALID_ID;
		}

		/** @brief  */
		bool	clearCurrent()
		{
			if(mCurrent != Thread::INVALID_ID)
			{
				assert( mIsInited );
				if (this->clearCurrentImpl())
					mCurrent = Thread::INVALID_ID;
			}
			return mCurrent == Thread::INVALID_ID;
		}

		/** @brief */
		Thread::ID	getCurrent() const		{return mCurrent;}

		/** @brief  */
		bool	checkCurrent(Thread::ID* ptid = NULL) const	
		{
			return mCurrent != Thread::INVALID_ID 
				&& (mCurrent == (ptid != NULL ? (*ptid=Thread::getCurrentID()) : Thread::getCurrentID()))
				&& this->checkCurrentImpl(); 
		}

		/** @brief  */
		bool	checkCurrent(Thread::ID tid) const { return mCurrent != Thread::INVALID_ID && mCurrent == tid && this->checkCurrentImpl(); }

		/** @brief  */
		bool	isInitialized() const	{return mIsInited;}

		/** @brief  */
		static GLESContext*	createContext();

	protected:

		/** @brief  */
		virtual bool	initializeImpl(IWindow* window, uint version) = 0;

		/** @brief  */
		virtual bool	initializeImpl(GLESContext* sharedContext, uint version) = 0;

		/** @brief  */
		virtual bool	deinitializeImpl() = 0;

		/** @brief  */
		virtual bool	makeCurrentImpl(bool rendering) = 0;

		/** @brief  */
		virtual bool	checkCurrentImpl() const = 0;

		/** @brief  */
		virtual bool	clearCurrentImpl() = 0;

		Thread::ID		mCurrent;
		bool			mIsInited;
	};
}//namespace Blade

#endif//BLADE_USE_GLES

#endif//__Blade_GLESContext_h__