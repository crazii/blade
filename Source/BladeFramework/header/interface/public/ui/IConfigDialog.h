/********************************************************************
	created:	2009/04/05
	filename: 	IConfigDialog.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IConfigDialog_h__
#define __Blade_IConfigDialog_h__
#include <interface/IConfig.h>
#include <utility/Delegate.h>

namespace Blade
{
	class IConfigDialog
	{
	public:
		typedef struct SModalInfo
		{
			bool		mModal;
			bool		mForceShowAll;		///force show all controls
			Delegate	mNoneModalDelegate;	///callback if a none-modal Dialog is confirmed, 
											///if canceled, this will not be called
			void*	mUserData;				///data parameter when calling delegate

			SModalInfo(bool bModal, bool showAll = false)	:mModal(bModal),mForceShowAll(showAll),mNoneModalDelegate(Delegate::EMPTY),mUserData(NULL)	{}
			SModalInfo()	:mModal(true),mForceShowAll(false),mNoneModalDelegate(Delegate::EMPTY),mUserData(NULL)	{}
			SModalInfo(bool modal, bool showAll, const Delegate& _delegate, void* userData) :mModal(modal),mForceShowAll(showAll),mNoneModalDelegate(_delegate),mUserData(userData)	{}
			SModalInfo(const Delegate& _delegate, void* userData)	:mModal(false),mForceShowAll(false),mNoneModalDelegate(_delegate),mUserData(userData)	{}
		}INFO;

		virtual ~IConfigDialog()	{}

		/**
		@describe config via dialog(maybe no change) or canceled
		@param modal if the dialog is modal
		@return show state
		@retval true showed & confirmed successfully
		@retval false dialog not showed or canceled
		*/
		virtual bool show(const INFO& info, const HCONFIG* configs, size_t count,
			const TString& caption = BTString("Blade Config") ) = 0;

	};//class IConfigDialog

}//namespace Blade

#endif // __Blade_IConfigDialog_h__