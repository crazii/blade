/********************************************************************
	created:	2009/02/28
	created:	28:2:2009   11:05
	filename: 	WindowSystem.cc
	author:		Crazii
	
	purpose:	IWindowSystem implementation
*********************************************************************/
#include <BladePCH.h>
#include "WindowSystem.h"
#include <interface/IPlatformManager.h>
#include <interface/IConfigManager.h>
#include <interface/IEnvironmentManager.h>
#include <interface/public/window/IWindowEventHandler.h>
#include <ConfigTypes.h>
#include <BladeWindow_blang.h>

namespace Blade
{
	namespace WindowOptionString
	{
		static const TString WindowOptionName = BXLang(BLANG_WINDOW_SETTTING);
		static const TString WindowDeviceType = BXLang(BLANG_WINDOW_API);
		static const TString AutoWindow = BXLang(BLANG_AUTOWINDOW);
		static const TString FullScreen = BXLang(BLANG_FULL_SCREEN);
		static const TString ColorDepth = BXLang(BLANG_COLOR_DEPTH);
		static const TString Resolution = BXLang(BLANG_WINDOW_RESULOTION);
	
	}//namespace WindowOption
	
	const TString WindowSystem::WINDOW_SYSTEM_NAME = BTString("BladeWindowSystem");

	//////////////////////////////////////////////////////////////////////////
	//ctor & dector
	WindowSystem::WindowSystem()
		:mAutoCreateWindow(true)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	WindowSystem::~WindowSystem()
	{

	}

	/************************************************************************/
	/*common ISubsystem specs                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	WindowSystem::getName()
	{
		return WINDOW_SYSTEM_NAME;
	}

	//////////////////////////////////////////////////////////////////////////
	void WindowSystem::install()
	{
		this->genterateOptions();
		RegisterSingleton(WindowSystem, IWindowService);
	}

	//////////////////////////////////////////////////////////////////////////
	void			WindowSystem::uninstall()
	{
		IConfigManager::getSingletonPtr()->removeConfig(mWinOption);
		UnregisterSingleton(WindowSystem, IWindowService);
	}

	//////////////////////////////////////////////////////////////////////////
	void WindowSystem::initialize()
	{
		//read config
		if( mWindowType.empty() )
		{
			IConfig* winCfg = mWinOption->getSubConfigByName( WindowOptionString::WindowDeviceType );
			mWindowType = winCfg->getValue();
		}
		IWindowMessagePump::interchange(mWindowType);

		if( mWindows.size() != 0 )
			return;

		IConfig* config = mWinOption->getSubConfigByName( WindowOptionString::AutoWindow );
		mAutoCreateWindow = TStringHelper::toBool( config->getValue() );

		if( mRootWindow == NULL && mAutoCreateWindow)
		{
			const TString& windowname = BTString("DefaulWindow");
			this->createWindow(windowname);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void WindowSystem::update()
	{
		//process created window msg
		for( WindowList::iterator iter = mWindows.begin(); iter != mWindows.end(); )
		{
			IWindowDevice* window = static_cast<IWindowDevice*>(iter->second);
			if( !window->update() && /*mAutoCreateWindow &&*/ mRootWindow == NULL )	//delete auto window if window not valid (i.e.closed)
				mWindows.erase(iter++);
			else
				++iter;
		}
		//if window is auto created, signal terminating after window close
		if( mWindows.size() == 0 && /*mAutoCreateWindow &&*/ mRootWindow == NULL )
			IEnvironmentManager::getSingletonPtr()->setVariable(ConstDef::EnvBool::WORK_TERMINATE, true);
		else
			IWindowMessagePump::getSingleton().processMessage();
	}

	//////////////////////////////////////////////////////////////////////////
	void WindowSystem::shutdown()
	{
		for( WindowList::iterator iter = mWindows.begin(); iter != mWindows.end(); ++iter)
		{
			( static_cast<IWindowDevice*>(iter->second) )->close();
		}

		mWindows.clear();
		//this need clear too
		mRootWindow.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*	WindowSystem::createScene(const TString& name)
	{
		BLADE_UNREFERENCED(name);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	WindowSystem::destroyScene(const TString& name)
	{
		//empty
		BLADE_UNREFERENCED(name);
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*	WindowSystem::getScene(const TString& name) const
	{
		BLADE_UNREFERENCED(name);
		return NULL;
	}

	/************************************************************************/
	/*IWindowSystem specs                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t WindowSystem::getNumWindows() const
	{
		return mWindows.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const HWINDOW&	WindowSystem::getWindow(index_t windowindex) const
	{
		assert( windowindex < mWindows.size() );
		if( windowindex < mWindows.size() )
		{
			WindowList::const_iterator it = mWindows.begin();
			for( size_t i = 0; i < windowindex; ++i )
				++it;
			return it->second;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("window index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	const HWINDOW&	WindowSystem::getWindow(const TString& name) const
	{
		WindowList::const_iterator it = mWindows.find( name );
		if( it == mWindows.end() )
			return HWINDOW::EMPTY;
		else
			return it->second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool WindowSystem::registerEventHandler(IWindowEventHandler *handler)
	{
		if( handler == NULL )
			return false;

		return mHandlers.addEventHandler(handler);
	}

	bool WindowSystem::removeEventHandler(IWindowEventHandler *handler)
	{
		if( handler == NULL )
			return false;

		return mHandlers.removeEventHandler(handler);
	}

	//////////////////////////////////////////////////////////////////////////
	const HWINDOW&	WindowSystem::createWindow(const TString& winname)
	{
		HWINDOW& hWindow = mWindows[winname];

		if( hWindow != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a window named \"")+winname+BTString("\" already exist.") );

		size_t x = 0;
		size_t y = 0;
		size_t bits = 16;
		bool fullscreen = false;
		IConfig* config;

		//display resolution
		config = mWinOption->getSubConfigByName( WindowOptionString::Resolution );
		TStringStream tss;
		tss.str( config->getValue() );
		tchar d = TEXT('x');
		tss >> x >> d >> y;

		if( x == 0 || y == 0 )
		{
			//TODO: re-config value trough dialog or throw exception
			assert(false);
		}

		//full screen
		config = mWinOption->getSubConfigByName(WindowOptionString::FullScreen);
		fullscreen = TStringHelper::toBool( config->getValue() );

		//
		config = mWinOption->getSubConfigByName(WindowOptionString::ColorDepth);
		bits = (size_t)TStringHelper::toInt( config->getValue() );

		hWindow.bind( static_cast<IWindowDevice*>( BLADE_FACTORY_CREATE(IWindowDevice, mWindowType) ) );

		if( mRootWindow != NULL )
			hWindow->setParent( mRootWindow->getNativeHandle() );
		else
			hWindow->setFullScreen(fullscreen);

		hWindow->setCaption(winname);
		hWindow->setSize(x,y);
		hWindow->addEventHandler(&mHandlers);

		IWindowDevice* pWinDev = static_cast<IWindowDevice*>(hWindow);
		pWinDev->setColorDepth((int)bits);
		pWinDev->open();

		return hWindow;
	}

	//////////////////////////////////////////////////////////////////////////
	void			WindowSystem::destroyWindow(const TString& winname)
	{
		WindowList::iterator i = mWindows.find( winname );
		if( i != mWindows.end() )
		{
			IWindowDevice* pWinDev = static_cast<IWindowDevice*>(i->second);
			if(mRootWindow == static_cast<IWindow*>(pWinDev) )
				mRootWindow.clear();
			pWinDev->destroyWindow();
			mWindows.erase(i);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const HWINDOW&	WindowSystem::attachWindow(uintptr_t internalImpl)
	{
		HWINDOW Win(BLADE_FACTORY_CREATE(IWindowDevice, mWindowType));
		static_cast<IWindowDevice*>(Win)->attachWindow(internalImpl);

		const TString& winname = Win->getUniqueName();

		HWINDOW& hWindow = mWindows[winname];
		if( hWindow != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a window named \"")+winname+BTString("\" already exist.") );
		hWindow = Win;

		size_t bits = 32;
		bool fullscreen = false;
		IConfig* config;

		//full screen
		config = mWinOption->getSubConfigByName( WindowOptionString::FullScreen );
		fullscreen = TStringHelper::toBool( config->getValue() );

		//
		config = mWinOption->getSubConfigByName( WindowOptionString::ColorDepth );
		bits = (size_t)TStringHelper::toInt( config->getValue() );

		//hWindow->setSize(x,y);
		hWindow->setFullScreen(fullscreen);
		hWindow->addEventHandler(&mHandlers);

		IWindowDevice* pWinDev = static_cast<IWindowDevice*>(hWindow);
		pWinDev->setColorDepth((int)bits);

		return hWindow;
	}

	//////////////////////////////////////////////////////////////////////////
	void WindowSystem::setAppWindow(const HWINDOW& root)
	{
		assert(root != NULL);
		mRootWindow = root;
	}

	//////////////////////////////////////////////////////////////////////////
	const HWINDOW&	WindowSystem::getAppWindow() const
	{
		return mRootWindow;
	}

	//////////////////////////////////////////////////////////////////////////
	void			WindowSystem::setAutoCreatingWindow(bool autoWindow)
	{
		if( mAutoCreateWindow != autoWindow )
		{
			mAutoCreateWindow = autoWindow;
			TString TRUE_FALSE = TStringHelper::fromBool(mAutoCreateWindow);
			mWinOption->getSubConfigByName(WindowOptionString::AutoWindow)->setValue( TRUE_FALSE );
		}	
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WindowSystem::isAutoCreatingWindow() const
	{
		return mAutoCreateWindow;
	}

	//////////////////////////////////////////////////////////////////////////
	void			WindowSystem::setWindowTypeOverride(const TString& winType)
	{
		if(!WindowDeviceFactory::getSingleton().isClassRegistered(winType)) //window type not registered
		{
			assert(false);
			return;
		}
		mWindowType = winType;
		IWindowMessagePump::interchange(winType);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		WindowSystem::getWindowType() const
	{
		return mWindowType;
	}

	//////////////////////////////////////////////////////////////////////////
	void	WindowSystem::genterateOptions()
	{
		if( mWinOption != NULL )
			return;

		ConfigGroup* pWinOption = BLADE_NEW ConfigGroup(WindowOptionString::WindowOptionName);

		//main API
		{
			TStringParam list;
			for( size_t i = 0; i < Factory<IWindowDevice>::getSingleton().getNumRegisteredClasses(); ++i )
			{
				const TString& name = Factory<IWindowDevice>::getSingleton().getRegisteredClass(i);
				list.push_back(name);
			}
			ConfigAtom* config = BLADE_NEW ConfigAtom(WindowOptionString::WindowDeviceType, list, CAF_NORMAL);
			pWinOption->addSubConfigPtr( config );
		}

		ConfigAtom* config;
		//resolution
		{
			TStringParam list;
			size_t current = 0;
			size_t count = IPlatformManager::getSingleton().getScreenResolutions(current);
			if( count != 0 )
			{
				list.reserve(count);
				TempVector<size_t> widths(count),heights(count);
				IPlatformManager::getSingleton().getScreenResolutions(current, &widths[0], &heights[0]);
				for(size_t i = 0; i < count; ++i)
				{
					TString mode = TStringHelper::fromUInt(widths[i]) + tchar('x') + TStringHelper::fromUInt(heights[i]);
					list.push_back(mode);
				}

				config = BLADE_NEW ConfigAtom(WindowOptionString::Resolution, list, CAF_NORMAL);
				assert(current < count);
				config->setValue(list[current]);
				pWinOption->addSubConfigPtr(config);
			}
			else
			{
				config = BLADE_NEW ConfigAtom(WindowOptionString::Resolution, BTString("1024x768"), CAF_NORMAL);
				pWinOption->addSubConfigPtr(config);
			}
		}

		//full screen
		config = BLADE_NEW ConfigAtom(WindowOptionString::FullScreen, false);
		pWinOption->addSubConfigPtr(config);

		//color depth
		{
			TStringParam list;
			list.push_back( BTString("32") );
			list.push_back( BTString("16") );
			config = BLADE_NEW ConfigAtom(WindowOptionString::ColorDepth, list, CAF_NORMAL);
			pWinOption->addSubConfigPtr(config);
		}

		config = BLADE_NEW ConfigAtom(WindowOptionString::AutoWindow,true);
		pWinOption->addSubConfigPtr(config);

		IConfigManager* configMgr = IConfigManager::getSingletonPtr();
		mWinOption.bind( pWinOption );
		configMgr->addConfig(mWinOption);
	}

}//namespace Blade