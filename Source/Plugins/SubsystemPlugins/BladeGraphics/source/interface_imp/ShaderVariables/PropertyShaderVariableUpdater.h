/********************************************************************
	created:	2011/06/01
	filename: 	PassShaderVariableUpdater.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_PassShaderVariableUpdater_h__
#define __Blade_PassShaderVariableUpdater_h__

#include "ShaderVariableUpdaters.h"

namespace Blade
{
	class PropertyDiffuseUpdater : public PropertyVariableUpdater
	{
	public:
		PropertyDiffuseUpdater()
			:PropertyVariableUpdater(SCT_FLOAT4,1)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const RenderPropertySet* propertySet = this->getPropertySet();
			IRenderProperty* prop;
			if( propertySet == NULL || ( prop = propertySet->getProperty(RPT_COLOR) ) == NULL)
				return &Color::WHITE;
			else
			{
				ColorProperty* colorProp = static_cast<ColorProperty*>(prop);
				return &colorProp->getDiffuse();
			}
		}
	protected:
	};//class PropertyDiffuseUpdater

	//////////////////////////////////////////////////////////////////////////
	class PropertySpecularUpdater : public PropertyVariableUpdater
	{
	public:
		PropertySpecularUpdater()
			:PropertyVariableUpdater(SCT_FLOAT4,1)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const RenderPropertySet* propertySet = this->getPropertySet();
			IRenderProperty* prop;
			if( propertySet == NULL || ( prop = propertySet->getProperty(RPT_COLOR) ) == NULL)
				return &Color::WHITE;
			else
			{
				ColorProperty* colorProp = static_cast<ColorProperty*>(prop);
				return &colorProp->getSpecular();
			}
		}
	};////class PropertySpecularUpdater

	//////////////////////////////////////////////////////////////////////////
	class PropertyEmissiveUpdater : public PropertyVariableUpdater
	{
	public:
		PropertyEmissiveUpdater()
			:PropertyVariableUpdater(SCT_FLOAT4,1)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const RenderPropertySet* propertySet = this->getPropertySet();
			IRenderProperty* prop;
			if( propertySet == NULL || ( prop = propertySet->getProperty(RPT_COLOR) ) == NULL)
				return &Color::WHITE;
			else
			{
				ColorProperty* colorProp = static_cast<ColorProperty*>(prop);
				return &colorProp->getEmissive();
			}
		}

	};////class PropertyEmissiveUpdater

	//////////////////////////////////////////////////////////////////////////
	class PropertyAlphaUpdater : public PropertyVariableUpdater
	{
	public:
		PropertyAlphaUpdater()
			:PropertyVariableUpdater(SCT_FLOAT4, 1)
		{

		}

		/** @brief  */
		virtual const void*	updateData() const
		{
			const RenderPropertySet* propertySet = this->getPropertySet();
			IRenderProperty* prop;
			if( propertySet == NULL || ( prop = propertySet->getProperty(RPT_ALPHABLEND) ) == NULL)
				return &Vector4::UNIT_ALL;
			else
			{
				AlphaBlendProperty* alphaProp = static_cast<AlphaBlendProperty*>(prop);
				mAlphaFactor[0] = alphaProp->getBlendFactor();
				return &mAlphaFactor;
			}
		}

		mutable Vector4 mAlphaFactor;
	};////class PropertyEmissiveUpdater

}//namespace Blade



#endif // __Blade_PassShaderVariableUpdater_h__