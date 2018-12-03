/********************************************************************
	created:	2012/03/28
	filename: 	IGraphicsPhase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IGraphicsPhase_h__
#define __Blade_IGraphicsPhase_h__
#include <utility/String.h>

namespace Blade
{
	class IGraphicsView;
	class IRenderTarget;

	class IGraphicsPhase
	{
	public:
		virtual ~IGraphicsPhase()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool			isEnabled() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setEnable(bool enabled) = 0;
	};
	

}//namespace Blade


#endif //__Blade_IGraphicsPhase_h__