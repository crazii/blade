/********************************************************************
	created:	2012/03/25
	filename: 	IAABBRenderer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IAABBRenderer_h__
#define __Blade_IAABBRenderer_h__
#include <BladeGraphics.h>
#include <interface/public/graphics/Color.h>
#include <interface/ISpaceContent.h>

namespace Blade
{

	class IAABBTarget
	{
	public:
		virtual ~IAABBTarget() {}

		/** @brief  */
		virtual size_t		getAABBCount() const = 0;
		/** @brief  */
		virtual const AABB& getAABB(index_t index) const = 0;
		/** @brief  */
		virtual Color		getAABBColor(index_t index) const = 0;
	};

	class BLADE_GRAPHICS_API IAABBRenderer
	{
	public:
		virtual ~IAABBRenderer()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		addAABB(ISpaceContent* content, const Color& color) = 0;

		/**
		@describe 
		@param
		@return
		*/

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		removeAABB(ISpaceContent* content) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		changeAABBColor(ISpaceContent* content, const Color& color) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		addAABB(IAABBTarget* target) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		removeAABB(IAABBTarget* target) = 0;
	};
	

}//namespace Blade


#endif //__Blade_IAABBRenderer_h__