/********************************************************************
	created:	2011/09/03
	filename: 	Atmosphere.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Atmosphere_h__
#define __Blade_Atmosphere_h__
#include <interface/IAtmosphere.h>
#include <Element/LightElement.h>
#include "../AtmosphereElement.h"
#include <utility/IOBuffer.h>

namespace Blade
{

	class Atmosphere : public IAtmosphere, public EmptySerializable, public Singleton<Atmosphere>
	{
	public:
		using Singleton<Atmosphere>::getSingleton;
		using Singleton<Atmosphere>::getSingletonPtr;
	public:
		Atmosphere();
		~Atmosphere();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&		getAtmosphereConfig() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsType*		getAtmosphereType() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				initialize(IElement* elem);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				shutdown();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe register the render type
		@param 
		@return 
		*/
		void				registerRenderType();

		/*
		@describe 
		@param 
		@return 
		*/
		void				unregisterRenderType();


	protected:

		/** @brief  */
		void				notifyConfigChange(void* data);

		/*
		@describe 
		@param 
		@return 
		*/
		void				applyConfig();

		AtmosphereElement*	mAtmosphere;
		LightElement*		mLightElement;
		LightElement*		mSecondaryLight;
		HCONFIG				mConfig;

		TString	mType;
		TString mSkyTexture;
		Color	mAmbient;
		Color	mDiffuse;
		Color	mSpecular;
		Color	mSecondDiffuse;
		Color	mSecondSpecular;
		fp32	mRadius;
		uint32	mDensity;
		bool	mInited;
		bool	mSecondLight;
	};
	

}//namespace Blade



#endif // __Blade_Atmosphere_h__