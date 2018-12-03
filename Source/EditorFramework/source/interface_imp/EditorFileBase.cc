/********************************************************************
	created:	2011/05/13
	filename: 	EditorFileBase.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/IStageConfig.h>
#include <interface/IEventManager.h>
#include <interface/public/EditorFileBase.h>
#include <interface/public/ui/IUIWindow.h>
#include "EditorScene.h"

namespace Blade
{
	namespace Impl
	{
		class EditableList : public List<IEditable*> , public Allocatable	{};
	}//namespace Impl
	using namespace Impl;


	//////////////////////////////////////////////////////////////////////////
	EditorFileBase::EditorFileBase(const TString& type)
		:mType(type)
		,mSelectedEntity(NULL)
		,mSelectedEditables( BLADE_NEW EditableList() )
	{
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	EditorFileBase::~EditorFileBase()
	{
	}

	/************************************************************************/
	/* IEditorFile interface                                                                     */
	/************************************************************************/
	void				EditorFileBase::initialize(const SEditorFileInfo* info, FileInstanceID fileID, IUIWindow* win)
	{
		assert( mEditorData.mTypeInfo == NULL );
		mEditorData.mTypeInfo = info;
		mEditorData.mFileID = fileID;
		this->setEditorWindow(win);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		EditorFileBase::getType() const
	{
		return mType;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorFileBase::isOpened() const
	{
		return mOpened;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorFileBase::isModified() const
	{
		return mModified;
	}

	//////////////////////////////////////////////////////////////////////////
	void				EditorFileBase::setModified()
	{
		if( !this->isFileOpened() )
		{
			assert(false);
			return;
		}
		mModified = true;

		IUIWindow* pWin = this->getWindow();
		if( pWin != NULL )
		{
			const TString& cap = pWin->getWindow()->getCaption();
			if( cap.find(TEXT('*')) == TString::npos )
				pWin->getWindow()->setCaption(cap + BTString("*") );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const ED_FD&		EditorFileBase::getEditorData() const
	{
		return mEditorData;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorFileBase::setSelectedEditable(IEditable* editable)
	{
		//clear the old selected
		for(EditableList::iterator i = mSelectedEditables->begin(); i != mSelectedEditables->end(); ++i)
		{
			IEditable* oldSel = *i;
			oldSel->setSelected(false);
		}
		mSelectedEditables->clear();

		IEntity* entity = editable == NULL ? NULL : editable->getEditorElement()->getEntity();
		mSelectedEntity = entity;
		IEventManager::getSingleton().dispatchEvent(EntitySlectedEvent(entity));

		//set new
		if(editable != NULL )
		{
			bool ret = editable->setSelected(true);
			if(ret)
				mSelectedEditables->push_back(editable);
			return ret;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorFileBase::addSelectedEditable(IEditable* editable)
	{
		if(editable != NULL )
		{
			if( editable->isSelected() )
				return false;
			bool ret = editable->setSelected(true);
			if(ret)
			{
				mSelectedEditables->push_back(editable);
			}
			return ret;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorFileBase::removeSelectedEditable(IEditable* editable)
	{
		if( editable == NULL || !editable->isSelected() )
			return false;

		EditableList::iterator i = std::find( mSelectedEditables->begin(),mSelectedEditables->end(),editable );
		if( i == mSelectedEditables->end() )
		{
			assert(false);
			return false;
		}

		editable->setSelected(false);
		mSelectedEditables->erase(i);

		IEntity* entity = editable->getEditorElement()->getEntity();
		if (mSelectedEntity == entity)
		{
			mSelectedEntity = NULL;
			IEventManager::getSingleton().dispatchEvent(EntitySlectedEvent(mSelectedEntity));
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IEditable* const*			EditorFileBase::getSelectedEditables(size_t& count) const
	{
		count = mSelectedEditables->size();
		if( count == 0)
			return NULL;
		return &( mSelectedEditables->front() );
	}

	//////////////////////////////////////////////////////////////////////////
	TString						EditorFileBase::getValidEntityName(const TString& namePrefix, const TString& suffix/* = TString::EMPTY*/)
	{
		IStage* stage = this->getStage();
		if (stage == NULL)
			return TString::EMPTY;

		RefCount& start = mNameIndices[namePrefix];
		for (size_t i = start.count(); i < INVALID_INDEX; ++i)
		{
			if (stage->getEntity(namePrefix + BTString("_") + TStringHelper::fromUInt(i) + suffix) != NULL)
				continue;
			start.set(i);
			break;
		}
		assert(start.count() != INVALID_INDEX);
		index_t number = start.count();
		start.increment();
		return namePrefix + BTString("_") + TStringHelper::fromUInt(number) + suffix;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		EditorFileBase::setStage(IStage* stage)
	{
		mEditorData.mStage = stage;
		mEditorData.mEditorScene = stage->getScene( EditorScene::EDITOR_SCENE_TYPE );
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorFileBase::setWorld(ILogicWorld* world)
	{
		mEditorData.mWorld = world;
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorFileBase::setUnModified()
	{
		if( this->isFileModified() )
		{
			IUIWindow* pWin = this->getWindow();

			const TString& caption = pWin->getWindow()->getCaption();
			assert( caption[ caption.size()-1 ] == TEXT('*') );
			pWin->getWindow()->setCaption(caption.substr(0,caption.size()-1));
			mModified = false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorFileBase::reset()
	{
		::memset(&mEditorData, 0,sizeof(mEditorData) );
		mModified = false;
		mOpened = false;
	}


}//namespace Blade
