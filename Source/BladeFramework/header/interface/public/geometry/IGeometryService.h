/********************************************************************
	created:	2010/05/07
	filename: 	IGeometryService.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IGeometryService_h__
#define __Blade_IGeometryService_h__
#include <interface/InterfaceSingleton.h>
#include <interface/public/geometry/IGeometryScene.h>

namespace Blade
{
	class IGeometryService : public InterfaceSingleton<IGeometryService>
	{
	public:
		virtual ~IGeometryService()	{}

		/**
		@describe 
		@param
		@return
		*/
		virtual IGeometryScene*		createGeometryScene(const TString& name) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IGeometryScene*		getGeometryScene(const TString& name) const = 0;


		/**
		@describe get update method. default GUM_MANUAL
		@param
		@return
		*/
		virtual GEOM_UPDATE_METHOD	getUpdateMethod() const = 0;

		/**
		@describe set update method. default GUM_MANUAL
		@param
		@return
		*/
		virtual void				setUpdateMethod(GEOM_UPDATE_METHOD method) = 0;

		/**
		@describe update the parallel state for geometry scenes.
		if update method is GUM_MANUAL, you MUST call it.Otherwise, DO NOT call it
		@param
		@return
		*/
		virtual void				updateScenes() = 0;

	};//class IGeometryService

	extern template class BLADE_FRAMEWORK_API Factory<IGeometryService>;
	
}//namespace Blade


#endif //__Blade_IGeometryService_h__