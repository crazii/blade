/********************************************************************
	created:	2011/05/23
	filename: 	EditableBase.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IStageConfig.h>
#include <interface/IPage.h>
#include <interface/public/EditableBase.h>
#include <interface/IEditorFramework.h>
#include <interface/public/IEditorFile.h>
#include <interface/EditorEvents.h>
#include <interface/IEventManager.h>

namespace Blade
{
	static const TString PAGE_PREFIX = BTString("P_");

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		EditableBase::onTransformed(IEditorFile* file)
	{
		assert(file != NULL);
		if (!(this->getEditableFlag()&(EDF_SCALABLE | EDF_ROTATABLE | EDF_MOVABLE)))
			return true;
		if (file->getStage() == NULL)
			return false;
		if (!(this->getEditableFlag()&EDF_PAGED))
			return false;
		return this->notifySpaceChange(file);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	/*static */TString EditableBase::generatePageSuffix(IPage* page)
	{
		return BTString("[") + PAGE_PREFIX + TStringHelper::fromInt(page->getIndices().x) + BTString("_") + TStringHelper::fromInt(page->getIndices().y) + BTString("]");
	}

	//////////////////////////////////////////////////////////////////////////
	bool		EditableBase::createEditorElement(const TString& type, IEntity* entity/* = NULL*/, const TString& name/* = TString::EMPTY*/)
	{
		IEditorFile* file = IEditorFramework::getSingleton().getLoadingFile();
		if( file == NULL )
			file = IEditorFramework::getSingleton().getCurrentFile();

		if( file == NULL )
		{
			assert(false);
			return false;
		}
		
		IScene* editorScene = file->getEditorScene();
		mEditorElement = static_cast<EditorElement*>( editorScene->createElement(type) );
		this->getEditorElement()->setEditable(this);
		if (entity != NULL)
			entity->addElement(IStageConfig::ELEMENT_SKIP_PREFIX + (name.empty() ? EditorElement::EDITOR_ELEMENT_NAME : name), HELEMENT(mEditorElement));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EditableBase::notifySpaceChange(IEditorFile* file/* = NULL*/)
	{
		if (file == NULL)
		{
			file = IEditorFramework::getSingleton().getCurrentFile();
			if (file == NULL)
				return false;
			if (file->getEditableManager()->getEditable(this->getInstanceName()) != this)
				return false;
		}

		IEntity* entity = this->getTargetEntity();
		assert(entity != NULL);
		bool ret = file->getStage()->getPagingManager()->notifyEntity(entity, false, false);

		if (!ret)
			return ret;

		const TString& name = this->getInstanceName();

		//note: if an entity moved to another new page, it still have the original page suffix
		//and when the new page is unloaded, the original page still have a chance to use the entity's name
		//we need to rename the entity/editable to avoid name conflicts when pages are changed.
		index_t s = name.find(BTString("[") + PAGE_PREFIX);
		if (s != TString::npos)
		{
			index_t e = name.find(BTString("]"), s);
			if (e != TString::npos)
			{
				const TString pageSuffix = name.substr_nocopy(s, e);
				const POINT3 pos = this->getTargetEntity()->getInterface(IID_GEOMETRY())->getGlobalPosition();
				IPage* page = file->getStage()->getPagingManager()->getPageByPosition(pos.x, pos.z);
				assert(page != NULL);
				TString newSuffix = EditableBase::generatePageSuffix(page);
				if (newSuffix != pageSuffix)
				{
					//generate new name: simply replace pageSuffix with newSuffix may still have conflicts with existing entity in the new page.
					TString base = this->getBaseName();
					TString newName = file->getValidEntityName(base, newSuffix);
					ret = file->getEditableManager()->renameEditable(this, newName);
					assert(ret);
					//add name change event, scene(tree) view and potentially other things in the future.
					//property panel now use data binding for entity names, which will auto update.
					IEventManager::getSingleton().dispatchEvent(EntityRenameEvent(this->getTargetEntity()));
				}
			}
		}
		//note: if PAGE_PREFIX is not found, probably user change it? but paged entities may still have name conflict if they're not loaded yet.
		//so we better not let user to change the name of entity which is paged.
		return ret;
	}
	
}//namespace Blade