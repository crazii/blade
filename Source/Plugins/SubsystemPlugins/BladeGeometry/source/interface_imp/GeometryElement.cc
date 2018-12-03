/********************************************************************
	created:	2010/05/07
	filename: 	GeometryElement.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GeometryElement.h"
#include <interface/public/CommonState.h>
#include <interface/IParaState.h>
#include <interface/IEventManager.h>
#include <interface/public/graphics/IGraphicsSpaceCoordinator.h>
#include "GeometryUpdater.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GeometryElement::GeometryElement()
		:GeometryElementBase(GeomConsts::GEOMETRY_ELEMENT, PP_HIGH)
		,mPosition(&mPositionData)
		,mScale(&mScaleData)
		,mRotation(&mRotationData)
		,mBounding(&mAABData)
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	GeometryElement::~GeometryElement()
	{
		BLADE_DELETE mNode;
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GeometryElement::postProcess(const ProgressNotifier& notifier)
	{
		this->setGlobalPosition(mPositionData);
		this->setGlobalScale(mScaleData);
		this->setGlobalRotation(mRotationData);

		notifier.onNotify(1.0f);
	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t				GeometryElement::initParallelStates()
	{
		mParallelStates[CommonState::POSITION] = mPosition;
		mParallelStates[CommonState::SCALE] = mScale;
		mParallelStates[CommonState::ROTATION] = mRotation;
		mParallelStates[CommonState::BOUNDING] = mBounding;

		//reading data
		mBounding.setPriority(PP_LOWEST);
		mBounding.setUnTouched(AABB(BT_INFINITE));

		return mParallelStates.size();
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::onParallelStateChange(const IParaState& data)
	{
		if(  data == mPosition )
			mNode->setLocalPosition(mPosition);
		else if( data == mScale )
			mNode->setLocalScale(mScale);
		else if( data == mRotation )
			mNode->setLocalRotation(mRotation);
		else if( data == mBounding )
		{

		}
		else
		{
			assert(false);
		}
	}

	/************************************************************************/
	/* IGeometry interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::setGlobalPosition(const Vector3& pos)
	{
		mPosition = pos;	//node update has delay, update parallel state immediately. TODO:
		mNode->setWorldPosition(pos);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::setGlobalRotation(const Quaternion& rotation)
	{
		mRotation = rotation;	//node update has delay, update parallel state immediately. TODO:
		mNode->setWorldRotation(rotation);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::setGlobalScale(const Vector3& scale)
	{
		mScale = scale;	//node update has delay, update parallel state immediately. TODO:
		mNode->setWorldScale(scale);
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		GeometryElement::getGlobalPosition() const
	{
		return mNode->getWorldPosition();
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&	GeometryElement::getGlobalRotation() const
	{
		return mNode->getWorldRotation();
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		GeometryElement::getGlobalScale() const
	{
		return mNode->getWorldScale();
	}

	//////////////////////////////////////////////////////////////////////////
	const AABB&	GeometryElement::getLocalBounding() const
	{
		return mBounding;
	}

	/************************************************************************/
	/* IGeometryElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	INode*				GeometryElement::getGeometryNode() const
	{
		return mNode;
	}

	//////////////////////////////////////////////////////////////////////////
	Vector3				GeometryElement::getDirection(const Vector3& DirDefinitionVec/* = Vector3::NEGATIVE_UNIT_Z*/,GEOM_SPACE gspace/* = GS_WORLD*/) const
	{
		if( gspace == GS_WORLD )
			return DirDefinitionVec * mNode->getWorldRotation();
		else
			return DirDefinitionVec * mNode->getLocalRotation();
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::setDirection(const Vector3& newDirection,GEOM_SPACE gspace/* = GS_WORLD*/,const Vector3& DirDefinitionVec/* = Vector3::NEGATIVE_UNIT_Z*/)
	{
		mNode->setDirection(newDirection,gspace,DirDefinitionVec);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::lookAt(const Vector3& target,GEOM_SPACE gspace/* = GS_WORLD*/,const Vector3& DirDefinitionVec/* = Vector3::NEGATIVE_UNIT_Z*/)
	{
		mNode->lookAt(target, gspace, DirDefinitionVec);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::setFixedYawAxis(bool useFixedAxis,const Vector3& FixedYawAxis/* = Vector3::UNIT_Y*/)
	{
		mNode->setFixedYawAxis(useFixedAxis, FixedYawAxis);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				GeometryElement::getFixedYawAxis(Vector3& axis) const
	{
		return mNode->getFixedYawAxis(axis);
	}

	//////////////////////////////////////////////////////////////////////////
	/************************************************************************/
	/* GeometryNode::Listener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::notifyPositionChange(GeometryNode* node)
	{
		assert( node == mNode );
		mPosition = node->getWorldPosition();
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::notifyScaleChange(GeometryNode* node)
	{
		assert( node == mNode );
		mScale = node->getWorldScale();
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryElement::notifyRotationChange(GeometryNode* node)
	{
		assert( node == mNode );
		mRotation = node->getWorldRotation();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GeometryElement::initialize(IScene* scene, ParaStateQueue* queue, GeometryUpdater* updater)
	{
		this->setScene(scene);
		mParallelStates.setQueue(queue);

		mPositionData = Vector3::ZERO;
		mScaleData = Vector3::UNIT_ALL;
		mRotationData = Quaternion::IDENTITY;

		mNode = BLADE_NEW GeometryNode(updater, 0, NULL);
		mNode->addListener(this);
	}

	//////////////////////////////////////////////////////////////////////////
	GeometryNode*	GeometryElement::getNode() const
	{
		return mNode;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GeometryElement::onConfigChange(void* data)
	{
		if( data == &mPositionData )
		{
			this->setGlobalPosition(mPositionData);
		}
		else if( data == &mScaleData )
		{
			this->setGlobalScale(mScaleData);
		}
		else if( data == &mRotationData )
		{
			this->setGlobalRotation(mRotationData);
		}
	}
	
}//namespace Blade