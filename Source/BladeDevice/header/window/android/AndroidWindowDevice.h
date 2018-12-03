/********************************************************************
	created:	2013/12/28
	filename: 	AndroidWindowDevice.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AndroidWindowDevice_h__
#define __Blade_AndroidWindowDevice_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <android/native_activity.h>

#include <interface/public/window/IWindowDevice.h>
#include <interface/public/window/IWindowMessagePump.h>
#include <utility/BladeContainer.h>
#include <utility/Variant.h>

namespace Blade
{
	/* @note: currently there's only one unique window existing on android  */
	class AndroidWindowDevice : public IWindowDevice, public IWindowMessagePump::IMessageListener, public Allocatable
	{
	public:
		static const TString ANDROID_WINDOW_TYPE;
	public:
		AndroidWindowDevice();
		~AndroidWindowDevice();

		/************************************************************************/
		/* IDevice specs                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open();

		/** @brief check if it is open */
		virtual bool	isOpen() const;

		/** @brief close the device */
		virtual void	close();

		/** @brief reset the device */
		virtual bool	reset();

		/** @brief update data(I/O) */
		virtual bool	update();

		/************************************************************************/
		/*IWindow specs                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual const TString& getWindowType() const
		{
			return ANDROID_WINDOW_TYPE;
		}
		/** @brief  */
		virtual const  TString&	getUniqueName() const;

		/** @brief set the window size */
		virtual void setSize(size_t width,size_t height);

		/** @brief get the window size */
		virtual size_t getWidth() const;

		/** @brief get the window size */
		virtual size_t getHeight() const;

		/** @brief set the window caption */
		virtual void setCaption(const TString& caption);

		/** @brief get the window caption */
		virtual const TString& getCaption() const;

		/** @brief set the window visibility */
		virtual void setVisibility(bool visibile);

		/** @brief get the window visibility */
		virtual bool isVisible() const;

		/** @brief set the window's full screen mode */
		virtual void setFullScreen(bool fullscreen);

		/** @brief get whether the window is in full screen mode */
		virtual bool getFullScreen() const;

		/** @brief get the window's specific color depth */
		virtual int  getColorDepth() const;

		/** @brief get the platform/API related window data(i.e. HWND for win32) */
		virtual uintptr_t	getNativeHandle() const;

		/** @brief set the parent of the window */
		virtual void setParent(uintptr_t internalImpl);

		/** @brief set the parent of the window */
		virtual uintptr_t getParent() const;

		/** @brief try to close the window */
		virtual bool closeWindow();

		/** @brief  */
		virtual bool addEventHandler(IWindowEventHandler* handler);

		/** @brief  */
		virtual bool removeEventHandler(IWindowEventHandler* handler);

		/************************************************************************/
		/* IWindowDevice interface                                                                    */
		/************************************************************************/
		/** @brief specify a external window to the device */
		/* @remark in this situation,when calling destroyWindow, it doesn't destroy the internal window and just detach it */
		virtual void attachWindow(uintptr_t internalImpl);

		/** @brief set the window's specific color depth */
		virtual void setColorDepth(int depth);

		/** @brief  */
		virtual void destroyWindow();

		/************************************************************************/
		/* IWindowMessagePump::IMessageListener interface                                                                     */
		/************************************************************************/
		/** @brief  return true to continue message processing,
		false to skip this message */
		virtual bool preProcessMessage(void* msg);
		/** @brief called after processing each message */
		virtual void postProcessMessage(void* )	{}
		/** @brief called on each loop before processing all messages */
		virtual void preProcessMessages()		{}
		/** @brief called on each loop after processing all messages */
		virtual void postProcessMessages()		{}

	protected:
		typedef Set<IWindowEventHandler*>	WinHandlerList;

		ANativeWindow*	mNativeHandle;
		TString			mName;
		WinHandlerList	mNativeHandlers;
		//store a depth so that render system will use it
		int				mColorDepth;
		bool			mVisible;
	};
	
}//namespace Blade


#endif//BLADE_PLATFORM

#endif //  __Blade_AndroidWindowDevice_h__