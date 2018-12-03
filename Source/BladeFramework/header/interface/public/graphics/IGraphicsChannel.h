/********************************************************************
	created:	2010/06/27
	filename: 	IGraphicsChannel.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IGraphicsChannel_h__
#define __Blade_IGraphicsChannel_h__
#include <utility/String.h>
#include <interface/public/graphics/GraphicsBaseEnums.h>

namespace Blade
{
	class IGraphicsChannel
	{
	public:
		virtual ~IGraphicsChannel()		{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&		getRenderType() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isEnabled() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setEnable(bool enabled) = 0;

		inline void					enable()	{this->setEnable(true);}
		inline void					disable()	{this->setEnable(false);}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setFillMode(FILL_MODE mode) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual FILL_MODE			getFillMode() const = 0;

	};//class IGraphicsChannel
	
}//namespace Blade


#endif //__Blade_IGraphicsChannel_h__