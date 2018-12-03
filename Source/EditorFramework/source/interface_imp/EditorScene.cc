/********************************************************************
	created:	2013/03/09
	filename: 	EditorScene.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EditorScene.h"
#include <interface/EditorElement.h>
#include <interface/IStageConfig.h>

namespace Blade
{
	const TString EditorScene::EDITOR_SCENE_TYPE = IStageConfig::SCENE_TYPE_SKIP_PREFIX + BTString("EditorScene");

	//////////////////////////////////////////////////////////////////////////
	EditorScene::EditorScene()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	EditorScene::~EditorScene()
	{

	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			EditorScene::postProcess(const ProgressNotifier& notifier)
	{
		notifier.onNotify(1.0);
	}

	/************************************************************************/
	/* IScene interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	EditorScene::getSceneType() const
	{
		return EDITOR_SCENE_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	void			EditorScene::getAvailableElementTypes(ElementTypeList& elemList) const
	{
		size_t count = EditorElementFactory::getSingleton().getNumRegisteredClasses();

		for( size_t i = 0; i < count; ++i)
		{
			const TString& name = EditorElementFactory::getSingleton().getRegisteredClass(i);
			elemList.push_back( name );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IElement*		EditorScene::createElement(const TString& type)
	{
		if( !EditorElementFactory::getSingleton().isClassRegistered(type) )
			return NULL;

		EditorElement* elem = BLADE_FACTORY_CREATE(EditorElement, type );
		elem->initialize(this);
		return elem;;
	}

	//////////////////////////////////////////////////////////////////////////
	void			EditorScene::onAttachToStage(IStage* /*pstage*/)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			EditorScene::onDetachFromStage(IStage* /*pstage*/)
	{

	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	
}//namespace Blade