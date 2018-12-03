/********************************************************************
	created:	2011/04/22
	filename: 	EditorCommand.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IPlatformManager.h>
#include <interface/IConfigManager.h>
#include <interface/public/ui/IUIWidget.h>
#include <interface/public/ui/IUILayoutManager.h>
#include <interface/public/EditorToolTypes.h>
#include <interface/IEditorToolManager.h>
#include <BladeBase_blang.h>
#include <BladeFramework_blang.h>
#include "EditorCommand.h"
#include "EditorFramework.h"
#include "ViewportCommand.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class SaveFileCommand : public IUICommand, public StaticAllocatable
	{
	public:
		virtual void execute(ICommandUI* /*cui*/)
		{
			EditorFramework::getSingleton().saveFile();
		}

		virtual bool update(ICommandUI* ui, index_t /*instance*/)
		{
			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();

			if (file != NULL)
			{
				const SEditorFileInfoEx* info = EditorFramework::getSingleton().getFileType(file->getType());
				ui->setEnabled((info->mOperations&EFO_SAVE) != 0 && EditorFramework::getSingleton().getCurrentFile()->isModified());
				return true;
			}
			else
				ui->setEnabled(false);
			return true;
		}
	};
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class SaveAllFileCommand : public IUICommand, public StaticAllocatable
	{
	public:
		virtual void execute(ICommandUI* /*cui*/)
		{
			EditorFramework::getSingleton().saveAll();
		}

		virtual bool update(ICommandUI* ui, index_t /*instance*/)
		{
			ui->setEnabled( EditorFramework::getSingleton().isSaveAllNeeded() );
			return true;
		}
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class Edit_EngineConfigCommand : public IUICommand, public StaticAllocatable
	{
	public:
		virtual void execute(ICommandUI* /*cui*/)
		{
			TPointerParam<IConfig> configs;
			IConfigManager::getSingleton().getAllConfigs(configs);
			if (configs.size() > 0)
			{
				IConfigDialog::INFO modal(false, false);
				EditorFramework::getSingleton().getEditorUI()->showMultipleGroupConfig(BTLang("Engine Configs"), &configs[0], configs.size(), modal);
			}
		}
		virtual bool update(ICommandUI* /*ui*/, index_t /*instance*/)
		{
			return true;
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ToggleWidgetCommand : public IUICommand, public StaticAllocatable
	{
	public:
		ToggleWidgetCommand() {}
		~ToggleWidgetCommand() {}

		/** @brief  */
		virtual void execute(ICommandUI* cui)
		{
			IUILayoutManager* layoutManager = EditorFramework::getSingleton().getEditorUI()->getLayoutManager();
			IUIWidget* widget = layoutManager->getWidget(cui->getName());
			bool visible = !widget->isVisible();
			layoutManager->toggleWidget(widget, visible);
			layoutManager->activateWidget(widget, visible);
		}

		/** @brief  */
		virtual bool update(ICommandUI* cui, index_t /*instance*/ /* = INVALID_INDEX */)
		{
			IUIWidget* widget = EditorFramework::getSingleton().getEditorUI()->getLayoutManager()->getWidget(cui->getName());
			if (widget != NULL)
				cui->setChecked(widget->isVisible());
			return widget != NULL;
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class EditorToolCommand : public IUICommand, public StaticAllocatable
	{
	public:
		EditorToolCommand(IEditorTool* tool) :mTool(tool) {}
		~EditorToolCommand() {}

		/** @brief  */
		virtual void execute(ICommandUI* /*cui*/)
		{
			if (mTool->getType() == EditorToolTypes::IMMEDIATE_EDIT_TYPE)
				mTool->setSelected(true);
			else if (mTool->getType() == EditorToolTypes::IMMEDIATE_STATUS_EDIT_TYPE)
				mTool->setSelected(true);
			else
			{
				IEditorTool* oldTool = IEditorToolManager::getSingleton().getSelecctedTool();

				if (oldTool != NULL && oldTool != mTool)
					oldTool->setSelected(false);

				mTool->setSelected(true);
				IEditorToolManager::getSingleton().selectTool(mTool);
			}
		}

		/** @brief  */
		virtual bool update(ICommandUI* cui, index_t /*instance*/)
		{
			cui->setEnabled(mTool->isEnable());
			if (mTool->getType() == EditorToolTypes::IMMEDIATE_EDIT_TYPE) {}
			else if (mTool->getType() != EditorToolTypes::IMMEDIATE_STATUS_EDIT_TYPE)
				cui->setChecked(mTool->isSelected());
			return true;
		}

		/** @brief  */
		virtual void config(ICommandUI* /*cui*/)
		{
			if (!mTool->isEnable())
				return;

			const HCONFIG& toolConfig = mTool->getToolConfig();
			if(toolConfig == NULL )
				return;

			IConfigDialog::INFO dlgInfo(false);
			const TString& dialogName = BTString2Lang(mTool->getName());
			IEditorFramework::getSingleton().getEditorUI()->showSingleGroupConfig(dialogName, toolConfig, dlgInfo);
		}
	protected:
		IEditorTool*	mTool;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ConfigCommand : public Blade::IUICommand, public StaticAllocatable
	{
	public:
		ConfigCommand(IConfig* config) :mConfig(config) {}

		virtual void execute(ICommandUI* cui)
		{
			mConfig->setValue(cui->getName());
			//note: immediate reflection change, some ui implementation may have delayed calling of update()
			cui->setChecked(true);
		}

		virtual bool update(ICommandUI* cui, index_t /*instance = INVALID_INDEX*/)
		{
			cui->setChecked(mConfig->getValue() == cui->getName());
			return true;
		}
	protected:
		IConfig* mConfig;
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class HelpAboutCommand : public IUICommand, public StaticAllocatable
	{
	public:
		HelpAboutCommand(Version ver)
			:mVersion(ver) {}
		~HelpAboutCommand() {}

		/** @brief  */
		virtual void execute(ICommandUI* /*cui*/)
		{
			const TString& info = BTLang(BLANG_BLADE) + BTLang(BLANG_EDITOR)
				+ BTString(" ") + mVersion.getVersionString()
				+ BTString("\n\n")
				+ BTString("This software uses the FreeImage open source image library. See http://freeimage.sourceforge.net for details.\n\
					FreeImage is used under the FIPL License, version 1.0.\n\n")
				+ BTLang(BLANG_COPYRIGHT) + BTString("\t\t\t\n\n");

			IPlatformManager::getSingleton().showMessageBox(info.c_str(), BTLang(BLANG_ABOUT).c_str());
		}

	protected:
		Version mVersion;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	EditorCommandManager::EditorCommandManager()
	{

	}
	//////////////////////////////////////////////////////////////////////////
	EditorCommandManager::~EditorCommandManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorCommandManager::initialize(Version ver)
	{
		EditorFramework* edfm = EditorFramework::getSingletonPtr();

		mCmdList[UIC_NEW] = BLADE_NEW UICommandDelegate(edfm, &EditorFramework::creatFile  );
		mCmdList[UIC_OPEN] = BLADE_NEW UICommandDelegate(edfm, &EditorFramework::openFile );
		mCmdList[UIC_SAVE] = BLADE_NEW SaveFileCommand();
		mCmdList[UIC_SAVE_ALL] = BLADE_NEW SaveAllFileCommand();
		mCmdList[UIC_EXIT] = BLADE_NEW UICommandDelegate(edfm, &EditorFramework::exit );
		mCmdList[UIC_EDIT_ENGINECFG] = BLADE_NEW Edit_EngineConfigCommand();
		mCmdList[UIC_VIEWMODE] = BLADE_NEW ViewModeCommand();
		mCmdList[UIC_VIEWMAX] = BLADE_NEW MaximizeViewCommand();
		mCmdList[UIC_VIEW_REALTIME] = BLADE_NEW RealtimeViewCommand();
		mCmdList[UIC_VIEW_CHANNEL] = BLADE_NEW ViewportChannelCommand();
		mCmdList[UIC_VIEW_PHASE] = BLADE_NEW ViewportPhaseCommand();
		mCmdList[UIC_VIEW_WIDGET] = BLADE_NEW ToggleWidgetCommand();
		mCmdList[UIC_VIEW_RESETLAYOUT] = BLADE_NEW UICommandDelegate(edfm, &EditorFramework::resetUILayout);

		mCmdList[UIC_HELP_ABOUT] = BLADE_NEW HelpAboutCommand(ver);
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorCommandManager::shutdown()
	{
		for(size_t i = 0; i < UIC_COUNT; ++i)
		{
			BLADE_DELETE mCmdList[i];
			mCmdList[i] = NULL;
		}

		while (!mDynamicCmdList.empty())
		{
			BLADE_DELETE mDynamicCmdList.back();
			mDynamicCmdList.pop_back();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IUICommand*	EditorCommandManager::getEditorCommand(EUICommand eCmd) const
	{
		if( eCmd >= UIC_COUNT )
			return NULL;
		return mCmdList[eCmd];
	}

	//////////////////////////////////////////////////////////////////////////
	IUICommand*	EditorCommandManager::addConfigCommand(IConfig* cfg)
	{
		IUICommand* cmd = BLADE_NEW ConfigCommand(cfg);
		mDynamicCmdList.push_back(cmd);
		return cmd;
	}

	//////////////////////////////////////////////////////////////////////////
	IUICommand*	EditorCommandManager::addToolCommand(IEditorTool* tool)
	{
		IUICommand* cmd = BLADE_NEW EditorToolCommand(tool);
		mDynamicCmdList.push_back(cmd);
		return cmd;
	}


}//namespace Blade
