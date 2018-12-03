/********************************************************************
	created:	2016/07/30
	filename: 	QtMessagePump.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtMessagePump_h__
#define __Blade_QtMessagePump_h__
#include <BladeQtDevice.h>
#include <utility/BladeContainer.h>
#include <interface/public/window/IWindowMessagePump.h>

namespace Blade
{
	class QtMessagePump : public IWindowMessagePump, public QObject, public Singleton<QtMessagePump>
	{
	public:
		using Singleton<QtMessagePump>::getSingleton;
		using Singleton<QtMessagePump>::getSingletonPtr;
	public:
		QtMessagePump();
		virtual ~QtMessagePump();

		/**
		@describe 
		@param
		@return
		*/
		virtual IKeyboardDevice* getGlobalKeyboard() const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	processMessage();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	addMessageListener(IMessageListener* listener);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	removeMessageListener(IMessageListener* listener);


	protected:
		/** @brief  */
		virtual bool eventFilter(QObject *watched, QEvent *evt);

	protected:
		typedef Set<IMessageListener*>		MessageListenerSet;

		MessageListenerSet		mMessageListeners;
	};
	
}//namespace Blade


#endif // __Blade_QtMessagePump_h__