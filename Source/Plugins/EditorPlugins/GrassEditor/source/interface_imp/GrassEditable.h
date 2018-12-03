/********************************************************************
	created:	2018/05/27
	filename: 	GrassEditable.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GrassEditable_h__
#define __Blade_GrassEditable_h__
#include <interface/public/EditableBase.h>

namespace Blade
{
	class GrassEditable : public EditableBase
	{
	public:
		static const TString GRASS_EDITABLE_TYPE;
	public:
		GrassEditable();
		virtual ~GrassEditable();

		/************************************************************************/
		/* IEditable interface                                                                     */
		/************************************************************************/
		/*
		@describe run time creation
		@param
		@return
		*/
		virtual bool			initialize(const IConfig*, const ParamList*) { return true; }

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

	};//class GrassEditable
	
}//namespace Blade



#endif // __Blade_GrassEditable_h__