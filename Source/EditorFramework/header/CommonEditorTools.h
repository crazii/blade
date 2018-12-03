/********************************************************************
	created:	2011/05/17
	filename: 	CommonEditorTools.h
	author:		Crazii
	purpose:	common editor tools and are built-in to framework.
*********************************************************************/
#ifndef __Blade_CommonEditorTools_h__
#define __Blade_CommonEditorTools_h__
#include <math/Ray.h>
#include <math/Plane.h>
#include <interface/public/EditorToolTypes.h>
#include <interface/public/geometry/GeomTypes.h>
#include <BladeEditor_blang.h>

namespace Blade
{
	class BLADE_EDITOR_API SelectTool : public EditorCommonTool, public Allocatable
	{
	public:
		SelectTool(IconIndex icon,HOTKEY hotkey = KC_UNDEFINED,const TString name = BXLang(BLANG_TOOL_SELECT) );
		~SelectTool();

		/************************************************************************/
		/* IEditorTool interface                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftClick(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftUp(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return 
		*/
		virtual bool				update(scalar x, scalar y, const POINT3& deltaMove, uint32 keyModifier,scalar deltaTime,IEditorFile* file);

		/*
		@describe 
		@param 
		@return 
		@note update in the main synchronous time
		*/
		virtual bool				mainUpdate(scalar x,scalar y,uint32 keyModifier,scalar deltaTime,IEditorFile* file);

		/*
		@describe use defined category,\n
		tools with the same category will be placed in the same toolbox
		@param 
		@return 
		*/
		virtual const TString&		getMannaulCategory() const;

		/*
		@describe tool name
		@param 
		@return 
		*/
		virtual const TString&		getName() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IconIndex			getToolIcon() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HOTKEY				getHotkey() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&		getToolConfig() const;

		/*
		@describe get the types of file can be applied by this tool
		@param 
		@return -1 if support all files
		@see SEditorFileInfoEx::mFileClassName
		*/
		virtual size_t				getSupportedFiles(TStringParam& outFiles);

		/************************************************************************/
		/* EditorCommonTool overrides                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				onSelection();

		/*
		@describe file switch notification, this usually happens when open/create a file,\n
		or user click the sub window tabs of the editor to switch to another sub window
		@param 
		@return 
		@note DO NOT keep any reference of the params 'prevFile' nor 'currentFile'
		you should delete any related data on prevFile when switch happens,
		because currently, there's no 'file close' notification yet.
		*/
		virtual void				onFileSwitch(const IEditorFile* /*prevFile*/,const IEditorFile* /*currentFile*/)
		{
			mHighLight = NULL;
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual void	onEditableRemoved(IEditable* editable)
		{
			if (editable == mHighLight)
				mHighLight = NULL;
		}

	protected:

		/** @brief  */
		size_t	markAllSelected(bool showMark);

		IEditable*	mHighLight;
		bool		mEnableHighLight;
	};//class SelectTool

	namespace Impl
	{
		class EditableGizmoSetImpl;
	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	class BLADE_EDITOR_API TransformTool : public SelectTool
	{
	public:
		TransformTool(IconIndex icon,HOTKEY hotkey, const TString& name, EGizmoType type);

		virtual ~TransformTool() {}

		/************************************************************************/
		/* EditorCommonTool overrides                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				onSelection();

		/*
		@describe file switch notification, this usually happens when open/create a file,\n
		or user click the sub window tabs of the editor to switch to another sub window
		@param 
		@return 
		@note DO NOT keep any reference of the params 'prevFile' nor 'currentFile'
		you should delete any related data on prevFile when switch happens,
		because currently, there's no 'file close' notification yet.
		*/
		virtual void				onFileSwitch(const IEditorFile* prevFile,const IEditorFile* currentFile);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftClick(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe 
		@param [in] x, y : normalized mouse position
		@return whether tool needs update
		if return true, IEditorTool::update will be called on each loop, until onMouseLeftUp() event
		@note this function is called though main synchronous state
		*/
		virtual bool				onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe 
		@param 
		@return reserved
		@note this function is called though main synchronous state
		*/
		virtual bool				onMouseLeftUp(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe 
		@param 
		@return 
		@note update in the main synchronous time
		*/
		virtual bool				mainUpdate(scalar x,scalar y,uint32 keyModifier,scalar deltaTime,IEditorFile* file);

		/**
		@describe
		@param
		@return
		*/
		virtual void	onEditableRemoved(IEditable* editable)
		{
			if (editable == mUpdateTarget)
				this->clearTarget();

			SelectTool::onEditableRemoved(editable);
		}

	protected:
		//add one editable into selection target
		bool	addEditableGizmo(IEditable* editable);

		/** @brief  */
		bool	removeEditableGizmo(IEditable* editable);

		/** @brief  */
		void	clearTarget();

		/** @brief  */
		static Vector3				getAxis(EAxis axis, const Quaternion& rotation);

		/** @brief  */
		const Quaternion&			getAxisRotation() const;

		/** @brief  */
		Plane						getGizmoPlane(const Ray& ray, IGraphicsCamera* camera);

		/** @brief  */
		void						onTransformSpaceChange(const Event& data);

	private:
		//set to store all gizmo helpers
		Pimpl<Impl::EditableGizmoSetImpl>	mGizmoSet;
	protected:
		//updating states
		IEditable*	mUpdateTarget;
		EAxis		mUpdateAxis;

		GEOM_SPACE	mTranslationSpace;
		EGizmoType	mType;
		bool		mUpdating;
		bool		mCustomSpace;		//whether to use mTranslationSpace
	};


	class BLADE_EDITOR_API TranslateTool : public TransformTool
	{
	public:
		TranslateTool(IconIndex icon,HOTKEY hotkey = KC_UNDEFINED);
		~TranslateTool();	


		/*
		@describe 
		@param [in] x, y : normalized mouse position
		@return whether tool needs update
		if return true, IEditorTool::update will be called on each loop, until onMouseLeftUp() event
		@note this function is called though main synchronous state
		*/
		virtual bool				onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return reserved
		@note update in the task 'run' step
		*/
		virtual bool				update(scalar x, scalar y, const POINT3& deltaMove, uint32 keyModifier,scalar deltaTime,IEditorFile* file);

	protected:
		//keep track of mouse ray projected pos to axis gizmo plane
		Vector3		mUpdatePos;
	};//class TranslateTool

	class RotateTool : public TransformTool
	{
	public:

		RotateTool(IconIndex icon,HOTKEY hotkey = KC_UNDEFINED);
		~RotateTool();

		/*
		@describe 
		@param [in] x, y : normalized mouse position
		@return whether tool needs update
		if return true, IEditorTool::update will be called on each loop, until onMouseLeftUp() event
		@note this function is called though main synchronous state
		*/
		virtual bool				onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return reserved
		@note update in the task 'run' step
		*/
		virtual bool				update(scalar x, scalar y, const POINT3& deltaMove, uint32 keyModifier,scalar deltaTime,IEditorFile* file);
	protected:
		Vector3		mProjectedTangent;
	};//class RotateTool 


	class ScaleTool : public TransformTool
	{
	public:
		ScaleTool(IconIndex icon,HOTKEY hotkey = KC_UNDEFINED);
		~ScaleTool();

		/*
		@describe 
		@param [in] x, y : normalized mouse position
		@return whether tool needs update
		if return true, IEditorTool::update will be called on each loop, until onMouseLeftUp() event
		@note this function is called though main synchronous state
		*/
		virtual bool				onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return reserved
		@note update in the task 'run' step
		*/
		virtual bool				update(scalar x, scalar y, const POINT3& deltaMove, uint32 keyModifier,scalar deltaTime,IEditorFile* file);

	protected:
		scalar		mUpdateLength;
	};//class ScaleTool 
	

	class BLADE_EDITOR_API LightCreationTool : public EditorCommonTool, public Allocatable
	{
	public:
		LightCreationTool(IconIndex icon, HOTKEY hotKey = HOTKEY());

		~LightCreationTool();

		/**
		@describe 
		@param
		@return
		*/
		virtual bool				onMouseLeftClick(scalar /*x*/,scalar /*y*/,uint32 /*keyModifier*/,IEditorFile* /*file*/)
		{
			return false;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftUp(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);
		
		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return 
		*/
		virtual bool				update(scalar /*x*/, scalar /*y*/, const POINT3& /*deltaMove*/, uint32 /*keyModifier*/, scalar /*deltaTime*/, IEditorFile* /*file*/);

		/*
		@describe 
		@param 
		@return 
		@note update in the main synchronous time
		*/
		virtual bool				mainUpdate(scalar x, scalar y, uint32 keyModifier,scalar deltaTime,IEditorFile* file);

		/**
		@describe 
		@param
		@return
		*/
		virtual void				onFileSwitch(const IEditorFile* /*prevFile*/,const IEditorFile* /*currentFile*/)
		{
			mCreating = false;
			mLightEditable = NULL;
		}

		/*
		@describe use defined category,\n
		tools with the same category will be placed in the same toolbox
		@param 
		@return 
		*/
		virtual const TString&		getMannaulCategory() const;

		/*
		@describe get the types of file can be applied by this tool
		@param 
		@return -1 if support all files
		@see SEditorFileInfoEx::mFileClassName
		*/
		virtual size_t				getSupportedFiles(TStringParam& outFiles);

		/**
		@describe
		@param
		@return
		*/
		virtual void	onEditableRemoved(IEditable* editable)
		{
			if (editable == mLightEditable)
			{
				mCreating = false;
				mLightEditable = NULL;
			}
			EditorCommonTool::onEditableRemoved(editable);
		}

	protected:
		scalar	mCreationDistance;
		IEditable* mLightEditable;
		bool	mCreating;
	};

}//namespace Blade



#endif // __Blade_CommonEditorTools_h__