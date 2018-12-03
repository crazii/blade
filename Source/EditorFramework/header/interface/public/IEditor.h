/********************************************************************
	created:	2010/06/13
	filename: 	IEditor.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IEditor_h__
#define __Blade_IEditor_h__
#include <interface/public/IEditorFile.h>

namespace Blade
{
	class IEditor
	{
	public:
		virtual ~IEditor()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual void			initEditor() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			shutdownEditor() = 0;

	};//class IEditor
	
}//namespace Blade


#endif //__Blade_IEditor_h__