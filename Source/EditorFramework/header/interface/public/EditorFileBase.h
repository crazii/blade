/********************************************************************
	created:	2011/05/13
	filename: 	EditorFileBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorFileBase_h__
#define __Blade_EditorFileBase_h__
#include <Pimpl.h>
#include <utility/StringList.h>
#include <interface/public/IEditorFile.h>
#include <BladeEditor.h>

namespace Blade
{
	namespace Impl
	{
		class	EditableList;
	}//namespace Impl

	class BLADE_EDITOR_API EditorFileBase : public IEditorFile
	{
	public:
		EditorFileBase(const TString& type);
		~EditorFileBase();

		/************************************************************************/
		/* IEditorFile interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void				initialize(const SEditorFileInfo* info, FileInstanceID fileID, IUIWindow* win);

		/** @brief  */
		virtual const TString&		getType() const;

		/** @brief  */
		virtual bool				isOpened() const;

		/** @brief  */
		virtual bool				isModified() const;

		/** @brief the default behavior is to check the binding window and add a mark '*' to the caption */
		virtual void				setModified();

		/** @brief  */
		virtual const ED_FD&		getEditorData() const;

		/** @brief  */
		virtual bool				setSelectedEditable(IEditable* editable);

		/** @brief  */
		virtual bool				addSelectedEditable(IEditable* editable);

		/** @brief  */
		virtual bool				removeSelectedEditable(IEditable* editable);

		/** @brief  */
		virtual IEditable*const*	getSelectedEditables(size_t& count) const;

		/** @brief  */
		virtual const IEntity*		getSelectedEntity() const	{return mSelectedEntity;}

		/** @brief  */
		virtual TString				getValidEntityName(const TString& namePrefix, const TString& suffix = TString::EMPTY);
	protected:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief direct access and no virtual calls */
		bool		isFileOpened() const	{return mOpened;}

		/** @brief  */
		bool		isFileModified() const	{return mModified;}

		/** @brief  */
		IUIWindow*	getWindow() const	{return mEditorData.mWindow;}

		/** @brief  */
		void		setOpened()				{mOpened = true;}

		/** @brief  */
		void		setView(IGraphicsView* view)	{mEditorData.mView = view;}

		/** @brief  */
		void		setGraphicsScene(IGraphicsScene* scene)	{mEditorData.mGraphicsScene = scene;}

		/** @brief  */
		void		setGeometryScene(IGeometryScene* scene) { mEditorData.mGeometryScene = scene; }

		/** @brief  */
		void		setLogicScene(ILogicScene* scene)	{mEditorData.mLogicScene = scene;}

		/** @brief  */
		void		setEditorWindow(IUIWindow* window)	{mEditorData.mWindow = window;}

		/** @brief  */
		void		setCamera(CameraActor* cam)		{mEditorData.mCamera = cam;}

		/** @brief  */
		void		setUpdater(IEditorFileUpdater* updater)	{mEditorData.mUpdater = updater;}

		/** @brief  */
		void		setEditableManager(IEditableManager* manager) {mEditorData.mEditableManager = manager;}

		/** @brief  */
		void		setStage(IStage* stage);

		/** @brief  */
		void		setWorld(ILogicWorld* world);

		/** @brief  the default behavior is to check the binding window and remove the mark '*' from the caption */
		void		setUnModified();

		/** @brief  reset editor data to NULL and init other flags*/
		void		reset();

		typedef TStringMap<RefCount> NameIndexMap;

		TString		mType;
		TStringList	mSceneTypeList;
		IEntity*	mSelectedEntity;
		
		mutable NameIndexMap	mNameIndices;
		ED_FD		mEditorData;
		bool		mOpened;
		bool		mModified;
	private:
		EditorFileBase&	operator=(const EditorFileBase&);
		Pimpl<Impl::EditableList>	mSelectedEditables;
	};//class EditorFileBase 
	

}//namespace Blade



#endif // __Blade_EditorFileBase_h__