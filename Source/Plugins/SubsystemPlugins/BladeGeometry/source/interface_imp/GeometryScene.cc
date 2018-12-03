/********************************************************************
	created:	2010/05/07
	filename: 	GeometryScene.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GeometryScene.h"
#include "GeometryTask.h"
#include <interface/IStage.h>
#include <interface/ITaskManager.h>

namespace Blade
{
	const TString GeometryScene::GEOMETRY_SCENE_TYPE = BTString("GeometryScene");

	//////////////////////////////////////////////////////////////////////////
	GeometryScene::GeometryScene()
		:mUpdateMethod(GUM_MANUAL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GeometryScene::~GeometryScene()
	{

	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	void			GeometryScene::postProcess(const ProgressNotifier& /*callback*/)
	{

	}

	/************************************************************************/
	/* IScene Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	GeometryScene::getSceneType() const
	{
		return GEOMETRY_SCENE_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GeometryScene::getAvailableElementTypes(ElementTypeList& elemList) const
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		size_t count = GeometryFactory::getSingleton().getNumRegisteredClasses();
		for (size_t i = 0; i < count; ++i)
		{
			const TString& name = GeometryFactory::getSingleton().getRegisteredClass(i);
			elemList.push_back(name);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IElement*		GeometryScene::createElement(const TString& type)
	{
		if (GeometryFactory::getSingleton().isClassRegistered(type))
		{
			GeometryElementBase* elem = BLADE_FACTORY_CREATE(GeometryElementBase, type);
			elem->initialize(this, &mStateQueue, &mUpdater);
			return elem;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GeometryScene::onAttachToStage(IStage* pstage)
	{
		if( mGeomTask == NULL )
		{
			mGeomTask.lock();
			if( mGeomTask == NULL )
			{
				GeometryTask* task = BLADE_NEW GeometryTask( pstage->getName() + BTString("'s geometry task"),mUpdater, mStateQueue);
				mGeomTask.bind( task );
			}
			mGeomTask.unlock();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			GeometryScene::onDetachFromStage(IStage* pstage)
	{
		BLADE_UNREFERENCED(pstage);

		if( mUpdateMethod != GUM_MANUAL )
			ITaskManager::getSingleton().removeTask(mGeomTask);
	}

	/************************************************************************/
	/* IGeometryScene interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	GEOM_UPDATE_METHOD	GeometryScene::getUpdateMethod() const
	{
		return mUpdateMethod;
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryScene::setUpdateMethod(GEOM_UPDATE_METHOD method)
	{
		mUpdateMethod = method;
		GeometryTask* task = static_cast<GeometryTask*>(mGeomTask);
		if( mUpdateMethod != GUM_MANUAL )
		{
			task->setUpdateMethod(method);
			ITaskManager::getSingleton().addTask(task);
		}
		else
			ITaskManager::getSingleton().removeTask(task);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryScene::update()
	{
		if( mUpdateMethod == GUM_MANUAL )
		{
			GeometryTask* task = static_cast<GeometryTask*>(mGeomTask);
			task->updateData();
		}
	}
	
}//namespace Blade