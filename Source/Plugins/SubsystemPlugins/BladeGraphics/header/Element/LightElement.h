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

	class BLADE_GRAPHICS_API LightElement : public GraphicsElement , public Allocatable
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
		virtual void				onParallelStateChange(const IParaState& data);

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
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		ILight*				getLight() const;

		/** @brief  */
		bool				setLightType(ELightType type);

		/** @brief  */
		bool				setLightPosition(const Vector3& pos);

		/** @brief  */
		bool				setLightDirection(const Vector3& dir);

		/** @brief  */
		bool				setLightRange(scalar range);

		/** @brief  */
		bool				setSpotLightInnerAngle(scalar inner);

		/** @brief  */
		bool				setSpotLightOuterAngle(scalar outer);

		/** @brief  */
		bool				setLightAttenuation(const scalar& attenuation);

		/** @brief  */
		bool				setLightDiffuse(const Color& diff);

		/** @brief  */
		bool				setLightSpecular(const Color& spec);


		using GraphicsElement::activateContent;
		using GraphicsElement::deactivateContent;

	protected:
		ILight*		mLight;

	private:
		/** @brief data binding */
		/** @brief  */
		const uint32&		getLightType(index_t) const						{return mLight->getType();}
		bool				setLightType(index_t, const uint32& type)		{return this->setLightType((ELightType)type);}

		/** @brief  */
		const Vector3&		getLightDirection(index_t) const				{return mLight->getDirection();}

		/** @brief  */
		const scalar&		getLightRange(index_t)	const					{return mLight->getRange();}
		bool				setLightRange(index_t, const scalar& range)	{return this->setLightRange(range);}

		/** @brief  */
		const scalar&		getLightInnerAngle(index_t) const { return mLight->getSpotInnerAngle(); }
		bool				setLightInnerAngle(index_t, const scalar& angle) { return this->setSpotLightInnerAngle(angle); }

		/** @brief  */
		const scalar&		getLightOuterAngle(index_t) const { return mLight->getSpotOuterAngle(); }
		bool				setLightOuterAngle(index_t, const scalar& angle) { return this->setSpotLightOuterAngle(angle); }

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