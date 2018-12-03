/********************************************************************
	created:	2015/11/23
	filename: 	IGraphicsInterface.h
	author:		Crazii
	
	purpose:	IGraphicsInterface contained in graphics elements
*********************************************************************/
#ifndef __Blade_IGraphicsInterface_h__
#define __Blade_IGraphicsInterface_h__
#include <BladeFramework.h>
#include <interface/Interface.h>
#include <interface/public/graphics/IGraphicsEffect.h>

namespace Blade
{
	class IGraphicsScene;

	/*
	Async behavior/spec of IGraphicsInterface:
	IGraphicsInterface can ONLY be accessed at TS_MAIN_SYNC or TS_ASYNC_RUN state. @see ETaskParallelState
	*/
	class IGraphicsInterface : public Interface
	{
	public:
		/** @brief add an effect */
		virtual bool	addEffect(const HGRAPHICSEFFECT& effect) = 0;

		/** @brief  */
		virtual bool	removeEffect(const HGRAPHICSEFFECT& effect) = 0;

		/** @brief  */
		virtual const HGRAPHICSEFFECT& getEffect(const TString& type) const = 0;

		/** @brief  */
		virtual IGraphicsScene* getGraphicsScene() const = 0;

		/** @brief  */
		virtual void setVisible(bool visible) = 0;
		/** @brief  */
		virtual void enablePicking(bool enable) = 0;

		/** @brief  */
		virtual const Vector3&	getStaticPosition() const = 0;
		/** @brief  */
		virtual const Quaternion&	getStaticRotation() const = 0;
		/** @brief  */
		virtual const Vector3&	getStaticScale() const = 0;
		/** @brief  */
		virtual const AABB& getStaticLocalBounds() const = 0;

	protected:

		/** @brief  */
		inline bool attachEffect(IGraphicsEffect* effect)
		{
			return effect->attach(this);
		}

		/** @brief  */
		inline bool	detachEffect(IGraphicsEffect* effect)
		{
			return effect->detach(this);
		}

		/** @brief  */
		inline bool	dispatchPositionChange(IGraphicsEffect* effect)
		{
			return effect->onPositionChange(this);
		}

		/** @brief  */
		inline bool	dispatchRotationChange(IGraphicsEffect* effect)
		{
			return effect->onRotationChange(this);
		}

		/** @brief  */
		inline bool	dispatchScaleChange(IGraphicsEffect* effect)
		{
			return effect->onScaleChange(this);
		}

		/** @brief  */
		inline bool	dispatchLocalBoundsChange(IGraphicsEffect* effect)
		{
			return effect->onLocalBoundsChange(this);
		}
	};

	typedef InterfaceID<IGraphicsInterface> IID_GRAPHICS;
	
}//namespace Blade

#endif // __Blade_IGraphicsInterface_h__
