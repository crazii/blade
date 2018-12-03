/********************************************************************
	created:	2013/04/23
	filename: 	ViewportManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ViewportManager_h__
#define __Blade_ViewportManager_h__
#include <Singleton.h>
#include <utility/BladeContainer.h>
#include <utility/DataBinding.h>
#include <interface/public/ui/IViewportManager.h>

namespace Blade
{
	class ViewportManager : public IViewportManager, public Bindable, public Singleton<ViewportManager>
	{
	public:
		using Singleton<ViewportManager>::getSingleton;
		using Singleton<ViewportManager>::getSingletonPtr;
	public:
		ViewportManager();
		~ViewportManager();

		/************************************************************************/
		/* IViewportManager                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	shutdown();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t	getLayoutCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual index_t	getCurrentLayout() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	setCurrentLayout(index_t index);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const VIEWPORT_LAYOUT&	getLayout(index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IViewportSet*	createViewportSet(const VIEWPORT_LAYOUT& layout);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	destroyViewportSet(IViewportSet* vps);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&	getViewportConfig(index_t index) const;

		/**
		@describe
		@param
		@return
		*/
		virtual const HCONFIG&	getAllConfig() const { return mConfig; }

	protected:

		/** @brief  */
		void	onViewportConfigChange(void* data);

		typedef Vector<VIEWPORT_LAYOUT>	LayoutList;

		LayoutList	mLayouts;
		uint32		mCurrentLayout;
		HCONFIG		mViewportCameraConfig[MAX_VIEWPORT_COUNT];
		HCONFIG		mConfig;
	};

	
}//namespace Blade

#endif//__Blade_ViewportManager_h__