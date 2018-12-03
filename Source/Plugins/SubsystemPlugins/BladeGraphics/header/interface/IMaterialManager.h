/********************************************************************
	created:	2012/01/19
	filename: 	IMaterialManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IMaterialManager_h__
#define __Blade_IMaterialManager_h__
#include <interface/InterfaceSingleton.h>
#include <BladeGraphics.h>
#include <math/Vector3.h>

namespace Blade
{

	class Material;
	class IRenderDevice;
	class Technique;

	class IMaterialManager : public InterfaceSingleton<IMaterialManager>
	{
	public:
		virtual ~IMaterialManager()	{}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			initialize() = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			shutdown() = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual Material*		getMaterial(const TString& name) const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			destroyMaterial(const TString& name) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual Material*		createMaterial(const TString& name) = 0;

	};


	extern template class BLADE_GRAPHICS_API Factory<IMaterialManager>;

}//namespace Blade



#endif // __Blade_IMaterialManager_h__