/********************************************************************
	created:	2009/02/28
	filename: 	IGraphicsSystem.h
	author:		Crazii
	
	purpose:	GraphicsSystem interface
*********************************************************************/
#ifndef __Blade_IGraphicsSystem_h__
#define __Blade_IGraphicsSystem_h__
#include <interface/InterfaceSingleton.h>
#include <interface/public/ISubsystem.h>
#include <interface/public/IResource.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <BladeGraphics.h>

namespace Blade
{
	class ICamera;

	class IGraphicsSystem  : public ISubsystem, public IGraphicsService, public InterfaceSingleton<IGraphicsSystem>
	{
	public:
		using InterfaceSingleton<IGraphicsSystem>::getSingleton;
		using InterfaceSingleton<IGraphicsSystem>::getSingletonPtr;
		enum EBuiltinTexture
		{
			BT_WHITE,
			BT_BLACK,
			BT_GRAY,
			BT_RED,
			BT_GREEN,
			BT_BLUE,
			BT_CYAN,
			BT_MAGENTA,
			BT_YELLOW,
			BT_UNIT_NORMAL,

			BT_COUNT,
		};
	public:
		virtual ~IGraphicsSystem()		{}

		/*
		@describe
		@param
		@return
		*/
		virtual bool					isDeviceValid() const = 0;

		/*
		@describe 
		@param
		@return
		*/
		virtual const TString&			getCurrentProfile() const= 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&			getGraphicsSceneType() const = 0;

		/*
		@describe add advanced config for graphics system (the basic config is internal used)
		@param 
		@return 
		@note	you should remove your config on your module end @see removeAdvanceConfig
		*/
		virtual bool					addAdvanceConfig(const HCONFIG& config) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool					removeAdvanceConfig(const HCONFIG& config) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const HCONFIG&			getAdvanceConfig(const TString& name) const = 0;
	
		/*
		@describe create a single camera by internal use (render schemes etc.)
		@param 
		@return 
		@note: camera created by this way cannot be manipulated(set position/orientation)
		camera element attached to a entity can be.since entities must be in scenes,
		so you can create camera entity to manipulate it.
		or use the scene's camera manager to get the existing ICamera interface(which also cannot be move/rotated).
		*/
		virtual ICamera*				createCamera() const = 0;

		/**
		@describe check if current thread context is main sync or graphics task
		some graphics resource need this valid context to perform post-processing(update graphics resources to graphics card)
		this method should be used as runtime check only.
		@param
		@return
		*/
		virtual bool					isGraphiscContext() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual HRESOURCE				getBuiltinTextures(EBuiltinTexture builtin) const = 0;
	};//class IGraphicsSystem

	extern template class BLADE_GRAPHICS_API Factory<IGraphicsSystem>;

	
}//namespace Blade


#endif // __Blade_IGraphicsSystem_h__