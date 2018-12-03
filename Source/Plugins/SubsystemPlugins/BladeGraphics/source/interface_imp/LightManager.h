/********************************************************************
	created:	2011/08/30
	filename: 	LightManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LightManager_h__
#define __Blade_LightManager_h__
#include <Singleton.h>
#include <utility/BladeContainer.h>

#include <interface/ILightManager.h>
#include "Light.h"

namespace Blade
{

	class LightManager : public ILightManager, public Allocatable
	{
	public:
		LightManager();
		~LightManager();

		/************************************************************************/
		/* ILightSource interface                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Color&	getGlobalAmbient() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setGlobalAmbient(const Color& color);

		/**
		@describe get global directional light count, limited to MAX_GLOBAL_LIGHT_COUNT
		@param
		@return
		*/
		virtual size_t			getGlobalLightCount() const;

		/**
		@describe get global directional lights
		@param
		@return
		*/
		virtual ILight**		getGlobalLightList();

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getLightList(ILight** outList = NULL, size_t count = 0);

		/*
		@describe
		@param
		@return
		*/
		virtual ILight*			getMainLight() const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			setMainLight(ILight* light);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		void	notifyLightCreated(ILight* light);

		/*
		@describe 
		@param 
		@return 
		*/
		void	notifyLightDeleted(ILight* light);

		/**
		@describe 
		@param
		@return
		*/
		void	notifyLightTypeChanged(ILight* light, ELightType oldType, ELightType newType);

		/** @brief  */
		void	setSpace(ISpace* space);

	protected:
		typedef Vector<ILight*>	LightArray;
		typedef List<ILight*>	LightList;

		//non-directional lights
		LightList	mLocalLightList;
		//directional lights
		LightArray	mGlobalLightList;

		Color		mGlobalAmbient;
		Light*		mAmbient;
		ISpace*		mSpace;	//observer

		ILight*		mMainLight;
	};

	

}//namespace Blade



#endif // __Blade_LightManager_h__