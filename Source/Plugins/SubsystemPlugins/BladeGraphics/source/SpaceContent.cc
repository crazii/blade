/********************************************************************
	created:	2010/04/28
	filename: 	SpaceContent.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <SpaceContent.h>
#include <interface/ISpace.h>
#include <Element/GraphicsElement.h>

namespace Blade
{
	//static const size_t	SIZE1 = sizeof(SpaceContentBase);
	//static const size_t	SIZE2 = sizeof(SpaceContent);

	namespace Impl
	{
		class DataMap : public TStringMap<ISpaceData*>,public Allocatable{};
	}//namespace Impl
	using namespace Impl;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	SpaceContentBase::SpaceContentBase()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	SpaceContentBase::~SpaceContentBase()
	{
		ISpace* space = this->getSpace();
		if (space != NULL)
		{
			bool ret = space->removeContent(this);
			assert(ret); BLADE_UNREFERENCED(ret);
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	SpaceContent::SpaceContent()
		:mPosition(Vector3::ZERO)
		,mRotation(Quaternion::IDENTITY)
		,mScale(Vector3::UNIT_ALL)
		,mLocalAABB(BT_NULL)
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	SpaceContent::~SpaceContent()
	{

	}

	/************************************************************************/
	/* ISpaceContent interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const Vector3&			SpaceContent::getPosition() const
	{
		return mPosition;
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&		SpaceContent::getRotation() const
	{
		return mRotation;
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&			SpaceContent::getScale() const
	{
		return mScale;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void					SpaceContent::setPosition(const Vector3& pos)
	{
		mPosition = pos;
		mDirtyFlag.raiseBits(DF_POSITION);
		this->notifyPositionChange();
	}

	//////////////////////////////////////////////////////////////////////////
	void					SpaceContent::setRotation(const Quaternion& rotation)
	{
		mRotation = rotation;
		mDirtyFlag.raiseBits(DF_ROTATION);
		this->notifyRotationChange();
	}

	//////////////////////////////////////////////////////////////////////////
	void					SpaceContent::setScale(const Vector3& scale)
	{
		mScale = scale;
		mDirtyFlag.raiseBits(DF_SCALE);
		this->notifyScaleChange();
	}

	//////////////////////////////////////////////////////////////////////////
	void			SpaceContent::updateWorldAABB() const
	{
		mDirtyFlag &= (uint32)(~DF_MASK);

		//skip matrix generation
		if (mLocalAABB.isNull() || mLocalAABB.isInfinite())
			mWorldAABB = mLocalAABB;
		else
		{
			Matrix44 xform;
			Matrix44::generateTransform(xform, mPosition, mScale, mRotation);
			mWorldAABB = mLocalAABB*xform;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const AABB&	SpaceContent::getLocalAABB() const
	{
		return mLocalAABB;
	}

	//////////////////////////////////////////////////////////////////////////
	void					SpaceContent::setLocalAABB(const AABB& localAABB, bool notify/* = true*/)
	{
		if( (mLocalAABB.isNull() && localAABB.isNull())
			||
			(mLocalAABB.isInfinite() && localAABB.isInfinite()) )
			return;

		mLocalAABB = localAABB;
		mDirtyFlag.raiseBits(DF_LOCAL_AABB);
		if(notify)
			this->notifyLocalBoundsChange();
	}

	//////////////////////////////////////////////////////////////////////////
	void			SpaceContent::notifyPositionChange()
	{
		if (mElement != NULL)
			static_cast<GraphicsElement*>(mElement)->dispatchPositionChanges(mPosition);
	}

	//////////////////////////////////////////////////////////////////////////
	void			SpaceContent::notifyRotationChange()
	{
		if (mElement != NULL)
			static_cast<GraphicsElement*>(mElement)->dispatchRotationChanges(mRotation);
	}

	//////////////////////////////////////////////////////////////////////////
	void			SpaceContent::notifyScaleChange()
	{
		if (mElement != NULL)
			static_cast<GraphicsElement*>(mElement)->dispatchScaleChanges(mScale);
	}

	//////////////////////////////////////////////////////////////////////////
	void			SpaceContent::notifyLocalBoundsChange()
	{
		if (mElement != NULL)
			static_cast<GraphicsElement*>(mElement)->dispatchLocalBoundsChanges(mLocalAABB);
	}
	
}//namespace Blade