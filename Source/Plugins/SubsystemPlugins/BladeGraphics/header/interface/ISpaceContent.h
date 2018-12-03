/********************************************************************
	created:	2016/03/24
	filename: 	ISpaceContent.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ISpaceContent_h__
#define __Blade_ISpaceContent_h__
#include <interface/ICamera.h>
#include <interface/ISpaceData.h>
#include <interface/public/graphics/SpaceQuery.h>

namespace Blade
{
	class ISpace;
	class IRenderQueue;
	class GraphicsElement;
	class ISpaceContent;

	/** @brief Content flags */
	/** @note these flags are tested only when content is inside the frustum\n
	for fixed force updating function in each frame,using the IUpdatable interface
	*/
	enum EContentUpdateFlag
	{
		CUF_NONE = 0x00000000,
		CUF_DEFAULT_VISIBLE = 0x00000001,	//enable frustum culling routine, if 0, frustum culling is skipped and content will NOT be drawn
		CUF_FORCE_VISIBLE = 0x00000002,		//skip frustum culling and force it visible, if 1, frustum culling is skipped and content will ALWAYS be drawn
		CUF_VISIBLE_UPDATE = 0x00000040,	//update relative camera if visible

		//runtime flags, may change during running
		CUF_HIDDEN		= 0x10000000,
		CUF_RUNTIME_MASK = 0xF0000000,
		CUF_FORCEWORD = 0xFFFFFFFF,
	};

	enum EContentSpaceFlag
	{
		CSF_NONE = 0x0,
		CSF_ELEMENT =		0x00000001,
		CSF_CONTENT =		0x00000002,
		CSF_CAMERA =		0x00000004,
		CSF_LIGHT =			0x00000008,
		CSF_INDEPENDENT =	0x00000010,	//indicates content should be put in independent space. usually for persistent contents, 
										//i.e. contents not affected by dynamic paging, or contents with null/infinite bounding
		CSF_ONSCREEN =		0x00000020,	//on screen content (no 3d geometry)
		CSF_DYNAMIC =		0x00000040,	//dynamic moving content

		//runtime flags, may change during running
		CSF_VIRTUAL =		0x10000000,	//disable query
		CSF_SHADOWCASTER =	0x20000000,

		CSF_ALL = 0xFFFFFFFF,
		CSF_RUNTIME_MASK = 0xF0000000,
	};

	class IVisibleContentSet
	{
	public:
		IVisibleContentSet()
		{
			mContentMask = (uint32)CSF_ALL;
			mCullingMask = FULL_APPFLAG;
		}
		virtual ~IVisibleContentSet() {}

		/** @brief  */
		virtual void	addVisibleContent(ISpaceContent* content, ICamera::VISIBILITY _visibility) = 0;

		/** @brief return uint32 because MSVC's RVO is not doing so */
		inline uint32	getContentMask() const { return mContentMask.getMask(); }
		/** @brief return uint32 because MSVC's RVO is not doing so */
		inline uint32	getCullingMask() const { return mCullingMask.getMask(); }

	protected:
		Mask	mContentMask;
		AppFlag	mCullingMask;
	};//class IVisibleContentSet

	class ISpaceContent
	{
	public:
		static const uint32 AAB_DIRTY_MASK = 0x0F;	//if any bit of mask in mDirtyFlag is set, updateWorldAABB() is called

	public:
		inline ISpaceContent()
			:mSpaceFlags(CSF_NONE)
			,mUpdateFlags(CUF_NONE)
			,mWorldAABB(BT_NULL)
			,mSpaceMask(0x80000000)	//ISpace::INVALID_PARTITION
			,mDirtyFlag(0)
			,mElement(NULL)
		{
			mSpace = NULL;
			mSpaceData = NULL; 
		}

		inline virtual ~ISpaceContent() {}

		/**
		@describe
		@param
		@return
		*/
		inline uint32		getSpaceFlags() const { return mSpaceFlags.getMask(); /*return uint32 because MSVC's RVO is not doing so */ }

		/**
		@describe
		@param
		@return
		*/
		inline void			setSpaceFlags(uint32 flags, bool raise)
		{
			assert((flags&(~CSF_RUNTIME_MASK)) == 0);
			if (raise)
				mSpaceFlags.raiseBits(flags);
			else
				mSpaceFlags.clearBits(flags);
		}

		/**
		@describe
		@param
		@return
		*/
		inline uint32		getUpdateFlags() const { return mUpdateFlags.getMask(); /*return uint32 because MSVC's RVO is not doing so */ }

		/**
		@describe
		@param
		@return
		*/
		inline void			setUpdateFlags(uint32 flags, bool raise)
		{
			assert((flags&(~CUF_RUNTIME_MASK)) == 0);
			if (raise)
				mUpdateFlags.raiseBits(flags);
			else
				mUpdateFlags.clearBits(flags);
		}
		/** @brief  */
		inline bool	isVisible() const { return !mUpdateFlags.checkBits(CUF_HIDDEN); }
		inline void setVisible(bool visible) { this->setUpdateFlags(CUF_HIDDEN, !visible); }

		/**
		@describe space partition mask used to boost locate the sub partition of a space
		@param
		@return
		*/
		inline const uint32&	getSpacePartitionMask() const { return mSpaceMask; }

		/**
		@describe
		@param
		@return
		*/
		inline void			setSpacePartitionMask(uint32 mask) { mSpaceMask = mask; }

		/**
		@describe dynamic flag is not runtime flag but is allowed to change during data-binding (editor, initial loading)
		@param
		@return
		*/
		inline bool		isDynamic() const { return (mSpaceFlags&CSF_DYNAMIC) != 0; }

		/**
		@describe 
		@param
		@return
		*/
		inline void		setDynamic(bool _dynamic)
		{
			if (_dynamic)
				mSpaceFlags.raiseBits(CSF_DYNAMIC);
			else
				mSpaceFlags.safeClearBits(CSF_DYNAMIC);
		}

		/**
		@describe get bounding in world space
		@note returned result maybe different references and DON'T keep it
		@param
		@return
		*/
		inline const AABB&		getWorldAABB() const
		{
			if (mDirtyFlag.checkBits(AAB_DIRTY_MASK))
				this->updateWorldAABB();
			return mWorldAABB; 
		}

		/**
		@describe
		@param
		@return
		*/
		inline GraphicsElement*		getElement() const { return mElement; }

		/**
		@describe get bounding in local space
		@param
		@return
		*/
		virtual const AABB&		getLocalAABB() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const Vector3&			getPosition() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const Quaternion&		getRotation() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&			getScale() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setPosition(const Vector3& pos) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setRotation(const Quaternion& rotation) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			setScale(const Vector3& scale) = 0;

		/**
		@describe add renderables to render queue
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void			updateRender(IRenderQueue* rqueue) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual	uint32			getAppFlag() const { return 0; }

		/**
		@describe this function will be called if content is visible and CUF_VISIBLE_UPDATE is set
		@param
		@return
		*/
		virtual void			visibleUpdate(const ICamera* current, const ICamera* /*main*/, const ICamera::VISIBILITY /*_visibility*/)
		{
			BLADE_UNREFERENCED(current);
		}

		/**
		@describe
		@param
		@return
		*/
		virtual	bool			queryNearestPoint(SpaceQuery& query, scalar& distance) const
		{
			BLADE_UNREFERENCED(distance);
			BLADE_UNREFERENCED(query);
			return false;
		}

		/**
		@describe
		@param
		@return
		*/
		virtual bool			intersectTriangles(SpaceQuery& query, POS_VOL contentPos = PV_INTERSECTED) const
		{
			BLADE_UNREFERENCED(query);
			BLADE_UNREFERENCED(contentPos);
			return false;
		}

		/** @brief  */
		inline ISpace*			getSpace() const { return mSpace; }

		/** @brief  */
		inline ISpaceData*		getSpaceData() const { return mSpaceData; }

	protected:
		/**
		@describe 
		@param
		@return
		*/
		virtual void			updateWorldAABB() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			notifySpaceChange() {}

		/** @brief  */
		inline void		copySpace(const ISpaceContent& src)
		{
			mSpace = src.getSpace();
		}

	protected:
		Mask			mSpaceFlags;
		Mask			mUpdateFlags;
		mutable AABB	mWorldAABB;
		uint32			mSpaceMask;	//space partition mask
		mutable Mask	mDirtyFlag;
		GraphicsElement*mElement;

	private:
		ISpace*		mSpace;
		ISpaceData*	mSpaceData;

		/** @brief  */
		inline void			setSpaceData(ISpaceData* data)
		{
			if (mSpaceData != data)
			{
				ISpaceData* oldData = mSpaceData;

				mSpaceData = data;
				if (oldData != NULL)
					oldData->onDetach(this);

				if (mSpaceData != NULL)
					mSpaceData->onAttach(this);
			}
		}
		friend class ISpace;
	};

}//namespace Blade

#endif//__Blade_ISpaceContent_h__