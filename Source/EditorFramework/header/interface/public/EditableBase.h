/********************************************************************
	created:	2011/05/23
	filename: 	EditableBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditableBase_h__
#define __Blade_EditableBase_h__
#include <interface/public/IEditable.h>

namespace Blade
{
	class IPage;

	class BLADE_VCLASS_EXPORT EditableBase : public IEditable
	{
	public:
		EditableBase(const TString& editableName, const TString& editableClassName)
			:mEditableName(editableName)
			,mEditableClassName(editableClassName)
			,mEditorElement(NULL)
			,mFlag(EDF_NONE)
			,mSelected(false)
			,mModified(false)
		{

		}

		virtual ~EditableBase()	{}

		/************************************************************************/
		/* IEditable interface                                                                      */
		/************************************************************************/
		/**
		@describe get editable name
		@note the name must be the same as mEditableName in struct SEditableInfo
		@param 
		@return 
		*/
		virtual const TString&	getName() const
		{
			return mEditableName;
		}

		/**
		@describe editable factory class
		@note the name must be the same as mClassName in struct SEditableInfo
		@param 
		@return 
		*/
		virtual const TString&	getEditableClass() const
		{
			return mEditableClassName;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getInstanceName() const
		{
			return this->getTargetEntity()->getName();
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual int				getEditableFlag() const
		{
			return mFlag;
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual EAxis				getScaleConstraint() const
		{
			return GA_NONE;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual EditorElement*	getEditorElement() const
		{
			return mEditorElement;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			isSelected() const
		{
			return mSelected;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			setSelected(bool selected)
		{
			if( this->isSelectable() )
			{
				mSelected = selected;
				return true;
			}
			else
				return false;
		}

		/**
		@describe
		@param
		@return
		*/
		BLADE_EDITOR_API virtual bool			onTransformed(IEditorFile* file);

		/**
		@describe whether this object need save
		@param 
		@return 
		*/
		virtual bool			isModified() const
		{
			return mModified;
		}

		/** @brief  */
		BLADE_EDITOR_API static TString generatePageSuffix(IPage* page);

	protected:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		BLADE_EDITOR_API bool			createEditorElement(const TString& type, IEntity* entity = NULL, const TString& name = TString::EMPTY);

		/**
		@describe 
		@param
		@return
		*/
		BLADE_EDITOR_API bool			notifySpaceChange(IEditorFile* file = NULL);

		/** @brief  */
		inline void		setModified(bool modified)
		{
			mModified = modified;
		}

		/** @brief  */
		inline void		setEditorElement(EditorElement* element)
		{
			assert( mEditorElement == NULL && element != NULL );
			mEditorElement = element;
		}

		/** @brief basic name used to auto generate entity name */
		virtual const TString	getBaseName() const { return mEditableName; }

	protected:
		TString			mEditableName;
		TString			mEditableClassName;
		EditorElement*	mEditorElement;
		int				mFlag; /* EEditableFlag */
		bool			mSelected;
		bool			mModified;		//whether need save
	};//class EditableBase 
	

}//namespace Blade



#endif // __Blade_EditableBase_h__