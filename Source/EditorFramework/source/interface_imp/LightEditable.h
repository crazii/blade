/********************************************************************
	created:	2016/01/03
	filename: 	LightEditable.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_LightEditable_h__
#define __Blade_LightEditable_h__
#include <utility/Variant.h>
#include <interface/IEditorFramework.h>
#include <interface/public/EditableBase.h>
#include <interface/IStage.h>
#include <BladeBase_blang.h>
#include <databinding/DataBindingHelper.h>
#include <interface/public/graphics/GraphicsDefs.h>
#include <BladeFramework_blang.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>

namespace Blade
{

	class LightEditable : public EditableBase, public Allocatable
	{
	public:
		static const TString LIGHT_EDITABLE_TYPE;
		static const TString LIGHT_EDITABLE_NAME;
	public:
		LightEditable()
			:EditableBase(LIGHT_EDITABLE_NAME, LIGHT_EDITABLE_TYPE)
		{
			mFlag |= EDF_MOVABLE|EDF_ROTATABLE|EDF_SELECTABLE|EDF_DELETABLE|EDF_PAGED;
		}

		~LightEditable()
		{
			DataBindingHelper::removeDelegate(*this->getTargetEntity(), BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_TYPE), this);
			DataBindingHelper::removeDelegate(*this->getTargetEntity(), BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_RANGE), this);
			DataBindingHelper::removeDelegate(*this->getTargetEntity(), BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_OUTER_ANGLE), this);
		}

		/** @copydoc IEditable::getScaleConstraint  */
		virtual EAxis getScaleConstraint() const
		{
			Variant val = this->getEditorElement()->getElementConfig(BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_TYPE) );
			uint32 lightType = val;
			if( lightType == LT_SPOT )
				return GA_XY;
			else
				return GA_XYZ;
		}

		/** @copydoc IEditable::initailize  */
		virtual bool			initialize(const IConfig* /*creationConfig*/,const ParamList* /*additionalParams = NULL*/)
		{
			return false;
		}

		/** @copydoc IEditable::initailize  */
		virtual bool			initialize(EditorElement* element)
		{
			mEditorElement = element;
			mEditorElement->setEditable(this);
			mEditorElement->setHUDIcon(BTString("light.png"));
			mEditorElement->showHUD(true);

			DataBindingHelper::addElementDelegate(*this->getTargetEntity(), BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_TYPE), Delegate(this, &LightEditable::onLightTypeChange));
			DataBindingHelper::addElementDelegate(*this->getTargetEntity(), BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_RANGE), Delegate(this, &LightEditable::onLightScaleChange));
			DataBindingHelper::addElementDelegate(*this->getTargetEntity(), BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_OUTER_ANGLE), Delegate(this, &LightEditable::onLightScaleChange));
			return true;
		}

		/** @copydoc IEditable::save  */
		virtual bool			save()
		{
			return false;
		}

	protected:

		/** @brief  */
		void onLightTypeChange(void* /*type*/)
		{
			this->notifySpaceChange();
		}

		/** @brief  */
		void onLightScaleChange(void* /*range*/)
		{
			//light range/outer angle uses scale.
			//notify light scale to geometry, or they'll mismatch
			Variant vscale = DataBindingHelper::getElementData(*this->getTargetEntity(), BTString(BLANG_LIGHT_ELEMENT), BTString(BLANG_SCALE));
			this->getEditorElement()->getGeomtry()->setGlobalScale(vscale);
		}
	};
	
}//namespace Blade


#endif // __Blade_LightEditable_h__