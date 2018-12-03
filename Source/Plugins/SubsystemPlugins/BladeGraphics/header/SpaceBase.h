/********************************************************************
	created:	2010/04/26
	filename: 	SpaceBase.h
	author:		Crazii
	
	purpose:	optional space base class
*********************************************************************/
#ifndef __Blade_SpaceBase_h__
#define __Blade_SpaceBase_h__
#include <BladeGraphics.h>
#include <interface/ISpace.h>
#include <Element/GraphicsElement.h>

namespace Blade
{
	class ISpaceCoordinator;

	class BLADE_GRAPHICS_API SpaceBase : public ISpace
	{
	public:
		SpaceBase(const TString& type);
		virtual ~SpaceBase();

		/************************************************************************/
		/* ISpace interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const TString&			getSpaceType() const;

		/*
		@describe get coordinator of the space
		@param
		@return
		*/
		virtual ISpaceCoordinator*		getCoordinator() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual uint32					getSpaceFlag() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void					setSpaceFlag(uint32 flag);
		
		/*
		@describe add content to space
		@param [in] strictFit: check content's size against space's internal cell size, should be exactly match, or return false.
		@return
		*/
		virtual bool					addContent(ISpaceContent* content, uint32 partitionMask = ISpace::INVALID_PARTITION, bool strictFit = false);

		/*
		@describe
		@param [in] content's spaceData if has any
		@return
		*/
		virtual bool					removeContent(ISpaceContent* content);

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/**
		@describe atomic content query
		@param
		@return whether to continue query
		*/
		static BLADE_ALWAYS_INLINE bool					queryContent(SpaceQuery& query, ISpaceContent* content, EContentSpaceFlag flag, scalar& distance)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

			if (!(content->getSpaceFlags()&flag) || (content->getSpaceFlags()&CSF_VIRTUAL))
				return true;

			uint32 appFlag = content->getAppFlag();
			if (appFlag == INVALID_APPFLAG || (appFlag&query.getAppFlag()) != appFlag)
				return true;

			scalar dist = distance + Math::LOW_EPSILON;
			POS_VOL cpos = query.intersect(content->getWorldAABB(), dist);
			if (cpos == PV_OUTSIDE)
				return true;

			if (query.needTriangles())
				content->intersectTriangles(query, cpos);

			IElement* elem = content->getElement();
			if (elem != NULL /* internal content may not have element i.e. ambient light */
				&& (!query.isQueringNearest() || dist <= distance))
			{
				if (query.isQueringNearest())
				{
					//use precise distance, bounding distance may blocks precise distance
					if (query.isPrecise())
					{
						scalar preciseDistance = distance;
						if (!(content->queryNearestPoint(query, preciseDistance) && preciseDistance < query.getDistanceLimit()))
							return true;
						distance = preciseDistance;
					}
					else
						distance = dist;
				}


				if (query.getCallback() == NULL || query.getCallback()->validate(elem, distance))
				{
					index_t index = query.find(elem);
					if (index == INVALID_INDEX)
						query.push_back(distance, elem);
					else if (query[index].distance > distance)
					{
						ScreenQuery::RESULT& result = query[index];
						result.distance = distance;
					}

					//callback
					if (query.getCallback() != NULL && !query.getCallback()->onQuery(elem, distance, query.size()))
						return false;
				}
			}
			return true;
		}

		/**
		@describe 
		@param
		@return
		*/
		static BLADE_ALWAYS_INLINE bool			cullContent(const Frustum& frustum, IVisibleContentSet& vcs, ISpaceContent* content, unsigned int frustumPlaneMask = Frustum::FPM_ALL)
		{
			Mask flags = content->getUpdateFlags();

			if (!flags.checkBits(CUF_FORCE_VISIBLE))
			{
				if (flags.checkBits(CUF_DEFAULT_VISIBLE) && !flags.checkBits(CUF_HIDDEN)
					&& (content->getSpaceFlags()&vcs.getContentMask()) != 0
					&& (vcs.getCullingMask() == FULL_APPFLAG || (content->getAppFlag()&vcs.getCullingMask()) != 0)
					)
				{
					ICamera::VISIBILITY _visibility;
					if ((_visibility = frustum.getVisibility(content->getWorldAABB(), &frustumPlaneMask)) != ICamera::CV_NULL)
						vcs.addVisibleContent(content, _visibility);
				}
				return false;
			}
			else
				vcs.addVisibleContent(content, ICamera::CV_ALL);
			return true;
		}

		static BLADE_ALWAYS_INLINE bool			addVisibleContent(IVisibleContentSet& vcs, ISpaceContent* content)
		{
			Mask flags = content->getUpdateFlags();

			if (flags.checkBits(CUF_DEFAULT_VISIBLE | CUF_FORCE_VISIBLE) && !flags.checkBits(CUF_HIDDEN)
				&& (content->getSpaceFlags()&vcs.getContentMask()) != 0
				&& (vcs.getCullingMask() == FULL_APPFLAG || (content->getAppFlag()&vcs.getCullingMask()) != 0))
				vcs.addVisibleContent(content, ICamera::CV_ALL);
			else
				return false;
			return true;
		}

	protected:
		/*
		@describe
		@param [in] content's spaceData if has any
		@return
		*/
		virtual bool				removeContentImpl(ISpaceContent* content) = 0;
		/*
		@describe add content to space
		@param [in] partitionMask: mask to boost up cell fitting
		@return
		*/
		virtual bool				addContentImpl(ISpaceContent* content, uint32 partitionMask, bool strictFit = false) = 0;

		TString					mType;
		Mask					mFlag;
		ISpaceCoordinator*		mCoordinator;
	};

}//namespace Blade


#endif //__Blade_SpaceBase_h__
