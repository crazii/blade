/********************************************************************
	created:	2010/05/07
	filename: 	ISpace.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ISpace_h__
#define __Blade_ISpace_h__
#include <BladeGraphics.h>
#include <math/AxisAlignedBox.h>
#include <math/Ray.h>
#include <Frustum.h>
#include <Factory.h>
#include <utility/Mask.h>
#include <interface/IPage.h>
#include <interface/public/graphics/SpaceQuery.h>
#include <interface/ISpaceContent.h>

namespace Blade
{
	class ISpaceCoordinator;

	enum ESpaceFlag
	{
		SF_VISIBLE			= 0x01,	//enable for rendering, default:1
		SF_SOLID			= 0x02,	//enable space query, default:1
		SF_SEPARATED		= 0x04,	//separated, default:0
		SF_DYNAMIC			= 0x08,	//dynamic: update is needed

		//
		SF_EMPTY			= 0x10000000,	//set by framework
		SF_DEFAULT = SF_VISIBLE|SF_SOLID,
	};

	/*
		Async behavior/spec of ISpace:
		1. a space should be read only on TS_ASYNC_RUN (async run state), so that any other tasks from other subsystem can query the space [shared read].
		2. a space can update its hierarchy when it's dynamic space. updating process happens in TS_ASYNC_UPDATE (async update state), at this time the space is writable.
		3. content should be added/removed at TS_ASYNC_UPDATE state.
	*/
	class BLADE_GRAPHICS_API ISpace
	{
	public:
		static const uint32 INVALID_PARTITION = 0x80000000; //1 bit
		static const uint32 EXTRA_PARTITION_BITCOUNT = IPage::VISIBLE_PAGE_BITCOUNT*2+1;			//+INVALID_PARTITION bit
		static const uint32 PARTITION_ENDBIT_COUNT = 1;
		static const uint32 PARTITION_BIT_COUNT = 32 - EXTRA_PARTITION_BITCOUNT;					//1 end bit contained here
		static const uint32 PARTITION_MASK = (1 << (PARTITION_BIT_COUNT)) - 1;
		static const uint32 EXTRA_PARTITION_MASK = ~PARTITION_MASK;
		static const uint32 SUB_PARTITION_SHIFT = 3;												//3 bits for octree, compatible for quadtree
		static const uint32 SUB_PARTITION_MASK = (1 << SUB_PARTITION_SHIFT) - 1;
		static const uint32 SUB_PARTITION_ROOTEND = 1;												//end bit for root partition
		static const uint32 SUB_PARTITION_END = SUB_PARTITION_MASK + 1;								//floating end bit
		static const uint32 MAX_PARTITION_LEVEL = (PARTITION_BIT_COUNT - PARTITION_ENDBIT_COUNT) / SUB_PARTITION_SHIFT;	//root doesn't need partition mask, so add one extra for depth
		//| EXTRA_PARTITION_MASK | PARTITION_MASK |
		//| EXTRA_PARTITION_MASK | 7 | 7 | .. | 7 |
		//a space should never care about EXTRA_PARTITION_MASK
		//a space doesn't support partition don't care about all masks

		//////////////////////////////////////////////////////////////////////////
		static inline uint32 generatePartitionMask(int index, size_t level, uint32 parentMask)
		{
			assert(level > 0 && level <= MAX_PARTITION_LEVEL);
			assert((uint32)index < SUB_PARTITION_MASK);
			uint32 levelIndex = (uint32)level - 1;
			//remove end bit for parent
			uint32 endbit = 1u << (levelIndex * SUB_PARTITION_SHIFT);
			parentMask &= (~endbit) | EXTRA_PARTITION_MASK;
			//current endbit
			endbit <<= SUB_PARTITION_SHIFT;
			uint32 mask = (index << (levelIndex * SUB_PARTITION_SHIFT)) | endbit | parentMask;
			assert((mask&EXTRA_PARTITION_MASK) == (parentMask&EXTRA_PARTITION_MASK));
			return mask;
		}

		//////////////////////////////////////////////////////////////////////////
		static inline bool matchPartition(uint32 spacePartition, uint32 contentPartition)
		{
			return (spacePartition&PARTITION_MASK) == contentPartition;
		}

		//////////////////////////////////////////////////////////////////////////
		static inline int extractIndex(uint32 partitionMask, size_t level)
		{
			assert(level > 0 && level <= MAX_PARTITION_LEVEL);
			uint32 levelIndex = (uint32)level - 1;
			return int((partitionMask >> (levelIndex*SUB_PARTITION_SHIFT))&SUB_PARTITION_MASK);
		}

	public:
		virtual ~ISpace()		{}

		/*
		@describe get the space type
		@param
		@return
		*/
		virtual const TString&			getSpaceType() const = 0;

		/*
		@describe 
		@param [in] maxBound: max limit of the whole space.
		@param [in] maxLevel: sub space divide level.
		@return 
		@remarks: maxBound & maxLevel are just hint for the implementation, they may not be used.
		TODO: use implementation defined config via IConfig interface.
		*/
		virtual bool					initialize(ISpaceCoordinator* coorinator, const AABB& maxBound, uint32 partitionMask) = 0;

		/*
		@describe get coordinator of the space
		@param
		@return
		*/
		virtual ISpaceCoordinator*		getCoordinator() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual uint32					getSpaceFlag() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void					setSpaceFlag(uint32 flag) = 0;

		inline void raiseSpaceFlag(uint32 flags)
		{
			Mask flag = this->getSpaceFlag();
			flag.raiseBits(flags);
			this->setSpaceFlag(flag.getMask());
		}

		inline void clearSpaceFlag(uint32 flags)
		{
			Mask flag = this->getSpaceFlag();
			flag.raiseBits(flags);
			this->setSpaceFlag(flag.getMask());
		}

		/**
		@describe update space internal hierarchy if content moved
		@param
		@return
		*/
		virtual void					update() = 0;

		/*
		@describe get space bounds
		@param
		@return
		*/
		virtual const AABB&				getSpaceBound() const = 0;

		/*
		@describe directly set the space bound, maybe invalid operation and return false on some implementation.
		@param
		@return
		*/
		virtual bool					setSpaceBound(const AABB& box) = 0;

		/**
		@describe add content using partition info.
		@param [in] partitionMask: used for fast locating partition, space should mask out EXTRA_PARITION_MASK by using PARTITION_MASK
		@param [in] strictFit: check content's size against space's internal cell size, should be exactly match, or return false.
		@return
		*/
		virtual bool					addContent(ISpaceContent* content, uint32 partitionMask = ISpace::INVALID_PARTITION, bool strictFit = false) = 0;

		/*
		@describe
		@param 
		@return
		*/
		virtual bool					removeContent(ISpaceContent* content) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool					notifyContentChanged(ISpaceContent* content) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void					findVisibleContent(const Frustum& frustum, IVisibleContentSet& vcs) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool					queryContents(SpaceQuery& query, EContentSpaceFlag flag, scalar& distLimit) const = 0;

		/**
		@describe clear all contents
		@param
		@return
		*/
		virtual void					clearContents() = 0;

		/**
		@describe move space to another position, used for page switching
		@param
		@return
		*/
		virtual void					translateEmptySpaceImpl(const Vector3& center) = 0;

		/** @brief  */
		inline void					translateEmptySpace(const Vector3& center)
		{
			assert(this->getSpaceFlag()&SF_EMPTY);
			this->translateEmptySpaceImpl(center);
		}
	protected:

		/** @brief  */
		inline void adoptContent(ISpaceContent* content)
		{
			assert(content != NULL);
			assert(content->mSpace == this || content->mSpace == NULL);
			content->mSpace = this;
			content->notifySpaceChange();
		}

		/** @brief  */
		inline static void clearSpace(ISpaceContent* content, bool notify)
		{
			assert(content != NULL);
			content->mSpace = NULL;
			if (notify)
				content->notifySpaceChange();
		}

		/** @brief  */
		inline static void setSpaceData(ISpaceContent* content, ISpaceData* spaceData)
		{
			assert(content != NULL);
			content->setSpaceData(spaceData);
		}


	};//class ISpace

	extern template class BLADE_GRAPHICS_API Factory<ISpace>;
	typedef Factory<ISpace> SpaceFactory;
	
}//namespace Blade


#endif //__Blade_ISpace_h__