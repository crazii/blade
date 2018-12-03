/********************************************************************
	created:	2016/6/12
	filename: 	EditorFramework_UI.cc
	author:		Crazii
	purpose:	Editor framework's user interface related functions
*********************************************************************/
#include <BladePCH.h>
#include "EditorFramework.h"
#include <interface/IFramework.h>
#include <interface/IResourceManager.h>
#include <interface/IPlatformManager.h>
#include <interface/IStageConfig.h>
#include <interface/IEnvironmentManager.h>
#include <interface/public/window/IWindowService.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/ui/IMenu.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/IIconManager.h>
#include <interface/public/ui/IUIWidget.h>
#include <interface/public/ui/IUILayoutManager.h>
#include <interface/public/ui/IUIToolBox.h>
#include <interface/public/ui/widgets/IUIToolPanel.h>
#include <interface/public/ui/IUIMediaLibrary.h>
#include <interface/public/ui/UIWidgetLayout.h>
#include <ConstDef.h>

#include <interface/IEditorToolManager.h>
#include <interface/public/ui/IUIWindow.h>
#include <interface/public/IEditorFile.h>
#include <interface/public/IEditorLogic.h>
#include <interface/public/IImporter.h>

#include "EditorCommand.h"
#include "CommonEditorTools.h"
#include "EditorConfigManager.h"


#include <BladeBase_blang.h>
#include <BladeFramework_blang.h>
#include <BladeEditor_blang.h>
#include <interface/public/geometry/GeometryInterface_blang.h>


namespace Blade
{
	static const ICONSIZE EDITOR_ICON_SIZE = IS_32;

	///default dock places, order is important since it may be used.
	///default dock places are used for layout manager to set to default layout
	enum EWidgetDockPlace
	{
		WDP_LEFT = 0,
		WDP_BOTTOM,
		WDP_RIGHT,

		WDP_COUNT,
		WDP_BEGIN = 0,
		WDP_MASK = 0x3,
	};
	struct SWidgetDataEx
	{
		WIDGET_DATA mData;
		uint16		mGroup;	///default grouped index in parent, or sub dock/pane index
		int16		mOrder; ///default order of widget. order is implementation defined.
							///might be z - order or index of same dock
							///default order are used by layout manager to set to default layout
		EWidgetDockPlace mDockPlace;

		/** @brief  */
		inline bool operator<(const SWidgetDataEx& rhs) const
		{
			return mOrder < rhs.mOrder;
		}
		/** @brief  */
		static inline bool comparePtr(const SWidgetDataEx* lhs, const SWidgetDataEx* rhs)
		{
			return lhs->mOrder < rhs->mOrder;
		}
	};

	namespace Impl
	{
		static StaticLock msWidgetInitLock;
		
		//////////////////////////////////////////////////////////////////////////
		static SWidgetDataEx* getBuiltInWidgets(size_t& count)
		{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4640)	//construction of local static object is not thread-safe
#endif
			ScopedLock sl(msWidgetInitLock);

			//built-in widgets
			static const TString ext = BTString(".png");

			static SWidgetDataEx precreatedWidgets[] =
			{
				BTString(BLANG_TOOL),
				IEditorFramework::EDITOR_WIDGET_TOOL,
				BTLang(BLANG_TOOL),
				POINT2I(0,0),
				SIZE2I(94,0),
				WMF_FIXED | WMF_VISIBLE,
				IIconManager::getSingleton().loadSingleIconImage(BTString(BLANG_TOOL) + ext),
				0, 0, WDP_LEFT,

				BTString(BLANG_PROPERTY),
				IEditorFramework::EDITOR_WIDGET_PROPERTY,
				BTLang(BLANG_PROPERTY),
				POINT2I(0,0),
				SIZE2I(0,0),
				WDF_MOVABLE | WMF_VISIBLE,
				IIconManager::getSingleton().loadSingleIconImage(BTString(BLANG_PROPERTY) + ext),
				0, 0, WDP_RIGHT,

				BTString(BLANG_TRACKVIEW),
				IEditorFramework::EDITOR_WIDGET_TRACKVIEW,
				BTLang(BLANG_TRACKVIEW),
				POINT2I(0,0),
				SIZE2I(0,0),
				WDF_MOVABLE | WMF_VISIBLE,
				IIconManager::getSingleton().loadSingleIconImage(BTString(BLANG_TRACKVIEW) + ext),
				1, 0, WDP_RIGHT,

				BTString(BLANG_MEDIAVIEW),
				IEditorFramework::EDITOR_WIDGET_MEDIAVIEW,
				BTLang(BLANG_MEDIAVIEW),
				POINT2I(0,0),
				SIZE2I(0,0),
				WDF_MOVABLE | WMF_VISIBLE,
				IIconManager::getSingleton().loadSingleIconImage(BTString(BLANG_MEDIAVIEW) + ext),
				2, 0, WDP_RIGHT,

				BTString(BLANG_LOG),
				IEditorFramework::EDITOR_WIDGET_LOGVIEW,
				BTLang(BLANG_LOG),
				POINT2I(0,0),
				SIZE2I(0,0),
				WDF_MOVABLE | WMF_VISIBLE,
				IIconManager::getSingleton().loadSingleIconImage(BTString(BLANG_LOG) + ext),
				0, 0, WDP_BOTTOM,
			};

			count = countOf(precreatedWidgets);
			return precreatedWidgets;
		}

	}//namespace Impl


	//////////////////////////////////////////////////////////////////////////
	void			EditorFramework::setupMenus()
	{
		EditorCommandManager& EdUICmd = EditorCommandManager::getSingleton();

		IMenu* menu_file = IMenuManager::getSingleton().addRootMenu(BXLang(BLANG_FILE), TEXT('F'));
		IMenu* menu_edit = IMenuManager::getSingleton().addRootMenu(BXLang(BLANG_EDIT), TEXT('E'));
		IMenu* menu_tool = IMenuManager::getSingleton().addRootMenu(BXLang(BLANG_TOOL), TEXT('T'));
		BLADE_UNREFERENCED(menu_tool);
		IMenu* menu_view = IMenuManager::getSingleton().addRootMenu(BXLang(BLANG_VIEW), TEXT('V'));

		//file
		IconIndex index = IIconManager::getSingleton().loadSingleIconImage(BTString("filenew.png"));
		menu_file->addItem(BXLang(BLANG_FILE_NEW), EdUICmd.getEditorCommand(UIC_NEW), index, true, TEXT('N'), HOTKEY(KC_N));

		index = IIconManager::getSingleton().loadSingleIconImage(BTString("fileopen.png"));
		menu_file->addItem(BXLang(BLANG_FILE_OPEN), EdUICmd.getEditorCommand(UIC_OPEN), index, true, TEXT('O'), HOTKEY(KC_O));
		menu_file->addSeparator();

		index = IIconManager::getSingleton().loadSingleIconImage(BTString("filesave.png"));
		IMenu* menu_save = menu_file->addItem(BXLang(BLANG_FILE_SAVE), EdUICmd.getEditorCommand(UIC_SAVE), index, false, TEXT('S'), HOTKEY(KC_S));

		index = IIconManager::getSingleton().loadSingleIconImage(BTString("filesaveall.png"));
		IMenu* menu_saveall = menu_file->addItem(BXLang(BLANG_FILE_SAVEALL), EdUICmd.getEditorCommand(UIC_SAVE_ALL), index, false, TEXT('A'), HOTKEY(KC_S, KC_CTRL, KC_SHIFT));
		menu_file->addSeparator();
		menu_file->addItem(BXLang(BLANG_FILE_EXIT), EdUICmd.getEditorCommand(UIC_EXIT), INVALID_ICON_INDEX, false, TEXT('Q'), HOTKEY(KC_Q));
		IMenu* menu_export = menu_file->addItem(BXLang(BLANG_FILE_EXPORT), NULL, INVALID_ICON_INDEX, true, TEXT('P'), HOTKEY(KC_P));

		//edit
		index = IIconManager::getSingleton().loadSingleIconImage(BTString("transform_space.png"));
		IConfig* config = EditorConfigManager::getSingleton().getTransformConfig();
		menu_edit->addSubMenu(config, EdUICmd.addConfigCommand(config), index);
		menu_edit->addSeparator();
		menu_edit->addItem(BXLang(BLANG_EDIT_ENGINECFG), EdUICmd.getEditorCommand(UIC_EDIT_ENGINECFG));

		//view
		TStringParam list;
		list.push_back(BXLang(BLANG_VIEW_VIEWSOLID));
		list.push_back(BXLang(BLANG_VIEW_VIEWWIRE));
		list.push_back(BXLang(BLANG_VIEW_VIEWPOINT));
		index = IIconManager::getSingleton().loadSingleIconImage(BTString("viewport_menu.png"));
		IMenu* menu_cur_view = menu_view->addSubMenu(BXLang(BLANG_VIEWPORT), index, BXLang(BLANG_VIEWPORT)[0]);
		tchar keys[] = { tchar('S'),tchar('W'),tchar('P'), };
		IconIndex icons[4];
		icons[0] = IIconManager::getSingleton().loadSingleIconImage(BTString("viewsolid.png"));
		icons[1] = IIconManager::getSingleton().loadSingleIconImage(BTString("viewwireframe.png"));
		icons[2] = IIconManager::getSingleton().loadSingleIconImage(BTString("viewpoint.png"));
		icons[3] = INVALID_ICON_INDEX;
		IMenu* viewMode = menu_cur_view->addSubMenu(BXLang(BLANG_VIEW_VIEWMODE), INVALID_ICON_INDEX, tchar('M'), UIMF_ENABLE | UIMF_GROUP);
		for (index_t i = 0; i < list.size(); ++i)
			viewMode->addCheckItem(list[i], EdUICmd.getEditorCommand(UIC_VIEWMODE), false, icons[i], keys[i]);

		IMenu* types = menu_cur_view->addSubMenu(BXLang(BLANG_VIEW_VIEWTYPE));
		IGraphicsService* graphicsSvc = IGraphicsService::getSingletonPtr();
		list.push_back(BXLang(BLANG_VIEW_VIEWHIDE));
		for (size_t i = 0; i < graphicsSvc->getGraphicsTypeCount(); ++i)
		{
			const TString& name = graphicsSvc->getGraphicsType(i)->getName();
			IMenu* type = types->addSubMenu(name, INVALID_ICON_INDEX, 0, UIMF_GROUP | UIMF_ENABLE);
			for (size_t j = 0; j < list.size(); ++j)
				type->addCheckItem(list[j], EdUICmd.getEditorCommand(UIC_VIEW_CHANNEL), false, icons[j]);
		}

		menu_cur_view->addSeparator();
		index = IIconManager::getSingleton().loadSingleIconImage(BTString("viewport_max.png"));
		IMenu* viewMax = menu_cur_view->addCheckItem(BXLang(BLANG_VIEW_VIEWMAX), EdUICmd.getEditorCommand(UIC_VIEWMAX), false, index, tchar('X'), HOTKEY(KC_W, KC_CTRL, KC_ALT));
		index = IIconManager::getSingleton().loadSingleIconImage(BTString("view_realtime.png"));
		IMenu* viewRealtime = menu_cur_view->addCheckItem(BXLang(BLANG_VIEW_VIEWREALTIME), EdUICmd.getEditorCommand(UIC_VIEW_REALTIME), false, index, tchar('R'), HOTKEY(KC_R, KC_ALT));

		IMenu* output = menu_cur_view->addSubMenu(BXLang(BLANG_VIEW_OUTPUT), EdUICmd.getEditorCommand(UIC_VIEW_PHASE));
		mInternalNotify.addCommonFileMenu(menu_save);
		mInternalNotify.addCommonFileMenu(menu_saveall);
		mInternalNotify.addCommonFileMenu(menu_export);
		mInternalNotify.addViewRelatedMenu(viewMode);
		mInternalNotify.addViewRelatedMenu(viewMax);
		mInternalNotify.addViewRelatedMenu(viewRealtime);
		mInternalNotify.addViewRelatedMenu(types);
		mInternalNotify.addViewRelatedMenu(output);
	}

	//////////////////////////////////////////////////////////////////////////
	void			EditorFramework::createWidgets()
	{
		size_t count = 0;
		SWidgetDataEx* widgetDatas = Impl::getBuiltInWidgets(count);
		for (size_t i = 0; i < count; ++i)
			mEditorUI->getLayoutManager()->createWidget(reinterpret_cast<WIDGET_DATA&>(widgetDatas[i]));
	}

	//////////////////////////////////////////////////////////////////////////
	void			EditorFramework::setupToolbox()
	{
		CHECK_EDITOR();

		IUIToolBox* tb = mEditorUI->createToolBox(BTString(BLANG_FILE), EDITOR_ICON_SIZE);

		IMenu* menu = IMenuManager::getSingleton().findMenu(BTString(BLANG_FILE_NEW));
		tb->addButton(menu);
		menu = IMenuManager::getSingleton().findMenu(BTString(BLANG_FILE_OPEN));
		tb->addButton(menu);
		menu = IMenuManager::getSingleton().findMenu(BTString(BLANG_FILE_SAVE));
		tb->addButton(menu);
		menu = IMenuManager::getSingleton().findMenu(BTString(BLANG_FILE_SAVEALL));
		tb->addButton(menu);

		tb = mEditorUI->createToolBox(BTString(BLANG_EDIT), EDITOR_ICON_SIZE);
		menu = IMenuManager::getSingleton().findMenu(EditorConfigManager::getSingleton().getTransformConfig()->getName());
		tb->addButton(menu, IUIToolBox::TBS_DROPDOWN | IUIToolBox::TBS_TEXT);

		//add widget toggle menu
		//setup view menu, add dock windows in view
		IMenu* ViewMenu = IMenuManager::getSingleton().findMenu(BTString(BLANG_VIEW));
		ViewMenu->addSeparator();

		//dock window icons & add related toolbar button
		tb = mEditorUI->createToolBox(BTLang(BLANG_WIDGETS), EDITOR_ICON_SIZE);
		size_t widgetCount = mEditorUI->getLayoutManager()->getWidgetCount();
		for (size_t i = 0; i < widgetCount; ++i)
		{
			IUIWidget* widget = mEditorUI->getLayoutManager()->getWidget(i);
			IMenu* widgetToggle = ViewMenu->addCheckItem(widget->getName(),
				EditorCommandManager::getSingleton().getEditorCommand(UIC_VIEW_WIDGET),
				true, widget->getIcon(), widget->getName()[0]);
			tb->addButton(widgetToggle);
		}
		ViewMenu->addSeparator();
		ViewMenu->addItem(BTLang("Reset Layout"), EditorCommandManager::getSingleton().getEditorCommand(UIC_VIEW_RESETLAYOUT));

		//add UI for editor tools
		IEditorToolManager& toolman = IEditorToolManager::getSingleton();
		size_t nCategory = toolman.getCategoryCount();
		IMenu* tool_menu = IMenuManager::getSingleton().getRootMenu(BTString(BLANG_TOOL));
		IUIToolPanel* toolpanel = static_cast<IUIToolPanel*>(mEditorUI->getLayoutManager()->getWidget(EDITOR_WIDGET_TOOL));
		if (toolpanel != NULL)
		{
			TString emptyCate = BTString(BLANG_COMMON);
			TStringParam categories;
			TempVector<IconIndex> icons;
			for (size_t i = 0; i < this->getFileTypeCount(); ++i)
			{
				const SEditorFileInfoEx* info = this->getFileType((FileTypeID)i);
				categories.push_back(info->mFileClassName);
				icons.push_back(info->mIconID);
			}
			assert(categories.size() == icons.size());
			toolpanel->setupCategory(emptyCate, INVALID_ICON_INDEX, categories.getBuffer(), icons.size() > 0 ? &icons[0] : NULL, icons.size());
		}

		for (size_t i = 0; i < nCategory; ++i)
		{
			tb = NULL;
			TStringParam tbCategories;
			bool alwaysShow = false;
			size_t nTool = toolman.getToolCountInCategory(i);
			const TString& categoryName = toolman.getCategoryName(i);
			IMenu* cateMenu = tool_menu->addSubMenu(categoryName, INVALID_ICON_INDEX, IMenu::AUTO_KEY);
			for (size_t n = 0; n < nTool; ++n)
			{
				IEditorTool* tool = toolman.getEditorTool(i, n);
				IconIndex icon = tool->getToolIcon();

				TStringParam fileList;
				size_t fileCount = tool->getSupportedFiles(fileList);
				if (fileCount == INVALID_INDEX)
					alwaysShow = true;	//if one tool in the tool box supports all files, then the tool box will be always shown
				else if (!alwaysShow)
				{
					for (size_t j = 0; j < fileCount; ++j)
					{
						if (tbCategories.find(fileList[j]) == INVALID_INDEX)
							tbCategories.push_back(fileList[j]);
					}
				}

				//add binding command
				IUICommand* cmd = EditorCommandManager::getSingleton().addToolCommand(tool);

				IMenu* menuItem;
				if (tool->getType() == EditorToolTypes::IMMEDIATE_EDIT_TYPE)
					menuItem = cateMenu->addItem(tool->getName(), cmd, icon, false, IMenu::AUTO_KEY, tool->getHotkey());
				else if (tool->getType() != EditorToolTypes::IMMEDIATE_STATUS_EDIT_TYPE)
					menuItem = cateMenu->addCheckItem(tool->getName(), cmd, false, icon, IMenu::AUTO_KEY, tool->getHotkey());
				else
					menuItem = cateMenu->addItem(tool->getName(), cmd, icon, false, IMenu::AUTO_KEY, tool->getHotkey());

				menuItem->setEnabled(tool->isEnable());

				if (tb == NULL)
					tb = mEditorUI->createToolBox(categoryName, EDITOR_ICON_SIZE);
				tb->addButton(menuItem);
			}
			assert(tb != NULL);
			if (alwaysShow)
				tbCategories.clear();
			if (toolpanel != NULL)
				toolpanel->addToolBox(tbCategories, tb, true);
		}
		if (toolpanel != NULL)
			toolpanel->switchCategory(toolpanel->getDefaultCategory());
	}

	//////////////////////////////////////////////////////////////////////////
	void			EditorFramework::setupUILayout()
	{
		if( !mEditorUI->getLayoutManager()->loadLastLayout() )
			this->resetUILayout();
	}
	
	//////////////////////////////////////////////////////////////////////////
	void			EditorFramework::resetUILayout()
	{
		UIWidgetLayout layout;
		layout.getRoot().setVisible(true);

		size_t count = 0;
		SWidgetDataEx* widgetDatas = Impl::getBuiltInWidgets(count);

		fp32	defaultSize[WDP_COUNT] = { 0.06f, 0.25f, 0.2f };
		typedef TempVector<const SWidgetDataEx*> OrderMap;
		typedef TempMap<uint, OrderMap> GroupMap;
		typedef TempVector<GroupMap> DockMap;
		DockMap dockMap(WDP_COUNT);

		//collect & group widgets from default docking init
		for (size_t i = 0; i < count; ++i)
		{
			const SWidgetDataEx& widgetData = widgetDatas[i];
			OrderMap& group = dockMap[widgetData.mDockPlace][widgetData.mGroup];
			group.push_back(&widgetData);
		}

		UILayoutNode* last = &layout.getRoot();
		size_t currentCount = 0;
		for (size_t i = 0; i < WDP_COUNT; ++i)
		{
			GroupMap& groups = dockMap[i];
			if (groups.empty())
				continue;

			UILayoutNode* next = NULL;
			if (i != WDP_LEFT /*&& i < WDP_COUNT-1 && currentCount + groups.size() < count*/)
			{
				next = BLADE_NEW UILayoutNode();
				next->setLayoutType(i == WDP_BOTTOM ? UILayoutNode::LT_HORIZONTAL : UILayoutNode::LT_VERTICAL);
				next->setSize(1.0f - defaultSize[i], 1.0f - defaultSize[i]);
				next->setVisible(true);
				last->addChild(next);
			}

			UILayoutNode* node = BLADE_NEW UILayoutNode();
			node->setLayoutType(i == WDP_BOTTOM ? UILayoutNode::LT_HORIZONTAL : UILayoutNode::LT_VERTICAL);
			node->setSize(defaultSize[i], defaultSize[i]);
			node->setVisible(true);
			last->addChild(node);

			if(i == WDP_LEFT /*&& currentCount + groups.size() < count*/)
			{
				next = BLADE_NEW UILayoutNode();
				next->setLayoutType(UILayoutNode::LT_VERTICAL);
				next->setSize(1.0f - defaultSize[i], 1.0f - defaultSize[i]);
				next->setVisible(true);
				last->addChild(next);
			}

			if (&layout.getRoot() == last)
				layout.getRoot().setLayoutType(node->getLayoutType() == UILayoutNode::LT_VERTICAL ? UILayoutNode::LT_HORIZONTAL : UILayoutNode::LT_VERTICAL);

			last = next;

			for (GroupMap::iterator gi = groups.begin(); gi != groups.end(); ++gi)
			{
				UILayoutNode* groupNode = BLADE_NEW UILayoutNode();
				groupNode->setLayoutType(node->getLayoutType() == UILayoutNode::LT_HORIZONTAL ? UILayoutNode::LT_VERTICAL : UILayoutNode::LT_HORIZONTAL);
				groupNode->setSize(1.0f / (fp32)groups.size(), 1.0f / (fp32)groups.size());
				groupNode->setVisible(true);
				node->addChild(groupNode);

				OrderMap& group = gi->second;
				std::sort(group.begin(), group.end(), SWidgetDataEx::comparePtr);

				for (size_t j = 0; j < group.size(); ++j)
				{
					++currentCount;
					const SWidgetDataEx* widgetData = group[j];
					groupNode->addWidget(widgetData->mData.mName, widgetData->mData.mPos, widgetData->mData.mSize, widgetData->mData.mFlags);
				}
				groupNode->setActiveWidget(0);
			}
		}

		last->addWidget(IUILayoutManager::WORKING_AREA, POINT2I::ZERO, SIZE2I::ZERO, 0);
		last->setVisible(true);

		mEditorUI->getLayoutManager()->applyLayout(layout);
	}

	//////////////////////////////////////////////////////////////////////////
	void			EditorFramework::saveLayout()
	{
		mEditorUI->getLayoutManager()->saveLayout();
	}

	/************************************************************************/
	/* editor command implementation                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			EditorFramework::saveFile()
	{
		CHECK_EDITOR();

		IEditorFile* file = this->getCurrentFile();

		if (file != NULL)
			file->saveFile();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EditorFramework::isSaveAllNeeded() const
	{
		bool dirty = false;
		for (OpenedFileMap::const_iterator i = mOpenedFiles.begin(); i != mOpenedFiles.end(); ++i)
		{
			const SEditorFileInfoEx* info = this->getFileType(i->second->getType());
			assert(info != NULL);

			if ((info->mOperations&EFO_SAVE))
			{
				if (i->second->isModified())
				{
					dirty = true;
					break;
				}
			}
		}
		return dirty;
	}

	//////////////////////////////////////////////////////////////////////////
	void			EditorFramework::saveAll()
	{
		CHECK_EDITOR();
		//save all opened files
		for (OpenedFileMap::iterator i = mOpenedFiles.begin(); i != mOpenedFiles.end(); ++i)
			i->second->saveFile();
	}
	
}//namespace Blade