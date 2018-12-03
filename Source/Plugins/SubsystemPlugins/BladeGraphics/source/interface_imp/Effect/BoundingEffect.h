/********************************************************************
	created:	2012/12/30
	filename: 	BoundingEffect.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BoundingEffect_h__
#define __Blade_BoundingEffect_h__
#include <interface/public/graphics/GraphicsEffect.h>
#include <interface/IAABBRenderer.h>
#include <interface/IRenderScene.h>
#include <Element/GraphicsElement.h>

namespace Blade
{

	class BoundingEffect : public IBoundingEffect, public Allocatable
	{
	public:
		BoundingEffect()	{}
		~BoundingEffect()	{}

		/************************************************************************/
		/* IGraphicsEffect interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	onAttach()
		{
			if( mShow)
				this->setShowBoundingImpl();
			return true;
		}
		/** @brief  */
		virtual bool	onDetach()
		{
			GraphicsElement* element = static_cast<GraphicsElement*>( this->getHost() );
			element->showBoundingVolume(false);
			return true;
		}
		/** @brief  */
		virtual bool	isReady() const
		{
			return true;
		}

		/************************************************************************/
		/* IBoundingEffect interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void	setBoundingColorImpl()
		{
			GraphicsElement* element = static_cast<GraphicsElement*>( this->getHost() );
			if(element != NULL) //attach may be delayed/queued
				element->showBoundingVolume(mShow, mColor);
		}

		/** @brief  */
		virtual void	setShowBoundingImpl()
		{
			GraphicsElement* element = static_cast<GraphicsElement*>( this->getHost() );
			if (element != NULL) //attach may be delayed/queued
				element->showBoundingVolume(mShow, mColor);
		}
	};
	
}//namespace Blade

#endif//__Blade_BoundingEffect_h__