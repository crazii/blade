/********************************************************************
	created:	2010/06/28
	filename: 	IGraphicsType.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IGraphicsType_h__
#define __Blade_IGraphicsType_h__
#include <utility/String.h>
#include <BladeFramework.h>
#include "GraphicsDefs.h"

namespace Blade
{
	class BLADE_FRAMEWORK_API IGraphicsType
	{
	public:
		virtual ~IGraphicsType()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const = 0;

		/**
		@describe set the application specified flag ,so that application can perform some operation to the type using a flag mask
		@param 
		@return 
		*/
		virtual AppFlag			getAppFlag() const = 0;
		
	};//class IGraphicsType
	
}//namespace Blade


#endif //__Blade_IGraphicsType_h__