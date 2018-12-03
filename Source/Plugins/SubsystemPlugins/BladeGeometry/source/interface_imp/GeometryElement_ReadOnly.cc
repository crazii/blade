/********************************************************************
	created:	2010/05/07
	filename: 	GeometryElement_ReadOnly.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GeometryElement_ReadOnly.h"
#include <interface/public/CommonState.h>
#include <interface/IEventManager.h>
#include <interface/public/graphics/IGraphicsSpaceCoordinator.h>


//this pointer as param in member class construction
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma  warning(disable:4355)
#endif

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GeometryElement_ReadOnly::GeometryElement_ReadOnly()
		:GeometryElementBase(GeomConsts::GEOMETRY_ELEMENT_READONLY, PP_LOWEST)
		,mPosition(&mPositionData)
		,mScale(&mScaleData)
		,mRotation(&mRotationData)
		,mBounding(&mAABData)
	{
		mPositionData = Vector3::ZERO;
		mScaleData = Vector3::UNIT_ALL;
		mRotationData = Quaternion::IDENTITY;
	}

	//////////////////////////////////////////////////////////////////////////
	GeometryElement_ReadOnly::~GeometryElement_ReadOnly()
	{
		
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GeometryElement_ReadOnly::postProcess(const ProgressNotifier& notifier)
	{
		notifier.onNotify(1.0f);
	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t				GeometryElement_ReadOnly::initParallelStates()
	{
		mParallelStates[CommonState::POSITION] = mPosition;
		mParallelStates[CommonState::SCALE] = mScale;
		mParallelStates[CommonState::ROTATION] = mRotation;
		mParallelStates[CommonState::BOUNDING] = mBounding;

		mPosition.setUnTouched(Vector3::ZERO);
		mScale.setUnTouched(Vector3::UNIT_ALL);
		mRotation.setUnTouched(Quaternion::IDENTITY);
		mBounding.setUnTouched(AABB(BT_INFINITE));

		return mParallelStates.size();
	}

	////////////////////////////////////////////////////////////////////////
	void				GeometryElement_ReadOnly::onParallelStateChange(const IParaState& data)
	{
		BLADE_UNREFERENCED(data);
	}
	/************************************************************************/
	/* IGeometry interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const Vector3&		GeometryElement_ReadOnly::getGlobalPosition() const
	{
		return mPosition;
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&	GeometryElement_ReadOnly::getGlobalRotation() const
	{
		return mRotation;
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		GeometryElement_ReadOnly::getGlobalScale() const
	{
		return mScale;
	}

	//////////////////////////////////////////////////////////////////////////
	const AABB&	GeometryElement_ReadOnly::getLocalBounding() const
	{
		return mBounding;
	}

	/************************************************************************/
	/* GeometryElementBase interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement_ReadOnly::initialize(IScene* scene, ParaStateQueue* queue, GeometryUpdater* /*updater*/)
	{
		this->setScene(scene);
		mParallelStates.setQueue(queue);
		mParallelStates.setPriority(PP_LOWEST);
	}
	
}//namespace Blade