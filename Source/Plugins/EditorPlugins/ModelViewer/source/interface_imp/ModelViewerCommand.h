/********************************************************************
	created:	2015/11/16
	filename: 	ModelViewerCommand.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelViewerCommand_h__
#define __Blade_ModelViewerCommand_h__
#include <interface/public/ui/IUICommand.h>
#include <interface/IEditorFramework.h>
#include <interface/public/IEditorFile.h>
#include <interface/ISyncAnimation.h>
#include <BladeModelViewer.h>
#include "ModelViewerFile.h"
#include "ModelViewEditable.h"

namespace Blade
{
	class ModelViewerCommand : public IUICommand
	{
	public:
		/** @brief  */
		virtual void excuteImpl(ModelViewerFile* file, ModelViewEditable* editable) = 0;
		/** @brief  */
		virtual bool updateImpl(ICommandUI* cui, ModelViewerFile* file) = 0;

		/** @brief  */
		virtual void execute(ICommandUI* /*cui*/)
		{
			IEditorFile* file = IEditorFramework::getSingleton().getCurrentFile();
			if( file != NULL && file->getType() == MODEL_FILE_TYPE )
			{
				ModelViewerFile* f = static_cast<ModelViewerFile*>(file);
				ModelViewEditable* editable = static_cast<ModelViewEditable*>(f->getEditable());
				if( editable != NULL )
					this->excuteImpl(f, editable);
			}
		}

		/** @brief  */
		virtual bool update(ICommandUI* cui, index_t /*instance = INVALID_INDEX*/)
		{
			IEditorFile* file = IEditorFramework::getSingleton().getCurrentFile();
			if( file != NULL && file->getType() == MODEL_FILE_TYPE )
			{
				ModelViewerFile* f = static_cast<ModelViewerFile*>(file);
				return this->updateImpl(cui, f);
			}
			return false;
		}
	};

	class MVShowBoundingCmd : public  ModelViewerCommand, public Singleton<MVShowBoundingCmd>
	{
	public:
		/** @brief  */
		virtual void excuteImpl(ModelViewerFile* file, ModelViewEditable* editable)
		{
			ISyncAnimation* syncAnim = editable->getTargetEntity()->getInterface(IID_SYNCANIM());
			if(syncAnim != NULL)
			{
				file->toggleSubmeshBoundingVisible();
				syncAnim->setAniamtedBoundingVisible(file->isSubmeshBoundingVisible());
			}
		}

		/** @brief  */
		virtual bool updateImpl(ICommandUI* cui, ModelViewerFile* file)
		{
			cui->setChecked(file->isSubmeshBoundingVisible());
			return true;
		}
	};

	class MVShowSkeletonCmd : public  ModelViewerCommand, public Singleton<MVShowSkeletonCmd>
	{
	public:

		/** @brief  */
		virtual void excuteImpl(ModelViewerFile* file, ModelViewEditable* editable)
		{
			ISyncAnimation* syncAnim = editable->getTargetEntity()->getInterface(IID_SYNCANIM());
			if(syncAnim != NULL)
			{
				file->toggleSkeletonVisible();
				syncAnim->setBoneVisible(file->isSkeletonVisible());
			}
		}

		/** @brief  */
		virtual bool updateImpl(ICommandUI* cui, ModelViewerFile* file)
		{
			cui->setChecked(file->isSkeletonVisible());
			return true;
		}
	};

	
}//namespace Blade

#endif // __Blade_ModelViewerCommand_h__
