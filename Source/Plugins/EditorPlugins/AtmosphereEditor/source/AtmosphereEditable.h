/********************************************************************
	created:	2011/09/03
	filename: 	AtmosphereEditable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AtmosphereEditable_h__
#define __Blade_AtmosphereEditable_h__
#include <interface/public/EditableBase.h>

namespace Blade
{

	class AtmosphereEditable : public EditableBase , public Allocatable
	{
	public:
		static const TString ATMOSPHERE_EDITABLE_NAME;
	public:
		AtmosphereEditable();
		~AtmosphereEditable();


		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe run time creation
		@param 
		@return 
		*/
		virtual bool			initialize(const IConfig* creationConfig,const ParamList* additionalParams = NULL);

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
		virtual bool			save()					{return true;}

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/

	protected:
		IEntity*		mAtmosphereEntity;
	};
	

}//namespace Blade



#endif // __Blade_AtmosphereEditable_h__