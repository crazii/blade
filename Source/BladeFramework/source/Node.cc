/********************************************************************
	created:	2010/04/25
	filename: 	Node.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Node.h>
#include <ExceptionEx.h>


namespace Blade
{
	namespace Impl
	{
		class ChildNameMap : public TStringMap<Node*>, public Allocatable				{};
		class ChildIndexMap : public Map<index_t,Node*>, public Allocatable			{};		
	}//namespace Impl
	using namespace Impl;

	RefCount Node::msNameCounter(0);

	//////////////////////////////////////////////////////////////////////////
	Node::Node(INodeUpdater* updater,index_t depth,Node* parent/* = NULL*/)
		:mIndex( INVALID_INDEX )
		,mLevel(depth)
		,mUpdater(updater)
		,mParent(parent)
		,mChildMap( BLADE_NEW ChildNameMap() )
		,mChildList( BLADE_NEW ChildIndexMap() )
	{
		//assert( mUpdater != NULL );
		Node::generateName(mName);

		mMask.raiseBitAtIndex(NM_INHERIT_ROTATION);
		mMask.raiseBitAtIndex(NM_INHERIT_SCALE);

		mPosition = Vector3::ZERO;
		mScale = Vector3::UNIT_ALL;
		mRotation = Quaternion::IDENTITY;

		mWorldPosition= Vector3::ZERO;
		mWorldScale = Vector3::UNIT_ALL;
		mWorldRotation = Quaternion::IDENTITY;
	}

	//////////////////////////////////////////////////////////////////////////
	Node::Node(INodeUpdater* updater,index_t depth,const TString& name,const Vector3& pos,const Vector3& scale,const Quaternion& rotation,Node* parent/* = NULL*/)
		:mName(name)
		,mIndex(INVALID_INDEX)
		,mLevel(depth)
		,mUpdater(updater)	
		,mParent(parent)
		,mChildMap( BLADE_NEW ChildNameMap() )
		,mChildList( BLADE_NEW ChildIndexMap() )
	{
		mMask.raiseBitAtIndex(NM_INHERIT_ROTATION);
		mMask.raiseBitAtIndex(NM_INHERIT_SCALE);

		mPosition = pos;
		mScale = scale;
		mRotation = rotation;

		mWorldPosition= pos;
		mWorldScale = scale;
		mWorldRotation = rotation;
	}

	//////////////////////////////////////////////////////////////////////////
	Node::~Node()
	{
		if( mParent != NULL )
			mParent->INode::removeChild( this );

		//well virtuals are not called in dector.
		//call it from outside
		//destroyAllChildren();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		Node::getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t				Node::getIndex() const
	{
		return mIndex;
	}


	//////////////////////////////////////////////////////////////////////////
	index_t				Node::getDepthLevel() const
	{
		return mLevel;
	}

	//////////////////////////////////////////////////////////////////////////
	INode*				Node::getParent() const
	{
		return mParent;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::setParent(INode* parent)
	{
		if( mParent == NULL || parent == NULL )
		{
			mParent = static_cast<Node*>(parent);
			this->onAttachToParentNode();
		}
		else
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot attach to parent:already has a parent.") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::deatchFromParent()
	{
		if( mParent != NULL )
		{
			this->onDetachFromParentNode();
			mParent = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				Node::getChildCount() const
	{
		return mChildMap->size();
	}

	//////////////////////////////////////////////////////////////////////////
	INode*				Node::getChild(size_t index) const
	{
		ChildIndexMap::const_iterator i = mChildList->find( index );
		if( i == mChildList->end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	Node*				Node::createChild(const TString& name,const Vector3& translation/* = Vector3::ZERO*/,
		const Quaternion& rotation/* = Quaternion::IDENTITY*/,const Vector3& scale/* = Vector3::UNIT_ALL*/)

	{
		std::pair<ChildNameMap::iterator,bool> ret = mChildMap->insert( std::make_pair(name,static_cast<Node*>(NULL) ) );
		if( ret.second )
		{
			Node*& emptyChild = ret.first->second;
			try
			{
				emptyChild = this->createChildImp(name,translation,rotation,scale);
			}
			catch(...)
			{
				mChildMap->erase(ret.first);
				throw;
			}
			(*mChildList)[ mChildMap->size() - 1 ] = emptyChild;
			emptyChild->setIndex( mChildMap->size() - 1);
			this->requestUpdate();
			return emptyChild;
		}
		else
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a node name with the same name already exist:")+ name);
	}

	//////////////////////////////////////////////////////////////////////////
	Node*				Node::createChild(const Vector3& translation/* = Vector3::ZERO*/,
		const Quaternion& rotation/* = Quaternion::IDENTITY*/,const Vector3& scale/* = Vector3::UNIT_ALL*/)

	{
		Node* child = this->createChildImp(translation,rotation,scale);
		Node*& emptyChild = (*mChildMap)[ child->getName() ];
		if( emptyChild == NULL )
		{
			emptyChild = child;
			(*mChildList)[mChildMap->size() - 1 ] = emptyChild;
			emptyChild->setIndex( mChildMap->size() - 1);
			this->requestUpdate();
			return child;
		}
		else
		{
			const TString name = child->getName();
			BLADE_DELETE child;
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a node name with the same name already exist:")+name);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::attachChild(INode* child)
	{
		if( child == NULL )
			return;

		if( child->getParent() == NULL )
		{
			Node*& emptyChild = (*mChildMap)[ child->getName() ];
			if( emptyChild == NULL )
			{
				emptyChild = static_cast<Node*>(child);
				(*mChildList)[ mChildMap->size() - 1 ] = emptyChild;
				emptyChild->setIndex( mChildMap->size() - 1);
				child->setParent(this);
				emptyChild->setDepthLevel( this->getDepthLevel() + 1 );
				this->requestUpdate();
			}
			else
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a node name with the same name already exist:")+ child->getName());
		}
		else
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("cannot attach child that already has a parent.") );
	}

	//////////////////////////////////////////////////////////////////////////
	INode*				Node::removeChild(const TString& name)
	{
		ChildNameMap::iterator i = mChildMap->find( name );
		if( i != mChildMap->end() )
		{
			Node* node = i->second;
			if( mChildList->size() == mChildMap->size() )
			{
				mChildList->erase( node->getIndex() );
				mChildMap->erase(i);
				return node;
			}
			else
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("child index or list corrupted.") );
		}
		else
			BLADE_EXCEPT(EXC_NEXIST,BTString("cannot find a child named")+name );
	}

	//////////////////////////////////////////////////////////////////////////
	INode*				Node::removeChild(size_t index)
	{
		ChildIndexMap::iterator i = mChildList->find( index );
		if( i != mChildList->end() )
		{
			Node* node = i->second;

			if( node->getIndex() != i->first || mChildList->size() != mChildMap->size() )
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("child index or list corrupted.") );

			mChildList->erase( i );
			mChildMap->erase(node->getName() );
			return node;
		}
		else
			BLADE_EXCEPT(EXC_NEXIST,BTString("cannot find a child with the index")+TStringHelper::fromInt((int)index) );
	}


	//////////////////////////////////////////////////////////////////////////
	bool				Node::hasChild(const INode* node) const
	{
		index_t index = node->getIndex();
		ChildIndexMap::const_iterator i = mChildList->find( index );
		if( i == mChildList->end() || i->second != node )
			return false;
		else
			return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::destroyChild(const TString& name)
	{
		ChildNameMap::iterator n = mChildMap->find( name );
		if( n == mChildMap->end() )
			return;

		Node* node = n->second;
		ChildIndexMap::iterator i = mChildList->find( node->getIndex() );
		if( i != mChildList->end() )
		{
			mChildList->erase(i);
			mChildMap->erase(n);
			node->setParent(NULL);
			this->destroyChildImp(node);
		}
		else
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("child node data corrupted.") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::destroyChild(index_t index)
	{
		ChildIndexMap::iterator i = mChildList->find( index );
		if( i != mChildList->end() )
		{
			Node* node = i->second;
			ChildNameMap::iterator n = mChildMap->find( node->getName() );
			if( n != mChildMap->end() )
			{
				mChildList->erase(i);
				mChildMap->erase(n);
				node->setParent(NULL);
				this->destroyChildImp(node);
			}
			else
				BLADE_EXCEPT(EXC_UNKNOWN,BTString("child node data corrupted.") );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::destroyChild(Node* node)
	{
		if( node != NULL )
		{
			ChildIndexMap::iterator i = mChildList->find( node->getIndex() );
			if( i != mChildList->end() )
			{
				if( i->second == node )
				{
					ChildNameMap::iterator n = mChildMap->find( node->getName() );

					if( n != mChildMap->end() && n->second == node )
					{
						mChildList->erase(i);
						mChildMap->erase(n);
						node->setParent(NULL);
						this->destroyChildImp(node);
					}
					else
						BLADE_EXCEPT(EXC_UNKNOWN,BTString("child node data corrupted.") );
				}
				else
					BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("node is not a child") );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::destroyAllChildren()
	{
		if( mChildMap->size() == mChildList->size() )
		{
			for( ChildIndexMap::iterator i = mChildList->begin(); i != mChildList->end(); ++i )
			{
				Node* node = i->second;
				node->setParent(NULL);
				node->destroyAllChildren();
				this->destroyChildImp(node);
			}
			mChildList->clear();
			mChildMap->clear();
		}
		else
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("child index or list corrupted.") );
	}


	//////////////////////////////////////////////////////////////////////////
	void				Node::requestUpdate()
	{
		if( !mMask.checkBitAtIndex(NM_UPDATE) && mUpdater != NULL )
		{
			mUpdater->notifyNodeChange(this);
			mMask.raiseBitAtIndex(NM_UPDATE);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::update()
	{
		if( mParent != NULL )
		{
			if( this->Node::isRotationInherited() )
				mWorldRotation = mParent->transformRotationLocalToWorld(mRotation);
			else
				mWorldRotation = mRotation;

			if( this->Node::isScaleInherited() )
				mWorldScale = mParent->transformScaleLocalToWorld(mScale);
			else
				mWorldScale = mScale;

			mWorldPosition = mParent->transformPositionLocalToWorld(mPosition);
		}
		else
		{
			mWorldPosition = mPosition;
			mWorldScale = mScale;
			mWorldRotation = mRotation;
		}

		bool positionDirty = mMask.checkBitAtIndex(NM_POSITION_DIRTY);
		bool rotationDirty = mMask.checkBitAtIndex(NM_ROTATION_DIRTY);
		bool scaleDirty = mMask.checkBitAtIndex(NM_SCALE_DIRTY);

		if( positionDirty )
			this->onPositionChange();
		if( rotationDirty )
			this->onRotationChange();
		if( scaleDirty )
			this->onScaleChange();

		for( ChildIndexMap::iterator i = mChildList->begin(); i != mChildList->end(); ++i)
		{
			Node* node = i->second;
			//check every bits to reduce invalid listener calls
			if( scaleDirty && node->Node::isScaleInherited() )
				node->mMask.raiseBitAtIndex(NM_SCALE_DIRTY);
			if( rotationDirty && node->Node::isRotationInherited() )
				node->mMask.raiseBitAtIndex(NM_ROTATION_DIRTY);

			node->mMask.raiseBitAtIndex(NM_POSITION_DIRTY);

			node->update();
		}

		//clear all except NM_INHERIT_ROTATION (bit 0) & NM_INHERIT_SCALE (bit 1)
		mMask &= 0x3;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::setLocalRotation(const Quaternion& rotation)
	{
		mRotation = rotation;
		mMask.raiseBitAtIndex(NM_ROTATION_DIRTY);
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::setWorldRotation(const Quaternion& rotation)
	{
		if( mParent != NULL )
			mRotation = mParent->transformRotationWorldToLocal( rotation );
		else
			mRotation = rotation;
		mWorldRotation = rotation;
		mMask.raiseBitAtIndex(NM_ROTATION_DIRTY);
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::setRotationInherited(bool inherit)
	{
		if(inherit != mMask.checkBitAtIndex(NM_INHERIT_ROTATION))
		{
			mMask.updateBitAtIndex(NM_INHERIT_ROTATION, inherit);
			mMask.raiseBitAtIndex(NM_ROTATION_DIRTY);
			this->requestUpdate();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::setLocalPosition(const Vector3& position)
	{
		mPosition = position;
		mMask.raiseBitAtIndex(NM_POSITION_DIRTY);
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::setWorldPosition(const Vector3& position)
	{
		if( mParent != NULL )
			mPosition = mParent->transformPositionWorldToLocal(position);
		else
			mPosition = position;
		mWorldPosition = position;
		mMask.raiseBitAtIndex(NM_POSITION_DIRTY);
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::setLocalScale(const Vector3& scale)
	{
		mScale = scale;
		mMask.raiseBitAtIndex(NM_SCALE_DIRTY);
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::setWorldScale(const Vector3& scale)
	{
		if( mParent != NULL )
			mScale = mParent->transformScaleWorldToLocal(scale);
		else
			mScale = scale;
		mWorldScale = scale;
		mMask.raiseBitAtIndex(NM_SCALE_DIRTY);
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::setScaleInherited(bool inherit)
	{
		if(inherit != mMask.checkBitAtIndex(NM_INHERIT_SCALE))
		{
			mMask.updateBitAtIndex(NM_INHERIT_SCALE, inherit);
			mMask.raiseBitAtIndex(NM_SCALE_DIRTY);
			this->requestUpdate();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::scale(const Vector3& scale_factor)
	{
		mScale *= scale_factor;
		mMask.raiseBitAtIndex(NM_SCALE_DIRTY);
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::translate(const Vector3& trans, GEOM_SPACE tspace/* = GS_OBJECT*/)
	{
		switch(tspace)
		{
		case GS_OBJECT:
			// position is relative to parent so transform downwards
			mPosition += trans * mRotation;
			break;
		case GS_WORLD:
			// position is relative to parent so transform upwards
			if (mParent != NULL)
				mPosition += (trans * mParent->getWorldRotation().getInversedQuaternion()) / mParent->getWorldScale();
			else
				mPosition += trans;
			break;
		case GS_LOCAL:
			mPosition += trans;
			break;
		}
		mMask.raiseBitAtIndex(NM_POSITION_DIRTY);
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::yaw(scalar fAngleRadians, GEOM_SPACE tspace/* = GS_OBJECT*/)
	{
		if (Math::Equal(fAngleRadians, 0.0f, Math::HIGH_EPSILON))
			return;

		Quaternion rotation(Vector3::UNIT_Y, fAngleRadians);
		this->rotate(rotation,tspace);
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::pitch(scalar fAngleRadians, GEOM_SPACE tspace/* = GS_OBJECT*/)
	{
		if (Math::Equal(fAngleRadians, 0.0f, Math::HIGH_EPSILON))
			return;

		Quaternion rotation(Vector3::UNIT_X,fAngleRadians);
		this->rotate(rotation,tspace);
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::roll(scalar fAngleRadians, GEOM_SPACE tspace/* = GS_OBJECT*/)
	{
		if (Math::Equal(fAngleRadians, 0.0f, Math::HIGH_EPSILON))
			return;

		Quaternion rotation(Vector3::UNIT_Z,fAngleRadians);
		this->rotate(rotation,tspace);
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::rotate(const Vector3& axis, scalar fAngleRadians, GEOM_SPACE tspace/* = GS_OBJECT*/)
	{
		if (Math::Equal(fAngleRadians, 0.0f, Math::HIGH_EPSILON))
			return;

		Quaternion rotation(axis, fAngleRadians);
		this->rotate(rotation,tspace);
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::rotate(const Quaternion& rotation,GEOM_SPACE tspace/* = GS_OBJECT*/)
	{
		assert(tspace>=GS_OBJECT && tspace <= GS_WORLD);
		//TODO: normalize rotation?
		switch(tspace)
		{
		case GS_LOCAL:
			mRotation = mRotation * rotation;
			break;
		case GS_WORLD:
			mRotation =  mWorldRotation * rotation * mWorldRotation.getInversedQuaternion() * mRotation;
			break;
		case GS_OBJECT:
			mRotation = rotation * mRotation;
			break;
		}
		mMask.raiseBitAtIndex(NM_ROTATION_DIRTY);
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::reset(const Vector3& posiotion/* = Vector3::ZERO*/, const Vector3& scale/* = Vector3::UNIT_ALL*/, const Quaternion& rotation/* = Quaternion::IDENTITY*/)
	{
		mPosition = posiotion;
		mScale = scale;
		mRotation = rotation;
		this->requestUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Node::generateName(TString& name)
	{
		size_t counter = msNameCounter.safeIncrement();
		name = BTString("[node]auto_name") + TStringHelper::fromUInt( counter );
	}
	
}//namespace Blade