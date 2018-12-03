/********************************************************************
	created:	2011/05/07
	filename: 	EditorToolManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EditorToolManager.h"
#include <BladeFramework_blang.h>
#include <BladeEditor_blang.h>
#include <ConfigTypes.h>
#include <interface/IEditorConfigManager.h>

namespace Blade
{
	template class Factory<IEditorToolManager>;

	//////////////////////////////////////////////////////////////////////////
	EditorToolManager::EditorToolManager()
	{
		mSelectedTool = NULL;
		mInited = false;
	}

	//////////////////////////////////////////////////////////////////////////
	EditorToolManager::~EditorToolManager()
	{

	}

	/************************************************************************/
	/* IEditorToolManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			EditorToolManager::initialize()
	{
		if( mInited )
		{
			assert(false);
			return false;
		}
		mInited = true;

		HCONFIG rootToolConfig;
		rootToolConfig.bind( BLADE_NEW ConfigGroup( BTString(BLANG_TOOL) ) );
		IConfig::IList* configList = rootToolConfig->addOrGetSubConfigs(Version::MAX);

		for(size_t i = 0; i < mCategoryList.size(); ++i)
		{
			const ToolCategory* category = mCategoryList[i];
			EditorToolList& list = category->getToolList();
			for(size_t j = 0; j < list.size(); ++j)
			{
				IEditorTool* tool = list[j];
				HCONFIG toolConfig;
				if( tool != NULL && (toolConfig=tool->getToolConfig()) != NULL )
					configList->addConfig( toolConfig );
			}
		}
		return IEditorConfigManager::getSingleton().addConfig( rootToolConfig );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EditorToolManager::addTool(IEditorTool* tool)
	{
		if( tool == NULL )
			return false;

		const TString& category = tool->getMannaulCategory();
		//const TString& name = tool->getName();

		const ToolCategory& toolCategory = (*mCategoriedTools.insert( ToolCategory(category) ).first);
		EditorToolList& toolList = toolCategory.getToolList();

		if( std::find_if(toolList.begin(),toolList.end(),FnToolFinder(tool) )  != toolList.end() )
			return false;

		if( toolList.size() == 0 )
			mCategoryList.push_back(&toolCategory);

		toolList.push_back(tool);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IEditorTool*	EditorToolManager::removeTool(const TString& category,const TString& name)
	{
		EditorToolCategory::iterator i = mCategoriedTools.find( ToolCategory(category) );
		if( i == mCategoriedTools.end() )
			return NULL;

		const ToolCategory& toolCategory = *i;
		EditorToolList& toolList = toolCategory.getToolList();

		EditorToolList::iterator n = std::find_if(toolList.begin(),toolList.end(),FnNameToolFinder(name) );
		if( n == toolList.end() )
			return NULL;

		IEditorTool* tool = *n;
		toolList.erase(n);
		return tool;
	}

	//////////////////////////////////////////////////////////////////////////
	IEditorTool*	EditorToolManager::getTool(const TString& category,const TString& name) const
	{
		EditorToolCategory::const_iterator i = mCategoriedTools.find( ToolCategory(category) );
		if( i == mCategoriedTools.end() )
			return NULL;

		const EditorToolList& toolList = (*i).getToolList();
		EditorToolList::const_iterator n = std::find_if(toolList.begin(),toolList.end(),FnNameToolFinder(name) );
		if( n == toolList.end() )
			return NULL;

		return *n;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			EditorToolManager::getCategoryCount() const
	{
		return mCategoryList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	EditorToolManager::getCategoryName(index_t categoryIndex) const
	{
		if( categoryIndex >= mCategoryList.size() )
		{
			assert(false);
			return TString::EMPTY;
		}

		return mCategoryList[categoryIndex]->getName();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			EditorToolManager::getToolCountInCategory(index_t categoryIndex) const
	{
		if( categoryIndex >= mCategoryList.size() )
		{
			assert(false);
			return 0;
		}
		else
			return mCategoryList[categoryIndex]->getToolList().size();
	}

	//////////////////////////////////////////////////////////////////////////
	IEditorTool*	EditorToolManager::getEditorTool(index_t categoryIndex,index_t index) const
	{
		if( categoryIndex >= mCategoryList.size() )
			return NULL;

		EditorToolList& tooList = mCategoryList[categoryIndex]->getToolList();
		if( index >= tooList.size() )
			return NULL;
		return tooList[index];
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EditorToolManager::selectTool(IEditorTool* tool)
	{
		mSelectedTool = tool;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IEditorTool*	EditorToolManager::getSelecctedTool() const
	{
		return mSelectedTool;
	}

}//namespace Blade
