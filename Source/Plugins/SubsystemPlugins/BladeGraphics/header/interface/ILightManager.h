/********************************************************************
	created:	2011/08/31
	filename: 	ILightManager.h
	author:		Crazii
	purpose:	per-scene light manager
*********************************************************************/
#ifndef __Blade_ILightSource_h__
#define __Blade_ILightSource_h__

#include <BladeGraphics.h>
#include <interface/public/graphics/Color.h>
#include <interface/ILight.h>

namespace Blade
{
	class ICamera;

	class BLADE_GRAPHICS_API ILightManager
	{
	public:
		virtual ~ILightManager() {}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Color&	getGlobalAmbient() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setGlobalAmbient(const Color& color) = 0;

		/**
		@describe get global directional light count
		@param
		@return
		*/
		virtual size_t			getGlobalLightCount() const = 0;

		/**
		@describe get global directional lights
		@param
		@return
		*/
		virtual ILight**		getGlobalLightList() = 0;

		/*
		@describe get non-directional lights
		@param
		@return
		*/
		virtual size_t			getLightList(ILight** outList = NULL, size_t count = 0) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual ILight*			getMainLight() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			setMainLight(ILight* light) = 0;
	};

}//namespace Blade



#endif // __Blade_ILightSource_h__