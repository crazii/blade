/********************************************************************
	created:	2010/05/07
	filename: 	IGeometryScene.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IGeometryScene_h__
#define __Blade_IGeometryScene_h__
#include <interface/public/IScene.h>
#include <interface/public/geometry/GeomTypes.h>

namespace Blade
{
	class IGeometryScene : public IScene
	{
	public:
		virtual ~IGeometryScene()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual GEOM_UPDATE_METHOD	getUpdateMethod() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void				setUpdateMethod(GEOM_UPDATE_METHOD method) = 0;

		/**
		@describe update the geometry if update method is manual otherwise,do not call it
		@param
		@return
		*/
		virtual void				update() = 0;

	};//class IGeometryScene
	
}//namespace Blade


#endif //__Blade_IGeometryScene_h__