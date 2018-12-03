/********************************************************************
	created:	2011/08/30
	filename: 	ILight.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ILight_h__
#define __Blade_ILight_h__
#include <BladeGraphics.h>
#include <math/Vector3.h>
#include <interface/public/graphics/Color.h>
#include <interface/public/graphics/GraphicsDefs.h>

namespace Blade
{
	class ILightManager;

	class BLADE_GRAPHICS_API ILight
	{
	public:
		virtual ~ILight()	{}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const uint32&	getType() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Vector3&	getPosition() const = 0;

		/**
		@describe fAttenuation = 1/(Constant + Linear * Distance + Quadratic * Distance ^ 2):
		fAttenuation = 1/(attenuation[0] + attenuation[1] * Distance + attenuation[2] * Distance ^ 2)
		now use Linear Attenuation factor
		@param
		@return
		*/
		virtual const scalar&	getAttenuation() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Vector3&	getDirection() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const Quaternion& getOrientation() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const scalar&	getRange() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const scalar&	getSpotInnerAngle() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const scalar&	getSpotOuterAngle() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Color&	getDiffuse() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Color&	getSpecular() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ILightManager*	getLightManager() const = 0;
	};
	

}//namespace Blade



#endif // __Blade_ILight_h__