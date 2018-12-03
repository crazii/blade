/********************************************************************
	created:	2010/05/06
	filename: 	DefaultRenderQueue.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ExceptionEx.h>
#include <utility/Profiling.h>
#include <interface/public/IRenderable.h>
#include <Technique.h>
#include <MaterialInstance.h>
#include "RenderSortHelper.h"
#include "DefaultRenderQueue.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	DefaultRenderGroup::DefaultRenderGroup()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DefaultRenderGroup::~DefaultRenderGroup()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	size_t					DefaultRenderGroup::size() const
	{
		return mRenderOperations.size();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					DefaultRenderGroup::reserve(size_t capacity)
	{
		mRenderOperations.reserve(capacity);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void					DefaultRenderGroup::addRenderable(IRenderable* pRenderable)
	{
		RenderOperation rop;
		std::memset(&rop, 0, sizeof(rop));
		rop.renderable = pRenderable;

		{
			//BLADE_FA_PROFILING_FUNCTION();
#if MULTITHREAD_CULLING
			ScopedLock sl(mLock);
#endif
			mRenderOperations.push_back(rop);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					DefaultRenderGroup::addRenderables(const RenderOperation* renderables, size_t count)
	{
#if MULTITHREAD_CULLING
		ScopedLock sl(mLock);
#endif
		mRenderOperations.insert(mRenderOperations.end(), renderables, renderables + count);
	}

	//////////////////////////////////////////////////////////////////////////
	void					DefaultRenderGroup::clear()
	{
		mRenderOperations.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					DefaultRenderGroup::getROPArray(RenderOperation*& outArray)
	{
		size_t outCount;
		if (mRenderOperations.empty())
		{
			outArray = NULL;
			outCount = 0;
		}
		else
		{
			outArray = &mRenderOperations[0];
			outCount = mRenderOperations.size();
		}
		return outCount;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderGroup&			DefaultRenderGroup::copyFrom(const IRenderGroup& src)
	{
		const DefaultRenderGroup& src_concrete = static_cast<const DefaultRenderGroup&>(src);
		mRenderOperations = src_concrete.mRenderOperations;
		mVisibleBounding = src_concrete.mVisibleBounding;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	void			DefaultRenderGroup::finalize(IRenderQueue::EUsage usage, const POINT3& camPos, const Vector3& camDir)
	{
		mVisibleBounding.setNull();

		for (size_t i = 0; i < mRenderOperations.size(); ++i)
		{
			RenderOperation& op = mRenderOperations[i];
			IRenderable* r = op.renderable;
			op.material = r->getMaterial();
			op.geometry = &r->getGeometry();

			const AABB& aab = r->getWorldBounding();
			if (aab.isFinite())
				mVisibleBounding.merge(aab);

			const TextureState* ts = op.material->getTextureState();
			op.texture = ts->getTextureStageCount() > 0 ? ts->getTextureStage(0)->getTexture().ptr() : NULL;

			//hack hack
			if (usage == IRenderQueue::RQU_SHADOW)
			{
				op.viewDistance = 0;
				op.radius = 0;
			}
			else
			{
				//op.viewDistance = ((const Vector3&)r->getWorldTransform().getTranslation() - camPos).dotProduct(camDir);
				if (aab.isFinite())
				{
					op.viewDistance = (aab.getCenter() - camPos).dotProduct(camDir);
					op.radius = aab.getHalfSize().getLength();
				}
				else
				{
					op.viewDistance = 0;
					op.radius = aab.isNull() ? 0 : 1e5f;
				}
			}
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	DefaultRenderQueue::DefaultRenderQueue()
		:mRenderTypeBinding( BTString("UNDEFINED") )
		,mGroupCount(0)
		,mUsage(RQU_SCENE)
		,mIndex(0)
	{
		mDynamic = false;
	}

	//////////////////////////////////////////////////////////////////////////
	DefaultRenderQueue::~DefaultRenderQueue()
	{
		this->clear();
	}

	/************************************************************************/
	/* IRenderQueue interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				DefaultRenderQueue::initialize(const RenderType& renderType)
	{
		mRenderTypeBinding = renderType.getName();
		if( renderType.getMaterial() == NULL )
		{
			mGroupCount = 1;
			mDynamic = true;
		}
		else
			mGroupCount = std::max<size_t>(1u, renderType.getMaterial()->getMaxGroupCount());
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		DefaultRenderQueue::getBoundRenderType() const
	{
		return mRenderTypeBinding;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			DefaultRenderQueue::getGroupCount() const
	{
		return mGroupList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderGroup*	DefaultRenderQueue::getRenderGroup(index_t index) const
	{
		if(index < mGroupList.size() )
			return mGroupList[index];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			DefaultRenderQueue::addRenderable(IRenderable* renderable)
	{
		const MaterialInstance* matInst = renderable->getMaterial();
		if( matInst == NULL )
			return true;
		size_t index = matInst->getActiveShaderGroup();

		if(index < mGroupList.size())
			mGroupList[index]->addRenderable(renderable);
		else
		{
			if( mDynamic )
			{
				//the branch chance is very low, lock the whole queue is ok.
#if MULTITHREAD_CULLING
				ScopedLock sl(mLock);
#endif
				size_t count = mGroupList.size();
				if(count <= index)
					mGroupList.resize(index+1);

				for (size_t i = count; i < mGroupList.size(); ++i)
				{
					if (mGroupList[i] == NULL)
						mGroupList[i] = BLADE_NEW DefaultRenderGroup();
				}
				mGroupList[index]->addRenderable(renderable);
			}
			else
			{
				assert(false);
				return false;
			}
		}
		
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			DefaultRenderQueue::addRenderable(const RenderType& /*type*/, IRenderable* renderable)
	{
		return this->addRenderable(renderable);
	}

	//////////////////////////////////////////////////////////////////////////
	void	DefaultRenderQueue::sort(IRenderSorter* sorter)
	{
		assert(sorter != NULL);
		RenderSortHelper::setSorter(sorter);
		for(size_t j = 0; j < mGroupList.size(); ++j)
		{
			RenderOperation* rops = NULL;
			size_t count = mGroupList[j]->getROPArray(rops);
			if( count > 1 )
				std::qsort(rops, count, sizeof(RenderOperation), &RenderSortHelper::compare);
		}
		RenderSortHelper::clearSorter();
	}

	//////////////////////////////////////////////////////////////////////////
	void			DefaultRenderQueue::clear()
	{
		for (size_t i = 0; i < mGroupList.size(); ++i)
		{
			BLADE_DELETE mGroupList[i];
		}
		mGroupList.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void			DefaultRenderQueue::reset()
	{
		mGroupList.reserve(4);
		if (mGroupCount > 0)
		{
			mGroupList.resize(mGroupCount);
			for (size_t i = 0; i < mGroupList.size(); ++i)
			{
				if (mGroupList[i] == NULL)
					mGroupList[i] = BLADE_NEW DefaultRenderGroup();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			DefaultRenderQueue::isEmpty() const
	{
		for(size_t j = 0; j < mGroupList.size(); ++j)
		{
			DefaultRenderGroup* group = mGroupList[j];
			if( group->size() != 0 )
				return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			DefaultRenderQueue::ensureGroups(size_t count)
	{
		if (mGroupCount < count)
		{
			mGroupCount = count;
			this->reset();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			DefaultRenderQueue::countGroups(const uint8* groups, size_t groupCount) const
	{
		assert(groupCount <= mGroupCount );

		size_t count = 0;
		for(size_t n = 0; n < groupCount; ++n)
		{
			uint8 group = groups[n];
			if(group >= mGroupCount )
			{
				assert(false);
				continue;
			}
			count += mGroupList[group]->size();
		}
		return count;
	}

	//////////////////////////////////////////////////////////////////////////
	AABB			DefaultRenderQueue::finalize(const POINT3& camPos, const Vector3& camDir)
	{
		AABB aab;
		for (size_t i = 0; i < mGroupList.size(); ++i)
		{
			mGroupList[i]->finalize((IRenderQueue::EUsage)mUsage, camPos, camDir);
			aab.merge(mGroupList[i]->getVisibleBounding());
		}
		return aab;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			DefaultRenderQueue::addRenderOP(const RenderOperation& rop)
	{
		size_t index = rop.material->getActiveShaderGroup();
		if (index < mGroupList.size())
			mGroupList[index]->addRenderables(&rop, 1);
		else
		{
			if (mDynamic)
			{
#if MULTITHREAD_CULLING
				ScopedLock sl(mLock);
#endif
				size_t count = mGroupList.size();
				if (count <= index)
					mGroupList.resize(index + 1);

				for (size_t i = count; i < mGroupList.size(); ++i)
				{
					if (mGroupList[i] == NULL)
						mGroupList[i] = BLADE_NEW DefaultRenderGroup();
				}
				mGroupList[index]->addRenderables(&rop, 1);
			}
			else
			{
				assert(false);
				return false;
			}
		}

		return true;
	}

}//namespace Blade