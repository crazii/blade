/********************************************************************
	created:	2013/03/09
	filename: 	IEditableManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IEditableManager_h__
#define __Blade_IEditableManager_h__
#include <interface/public/IEditable.h>

namespace Blade
{
	class IEditorFile;

	class BLADE_EDITOR_API IEditableManager
	{
	public:
		virtual ~IEditableManager()	{}
		
		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			registerEditableInfo(const ED_INFO& info) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			unregisterEditableInfo(const TString& editableName) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const ED_INFO*	getEditableInfo(const TString& editableName) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			addEditable(const HEDITABLE& pEditable) = 0;

		/**
		@describe rename the target entity's name of the editable.
		impl detail: entity names may be used as key for editable list, rename entity directly without calling this will cause data/sorting corruption.
		@param
		@return
		*/
		virtual bool			renameEditable(IEditable* editable, const TString& newName) { BLADE_UNREFERENCED(editable); BLADE_UNREFERENCED(newName);return false; }

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IEditable*		getEditable(const TString& instanceName) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeEditable(const TString& instanceName) = 0;

		inline bool				removeEditable(IEditable* pEditable)
		{
			if( pEditable != NULL )
				return this->removeEditable(pEditable->getInstanceName());
			else
				return false;
		}

		/**
		@describe check to see any editables modified
		@param 
		@return 
		*/
		virtual bool			isEditablesModified() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			saveAllEditable() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IEditorFile*	getEditorFile() const = 0;

	};//IEditableManager
	
}//namespace Blade

#endif//__Blade_IEditableManager_h__