/********************************************************************
	created:	2011/08/30
	filename: 	LightElement.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LightElement_h__
#define __Blade_LightElement_h__
#include <Element/GraphicsElement.h>
#include <interface/ILight.h>

namespace Blade
{
	//to resolve function name conflicts with IElement
	class LightInterfaceWrapper : public ILightInterface
	{
	public:
		/** @brief  */
		virtual const DESC& getLightDesc() const = 0;

		/** @brief  */
		virtual const ILightInterface::DESC& getDesc() const
		{
			return this->getLightDesc();
		}
	};

	class BLADE_GRAPHICS_API LightElement : public GraphicsElement, public LightInterfaceWrapper, public Allocatable
	{
	public:
		static const TString LIGHT_TYPE;
	public:
		LightElement();
		~LightElement();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data */
		virtual void	postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void			onParallelStateChange(const IParaState& data);

		/** @brief  */
		virtual Interface*		getInterface(InterfaceName type)
		{
			CHECK_RETURN_INTERFACE(ILightInterface, type, this);
			return GraphicsElement::getInterface(type);
		}

		/************************************************************************/
		/* GraphicsElement interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual bool				showBoundingVolume(bool show, const Color& color = Color::WHITE);

		/*
		@describe
		@param
		@return
		*/
		virtual void				onInitialize();

		/************************************************************************/
		/* ILightInterface (public) interface                                                                     */
		/************************************************************************/
	protected:
		/** @brief  */
		virtual bool				setDesc(const DESC& desc);

		/** @brief  */
		virtual bool				setType(ELightType type)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return this->addCommand(GraphicsElementCommand::make(type, &LightElement::setLightType));
		}

		/** @brief  */
		virtual bool				setRange(scalar range)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return this->addCommand(GraphicsElementCommand::make(range, &LightElement::setLightRange));
		}

		virtual bool		setSpotAngles(scalar inner, scalar outer)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return this->addCommand(GraphicsElementCommand::make(inner, &LightElement::setLightSpotInnerAngle))
			&& this->addCommand(GraphicsElementCommand::make(outer, &LightElement::setLightSpotOuterAngle));
		}

		/** @brief  */
		virtual bool				setAttenuation(scalar attenuation)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return this->addCommand(GraphicsElementCommand::make(attenuation, &LightElement::setLightAttenuation));
		}

		/** @brief  */
		virtual bool				setDiffuse(const Color& diff)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return this->addCommand(GraphicsElementCommand::make(diff, &LightElement::setLightDiffuse));
		}

		/** @brief  */
		virtual bool				setSpecular(const Color& spec)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return this->addCommand(GraphicsElementCommand::make(spec, &LightElement::setLightSpecular));
		}

		/** @brief  */
		virtual const ILightInterface::DESC&	getLightDesc() const;
	public:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		ILight*				getLight() const;

		/** @brief  */
		bool				setLightType(ELightType type);

		/** @brief  */
		bool				setLightRange(scalar range);

		/** @brief  */
		bool				setLightSpotInnerAngle(scalar inner);
		/** @brief  */
		bool				setLightSpotOuterAngle(scalar outer);

		/** @brief  */
		bool				setLightAttenuation(scalar attenuation);

		/** @brief  */
		bool				setLightDiffuse(const Color& diff);

		/** @brief  */
		bool				setLightSpecular(const Color& spec);

		/** @brief  */
		bool				setLightPosition(const Vector3& pos);
		/** @brief  */
		bool				setLightDirection(const Vector3& dir);

		using GraphicsElement::activateContent;
		using GraphicsElement::deactivateContent;

	protected:

		ILight*		mLight;
	private:
		/** @brief data binding */
		/** @brief  */
		const uint32&		getLightType(index_t) const						{return mLight->getType();}
		bool				setLightType(index_t, const uint32& type)		{return this->setType((ELightType)type);}

		/** @brief  */
		const Vector3&		getLightDirection(index_t) const				{return mLight->getDirection();}

		/** @brief  */
		const scalar&		getLightRange(index_t)	const					{return mLight->getRange();}
		bool				setLightRange(index_t, const scalar& range)	{return this->setRange(range);}

		/** @brief  */
		const scalar&		getLightInnerAngle(index_t) const { return mLight->getSpotInnerAngle(); }
		bool				setLightInnerAngle(index_t, const scalar& angle) { return this->setLightSpotInnerAngle(angle); }

		/** @brief  */
		const scalar&		getLightOuterAngle(index_t) const { return mLight->getSpotOuterAngle(); }
		bool				setLightOuterAngle(index_t, const scalar& angle) { return this->setLightSpotOuterAngle(angle); }

		/** @brief  */
		const scalar&		getLightAttenuation(index_t) const				{return mLight->getAttenuation();}
		bool				setLightAttenuation(index_t, const scalar& attenuation)	{return this->setLightAttenuation(attenuation);}

		/** @brief  */
		const Color&		getLightDiffuse(index_t) const { return mLight->getDiffuse(); }
		bool				setLightDiffuse(index_t, const Color& diffuse) { return this->setLightDiffuse(diffuse); }

		/** @brief  */
		const Color&		getLightSpecular(index_t) const { return mLight->getSpecular(); }
		bool				setLightSpecular(index_t, const Color& spec) { return this->setLightSpecular(spec); }

		friend class GraphicsSystemPlugin;
	};
	

}//namespace Blade



#endif // __Blade_LightElement_h__