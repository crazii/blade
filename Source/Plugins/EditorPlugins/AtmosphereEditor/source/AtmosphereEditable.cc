/********************************************************************
	created:	2011/09/03
	filename: 	AtmosphereEditable.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "AtmosphereEditable.h"
#include <interface/IStageConfig.h>
#include <interface/IEditorFramework.h>
#include <interface/IWorldEditableManager.h>
#include <interface/IAtmosphere.h>
#include <interface/public/logic/ILogicWorld.h>
#include <BladeAtmosphereEditor_blang.h>

namespace Blade
{
	const TString AtmosphereEditable::ATMOSPHERE_EDITABLE_NAME = BTString(BLANG_ATMOSPHERE);

	//////////////////////////////////////////////////////////////////////////
	AtmosphereEditable::AtmosphereEditable()
		:EditableBase( BTLang(BLANG_ATMOSPHERE), ATMOSPHERE_EDITABLE_NAME )
		,mAtmosphereEntity(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	AtmosphereEditable::~AtmosphereEditable()
	{
	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			AtmosphereEditable::initialize(const IConfig* creationConfig,const ParamList* additionalParams/* = NULL*/)
	{
		BLADE_UNREFERENCED(additionalParams);

		if( creationConfig == NULL)
			return false;

		ILogicWorld* pWorld = IWorldEditableManager::getInterface().getWorld();
		if( pWorld == NULL )
			return false;

		IElement* element = NULL;
		if( mAtmosphereEntity == NULL )
		{
			TString entityName = pWorld->getWorldName() + BTString("_AtmosphereEntity");
			mAtmosphereEntity = pWorld->getStage()->createEntity(entityName);
			HELEMENT	hAtmosphereElement( pWorld->getGraphicsScene()->createElement( AtmosphereConsts::ATMOSPHERE_ELEMENT_TYPE ) );
			mAtmosphereEntity->addElement(BTString("GraphicsAtmoSphere"),hAtmosphereElement);
			element = hAtmosphereElement;
		}
		else
			element = mAtmosphereEntity->getElement(AtmosphereConsts::ATMOSPHERE_ELEMENT_TYPE);

		//atmosphere doesn't need paging, adding it to page is OK, but unnecessary.
		//pWorld->getStage()->getPagingManager()->notifyEntity(mAtmosphereEntity, true, true);

		this->createEditorElement(EditorElement::EDITOR_ELEMENT_TYPE, mAtmosphereEntity, BTString("Atmosphere"));
		IAtmosphere::getInterface().initialize(element);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AtmosphereEditable::initialize(EditorElement* element)
	{
		this->setEditorElement(element);
		mAtmosphereEntity = element->getEntity();
		return true;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/	

}//namespace Blade
