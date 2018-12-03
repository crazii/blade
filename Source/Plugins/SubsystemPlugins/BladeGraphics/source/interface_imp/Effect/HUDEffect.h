/********************************************************************
	created:	2015/12/30
	filename: 	HUDEffect.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_HUDEffect_h__
#define __Blade_HUDEffect_h__
#include <interface/public/graphics/GraphicsEffect.h>
#include "HUDRenderable.h"
#include <Element/GraphicsElement.h>
#include <interface/ISpace.h>

namespace Blade
{

	class HUDEffect : public IHUDEffect, public Allocatable
	{
	public:
		HUDEffect()
		{
			mHUDContent = BLADE_NEW HUDRenderable();
			this->getHUD()->setVisible(mVisible);
		}
		~HUDEffect()
		{
			BLADE_DELETE mHUDContent;
		}

		/************************************************************************/
		/* IGraphicsEffect interface                                                                    */
		/************************************************************************/
		/** @brief  */
		virtual bool	onAttach()
		{
			GraphicsElement* element = static_cast<GraphicsElement*>( this->getHost() );
			this->getHUD()->setPosition(element->getPositionHint());
				
			element->getSpace()->addContent( mHUDContent );
			mHUDContent->setElement(element);
			this->getHUD()->setVisible(mVisible);
			return true;
		}
		/** @brief  */
		virtual bool	onDetach()
		{
			GraphicsElement* element = static_cast<GraphicsElement*>( this->getHost() );
			if(mHUDContent->getSpace() != NULL)
				element->getSpace()->removeContent( mHUDContent );
			mHUDContent->setElement(NULL);
			this->getHUD()->setVisible(false);
			return true;
		}

		/** @brief  */
		virtual bool	isReady() const
		{
			return mHUDContent != NULL && this->getHUD()->getMaterial()->isLoaded();
		}

		/** @brief  */
		virtual bool	onPositionChange(IGraphicsInterface* itf)
		{
			GraphicsElement* elem = static_cast<GraphicsElement*>(itf);
			if (elem->getContent() != NULL)
			{
				GraphicsElement* element = static_cast<GraphicsElement*>(this->getHost());
				if (element != NULL)
					this->getHUD()->setPosition(element->getPositionHint());
			}
			return true;
		}

		/** @brief  */
		virtual bool	onLocalBoundsChange(IGraphicsInterface* itf) { return this->onPositionChange(itf); }
		virtual bool	onRotationChange(IGraphicsInterface* itf) { return this->onPositionChange(itf); }
		virtual bool	onScaleChange(IGraphicsInterface* itf) { return this->onPositionChange(itf); }

		/************************************************************************/
		/* IHUDEffect interface                                                                    */
		/************************************************************************/
		/** @brief  */
		virtual void	showImpl(bool visible)
		{
			this->getHUD()->setVisible(visible);
			if( visible )
			{
				GraphicsElement* element = static_cast<GraphicsElement*>(this->getHost());
				if (element != NULL )
					this->getHUD()->setPosition(element->getPositionHint());
			}
		}

		/** @brief  */
		virtual void	setIconImpl(index_t index, const TString& texture)
		{
			this->getHUD()->setTexture(index, texture);
		}

		/** @brief  */
		virtual void	setPixelSize(size_t width, size_t height)
		{
			this->getHUD()->setPixelSize(width, height);
		}

		/** @brief  */
		virtual void	setHighLightImpl(bool highlit)
		{
			this->getHUD()->setHighLight(highlit);
		}

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/** @brief  */
		inline HUDRenderable* getHUD() const
		{
			return static_cast<HUDRenderable*>(mHUDContent);
		}

	protected:
		SpaceContent*	mHUDContent;
	};

	
}//namespace Blade

#endif // __Blade_HUDEffect_h__
