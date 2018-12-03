/********************************************************************
	created:	2011/05/22
	filename: 	EditorElement.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/EditorElement.h>
#include <utility/TList.h>
#include <interface/IEntity.h>
#include <interface/public/logic/ILogicService.h>
#include <interface/public/graphics/IGraphicsEffectManager.h>
#include <interface/public/graphics/GraphicsEffect.h>
#include <interface/public/geometry/IGeometry.h>
#include <parallel/ParaStateDef.h>
#include <databinding/DataBindingHelper.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/IEventManager.h>
#include <interface/EditorEvents.h>

#include <interface/IEditorFramework.h>
#include <interface/public/IEditable.h>
#include <interface/public/IEditorFile.h>
#include <BladeEditor_blang.h>

namespace Blade
{

	const TString EditorElement::EDITOR_ELEMENT_TYPE = BXLang(BLANG_EDITOR_ELEMENT);
	const TString EditorElement::EDITOR_ELEMENT_NAME = BTString("[Editor]");

	//////////////////////////////////////////////////////////////////////////
	EditorElement::EditorElement(const TString& type/* = EDITOR_ELEMENT_TYPE*/, const TString& iconHUD/* = TString::EMPTY*/)
		:ElementBase(EDITOR_ELEMENT_TYPE, type, PP_HIGH)
		,mEditable(NULL)
		,mHUDIcon(iconHUD)
	{
		ParaStateQueue* queue = ILogicService::getSingleton().getLogicStateQueue();
		mParallelStates.setQueue(queue);
	}

	//////////////////////////////////////////////////////////////////////////
	EditorElement::~EditorElement()
	{
		if (mEditable != NULL)
		{
			IEditorFile* file = IEditorFramework::getSingleton().getCurrentFile();
			if (file != NULL)
			{
				if (mEditable->isSelected())
					file->removeSelectedEditable(mEditable);

				IEditableManager* manager = file->getEditableManager();
				manager->removeEditable(mEditable);

				IEventManager::getSingleton().dispatchEvent(EditableUnLoadEvent(mEditable));
			}
			else
			{
				//file closing. do nothing
			}
		}
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			EditorElement::postProcess(const ProgressNotifier& notifier)
	{
		ElementBase::postProcess(notifier);
		if( mEditable != NULL )	//none-empty on reload, or paging load
			return;

		assert( mEditableClass != TString::EMPTY );
		//create editable
		IEditorFile* file = IEditorFramework::getSingleton().getLoadingFile();
		if (file == NULL) //runtime paging load
			file = IEditorFramework::getSingleton().getCurrentFile();
		assert(file != NULL);

		HEDITABLE hEditable(BLADE_FACTORY_CREATE(IEditable, mEditableClass));
		hEditable->initialize( this );
		IEditableManager* manager = file->getEditableManager();
		bool ret = manager->addEditable(hEditable);
		assert(ret); BLADE_UNREFERENCED(ret);

		this->showHUD(this->getEntity() != NULL);
		mEditable = hEditable;
	}

	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			EditorElement::initialize(IScene* scene)
	{
		assert( this->getScene() == NULL && scene != NULL );
		this->setScene(scene);
		this->onInitialize();
	}

	//////////////////////////////////////////////////////////////////////////
	void			EditorElement::setEditable(IEditable* editable)
	{
		assert( mEditable == NULL && editable != NULL);
		mEditable = editable;
		mEditableClass = editable->getEditableClass();
		this->showHUD(this->getEntity() != NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EditorElement::showBounding(bool enable, Color color/* = Color::WHITE*/)
	{
		InterfaceListParam list;
		this->getEntity()->getInterfaces(IID_GRAPHICS::getName(), list);
		for (size_t i = 0; i < list.size(); ++i)
		{
			IGraphicsInterface* itf = static_cast<IGraphicsInterface*>((Interface*)list[i]);
			IGraphicsScene* scene = static_cast<IGraphicsScene*>(itf->getGraphicsScene());
			IGraphicsEffectManager* manager = scene->getEffectManager();

			HGRAPHICSEFFECT hEffect = itf->getEffect(IBoundingEffect::TYPE);

			{
				IHUDEffect* hud = static_cast<IHUDEffect*>(itf->getEffect(IHUDEffect::TYPE));
				if (hud != NULL)
					hud->setHighLight(enable);
			}

			if( !enable )
			{
				//remove on disable
				itf->removeEffect(hEffect);
				return true;
			}
			if( hEffect == NULL )
			{
				hEffect = manager->createEffect( IBoundingEffect::TYPE );
				itf->addEffect( hEffect );
			}

			IBoundingEffect* effect = static_cast<IBoundingEffect*>(hEffect);
			effect->setBoundingColor(color);
			effect->setShowBounding(enable);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		EditorElement::showGizmoHelper(bool visible, EGizmoType type/* = GT_MOVE*/, GEOM_SPACE space/* = GS_WORLD*/)
	{
		if(!visible)
			mGizmoEffect = HGRAPHICSEFFECT::EMPTY;

		bool enable = this->getEditable()->isMovable();
		InterfaceListParam list;
		this->getEntity()->getInterfaces(IID_GRAPHICS::getName(), list);

		//for(size_t i = 0; i < list.size(); ++i)
		if( list.size() > 0 )
		{
			size_t i = 0;
			IGraphicsInterface* itf = static_cast<IGraphicsInterface*>((Interface*)list[i]);
			IGraphicsScene* scene = static_cast<IGraphicsScene*>(itf->getGraphicsScene() );
			IGraphicsEffectManager* manager = scene->getEffectManager();

			HGRAPHICSEFFECT hEffect = itf->getEffect(IEditGizmoEffect::TYPE);
			if( !visible )
			{
				//remove on disable
				itf->removeEffect(hEffect);
				return true;
			}

			if( hEffect == NULL )
			{
				hEffect = manager->createEffect( IEditGizmoEffect::TYPE );
				itf->addEffect( hEffect );
			}

			IEditGizmoEffect* effect = static_cast<IEditGizmoEffect*>(hEffect);
			effect->show(visible, enable, type, space);
			mGizmoEffect = hEffect;
			//only one is enough, because the effect is actually attach to internal shared content of the element
			//break;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EditorElement::showHUD(bool visible)
	{
		visible = visible && !mHUDIcon.empty();

		if(!visible)
			mHUDEffect = HGRAPHICSEFFECT::EMPTY;

		if( this->getEntity() == NULL )
			return false;

		InterfaceListParam list;
		this->getEntity()->getInterfaces(IID_GRAPHICS::getName(), list);

		//for(size_t i = 0; i < list.size(); ++i)
		if( list.size() > 0 )
		{
			size_t i = 0;
			IGraphicsInterface* itf = static_cast<IGraphicsInterface*>((Interface*)list[i]);
			IGraphicsScene* scene = static_cast<IGraphicsScene*>(itf->getGraphicsScene() );
			IGraphicsEffectManager* manager = scene->getEffectManager();

			HGRAPHICSEFFECT hEffect = itf->getEffect(IHUDEffect::TYPE);
			if( !visible )
			{
				//remove on disable
				itf->removeEffect(hEffect);
				return true;
			}

			if( hEffect == NULL )
			{
				hEffect = manager->createEffect( IHUDEffect::TYPE );
				itf->addEffect( hEffect );
			}

			IHUDEffect* effect = static_cast<IHUDEffect*>(hEffect);
			if( mHUDIcon.empty() )
			{
				//effect->setIcon(0, BTString("hud_background.png") );
				//effect->setIcon(1, BTString("question.png") );
			}
			else
			{
				effect->setIcon(0, IEditorFramework::EDITOR_ICON_PATH + BTString("hud_background.png") );
				effect->setIcon(1, IEditorFramework::EDITOR_ICON_PATH + mHUDIcon );
			}
			effect->setVisible(true);

			mHUDEffect = hEffect;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IGeometry*EditorElement::getGeomtry() const
	{
		if( mEditable == NULL || mEditable->getTargetEntity() == NULL )
			return NULL;
		IElement* element = mEditable->getTargetEntity()->getElementByType(GeomConsts::GEOMETRY_ELEMENT);
		if (element != NULL)
			return element->getInterface(IID_GEOMETRY());
		return NULL;
	}
	
	//////////////////////////////////////////////////////////////////////////
	Variant		EditorElement::getElementConfig(const TString& elementType, const TString& configName)
	{
		if(mEditable == NULL || mEditable->getTargetEntity() == NULL )
		{
			assert(false);
			return Variant();
		}

		IElement* element = mEditable->getTargetEntity()->getElementByType(elementType);
		return DataBindingHelper::getElementData(element, configName);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EditorElement::setElementConfig(const TString& elementType, const TString& configName, const Variant& val)
	{
		if(mEditable == NULL || mEditable->getTargetEntity() == NULL )
		{
			assert(false);
			return false;
		}

		IElement* element = mEditable->getTargetEntity()->getElementByType(elementType);
		return DataBindingHelper::setElementData(element, configName, val);
	}

}//namespace Blade
