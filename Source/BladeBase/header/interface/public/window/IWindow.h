/********************************************************************
	created:	2009/03/22
	filename: 	IWindow.h
	author:		Crazii
	
	purpose:	defines the window interface
*********************************************************************/
#ifndef __Blade_IWindow_h__
#define __Blade_IWindow_h__
#include <Handle.h>

namespace Blade
{
	class IWindowEventHandler;

	class IWindow
	{
	public:
		virtual ~IWindow()		{}

		/** @brief get implementation type string */
		virtual const TString&	getWindowType() const = 0;

		/** @brief  */
		virtual const  TString&	getUniqueName() const = 0;

		/** @brief set the window size */
		virtual void setSize(size_t width,size_t height) = 0;

		/** @brief get the window size */
		virtual size_t getWidth() const = 0;

		/** @brief get the window size */
		virtual size_t getHeight() const = 0;
		/** @brief get the window size */
		inline void getSize(size_t& width,size_t& height) const
		{
			width = this->getWidth();
			height = this->getHeight();
		}

		/** @brief set the window caption */
		virtual void setCaption(const TString& caption) = 0;

		/** @brief get the window caption */
		virtual const TString& getCaption() const = 0;

		/** @brief set the window visibility */
		virtual void setVisibility(bool visibile) = 0;

		/** @brief get the window visibility */
		virtual bool isVisible() const = 0;

		/** @brief set the window's full screen mode */
		virtual void setFullScreen(bool fullscreen) = 0;

		/** @brief get whether the window is in full screen mode */
		virtual bool getFullScreen() const = 0;

		/** @brief get the window's specific color depth */
		virtual int  getColorDepth() const = 0;

		/** @brief get the native platform/API related window pointer(i.e. HWND for win32) */
		virtual uintptr_t	getNativeHandle() const = 0;

		/** @brief get the implementation defined pointer */
		virtual uintptr_t	getImpl() const {return this->getNativeHandle();}

		/** @brief set the parent of the window by internal implementation
		note: internalImpl may NOT be a native handle */
		virtual void setParent(uintptr_t internalImpl) = 0;

		/** @brief get the internal implementation of parent window */
		virtual uintptr_t getParent() const = 0;

		/** @brief close the window */
		virtual bool closeWindow() = 0;

		/** @brief  */
		virtual bool addEventHandler(IWindowEventHandler* handler) = 0;

		/** @brief  */
		virtual bool removeEventHandler(IWindowEventHandler* handler) = 0; 

	};//class IWindow

	typedef Handle<IWindow> HWINDOW;

}//namespace Blade


#endif // __Blade_IWindow_h__