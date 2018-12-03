/********************************************************************
	created:	2013/11/11
	filename: 	IEditorConfigManager.h
	author:		Crazii
	purpose:	config management for editor framework - global config, tool config, view port config etc.
*********************************************************************/
#ifndef __Blade_IEditorConfigManager_h__
#define __Blade_IEditorConfigManager_h__
#include <BladeEditor.h>
#include <interface/InterfaceSingleton.h>
#include <Factory.h>
#include <interface/IConfig.h>
#include <interface/public/geometry/GeomTypes.h>

namespace Blade
{
	class IEditorConfigManager : public InterfaceSingleton<IEditorConfigManager>
	{
	public:
		virtual ~IEditorConfigManager() {}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	shutdown() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadConfig(const TString& config) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveConfig(const TString& config) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	addConfig(const HCONFIG& config) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&	getConfig(const TString& name) const = 0;

		/**
		@describe buit-in global config
		@param 
		@return 
		*/
		virtual GEOM_SPACE	getTransformSpace() const = 0;
	};

	extern template class BLADE_EDITOR_API Factory<IEditorConfigManager>;
	
}//namespace Blade

#endif //  __Blade_IEditorConfigManager_h__