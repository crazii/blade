/********************************************************************
	created:	2012/04/01
	filename: 	CameraManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_CameraManager_h__
#define __Blade_CameraManager_h__
#include <interface/ICameraManager.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class ISpace;

	class CameraManager : public ICameraManager, public Allocatable
	{
	public:
		CameraManager();
		~CameraManager();

		/*
		@describe
		@param
		@return
		*/
		virtual ICamera*	getCamera(const TString& name) const;

		/*
		@describe
		@param
		@return
		*/
		virtual	bool		addCamera(const TString& name, ICamera* camera);

		/*
		@describe
		@param
		@return
		*/
		virtual bool		removeCamera(const TString& name);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		void	setSpace(ISpace* space);

	protected:
		typedef TStringMap<ICamera*>	CameraNameMap;

		CameraNameMap	mCameraMap;
		ISpace*			mSpace;
	};
	

}//namespace Blade


#endif //__Blade_CameraManager_h__