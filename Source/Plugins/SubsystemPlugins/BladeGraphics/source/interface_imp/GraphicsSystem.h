/********************************************************************
	created:	2009/02/28
	filename: 	GraphicsSystem.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsSystem_h__
#define __Blade_GraphicsSystem_h__
#include <StaticHandle.h>
#include <threading/Thread.h>
#include <interface/IGraphicsSystem.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/IConfig.h>
#include <interface/IRenderScene.h>
#include "View.h"

namespace Blade
{
	class GraphicsSystem : public IGraphicsSystem , public Singleton<GraphicsSystem>
	{
	public:
		using Singleton<GraphicsSystem>::getSingleton;
		using Singleton<GraphicsSystem>::getSingletonPtr;
	public:
		static const TString GRAPHICS_SYSTEM_NAME;
	public:
		GraphicsSystem();
		virtual ~GraphicsSystem();

		/************************************************************************/
		/*SubSystem spec Interface                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName();


		/*
		@describe this would be called when added to Framework
		@param 
		@return 
		*/
		virtual void		install();

		/*
		@describe
		@param
		@return
		*/
		virtual void		uninstall();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		initialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		update();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		shutdown();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		createScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		destroyScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		getScene(const TString& name) const;

		/************************************************************************/
		/* IGraphicsService specs                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsView*	createView(const TString& name);

		/*
		@describe
		@param
		@return
		*/
		virtual void			destroyView(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			destroyView(IGraphicsView* view);

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsView*	getView(const TString& name) const;

		/*
		@describe get the created render target
		@note use a window's unique name to get the window's default render target
		@param 
		@return 
		*/
		virtual IRenderTarget*	getRenderTarget(const TString& name) const;

		/*
		@describe
		@param
		@return
		*/
		virtual IRenderTarget*	createRenderTarget(const TString& name,size_t width,size_t height,
			bool creatColorBuffer = false,bool createDepthBuffer = false,
			PixelFormat format = PF_UNDEFINED,PixelFormat depthFormat = PF_UNDEFINED);

		/*
		@describe
		@param
		@return
		*/
		virtual IRenderWindow*			createRenderWindow(IWindow* pwin);

		/*
		@describe
		@param
		@return
		*/
		virtual void			destroyRenderTarget(const TString& name);

		/** @brief  */
		virtual void			destroyRenderTarget(IRenderTarget* target)
		{
			this->destroyRenderTarget( target->getName() );
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual IGraphicsScene* createGraphicsScene(const TString& name)
		{
			return static_cast<IGraphicsScene*>( this->createScene(name) );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsScene*	getGraphicsScene(const TString& name) const;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getGraphicsSceneCount() const;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsScene*	getGraphicsScene(index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addGraphicsType(IGraphicsType* type);


		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getGraphicsTypeCount() const;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsType*	getGraphicsType(index_t index) const;

		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsType*	getGraphicsType(const TString& name) const;

		/**
		@describe
		@param
		@return render type's app flag, if render type exist, otherwise return INVALID_APPFLAG
		@note: the app flag generated should be same for the same render type
		and should be the same for ALL WORLD instances
		*/
		virtual AppFlag			generateGraphicsTypeAppFlag(IGraphicsType& type);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getSpaceTypes(TStringParam& outList) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getCoordinatorTypes(TStringParam& outList) const;

		/************************************************************************/
		/* IGraphicsSystem interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual bool					isDeviceValid() const
		{
			return mRenderDevice != NULL && mRenderDevice->isOpen();
		}

		/*
		@describe 
		@param
		@return
		*/
		virtual const TString&			getCurrentProfile() const
		{
			return mProfile;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&			getGraphicsSceneType() const
		{
			return IRenderScene::GRAPHICS_SCENE_TYPE;
		}

		/*
		@describe add advanced config for graphics system (the basic config is internal used)
		@param 
		@return 
		*/
		virtual bool					addAdvanceConfig(const HCONFIG& config)
		{
			return mAdvancedOption != NULL && mAdvancedOption->addSubConfig(config);
		}

		/*
		@describe 
		@param 
		@return 
		@note	you should remove your config on your module end @see removeAdvanceConfig
		*/
		virtual bool					removeAdvanceConfig(const HCONFIG& config)
		{
			return mAdvancedOption != NULL && mAdvancedOption->removeSubConfig(config->getName());
		}

		/**
		@describe 
		@param
		@return
		*/
		const HCONFIG&					getAdvanceConfig(const TString& name) const
		{
			return mAdvancedOption != NULL ? mAdvancedOption->getSubConfigByName(name) : HCONFIG::EMPTY;
		}

		/*
		@describe create a single camera by internal use (render schemes etc.)
		@param 
		@return 
		@note: camera created by this way cannot be manipulated(set position/orientation)
		camera element attached to a entity can be.since entities must be in scenes,
		so you can create camera entity to manipulate it.
		or use the scene's camera manager to get the existing ICamera interface(which also cannot be move/rotated).
		*/
		virtual ICamera*				createCamera() const;

		/**
		@describe check if current thread context is main sync or graphics task
		some graphics resource need this valid context to perform post-processing(update graphics resources to graphis card)
		@param
		@return
		*/
		virtual bool					isGraphiscContext() const;

		/**
		@describe 
		@param
		@return
		*/
		virtual HRESOURCE				getBuiltinTextures(EBuiltinTexture builtin) const
		{
			if (builtin >= 0 && builtin < BT_COUNT)
				return mBuiltinTextures[builtin];
			assert(false);
			return HRESOURCE::EMPTY;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline void				setGraphicsTaskID(Thread::ID id)
		{
			mTaskID = id;
		}

	protected:
		void			genterateOptions();

		typedef StaticHandle<View>			ViewHandle;
		typedef StaticHandle<IRenderScene>	SceneHandle;
		typedef StaticHandle<IRenderDevice>	RenderDeviceHandle;

		typedef	TStringMap<RenderDeviceHandle>	DeviceList;
		typedef	TStringMap<SceneHandle>			SceneList;
		typedef TStringMap<ViewHandle>			ViewList;
		typedef StaticTStringMap<AppFlag>		RenderTypeFlag;
		
		TString				mProfile;
		HCONFIG				mGraphicsOption;
		HCONFIG				mAdvancedOption;
		SceneList			mSceneList;
		DeviceList			mDevices;
		RenderDeviceHandle	mRenderDevice;
		ViewList			mViews;
		Thread::ID			mTaskID;
		size_t				mRenerTypeFlagIndex;
		HRESOURCE			mBuiltinTextures[BT_COUNT];
		RenderTypeFlag		msRenderTypeFlags;
	};//class GraphicsSystem

}//namespace Blade

#endif // __Blade_GraphicsSystem_h__