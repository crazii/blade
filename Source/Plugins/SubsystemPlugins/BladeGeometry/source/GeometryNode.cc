/********************************************************************
	created:	2010/04/27
	filename: 	GeometryNode.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <GeometryNode.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GeometryNode::GeometryNode(INodeUpdater* updater,index_t depth,Node* parent/* = NULL*/)
		:Node(updater,depth,parent)
		,mUseFixedYawAxis(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GeometryNode::GeometryNode(INodeUpdater* updater,index_t depth,const TString& name,const Vector3& pos,const Vector3& scale,const Quaternion& rotation,Node* parent/* = NULL*/)
		:Node(updater,depth,name,pos,scale,rotation,parent)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GeometryNode::~GeometryNode()
	{
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void				GeometryNode::addListener(IListener* listener)
	{
		if( listener != NULL )
			mListenerList.insert(listener);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryNode::removeListener(IListener* listener)
	{
		if( listener != NULL )
			mListenerList.erase(listener);
	}


	//////////////////////////////////////////////////////////////////////////
	void				GeometryNode::setDirection(const Vector3& direction,GEOM_SPACE tspace/* = GS_OBJECT*/,const Vector3& localDir/* = Vector3::NEGATIVE_UNIT_Z*/)
	{
		assert(tspace>=GS_OBJECT && tspace <= GS_WORLD);

		Vector3 destDir;
		if( mUseFixedYawAxis )
		{
			//project the direction to mFixedYawAxis perpendicular plane
			Vector3 xVec = mFixedYawAxis.crossProduct(direction);
			if( !xVec.isZero() )
				destDir = xVec.crossProduct(mFixedYawAxis);
			else
				destDir = direction;
		}
		else
			destDir = direction;

		Quaternion rotation;
		Quaternion::generateRotation(rotation, localDir, destDir);

		if( tspace == GS_WORLD )
			this->setWorldRotation(rotation);
		else
			this->setLocalRotation(rotation);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryNode::lookAt(const POINT3& target,GEOM_SPACE tspace/* = GS_OBJECT*/,const Vector3& localDir/* = Vector3::NEGATIVE_UNIT_Z*/)
	{
		assert(tspace >= GS_OBJECT && tspace <= GS_WORLD);
		this->setDirection(target - mPosition[tspace], tspace, localDir);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryNode::setFixedYawAxis(bool useFixedYawAxis,const Vector3& axis/* = Vector3::UNIT_Y*/)
	{
		mUseFixedYawAxis = useFixedYawAxis;
		if( mUseFixedYawAxis )
			mFixedYawAxis = axis;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				GeometryNode::getFixedYawAxis(Vector3& axis) const
	{
		if( mUseFixedYawAxis )
			axis = mFixedYawAxis;
		return mUseFixedYawAxis;
	}

	//////////////////////////////////////////////////////////////////////////
	void				GeometryNode::setGlobalPositionDirect(const Vector3& position)
	{
		mWorldPosition = position;
		if( mParent != NULL )
			mPosition = mParent->transformPositionWorldToLocal(position);
		else
			mPosition = mWorldPosition;
		this->requestUpdate();
	}

	/************************************************************************/
	/* Node Interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	Node*	GeometryNode::createChildImp(const TString& name,const Vector3& translation,const Quaternion& rotation,const Vector3& scale)
	{
		return BLADE_NEW GeometryNode(mUpdater,this->getDepthLevel()+1,name,translation,scale,rotation,this);
	}

	//////////////////////////////////////////////////////////////////////////
	Node*	GeometryNode::createChildImp(const Vector3& translation,const Quaternion& rotation,const Vector3& scale)
	{
		TString name;
		Node::generateName(name);
		return BLADE_NEW GeometryNode(mUpdater,this->getDepthLevel()+1,name,translation,scale,rotation,this);
	}

	//////////////////////////////////////////////////////////////////////////
	void	GeometryNode::destroyChildImp(Node* node)
	{
		BLADE_DELETE node;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GeometryNode::onDepthLevelChange()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void	GeometryNode::onAttachToParentNode()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void	GeometryNode::onDetachFromParentNode()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void	GeometryNode::onPositionChange()
	{
		std::for_each(mListenerList.begin(),mListenerList.end(),
			std::bind2nd( std::mem_fun(&IListener::notifyPositionChange),this)
			);
	}

	//////////////////////////////////////////////////////////////////////////
	void	GeometryNode::onRotationChange()
	{
		std::for_each(mListenerList.begin(),mListenerList.end(),
			std::bind2nd( std::mem_fun(&IListener::notifyRotationChange),this)
			);

	}

	//////////////////////////////////////////////////////////////////////////
	void	GeometryNode::onScaleChange()
	{
		std::for_each(mListenerList.begin(),mListenerList.end(),
			std::bind2nd( std::mem_fun(&IListener::notifyScaleChange),this)
			);

	}


	
	
}//namespace Blade