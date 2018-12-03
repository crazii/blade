/********************************************************************
	created:	2012/04/01
	filename: 	ICameraManager.h
	author:		Crazii
	purpose:	per scene manager
*********************************************************************/
#ifndef __Blade_ICameraManager_h__
#define __Blade_ICameraManager_h__
#include <utility/String.h>

namespace Blade
{
	class ICamera;

	class ICameraManager
	{
	public:
		virtual ~ICameraManager() {}

		/*
		@describe
		@param
		@return
		*/
		virtual ICamera*	getCamera(const TString& name) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual	bool		addCamera(const TString& name, ICamera* camera) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool		removeCamera(const TString& name) = 0;
	};
	

}//namespace Blade


#endif //__Blade_ICameraManager_h__