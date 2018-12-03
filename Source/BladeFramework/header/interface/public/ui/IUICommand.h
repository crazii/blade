/********************************************************************
	created:	2010/05/23
	filename: 	IUICommand.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IUICommand_h__
#define __Blade_IUICommand_h__
#include <utility/String.h>
#include <utility/Delegate.h>

namespace Blade
{

	class ICommandUI
	{
	public:
		/** @brief  */
		virtual const TString&	getName() const = 0;

		/** @brief  */
		virtual void setEnabled(bool enable) = 0;

		/** @brief  */
		virtual void setChecked(bool checked) = 0;

		/** @brief  */
		virtual ICommandUI*	getParentCUI() const {return NULL;}
	};

	class IUICommand
	{
	public:
		virtual ~IUICommand() {}

		/** @brief  */
		virtual void execute(ICommandUI* cui) = 0;

		/** @brief  */
		virtual bool update(ICommandUI* cui, index_t instance = INVALID_INDEX) { BLADE_UNREFERENCED(cui); BLADE_UNREFERENCED(instance); return false; }

		/** @brief config the command, if it has any options for user */
		virtual void config(ICommandUI* cui) { BLADE_UNREFERENCED(cui); }
	};//class IUICommand

	class UICommandDelegate : public IUICommand, public Delegate, public Allocatable
	{
	public:

		template<typename T,typename F>
		UICommandDelegate(T* ptr,F fn)
			:Delegate(ptr,fn)
		{
		}

		virtual void execute(ICommandUI* /* cui*/)
		{
			this->Delegate::call();
		}
	};
	
}//namespace Blade


#endif //__Blade_IUICommand_h__