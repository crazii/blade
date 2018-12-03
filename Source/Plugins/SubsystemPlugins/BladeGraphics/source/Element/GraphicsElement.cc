/********************************************************************
	created:	2010/04/09
	filename: 	GraphicsElement.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/CommonState.h>
#include <interface/public/graphics/GraphicsState.h>
#include <interface/IEventManager.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <interface/public/graphics/IGraphicsSpaceCoordinator.h>
#include <Element/GraphicsElement.h>
#include <interface/IRenderScene.h>
#include <SpaceContent.h>
#include <interface/IAABBRenderer.h>
#include <interface/ISpace.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	namespace Impl
	{
		struct FnEffectSort
		{
		public:
			bool operator()(const HGRAPHICSEFFECT& lhs, const HGRAPHICSEFFECT& rhs) const
			{
				return FnTStringFastLess::compare(lhs->getType(), rhs->getType());
			}
		};
		class EffectListImpl : public Set<HGRAPHICSEFFECT, FnEffectSort>, public Allocatable 
		{
		public:
			Lock mSyncLock;
		};

		class CommandListImpl : public TempSet<GraphicsElementCommandSlot*>
		{
		public:
			Lock	mSyncLock;
		};
	}//namespace Impl
	using namespace Impl;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	GraphicsElement::GraphicsElement(const TString& type)
		:ElementBase(GraphicsConsts::GRAPHICS_SYSTEM_TYPE, type, PP_MIDDLE)
		,mContent(NULL)
		,mPosition(&mPositionData)
		,mRotation(&mRotationData)
		,mScale(&mScaleData)
		,mBounding(&mAABData)
		,mPartitionMask(ISpace::INVALID_PARTITION)
		,mDynamic(false)
		,mContentInSpace(false)
		,mUnloaded(false)
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	GraphicsElement::~GraphicsElement()
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		this->clearEffects();

		if (mCommandList != NULL)	//detach all commands
		{
			for (CommandListImpl::iterator i = mCommandList->begin(); i != mCommandList->end(); ++i)
				BLADE_DELETE *i;
		}
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				GraphicsElement::postProcess(const ProgressNotifier& notifier)
	{
		ElementBase::postProcess(notifier);
		if (mContent != NULL)
		{
			//invalidate partition mask when previous resource unloaded & new resource with different bounding is loaded
			const AABB& bounding = mContent->getLocalAABB();
			if (mUnloaded &&
				(!bounding.getMinPoint().equal(mAABData.getMinPoint(), Math::LOW_EPSILON)
				|| !bounding.getMaxPoint().equal(mAABData.getMaxPoint(), Math::LOW_EPSILON)))
			{
				mPartitionMask = ISpace::INVALID_PARTITION;
			}
			
			mBounding = bounding;
			this->activateContent();
		}
	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t	GraphicsElement::initParallelStates()
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		mParallelStates[CommonState::POSITION] = mPosition;
		mParallelStates[CommonState::ROTATION] = mRotation;
		mParallelStates[CommonState::SCALE] = mScale;
		mParallelStates[CommonState::BOUNDING] = mBounding;
		return mParallelStates.size();
	}

	//////////////////////////////////////////////////////////////////////////
	void	GraphicsElement::onParallelStateChange(const IParaState& data)
	{
		if( mContent == NULL || mSpace == NULL )
			return;

		if( data == mPosition )
			mContent->setPosition( mPosition );
		else if( data == mRotation )
			mContent->setRotation( mRotation );
		else if (data == mScale)
			mContent->setScale(mScale);
		else
			return;
	}

	/************************************************************************/
	/* ElementBase interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void GraphicsElement::onResourceUnload()
	{
		mUnloaded = true;
		this->deactivateContent();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GraphicsElement::addEffect(const HGRAPHICSEFFECT& effect)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		if( effect == NULL)
			return false;

		ScopedLock lock(mEffectList->mSyncLock);
		if (!mEffectList->insert(effect).second)
			return false;

		this->addCommand(BLADE_NEW AttachEffectCommand(effect));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GraphicsElement::removeEffect(const HGRAPHICSEFFECT& effect)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		if( effect == NULL)
			return false;
		ScopedLock lock(mEffectList->mSyncLock);
		if (mEffectList->erase(effect) != 1)
			return false;
		this->addCommand(BLADE_NEW DetachEffectCommand(effect));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	class EffectFinder : public IGraphicsEffect, public Handle<IGraphicsEffect>, public NonAssignable
	{
	public:
		EffectFinder(const TString& type)	:IGraphicsEffect(type)	{mPtr = this;}
		~EffectFinder()												{mPtr = NULL;}
		virtual bool	onAttach()		{return true;}
		virtual bool	onDetach()		{return true;}
		virtual bool	isReady() const	{return true;}
	};

	const HGRAPHICSEFFECT& GraphicsElement::getEffect(const TString& type) const
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		if( mEffectList == NULL )	//prevent lazy construction
			return HGRAPHICSEFFECT::EMPTY;

		EffectFinder finder(type);
		EffectListImpl::const_iterator i = mEffectList->find(finder);
		if( i == mEffectList->end() )
			return HGRAPHICSEFFECT::EMPTY;
		else
			return *i;
	}

	//////////////////////////////////////////////////////////////////////////
	void GraphicsElement::setVisible(bool visible)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		if (mContent != NULL)
			static_cast<SpaceContent*>(mContent)->setVisible(visible);
	}

	//////////////////////////////////////////////////////////////////////////
	void GraphicsElement::enablePicking(bool enable)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		if (mContent != NULL)
			static_cast<SpaceContent*>(mContent)->setSpaceFlags(CSF_VIRTUAL, !enable);
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&	GraphicsElement::getStaticPosition() const
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		assert(mContent != NULL && !(mContent->getSpaceFlags()&CSF_DYNAMIC) && "dynamic element need use geometry to access data");
		return mPositionData;
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&	GraphicsElement::getStaticRotation() const
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		assert(mContent != NULL && !(mContent->getSpaceFlags()&CSF_DYNAMIC) && "dynamic element need use geometry to access data");
		return mRotationData;
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&	GraphicsElement::getStaticScale() const
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		assert(mContent != NULL && !(mContent->getSpaceFlags()&CSF_DYNAMIC) && "dynamic element need use geometry to access data");
		return mScaleData;
	}

	//////////////////////////////////////////////////////////////////////////
	const AABB& GraphicsElement::getStaticLocalBounds() const
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
		assert(mContent != NULL && !(mContent->getSpaceFlags()&CSF_DYNAMIC) && "dynamic element need use geometry to access data");
		return mAABData;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsElement::showBoundingVolume(bool show, const Color& color/* = Color::WHITE*/)
	{
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		ISpaceContent* content = this->getContent();
		if (content == NULL)
			return false;

		IRenderScene* scene = this->getRenderScene();
		assert(scene != NULL);
		if (show)
		{
			if (!scene->getAABBRenderer()->addAABB(content, color))
				scene->getAABBRenderer()->changeAABBColor(content, color);
		}
		else
			scene->getAABBRenderer()->removeAABB(content);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void		GraphicsElement::initialize(IRenderScene* scene, ISpace* initSpace)
	{
		assert( scene != NULL && scene->getSceneType() == IRenderScene::GRAPHICS_SCENE_TYPE );

		this->setScene(scene);
		mSpace = initSpace;

		ParaStateQueue* queue = scene->getStateQueue();
		mParallelStates.setQueue(queue);

		mAABData.setInfinite();
		mPositionData = Vector3::ZERO;
		mScaleData = Vector3::UNIT_ALL;
		mRotationData = Quaternion::IDENTITY;

		this->onInitialize();

		if( mContent != NULL )
			mBounding = mContent->getLocalAABB();
	}

	//////////////////////////////////////////////////////////////////////////
	ISpaceContent*		GraphicsElement::getContent() const
	{
		return mContent;
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		GraphicsElement::getPosition() const
	{
		return mPosition;
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&		GraphicsElement::getScale() const
	{
		return mScale;
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&	GraphicsElement::getRotation() const
	{
		return mRotation;
	}

	//////////////////////////////////////////////////////////////////////////
	void				GraphicsElement::setPosition(const Vector3& pos)
	{
		mPosition.setUnTouched( pos );
	}

	//////////////////////////////////////////////////////////////////////////
	void				GraphicsElement::setRotation(const Quaternion& rotation )
	{
		mRotation.setUnTouched( rotation );
	}

	//////////////////////////////////////////////////////////////////////////
	void				GraphicsElement::setScale(const Vector3& vscale)
	{
		mScale.setUnTouched( vscale );
	}

	//////////////////////////////////////////////////////////////////////////
	bool				GraphicsElement::dispatchPositionChanges(const Vector3& /*pos*/)
	{
		this->notifyContentChange();

		bool ret = true;
		for (EffectListImpl::iterator i = mEffectList->begin(); i != mEffectList->end(); ++i)
			ret = this->dispatchPositionChange(*i) && ret;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				GraphicsElement::dispatchRotationChanges(const Quaternion& /*rotation*/)
	{
		this->notifyContentChange();

		bool ret = true;
		for (EffectListImpl::iterator i = mEffectList->begin(); i != mEffectList->end(); ++i)
			ret = this->dispatchRotationChange(*i) && ret;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				GraphicsElement::dispatchScaleChanges(const Vector3& /*scale*/)
	{
		this->notifyContentChange();

		bool ret = true;
		for (EffectListImpl::iterator i = mEffectList->begin(); i != mEffectList->end(); ++i)
			ret = this->dispatchScaleChange(*i) && ret;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				GraphicsElement::dispatchLocalBoundsChanges(const AABB& /*aab*/)
	{
		this->notifyContentChange();

		bool ret = true;
		for (EffectListImpl::iterator i = mEffectList->begin(); i != mEffectList->end(); ++i)
			ret = this->dispatchLocalBoundsChange(*i) && ret;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GraphicsElement::notifyContentChange()
	{
		mPosition = mContent->getPosition();
		mScale = mContent->getScale();
		mRotation = mContent->getRotation();
		mBounding = mContent->getLocalAABB();

		//FIXED: codes below will cause low performance on serialization ( Entity::postLoad, synchronization)
		//handle object move in Non dynamic spaces
		if (this->isContentActive())
		{
			mSpace->notifyContentChanged(mContent);
			mPartitionMask = mContent->getSpacePartitionMask();
			mDynamic = mContent->isDynamic();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	GraphicsElement::onConfigChange(void* data)
	{
		if (mContent == NULL)
			return;

		if (data == &mPositionData)
			mContent->setPosition(mPositionData);
		else if (data == &mRotationData)
			mContent->setRotation(mRotationData);
		else if (data == &mScaleData)
			mContent->setScale(mScaleData);
	}

	//////////////////////////////////////////////////////////////////////////
	const uint32&	GraphicsElement::getSpacePartitionMask(index_t) const
	{
		if (mContent != NULL)
		{
			if(mContent->getSpaceData() != NULL)
				assert(mContent->getSpaceData()->getPartitionMask() == mContent->getSpacePartitionMask());
			if ((mContent->getSpaceFlags()&CSF_DYNAMIC))
				mPartitionMask = mContent->getSpacePartitionMask();
			assert(mContent->getSpacePartitionMask() == mPartitionMask);
			return mContent->getSpacePartitionMask();
		}
		return mPartitionMask;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GraphicsElement::setSpacePartitionMask(index_t, const uint32& mask)
	{
		assert(!this->isContentActive());
		mPartitionMask = mask;
		if (mContent != NULL)
		{
			mContent->setSpacePartitionMask(mask);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	const bool&	GraphicsElement::getDynamic(index_t) const
	{
		if (mContent != NULL)
			mDynamic = mContent->isDynamic();
		return mDynamic;
	}

	//////////////////////////////////////////////////////////////////////////
	bool GraphicsElement::setDynamic(index_t, const bool& _dynamic)
	{
		assert(!this->isContentActive());
		mDynamic = _dynamic;
		if (mContent != NULL)
		{
			mContent->setDynamic(_dynamic);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GraphicsElement::activateContent()
	{
		if (!mContentInSpace)
		{
			mContent->setSpacePartitionMask(mPartitionMask);
			mSpace->addContent(mContent);
		}
		
		mPartitionMask = mContent->getSpacePartitionMask();
		assert(mPartitionMask != ISpace::INVALID_PARTITION);
		mContentInSpace = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void GraphicsElement::deactivateContent()
	{
		if (mContent->getSpace() != NULL)
		{
			if ((mContent->getSpaceFlags()&CSF_DYNAMIC))
				mPartitionMask = mContent->getSpacePartitionMask();

			assert(mContent->getSpacePartitionMask() == mPartitionMask);
			mContent->getSpace()->removeContent(mContent);
		}
		mContentInSpace = false;
	}

	//////////////////////////////////////////////////////////////////////////
	void GraphicsElement::clearEffects()
	{
		if (mEffectList != NULL)
		{
			for (EffectListImpl::iterator i = mEffectList->begin(); i != mEffectList->end(); ++i)
				this->detachEffect(*i);
			mEffectList->clear();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool GraphicsElement::addCommand(GraphicsElementCommand* cmd)
	{
		assert(cmd != NULL && cmd->getRef() == NULL);
		GraphicsElementCommandSlot* slot = BLADE_NEW GraphicsElementCommandSlot(this, cmd);
		this->getRenderScene()->getUpdater()->addForUpdateOnce(cmd);

		ScopedLock sl(mCommandList->mSyncLock);
		return mCommandList->insert(slot).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool GraphicsElement::finishCommand(GraphicsElementCommand* cmd)
	{
		if (mCommandList != NULL)
		{
			ScopedLock sl(mCommandList->mSyncLock);
			GraphicsElementCommandSlot* slot = static_cast<GraphicsElementCommandSlot*>(cmd->getRef());
			bool ret = mCommandList->erase(slot) == 1;
			BLADE_DELETE slot;
			return ret;
		}
		return true;
	}

}//namespace Blade
