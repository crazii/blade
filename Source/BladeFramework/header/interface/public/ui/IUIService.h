/********************************************************************
	created:	2010/04/29
	filename: 	IUIService.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IUIService_h__
#define __Blade_IUIService_h__
#include <interface/InterfaceSingleton.h>
#include <interface/public/window/IWindow.h>
#include <interface/public/input/IKeyboard.h>
#include <interface/public/input/IMouse.h>

namespace Blade
{
	class IUIService : public InterfaceSingleton<IUIService>
	{
	public:
		virtual ~IUIService()	{}

		/**
		@describe 
		@param
		@return
		*/
		virtual IKeyboard*	getGlobalKeyboard() const = 0;

		/**
		@describe create mouse for specific window
		@param
		@return
		@note the created mouse will be auto delete when the window is closed
		*/
		virtual IMouse*		createInputMouse(IWindow* window) = 0;

		/**
		@describe create keyboard for specific window
		@param
		@return
		@note the created keyboard will be auto delete when the window is closed
		*/
		virtual IKeyboard*	createInputKeyboard(IWindow* window) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IMouse*		getInputMouse(IWindow* window) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IKeyboard*	getInputKeyboard(IWindow* window) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getKeyString(EKeyCode code) const = 0;
		/** @brief  */
		inline tchar getKeyChar(EKeyCode code) const
		{
			const TString& ks = this->getKeyString(code);
			return ks.size() == 1 ? ks[0] : TEXT('\0');
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual EKeyCode		getKeyFromString(const TString& keyString) const = 0;
		/** @brief  */
		inline EKeyCode	getKeyFromChar(tchar keyChar) const
		{
			const tchar ks[2] = { keyChar, TEXT('\0') };
			return this->getKeyFromString(BVTString(ks));
		}

		/**
		@describe 
		@param 
		@return 
		@remark if param pKeyboard is NULL, the listener is about to listening all keyboards
		*/
		virtual bool			addKeyboardListener(IKeyboard::IListener* listener,IKeyboard* pKeyboard = NULL) = 0;

		/**
		@describe 
		@param 
		@return 
		@remark if param pKeyboard is NULL, remove all the listened keyboards
		*/
		virtual bool			removeKeyboardListener(IKeyboard::IListener* listener,IKeyboard* pKeyboard = NULL) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			addMouseListener(IMouse::IListener* listener, IMouse* mouse) = 0;

		/**
		@describe 
		@param 
		@return 
		@remark if param mouse is NULL, remove all the listened mice
		*/
		virtual bool			removeMouseListener(IMouse::IListener* listener, IMouse* mouse = NULL) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void			setInputTypeOverride(const TString* keyboardType, const TString* mouseType) = 0;

	};//class IUIService
	
	extern template class BLADE_FRAMEWORK_API Factory<IUIService>;
	
}//namespace Blade


#endif //__Blade_IUIService_h__