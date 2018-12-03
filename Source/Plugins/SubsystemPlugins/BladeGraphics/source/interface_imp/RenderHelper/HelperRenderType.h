/********************************************************************
	created:	2011/05/21
	filename: 	HelperRenderType.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_HelperRenderType_h__
#define __Blade_HelperRenderType_h__
#include <RenderType.h>
#include <interface/IMaterialManager.h>
#include <BladeGraphics_blang.h>

namespace Blade
{
	class HelperRenderType : public RenderType, public Singleton<HelperRenderType>
	{
	public:
		HelperRenderType();
		~HelperRenderType()	{}

		/************************************************************************/
		/* IHelperRenderType interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const
		{
			return RenderType::getName();
		}

		/**
		@describe set the application specified flag ,so that application can perform some operation to the type using a flag mask
		@param 
		@return 
		*/
		virtual AppFlag			getAppFlag() const
		{
			return RenderType::getAppFlag();
		}

		/**
		@describe the app flag is set by application
		@param 
		@return 
		*/
		virtual void			setAppFlag(AppFlag flag)
		{
			return RenderType::setAppFlag(flag);
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual Material*				getMaterial() const
		{
			return NULL;
		}
	protected:
	};

}//namespace Blade



#endif // __Blade_AABBRenderType_h__