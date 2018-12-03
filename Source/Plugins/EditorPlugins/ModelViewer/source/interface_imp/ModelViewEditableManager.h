/********************************************************************
	created:	2013/11/03
	filename: 	ModelViewEditableManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelViewEditableManager_h__
#define __Blade_ModelViewEditableManager_h__
#include <interface/public/IEditableManager.h>
#include "ModelViewerFile.h"

namespace Blade
{
	class ModelViewEditableManager : public IEditableManager, public Allocatable
	{
	protected:
		typedef Set<HEDITABLE, FnEditableLess> EditableSet;
	public:
		ModelViewEditableManager()	{mFile = NULL;}
		~ModelViewEditableManager()	{}

		/************************************************************************/
		/* IEditableManager interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			registerEditableInfo(const ED_INFO& /*info*/)	{return false;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			unregisterEditableInfo(const TString& /*editableName*/)	{return false;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const ED_INFO*	getEditableInfo(const TString& /*editableName*/) const	{return NULL;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addEditable(const HEDITABLE& pEditable)
		{
			return mEditables.insert(pEditable).second;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IEditable*		getEditable(const TString& instanceName) const
		{
			EditableSetHelper helper(instanceName);
			EditableSet::const_iterator i = mEditables.find(helper);
			if( i != mEditables.end() )
				return *i;
			else
				return NULL;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeEditable(const TString& instanceName)
		{
			EditableSetHelper helper(instanceName);
			return mEditables.erase(helper) == 1;
		}

		/*
		@describe check to see any editables modified
		@param 
		@return 
		*/
		virtual bool			isEditablesModified() const	{return false;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			saveAllEditable() const		{	/*not savable*/ }

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IEditorFile*	getEditorFile() const		{return mFile;}

	protected:
		ModelViewerFile*	mFile;
		EditableSet			mEditables;
	};
	
}//namespace Blade

#endif //  __Blade_ModelViewEditableManager_h__