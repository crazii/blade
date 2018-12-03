/********************************************************************
	created:	2013/04/22
	filename: 	IViewportManager.h
	author:		Crazii
	purpose:	built-in ui view port info management
*********************************************************************/
#ifndef __Blade_IViewportManager_h__
#define __Blade_IViewportManager_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include "IViewport.h"

namespace Blade
{
	class IViewport;
	class IViewportManager : public InterfaceSingleton<IViewportManager>
	{
	public:
		virtual ~IViewportManager() {}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize() = 0;

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
		virtual size_t	getLayoutCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual index_t	getCurrentLayout() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	setCurrentLayout(index_t index) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const VIEWPORT_LAYOUT&	getLayout(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IViewportSet*	createViewportSet(const VIEWPORT_LAYOUT& layout) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	destroyViewportSet(IViewportSet* vps) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&	getViewportConfig(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&	getAllConfig() const = 0;
	};

	extern template class BLADE_FRAMEWORK_API Factory<IViewportManager>;
	
}//namespace Blade

#endif //  __Blade_IViewportManager_h__