/********************************************************************
	created:	2010/05/24
	filename: 	IEditorFile.h
	author:		Crazii
	purpose:	editor file information
*********************************************************************/
#ifndef __Blade_IEditorFile_h__
#define __Blade_IEditorFile_h__
#include <BladeEditor.h>
#include <interface/public/window/IWindow.h>

#include <interface/public/graphics/IGraphicsView.h>
#include <interface/IStage.h>
#include <interface/public/graphics/IGraphicsScene.h>
#include <interface/public/ui/IUIWindow.h>

#include <interface/public/IEditable.h>
#include <interface/public/IEditableManager.h>
#include <interface/EditorFileInfo.h>

#include <interface/public/geometry/IGeometryScene.h>

//generally an editor should be bind to a certain logic system, to make it \n
//handle the specific transaction
//currently the editor framework depend on an logic system
//if there is no logic system plugged in, the editor framework may not work properly
//TODO: better ways
#include <interface/public/logic/ILogicScene.h>
#include <interface/public/logic/CameraActor.h>

namespace Blade
{
	class IEditorFile;
	class IUIWindow;

	///update corresponding data synchronously at run time, for each file
	///it's more like a controller of a file
	class BLADE_EDITOR_API IEditorFileUpdater : public IUIWindow::IUpdater
	{
	public:
		/** @brief  */
		virtual void onActivate(IEditorFile* file, bool active) = 0;
		/** @brief  */
		virtual void update(IEditorFile* file) = 0;
	};

	typedef struct SEditorFileData
	{
		FileInstanceID		mFileID;
		const SEditorFileInfo* mTypeInfo;
		//all data members can be NULL
		IGraphicsView*		mView;			///file's binding view, could be NULL
		CameraActor*		mCamera;
		IStage*				mStage;			///file's binding stage,could be NULL
		ILogicWorld*		mWorld;			///file's own real transaction target
		IGraphicsScene*		mGraphicsScene;
		IGeometryScene*		mGeometryScene;
		ILogicScene*		mLogicScene;
		IUIWindow*			mWindow;
		IScene*				mEditorScene;
		IEditorFileUpdater*	mUpdater;
		IEditableManager*	mEditableManager;
	}ED_FD;


	class BLADE_EDITOR_API IEditorFile
	{
	public:
		virtual ~IEditorFile()	{}

		/** @brief  */
		virtual const TString&		getType() const = 0;

		/** @brief  */
		virtual bool				isOpened() const = 0;

		/** @brief  */
		virtual bool				isModified() const = 0;

		/** @brief  */
		virtual void				setModified() = 0;

		/** @brief  */
		virtual const ED_FD&		getEditorData() const = 0;
		const SEditorFileInfo*		getTypeInfo() const
		{
			return this->getEditorData().mTypeInfo;
		}
		inline FileTypeID			getTypeID() const
		{
			return this->getTypeInfo()->mTypeID;
		}
		inline FileInstanceID		getFileID() const
		{
			return this->getEditorData().mFileID;
		}
		inline IGraphicsView*		getView() const
		{
			return this->getEditorData().mView;
		}
		inline IStage*				getStage() const
		{
			return this->getEditorData().mStage;
		}
		inline ILogicWorld*			getWorld() const
		{
			return this->getEditorData().mWorld;
		}

		inline IGraphicsScene*		getGraphicsScene() const
		{
			return this->getEditorData().mGraphicsScene;
		}

		inline IGeometryScene*		getGeometryScene() const
		{
			return this->getEditorData().mGeometryScene;
		}

		inline ILogicScene*			getLogicScene() const
		{
			return this->getEditorData().mLogicScene;
		}

		inline IUIWindow*		getEditorWindow() const
		{
			return this->getEditorData().mWindow;
		}

		inline CameraActor*	getDefaultCamera() const
		{
			return this->getEditorData().mCamera;
		}

		inline IScene*	getEditorScene() const
		{
			return this->getEditorData().mEditorScene;
		}

		inline IEditorFileUpdater*	getFileUpdater() const
		{
			return this->getEditorData().mUpdater;
		}

		inline IEditableManager*	getEditableManager() const
		{
			return this->getEditorData().mEditableManager;
		}

		/** @brief  */
		virtual void				initialize(const SEditorFileInfo* info, FileInstanceID fileID, IUIWindow* win) = 0;

		/** @brief  */
		virtual void				openFile(const TString& dir, const TString& name) = 0;

		/** @brief  */
		virtual void				saveFile() = 0;

		/** @brief  */
		virtual void				newFile(const TString& dir, const TString& name) = 0;

		/** @brief  */
		virtual void				closeFile() = 0;

		/** @brief  */
		virtual bool				setSelectedEditable(IEditable* editable) = 0;

		/** @brief  */
		virtual bool				addSelectedEditable(IEditable* editable) = 0;

		/** @brief  */
		virtual bool				removeSelectedEditable(IEditable* editable) = 0;

		/** @brief  */
		virtual IEditable*const*	getSelectedEditables(size_t& count) const = 0;

		/** @brief  */
		virtual const IEntity*		getSelectedEntity() const = 0;

		/** @brief  */
		virtual TString				getValidEntityName(const TString& namePrefix, const TString& suffix = TString::EMPTY) = 0;
	};

	extern template class BLADE_EDITOR_API Factory<IEditorFile>;
	typedef Factory<IEditorFile> EditorFileFactory;
	
}//namespace Blade


#endif //__Blade_IEditorFile_h__