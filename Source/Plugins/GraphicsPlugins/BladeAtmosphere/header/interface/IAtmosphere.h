/********************************************************************
	created:	2011/09/01
	filename: 	IAtmosphere.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IAtmosphere_h__
#define __Blade_IAtmosphere_h__
#include <BladeAtmosphere.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/graphics/IGraphicsType.h>
#include <interface/public/IElement.h>

namespace Blade
{

	class IAtmosphere : public InterfaceSingleton<IAtmosphere>
	{
	public:
		virtual ~IAtmosphere() {}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&		getAtmosphereConfig() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsType*		getAtmosphereType() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				initialize(IElement* element) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				shutdown() = 0;
	};
	

	extern template class BLADE_ATMOSPHERE_API Factory<IAtmosphere>;

}//namespace Blade



#endif // __Blade_IAtmosphere_h__