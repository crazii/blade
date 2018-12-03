/********************************************************************
	created:	2009/02/28
	filename: 	GraphicsSystem.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GraphicsSystem.h"
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/PixelColorFormatter.h>

#include <interface/IEnvironmentManager.h>
#include <interface/ITimeService.h>
#include <interface/IConfigManager.h>
#include <interface/IPlatformManager.h>
#include <interface/IEventManager.h>
#include <ConfigTypes.h>

#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <interface/IRenderTypeManager.h>
#include <interface/IEffectManager.h>
#include <interface/IRenderSchemeManager.h>
#include <interface/IMaterialManager.h>
#include <interface/ISpace.h>
#include <interface/ISpaceCoordinator.h>
#include <BladeGraphics_blang.h>

#include <Element/CameraElement.h>
#include <Pass.h>
#include <RenderUtility.h>
#include "../Camera.h"

namespace Blade
{
	namespace GraphicsOptionString
	{
		static const TString AdvancedOptionName = BXLang(BLANG_GRAPHICS_ADVANCED_SETTING);
		static const TString OptionName = BXLang(BLANG_GRAPHICS_BASIC_SETTING);
		static const TString RenderScheme = BXLang(BLANG_RENDER_SCHEME);
		static const TString RenderDeivce = BXLang(BLANG_RENDER_API);
		static const TString TextureFilter = BXLang(BLANG_TEXTURE_FILTER);

		static const TString FilerBiLinear = BXLang(BLANG_TEXTURE_FILTER_BILINEAR);
		static const TString FilerTriLinear = BXLang(BLANG_TEXTURE_FILTER_TRILINEAR);
		static const TString FilerAnisotropic = BXLang(BLANG_TEXTURE_FILTER_ANISOTROPY);
	}//namespace GraphicsOptionString
	
	const TString GraphicsSystem::GRAPHICS_SYSTEM_NAME = BTString("BladeGraphicsSystem");

	//////////////////////////////////////////////////////////////////////////
	//ctor & dector
	GraphicsSystem::GraphicsSystem()
		:mTaskID(Thread::INVALID_ID)
	{
		mAdvancedOption.bind( BLADE_NEW ConfigGroup(GraphicsOptionString::AdvancedOptionName) );
	}
	//////////////////////////////////////////////////////////////////////////
	GraphicsSystem::~GraphicsSystem()
	{
	}

	/************************************************************************/
	/*SubSystem spec Interface                                                                      */
	/************************************************************************/
	const TString&	GraphicsSystem::getName()
	{
		return GRAPHICS_SYSTEM_NAME;
	}

	//////////////////////////////////////////////////////////////////////////
	void GraphicsSystem::install()
	{
		bool isEditorMode = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvString::WORKING_MODE) == BTString("editor");
		//editor uses same size for all render windows, and only ONE window visible at the same time,
		//so buffers can be shared among all scheme instances.
		//this is a editor specific hard coded optimization that depend on editor implementation,
		//if editor implementation is changed, this need to change too.
		IRenderSchemeManager::getSingleton().initialize(isEditorMode);

		this->genterateOptions();

		RegisterSingleton(GraphicsSystem, IGraphicsService);
	}

	void GraphicsSystem::uninstall()
	{
		IConfigManager::getSingleton().removeConfig(mGraphicsOption);
		mGraphicsOption.clear();
		mAdvancedOption.clear();
		IRenderSchemeManager::getSingleton().shutdown();

		UnregisterSingleton(GraphicsSystem, IGraphicsService);
	}

	//////////////////////////////////////////////////////////////////////////
	void GraphicsSystem::initialize()
	{
		//check the render device configs
		IConfig* driverconfig = mGraphicsOption->getSubConfigByName(GraphicsOptionString::RenderDeivce);
		const TString& DriverName = driverconfig->getValue();

		DeviceList::iterator i = mDevices.find( DriverName ); 
		if( i == mDevices.end() )
			//configured driver not found
			BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("config corrupted.") );

		mRenderDevice = i->second;

		assert( mRenderDevice != NULL );

		ParamList result;
		for( size_t n = 0; n < driverconfig->getSubConfigCount(); ++n)
		{
			IConfig* subconfig = driverconfig->getSubConfig(n);
			result[ subconfig->getName() ] = subconfig->getBinaryValue();
		}

		bool toolMode = IEnvironmentManager::getSingleton().getVariable( ConstDef::EnvString::WORKING_MODE) == BTString("tool");

		if( !mRenderDevice->applyDeviceOption(result) && !toolMode )
		{
			BLADE_LOG(Warning, BLADE_TFUNCTION << TEXT(":render device config invalid,try to popup config dialog."));

			IPlatformManager::getSingleton().showMessageBox(
				TEXT("RenderDevice config failed for GraphicsSystem,please re-config.")
				,TEXT("Warning")
				);

			//try default config
			for( size_t n = 0; n < driverconfig->getSubConfigCount(); ++n)
			{
				IConfig* subconfig = driverconfig->getSubConfig(n);
				subconfig->setValue( subconfig->getDefaultValue() );
			}

			//pop up for user config
			if( IConfigManager::getSingleton().showConfigDialog() )
			{
				//try again
				driverconfig = mGraphicsOption->getSubConfigByName(BTString("Render API"));
				const TString& GraphicsDriver = driverconfig->getValue();

				DeviceList::iterator iter = mDevices.find( GraphicsDriver ); 
				if( iter == mDevices.end() )
					BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("config corrupted.") );

				mRenderDevice = iter->second;
				assert( mRenderDevice != NULL );

				for( size_t n = 0; n < driverconfig->getSubConfigCount(); ++n)
				{
					IConfig* subconfig = driverconfig->getSubConfig(n);
					result[ subconfig->getName() ] = subconfig->getBinaryValue();
				}

				if( !mRenderDevice->applyDeviceOption(result) )
				{
					//using default
					//IPlatformManager::getSingleton().showMessageBox(TEXT("RenderDevice config failed again with default device config."),TEXT("Warning"));
					//default config failed
					BLADE_EXCEPT(EXC_API_ERROR,BTString("set render device config failed.") );
				}
			}
			else
			{
				BLADE_EXCEPT(EXC_API_ERROR,BTString("set render device config failed.") );
			}//pop up for user config
		}

		//get configuration
		//set default: tri-linear
		SAMPLER_OPTION		GlobalSamplerOption;
		GlobalSamplerOption.mag = TFM_LINEAR;
		GlobalSamplerOption.mip = TFM_LINEAR;
		GlobalSamplerOption.min = TFM_LINEAR;
		GlobalSamplerOption.maxAnisotropy = 1;
		GlobalSamplerOption.u = GlobalSamplerOption.v = GlobalSamplerOption.w = TAM_WRAP;

		IConfig* filter_cfg = mGraphicsOption->getSubConfigByName(GraphicsOptionString::TextureFilter);
		if( filter_cfg != NULL )
		{
			const TString& filterOption = filter_cfg->getValue();
			if( filterOption == GraphicsOptionString::FilerBiLinear )
			{
				GlobalSamplerOption.mag = TFM_LINEAR;
				GlobalSamplerOption.mip = TFM_POINT;
				GlobalSamplerOption.min = TFM_LINEAR;
			}
			else if( filterOption == GraphicsOptionString::FilerAnisotropic )
			{
				//TODO: check the device caps to see if the mag filter support anisotropic
				GlobalSamplerOption.mag = TFM_ANISOTROPIC;
				GlobalSamplerOption.mip = TFM_LINEAR;
				GlobalSamplerOption.min = TFM_ANISOTROPIC;
			}
			else{}////default,no change
		}

		mRenderDevice->setGlobalSamplerOption(GlobalSamplerOption);
		mRenderDevice->open();

		IMaterialManager::getSingleton().initialize();
	}

	//////////////////////////////////////////////////////////////////////////
	void GraphicsSystem::update()
	{
		for( SceneList::iterator i = mSceneList.begin(); i != mSceneList.end(); ++i )
		{
			IRenderScene* scene = i->second;
			scene->onMainLoopUpdate();
		}

		mRenderDevice->update();
	}

	//////////////////////////////////////////////////////////////////////////
	void GraphicsSystem::shutdown()
	{
		if( mRenderDevice != NULL && mRenderDevice->isOpen() )
			IEventManager::getSingleton().dispatchEvent(RenderDeviceCloseEvent());

		RenderUtility::releaseResources();
		IRenderSchemeManager::getSingleton().shutdown();

		mSceneList.clear();
		mViews.clear();

		for (int i = 0; i < BT_COUNT; ++i)
			mBuiltinTextures[i].clear();

		IMaterialManager::getSingleton().shutdown();
		mRenderDevice.unbind();
		mDevices.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		GraphicsSystem::createScene(const TString& name)
	{
		SceneHandle& hScene = mSceneList[name];

		if(hScene != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a scene with the same name already exist:")+name );

		IRenderScene* scene = BLADE_FACTORY_CREATE(IRenderScene, IRenderScene::GRAPHICS_SCENE_TYPE );
		scene->setupRenderDevice( mRenderDevice );
		hScene.bind( scene );

		return hScene;
	}

	//////////////////////////////////////////////////////////////////////////
	void		GraphicsSystem::destroyScene(const TString& name)
	{
		SceneList::iterator i = mSceneList.find(name);
		if( i == mSceneList.end() )
			return;

		mSceneList.erase(i);
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		GraphicsSystem::getScene(const TString& name) const
	{
		SceneList::const_iterator i = mSceneList.find(name);
		if( i == mSceneList.end() )
			return NULL;
		else
			return i->second;
	}

	/************************************************************************/
	/* IGraphicsSystem specs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IGraphicsView*	GraphicsSystem::createView(const TString& name)
	{
		ViewHandle& handle = mViews[name];

		if( handle != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("cannot create view,a view with the same name already exist:")+name );

		View* view = BLADE_NEW View(name);
		handle.bind( view );
		const TString& defaultScheme = mGraphicsOption->getSubConfigByName( GraphicsOptionString::RenderScheme )->getValue();
		view->setRenderScheme(defaultScheme);
		return view;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GraphicsSystem::destroyView(const TString& name)
	{
		//deleting view may re-enter to destroyView, because view object's render scheme may contain custom views
		//mViews.erase(name);
		ViewList::iterator i = mViews.find(name);
		i->second.clear();
		mViews.erase(i);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsSystem::destroyView(IGraphicsView* view)
	{
		if( view == NULL )
			return;

		View* graphicsview = static_cast<View*>(view);

		ViewList::iterator i = mViews.find( graphicsview->getName() );
		if( i == mViews.end() )
		{
			BLADE_EXCEPT(EXC_NEXIST,BTString("specific view not found.") );
		}
		mViews.erase(i);
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsView*	GraphicsSystem::getView(const TString& name) const
	{
		ViewList::const_iterator i = mViews.find(name);
		if( i == mViews.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderTarget*	GraphicsSystem::getRenderTarget(const TString& name) const
	{
		if( mRenderDevice != NULL )
			return mRenderDevice->getRenderTarget(name);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderTarget*	GraphicsSystem::createRenderTarget(const TString& name,size_t width,size_t height,
		bool creatColorBuffer/* = false*/,bool createDepthBuffer/* = false*/,
		PixelFormat format/* = PF_UNDEFINED*/,PixelFormat depthFormat/* = PF_UNDEFINED*/)
	{
		if( mRenderDevice != NULL )
			return mRenderDevice->createRenderTarget(name,width,height,creatColorBuffer,createDepthBuffer,format,depthFormat);
		else
			BLADE_EXCEPT(EXC_NEXIST,BTString("render device not ready.") );
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderWindow*			GraphicsSystem::createRenderWindow(IWindow* pwin)
	{
		if( mRenderDevice == NULL || pwin == NULL )
		{
			assert(false);
			return NULL;
		}
		else
		{
			bool bDeviceOpened = mRenderDevice->isOpen();
			IRenderWindow* pRW = mRenderDevice->createRenderWindow(pwin);

			if( !bDeviceOpened && mRenderDevice->isOpen() )
			{
				const TString& deviceProfile = IGraphicsResourceManager::getSingleton().getHighestProfile();

				const TString& defaultScheme = mGraphicsOption->getSubConfigByName( GraphicsOptionString::RenderScheme )->getValue();
				const TStringList& defSchemeProfiles = *IRenderSchemeManager::getSingleton().getSchemeProfiles(defaultScheme);
				assert( defSchemeProfiles.size() > 0 );

				if( deviceProfile < defSchemeProfiles[0] )
				{
					BLADE_LOG(Warning, BTString("device profile '") << deviceProfile << BTString("' is lower than scheme's lowest profile '") << defSchemeProfiles[0]
						<< BTString("' trying to select scheme of lower profile."));

					index_t highest = INVALID_INDEX;
					const TString* highestProfile = NULL;
					for(size_t i = 0; i < IRenderSchemeManager::getSingleton().getSchemeCount(); ++i)
					{
						const TStringList& profiles = *IRenderSchemeManager::getSingleton().getSchemeProfiles(i);
						assert( profiles.size() > 0 );
						index_t index = 0;
						while( index < profiles.size() && profiles[index] <= deviceProfile)
							++index;
						if( index > 1 )
						{
							if( highestProfile == NULL || *highestProfile < profiles[index-1] )
							{
								highestProfile = &profiles[index-1];
								highest = i;
							}
						}
					}

					if( highest == INVALID_INDEX )
						BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("current device spec is too low to run."));

					assert( highestProfile != NULL );

					const TString& scheme = IRenderSchemeManager::getSingleton().getScheme(highest);

					BLADE_LOG(Warning, BTString("select scheme '") << scheme
						<< BTString("' with profile '") << *highestProfile << BTString("'."));

					mGraphicsOption->getSubConfigByName( GraphicsOptionString::RenderScheme )->setValue(scheme);
					mProfile = *highestProfile;
				}
				else
				{
					index_t index = 0;
					while(index < defSchemeProfiles.size() && defSchemeProfiles[index] <= deviceProfile)
						++index;
					if( index > 0 )
						mProfile = defSchemeProfiles[index-1];
				}

				IEventManager::getSingleton().dispatchEvent(RenderDeviceReadyEvent(deviceProfile, mProfile));
				//ITimeService::getSingleton().resetTimeSource();

				//create built-in textures
				Color colors[BT_COUNT] =
				{
					Color::WHITE,	//BT_WHITE,
					Color::BLACK,	//BT_BLACK,
					Color::GRAY,	//BT_GREY,
					Color::RED,		//BT_RED,
					Color::GREEN,	//BT_GREEN,
					Color::BLUE,	//BT_BLUE,
					Color(0, 1, 1),	//BT_CYAN,
					Color(1, 0, 1),	//BT_MAGENTA
					Color(1, 1, 0),	//BT_YELLOW
					Color(0, 0, 1), //BT_UNIT_NORMAL
				};
				const PixelFormat pf = PF_ARGB;

				IMG_ORIENT orient = IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureDir;
				PixelFormat compressedFmt = IGraphicsResourceManager::getSingleton().getGraphicsConfig().CompressedRGB;
				PixelFormat compressedNormal = IGraphicsResourceManager::getSingleton().getGraphicsConfig().CompressedNormal;
				for (int i = 0; i < BT_COUNT; ++i)
				{
					HIMAGE img = IImageManager::getSingleton().createImage(4, 4, IP_TEMPORARY, pf, orient, 1);
					uint8* data = img->getBits();
					for (size_t j = 0; j < 4; ++j)
					{
						uint8* line = data;
						for (size_t k = 0; k < 4; ++k)
						{
							ColorFormatter::writeColorLDR(line, colors[i], pf);
							line += pf.getSizeBytes();
						}
						data += img->getPitch();
					}
					HIMAGE compressedImg = IImageManager::getSingleton().convertToFormat(img, BT_UNIT_NORMAL == i ? compressedNormal : compressedFmt, IP_TEMPORARY, 1);
					HTEXTURE texture = IGraphicsResourceManager::getSingleton().createTexture(*compressedImg, 1);
					mBuiltinTextures[i].bind(BLADE_NEW TextureResource());
					mBuiltinTextures[i].staticCast<TextureResource>()->setTexture(texture);
				}
			}

			return pRW;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsSystem::destroyRenderTarget(const TString& name)
	{
		IRenderTarget* target;
		if( mRenderDevice == NULL || ( target= mRenderDevice->getRenderTarget(name) ) == NULL )
			return;

		for(SceneList::const_iterator i = mSceneList.begin(); i != mSceneList.end(); ++i)
		{
			IRenderScene* scene = i->second;
			if( scene->isRenderTargetUsed(target) )
				BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("Render target is still in use by tasks,cannot destroy.") );
		}

		mRenderDevice->destroyRenderTarget(name);
	}


	//////////////////////////////////////////////////////////////////////////
	IGraphicsScene*	GraphicsSystem::getGraphicsScene(const TString& name) const
	{
		SceneList::const_iterator i = mSceneList.find(name);
		if( i == mSceneList.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			GraphicsSystem::getGraphicsSceneCount() const
	{
		return mSceneList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsScene*	GraphicsSystem::getGraphicsScene(index_t index) const
	{
		if( index < mSceneList.size() )
		{
			SceneList::const_iterator i = mSceneList.begin();
			std::advance(i, index);
			return i->second;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GraphicsSystem::addGraphicsType(IGraphicsType* type)
	{
		return IRenderTypeManager::getSingleton().addRenderType( static_cast<RenderType*>(type) );
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			GraphicsSystem::getGraphicsTypeCount() const
	{
		return IRenderTypeManager::getSingleton().getRenderTypeCount();
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsType*			GraphicsSystem::getGraphicsType(index_t index) const
	{
		return IRenderTypeManager::getSingleton().getRenderType(index);
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsType*			GraphicsSystem::getGraphicsType(const TString& name) const
	{
		return IRenderTypeManager::getSingleton().getRenderType(name);
	}

	//////////////////////////////////////////////////////////////////////////
	AppFlag					GraphicsSystem::generateGraphicsTypeAppFlag(IGraphicsType& type)
	{
		if (mRenerTypeFlagIndex < sizeof(AppFlag) * 8)
		{
			AppFlag& flag = msRenderTypeFlags[type.getName()];
			if (flag == 0)
				flag.raiseBitAtIndex(mRenerTypeFlagIndex++);
			static_cast<RenderType&>(type).setAppFlag(flag);
			return flag;
		}
		else
		{
			assert(false);
			//render type not found: not such render type
			//this render type is not registered
			//maybe the corresponding module is not loaded
			return INVALID_APPFLAG;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					GraphicsSystem::getSpaceTypes(TStringParam& outList) const
	{
		size_t count = SpaceFactory::getSingleton().getNumRegisteredClasses();
		for( size_t i = 0; i < count; ++i )
		{
			const TString& type = SpaceFactory::getSingleton().getRegisteredClass(i);
			outList.push_back(type);
		}
		return count;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					GraphicsSystem::getCoordinatorTypes(TStringParam& outList) const
	{
		size_t count = CoordinatorFactory::getSingleton().getNumRegisteredClasses();
		for( size_t i = 0; i < count; ++i )
		{
			const TString& type = CoordinatorFactory::getSingleton().getRegisteredClass(i);
			outList.push_back(type);
		}
		return count;
	}

	/************************************************************************/
	/* IGraphicsSystem interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	ICamera*				GraphicsSystem::createCamera() const
	{
		return BLADE_NEW Camera();
	}

	//////////////////////////////////////////////////////////////////////////
	bool					GraphicsSystem::isGraphiscContext() const
	{
		return BLADE_TS_CHECK(TS_MAIN_SYNC) || Thread::getCurrentID() == mTaskID;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	GraphicsSystem::genterateOptions()
	{
		if( mGraphicsOption != NULL )
			return;

		ConfigGroup* pGraphicsConfig = BLADE_NEW ConfigGroup(GraphicsOptionString::OptionName);
		//main API
		ConfigGroup* group = BLADE_NEW ConfigGroup(
			GraphicsOptionString::RenderDeivce,
			RenderDeviceFactory::getSingleton().getRegisteredClass(0) 
			);

		for( size_t i = 0; i < RenderDeviceFactory::getSingleton().getNumRegisteredClasses(); ++i )
		{
			const TString& name = RenderDeviceFactory::getSingleton().getRegisteredClass(i);

			group->addOption(name);

			RenderDeviceHandle& device = mDevices[name];

			assert( device == NULL );

			device.bind( BLADE_FACTORY_CREATE(IRenderDevice, name) );
			//device option
			ParamList devConfig;
			IDevice::Options options;
			device->getDeviceOption(devConfig, options);
			for( size_t optn = 0; optn < devConfig.size(); ++optn)
			{
				const TString& optName = devConfig.at(optn).getName();
				const Variant& val = devConfig.at(optn).getValue();
				const TStringParam& curOpt = options[optName];
				
				ConfigAtom* optConfig;
				if(curOpt.size() == 0)
					optConfig = BLADE_NEW ConfigAtom(optName, val, val.isReadOnly() ? CAF_NONE_CONFIG : CAF_NORMAL);
				else
					optConfig = BLADE_NEW ConfigAtom(optName, curOpt, CAF_NORMAL);

				group->addSubConfigPtr(optConfig,name);
			}
		}
		pGraphicsConfig->addSubConfigPtr( group );

		//get the render scheme configs
		{
			size_t schemeCount = IRenderSchemeManager::getSingleton().getSchemeCount();
			//assert( schemeCount > 0 );
			TStringParam list;
			for( size_t i = 0; i < schemeCount; ++i)
			{
				const TString& scheme = IRenderSchemeManager::getSingleton().getScheme(i);
				list.push_back( scheme );
			}
			ConfigAtom* cfg = BLADE_NEW ConfigAtom(GraphicsOptionString::RenderScheme, list, CAF_NORMAL_DEVMODE);
			pGraphicsConfig->addSubConfigPtr(cfg);
		}

		{
			TStringParam list;
			list.push_back( GraphicsOptionString::FilerBiLinear );
			list.push_back( GraphicsOptionString::FilerTriLinear );
			list.push_back( GraphicsOptionString::FilerAnisotropic );
			ConfigAtom* texture_cfg = BLADE_NEW ConfigAtom(GraphicsOptionString::TextureFilter, list, CAF_NORMAL);
			pGraphicsConfig->addSubConfigPtr(texture_cfg);
		}
	
		mGraphicsOption.bind( pGraphicsConfig );
		IConfigManager::getSingleton().addConfig(mGraphicsOption);

		////TODO:
		////- for now plugin adds config actively by calling IGraphicsSystem::addAdvanceConfig
		////Or: add interface for IPlatformManager to get all plugins depending on this,
		////and then add plugin options to advance option

		//for(size_t i = 0; i < manager->getModuleCount(); ++i)
		//{
		//	const HMODULE& module = manager->getModule(i);
		//	const HCONFIG& config = module->getConfig();
		//	if( config != NULL )
		//		mAdvancedOption->addSubConfig( config );
		//}
		IConfigManager::getSingleton().addConfig(mAdvancedOption);
	}

}//namespace Blade