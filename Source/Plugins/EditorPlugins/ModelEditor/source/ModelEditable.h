/********************************************************************
	created:	2017/2/27
	filename: 	ModelEditable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelEditable_h__
#define __Blade_ModelEditable_h__
#include <interface/public/EditableBase.h>
#include <interface/public/geometry/IGeometry.h>

namespace Blade
{
	class ModelEditable : public EditableBase, public Allocatable
	{
	public:
		static const TString MODEL_EDITABLE_CLASS_NAME;
	public:
		ModelEditable();
		~ModelEditable();

		/************************************************************************/
		/* IEditable interface                                                                     */
		/************************************************************************/
		/** @copydoc IEditable::getScaleConstraint  */
		virtual EAxis getScaleConstraint() const
		{
			//uniform scale only. TODO: use inverse transpose world matrix in mesh shader to support any scale
			return GA_XYZ;
		}

		/*
		@describe run time creation
		@param
		@return
		*/
		virtual bool			initialize(const IConfig* creationConfig, const ParamList* additionalParams = NULL);

		/*
		@describe serialization: loading
		@param
		@return
		*/
		virtual bool			initialize(EditorElement* element);

		/*
		@describe
		@param
		@return
		*/
		virtual bool			save();

	protected:

		/** @brief  */
		virtual const TString	getBaseName() const;
	};
	
}//namespace Blade


#endif//__Blade_ModelEditable_h__