/********************************************************************
	created:	2011/04/13
	filename: 	IEditable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IEditable_h__
#define __Blade_IEditable_h__
#include <utility/Delegate.h>
#include <BladeEditor.h>
#include <interface/IConfig.h>
#include <interface/IEntity.h>
#include <interface/public/graphics/IGraphicsType.h>
#include <interface/EditorElement.h>

namespace Blade
{
	class IEditorFile;

	typedef struct SEditableInfo
	{
		TString			mEditableName;		///key: registration name
		TString			mClassName;			///for factory creation

		IConfig*		mConfigOnCreation;	///config for creating new instance, could be NULL
		IConfig*		mGlobalConfig;		///config that could be changed during runtime, this is not per instance
		IGraphicsType*	mType;				///type identifier, could be NULL
		
		size_t			mInstanceLimit;		///limit of instances creation
		AppFlag			mAppFlag;			///not used for registration, set later by editor
		mutable Delegate mGlobalConfigNotify;
		bool			mModalConfig;
		bool			mNeedAppFlag;
		bool			mNeedMenuCreation;

	}ED_INFO;

	class BLADE_EDITOR_API IEditable
	{
	public:
		typedef enum EEditableFlag
		{
			EDF_NONE		= 0x00000000,
			EDF_MOVABLE		= 0x00000001,
			EDF_SCALABLE	= 0x00000002,
			EDF_ROTATABLE	= 0x00000004,
			EDF_SELECTABLE	= 0x00000008,
			EDF_DELETABLE	= 0x00000010,
			EDF_PAGED		= 0x00000020,

			EDF_TRASFORM_MASK = EDF_MOVABLE | EDF_SCALABLE | EDF_ROTATABLE,
			EDF_EDF_TRASFORMABLE = EDF_TRASFORM_MASK,
		}EDF;

		virtual ~IEditable()	{}

		/************************************************************************/
		/* IEditable interface                                                                     */
		/************************************************************************/
		/**
		@describe get editable name
		@note the name must be the same as mEditableName in struct SEditableInfo
		@param 
		@return 
		*/
		virtual const TString&	getName() const = 0;

		/**
		@describe editable factory class
		@note the name must be the same as mClassName in struct SEditableInfo
		@param 
		@return 
		*/
		virtual const TString&	getEditableClass() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getInstanceName() const = 0;

		/**
		@describe EEditableFlag
		@param 
		@return 
		*/
		virtual int	getEditableFlag() const = 0;

		inline bool	isTransformable() const {return (this->getEditableFlag()&EDF_TRASFORM_MASK) != 0; }

		inline bool	isMovable() const	{return (this->getEditableFlag()&EDF_MOVABLE) != 0;}

		inline bool isScalable() const	{return (this->getEditableFlag()&EDF_SCALABLE) != 0;}

		inline bool isRotatable() const	{return (this->getEditableFlag()&EDF_ROTATABLE) != 0;}

		inline bool isSelectable() const	{return (this->getEditableFlag()&EDF_SELECTABLE) != 0;}

		/**
		@describe 
		@param
		@return
		*/
		virtual EAxis getScaleConstraint() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual EditorElement*	getEditorElement() const = 0;

		inline IEntity*			getTargetEntity() const	{ return this->getEditorElement()->getEntity(); }

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			isSelected() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			setSelected(bool selected) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			onTransformed(IEditorFile* file) = 0;

		/**
		@describe whether this object need save
		@param 
		@return 
		*/
		virtual bool			isModified() const = 0;

		/**
		@describe run time creation
		@param 
		@return 
		*/
		virtual bool			initialize(const IConfig* creationConfig,const ParamList* additionalParams = NULL) = 0;

		/**
		@describe serialization: loading
		@param 
		@return 
		*/
		virtual bool			initialize(EditorElement* element) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			save() = 0;

	};//class IEditable

	extern template class BLADE_EDITOR_API Factory<IEditable>;

	typedef Handle<IEditable>	HEDITABLE;

	struct FnEditableLess
	{
		bool	operator()(const HEDITABLE& left, const HEDITABLE& right) const
		{
			return left->getInstanceName() < right->getInstanceName();
		}
	};//class FnEditableLess

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//helper for find editable in set container
	class EditableSetHelper : public IEditable, public HEDITABLE
	{
		const TString& mNameRef;
	public:

		EditableSetHelper(const TString& name) :mNameRef(name)	{ mPtr = this; }
		~EditableSetHelper()									{ mPtr = NULL; }

		virtual const TString&	getName() const				{return TString::EMPTY;}
		virtual const TString&	getEditableClass() const	{return TString::EMPTY;}
		virtual const TString&	getEditableClass()			{return TString::EMPTY;}
		virtual const TString&	getInstanceName() const		{return mNameRef;}
		virtual int				getEditableFlag() const		{return EDF_NONE;}
		virtual EAxis			getScaleConstraint() const	{return GA_NONE;}
		virtual EditorElement*	getEditorElement() const	{return NULL;}
		virtual bool			setSelected(bool)			{return false;}
		virtual bool			onTransformed(IEditorFile* /*file*/) {return false;}
		virtual bool			isSelected() const			{return false;}
		virtual bool			isModified() const			{return false;}


		virtual bool			initialize(const IConfig*,const ParamList*)	{return false;}
		virtual bool			initialize(EditorElement* /*element*/)			{return false;}
		virtual bool			save()						{return false;}
	private:
		EditableSetHelper&	operator=(const EditableSetHelper&);
	};//EditableSetHelper
	
}//namespace Blade



#endif // __Blade_IEditable_h__