/********************************************************************
	created:	2010/05/25
	filename: 	ConfigDialogProxy.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ConfigDialogProxy_h__
#define __Blade_ConfigDialogProxy_h__
#include <StaticHandle.h>
#include <ConfigDialog/ConfigDialog.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	class ConfigDialogProxy : public IConfigDialog
	{
	public:
		ConfigDialogProxy()
			:mConfigDialog(NULL)		{mHideAutoStartCheck = false; mHideBanner = false;}
		~ConfigDialogProxy()			{}

		/** @brief  */
		inline void	hideStartupOption()	{mHideAutoStartCheck = true;}

		/** @brief  */
		inline void	hideDialgBanner()	{mHideBanner = true;}

		/** @brief  */
		virtual bool show(const INFO& doModal, const TString& caption = BTString("Blade Config"))
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			assert( mConfigDialog == NULL );

			if( mConfigDialog == NULL )
			{
				mConfigDialog.lock();
				if( mConfigDialog == NULL )
				{
					//proxy the real object
					mConfigDialog = BLADE_NEW CConfigDialog( CWnd::FromHandle(::GetActiveWindow()) );
				}
				mConfigDialog.unlock();
			}

			for(ConfigOrderList::iterator i = mConfigEntries.begin(); i != mConfigEntries.end(); ++i)
			{
				mConfigDialog->addConfig(*i);
			}

			mConfigDialog->setCaption(caption);
			bool result = mConfigDialog->show(doModal, mHideAutoStartCheck, mHideBanner);

			mConfigDialog.clear();

			return result;
		}

		virtual bool show(const INFO& doModal, const HCONFIG* configs, size_t count,
			const TString& caption = BTString("Blade Config") )
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			assert( mConfigDialog == NULL );

			if( mConfigDialog == NULL )
			{
				mConfigDialog.lock();
				if( mConfigDialog == NULL )
				{
					//proxy the real object
					mConfigDialog = BLADE_NEW CConfigDialog( CWnd::FromHandle(::GetActiveWindow()) );
				}
				mConfigDialog.unlock();
			}

			for(size_t i = 0; i < count; ++i )
			{
				mConfigDialog->addConfig( configs[i] );
			}

			mConfigDialog->setCaption(caption);
			bool result = mConfigDialog->show(doModal, mHideAutoStartCheck, mHideBanner);

			mConfigDialog.clear();

			return result;
		}

		/** @brief  */
		virtual bool addConfig(IConfig* configureitem)
		{
			bool ret = mConfigRegistry.insert(configureitem).second;
			mConfigEntries.push_back(configureitem);
			return ret;
		}

	protected:
		typedef	StaticSet<IConfig*>		ConfigList;
		typedef StaticVector<IConfig*>	ConfigOrderList;

		ConfigList						mConfigRegistry;
		ConfigOrderList					mConfigEntries;
		StaticHandle<CConfigDialog>		mConfigDialog;
		bool							mHideAutoStartCheck;	//check box for auto-show on startup
		bool							mHideBanner;
	};//class ConfigDialogProxy
	
}//namespace Blade


#endif //__Blade_ConfigDialogProxy_h__