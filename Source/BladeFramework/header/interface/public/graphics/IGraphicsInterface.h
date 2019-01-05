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
#include <interface/public/graphics/Color.h>
#include <math/Vector4.h>
#include <interface/public/graphics/IGraphicsEffect.h>
#include <interface/public/graphics/GraphicsDefs.h>

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
	};//class IGraphicsInterface

	typedef InterfaceID<IGraphicsInterface> IID_GRAPHICS;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//ILightInterface can ONLY be accessed at TS_MAIN_SYNC/TS_ASYNC_RUN state (for editor).
	//in real time lights should be loaded from stages
	class ILightInterface : public Interface
	{
	public:
		typedef struct LightDescriptor
		{
			//shader parameters
			Color		mDiffuse;
			Color		mSpecular;
			//-shader parameters

			scalar		mRange;
			scalar		mSpotInnerAngle;	//in degrees
			scalar		mSpotOutAngle;		//in degrees
			scalar		mAttenuation;		//linear attenuations
			uint32		mType;
		}DESC;
	public:
		/** @brief  */
		virtual bool	setDesc(const ILightInterface::DESC& desc) = 0;

		/** @brief  */
		virtual bool	setType(ELightType lt) = 0;

		/** @brief  */
		virtual bool	setRange(scalar range) = 0;

		/** @brief  */
		virtual bool	setSpotAngles(scalar inner, scalar outer) = 0;

		/** @brief  */
		virtual bool	setAttenuation(scalar attenuation) = 0;

		/** @brief  */
		virtual bool	setDiffuse(const Color& diff) = 0;

		/** @brief  */
		virtual bool	setSpecular(const Color& spec) = 0;

		/** @brief  */
		virtual const ILightInterface::DESC& getDesc() const = 0;

		/** @brief  */
		inline ELightType getType() const
		{
			return (ELightType)this->getDesc().mType;
		}

		/** @brief  */
		inline const scalar& getRange() const
		{
			return this->getDesc().mRange;
		}

		/** @brief  */
		inline const scalar&	getSpotInnerAngle() const
		{
			return this->getDesc().mSpotInnerAngle;
		}

		/** @brief  */
		inline const scalar&	getSpotOuterAngle() const
		{
			return this->getDesc().mSpotOutAngle;
		}

		/** @brief  */
		inline const scalar&	getAttenuation() const
		{
			return this->getDesc().mAttenuation;
		}

		/** @brief  */
		inline const Color&	getDiffuse() const
		{
			return this->getDesc().mDiffuse;
		}

		/** @brief  */
		inline const Color&	getSpecular() const
		{
			return this->getDesc().mSpecular;
		}

	};//class ILightInterface

	typedef InterfaceID<ILightInterface> IID_LIGHT;
	
}//namespace Blade

#endif // __Blade_IGraphicsInterface_h__
