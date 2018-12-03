/********************************************************************
	created:	2011/09/01
	filename: 	AtmosphereElement.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AtmosphereElement_h__
#define __Blade_AtmosphereElement_h__
#include <Element/GraphicsElement.h>
#include "SkySphere.h"

namespace Blade
{

	class AtmosphereElement : public GraphicsElement, public Allocatable
	{
	public:
		AtmosphereElement();
		~AtmosphereElement();

		/************************************************************************/
		/* IConfigurabe interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data */
		virtual void			postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		inline void			initSkySphere(scalar radius, size_t density, const TString& texture)
		{
			if (mSkyBox != NULL)
			{
				this->deactivateContent();
				BLADE_DELETE mSkyBox;
				mSkyBox = NULL;
			}

			if (mSkySphere == NULL)
			{
				mSkySphere = BLADE_NEW SkySphere();
				mSkySphere->setElement(this);
				mContent = mSkySphere;
			}
			mSkySphere->initialize(radius, density, texture);

			this->activateContent();
		}

		/**
		@describe 
		@param
		@return
		*/
		inline void			initSkyBox(const TString& texture)
		{
			if (mSkySphere != NULL)
			{
				this->deactivateContent();
				BLADE_DELETE mSkySphere;
				mSkySphere = NULL;
			}

			if (mSkyBox == NULL)
			{
				mSkyBox = BLADE_NEW SkyBox();
				mSkyBox->setElement(this);
				mContent = mSkyBox;
			}
			mSkyBox->initialize(texture);

			this->activateContent();
		}

	protected:
		/*
		@describe
		@param
		@return
		*/
		virtual void		onInitialize();

	protected:
		SkySphere*		mSkySphere;
		SkyBox*			mSkyBox;
	};
	

}//namespace Blade



#endif // __Blade_AtmosphereElement_h__