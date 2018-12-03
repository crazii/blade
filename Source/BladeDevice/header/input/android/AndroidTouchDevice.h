/********************************************************************
	created:	2013/12/29
	filename: 	AndroidTouchDevice.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AndroidTouchDevice_h__
#define __Blade_AndroidTouchDevice_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <utility/BladeContainer.h>
#include <interface/public/input/ITouchDevice.h>
#include <interface/public/window/IWindowMessagePump.h>

#define BLADE_USE_PLATFORM_DATA_EXPLICIT
#include <platform/PlatformData.h>

namespace Blade
{
	class AndroidTouchDevice : public ITouchDevice, public IWindowMessagePump::IMessageListener, public Allocatable
	{
	public:
		AndroidTouchDevice();
		~AndroidTouchDevice();

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
		/* ITouchView specs                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TOUCH_DESC&	getDesc() const;

		/*
		@describe get current touch state
		@param 
		@return 
		*/
		virtual TOUCH_STATE			getState() const;

		/*
		@describe get current touch position, success only current touch state is down
		@param 
		@return 
		*/
		virtual const TOUCH_DATA&	getPosition() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ITouchView*		createChildView(const TOUCH_DESC& desc);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteChildView(ITouchView* view);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ITouchView*		getChildView(index_t index) const;

		/************************************************************************/
		/* ITouchDevice specs                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void		attach(IWindow* window);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		addListener(IListener* listener);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeListener(IListener* listener);

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
		typedef Set<IListener*>		ListenerList;

		TOUCH_DESC		mDesc;
		ANativeWindow*	mWindow;
		ITouchView*		mParent;
		ListenerList	mListeners;
		TOUCH_STATE		mState;
		TOUCH_DATA		mData;

		bool			mOpened;
	};
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#endif //  __Blade_AndroidTouchDevice_h__