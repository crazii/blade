/********************************************************************
	created:	2010/05/07
	filename: 	DynamicGeometry.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "DynamicGeometry.h"

namespace Blade
{
	DynamicGeometry::DynamicGeometry()
		:mNode(NULL)
	{

	}

	DynamicGeometry::~DynamicGeometry()
	{
		delete mNode;
	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	DynamicGeometry::getType()
	{
		static const TString type = BTString("DynamicGeometryElement");
		return type;
	}

	//////////////////////////////////////////////////////////////////////////
	const ParaStateMap& DynamicGeometry::getSubjects()
	{
		return mStaticState.getSubjects();
	}

	//////////////////////////////////////////////////////////////////////////
	const ParaStateMap& DynamicGeometry::getObservers()
	{
		return mStaticState.getObservers();
	}

	/************************************************************************/
	/* IStaticGeometry interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				DynamicGeometry::setGlobalPosition(const Vector3& pos)
	{
		mNode->setPosition(pos,Node::NTS_WORLD);
	}

	//////////////////////////////////////////////////////////////////////////
	void				DynamicGeometry::setGlobalRotation(const Quaternion& rotation)
	{
		mNode->setRotation(rotation,Node::NTS_WORLD);
	}

	//////////////////////////////////////////////////////////////////////////
	void				DynamicGeometry::setGlobalScale(const Vector3& scale)
	{
		mNode->setScale(scale,Node::NTS_WORLD);
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		DynamicGeometry::getGlobalPosition() const
	{
		return mNode->getPosition(Node::NTS_WORLD);
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&	DynamicGeometry::getGlobalRotation() const
	{
		return mNode->getRotation(Node::NTS_WORLD);
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		DynamicGeometry::getGlobalScale() const
	{
		return mNode->getScale(Node::NTS_WORLD);
	}

	/************************************************************************/
	/* IDynamicGeometry interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		DynamicGeometry::attachTo(IDynamicGeometry* parent)
	{
		if( parent != NULL )
		{
			DynamicGeometry* concreateParent = static_cast<DynamicGeometry*>(parent);
			concreateParent->getNode()->attachChild(mNode);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		DynamicGeometry::detach()
	{
		mNode->deatchFromParent();
	}


	//manipulation geometry data in parent's space
	//if no parent attached,it is equivalent to global method

	//////////////////////////////////////////////////////////////////////////
	void				DynamicGeometry::setLocalPosition(const Vector3& pos)
	{
		mNode->setPosition(pos,Node::NTS_PARENT);
	}

	//////////////////////////////////////////////////////////////////////////
	void				DynamicGeometry::setLocalRotation(const Quaternion& rotation)
	{
		mNode->setRotation(rotation,Node::NTS_PARENT);
	}

	//////////////////////////////////////////////////////////////////////////
	void				DynamicGeometry::setLocallScale(const Vector3& scale)
	{
		mNode->setScale(scale,Node::NTS_PARENT);
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		DynamicGeometry::getLocalPosition() const
	{
		return mNode->getPosition(Node::NTS_PARENT);
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&	DynamicGeometry::getLocalRotation() const
	{
		return mNode->getRotation(Node::NTS_PARENT);
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		DynamicGeometry::getLocalScale() const
	{
		return mNode->getScale(Node::NTS_PARENT);
	}

	/************************************************************************/
	/* custom interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				DynamicGeometry::initialize(IGeometryStateQueue& queue)
	{
		mStaticState.initialize(queue);

		mNode = new GeometryNode(&queue,0,NULL);
	}


	//////////////////////////////////////////////////////////////////////////
	GeometryNode*		DynamicGeometry::getNode() const
	{
		return mNode;
	}
	
}//namespace Blade