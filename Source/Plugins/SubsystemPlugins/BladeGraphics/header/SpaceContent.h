/********************************************************************
	created:	2010/04/27
	filename: 	SpaceContent.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_SpaceContent_h__
#define __Blade_SpaceContent_h__
#include <interface/ISpaceContent.h>

namespace Blade
{
	class Frustum;
	class IRenderQueue;

	///basic content without detail geometry, only world bounding for culling
	class BLADE_GRAPHICS_API SpaceContentBase : public ISpaceContent
	{
	public:
		SpaceContentBase();
		virtual ~SpaceContentBase();

	private:
		/************************************************************************/
		/* ISpaceContent interface                                                                     */
		/************************************************************************/
		/**
		@describe get the local AABB
		@param
		@return
		*/
		virtual const AABB&				getLocalAABB() const { assert(false); return AABB::EMPTY; }

		/**
		@describe
		@param
		@return
		*/
		virtual const Vector3&			getPosition() const { assert(false); return Vector3::ZERO; }

		/**
		@describe
		@param
		@return
		*/
		virtual const Quaternion&		getRotation() const { assert(false); return Quaternion::IDENTITY; }

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&			getScale() const { assert(false); return Vector3::UNIT_ALL; }

		/**
		@describe
		@param
		@return
		*/
		virtual void			setPosition(const Vector3& /*pos*/) { assert(false); }

		/**
		@describe
		@param
		@return
		*/
		virtual void			setRotation(const Quaternion& /*rotation*/) { assert(false); }

		/*
		@describe
		@param
		@return
		*/
		virtual void			setScale(const Vector3& /*scale*/) { assert(false); }

		/**
		@describe
		@param
		@return
		*/
		virtual void			updateWorldAABB() const {}
	public:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		inline void		setElement(GraphicsElement* elem) { mElement = elem; }

		/** @brief  */
		inline void		setWorldAABB(const AABB& aab) { mWorldAABB = aab; }		
	};

	///fully featured content
	class BLADE_GRAPHICS_API SpaceContent : public SpaceContentBase
	{
	public:
		SpaceContent();
		virtual ~SpaceContent();

		/************************************************************************/
		/* ISpaceContent interface                                                                     */
		/************************************************************************/
		/**
		@describe get the local AABB
		@param
		@return
		*/
		virtual const AABB&	getLocalAABB() const;

		/**
		@describe
		@param
		@return
		*/
		virtual const Vector3&			getPosition() const;

		/**
		@describe
		@param
		@return
		*/
		virtual const Quaternion&		getRotation() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&			getScale() const;

		/**
		@describe
		@param
		@return
		*/
		virtual void			setPosition(const Vector3& pos);

		/**
		@describe
		@param
		@return
		*/
		virtual void			setRotation(const Quaternion& rotation);

		/*
		@describe
		@param
		@return
		*/
		virtual void			setScale(const Vector3& scale);

		/**
		@describe 
		@param
		@return
		*/
		virtual void			updateWorldAABB() const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		void					setLocalAABB(const AABB& localAABB, bool notify = true);

	protected:
		//for subclass override
		/**
		@describe
		@param
		@return
		*/
		virtual	void			notifyPositionChange();

		/**
		@describe
		@param
		@return
		*/
		virtual void			notifyRotationChange();

		/**
		@describe
		@param
		@return
		*/
		virtual void			notifyScaleChange();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			notifyLocalBoundsChange();

		//although most compilers uses a byte for boolean,but this is better to use a bunch of bits
		//use bit mask to saving memory
		//internally, 4 bits are used, extra bits can be used by subclasses
		enum EDirtyFlag
		{
			DF_MASK = AAB_DIRTY_MASK,
			DF_LOCAL_AABB = 0x01,
			DF_POSITION = 0x02,
			DF_ROTATION = 0x04,
			DF_SCALE = 0x08,
		};

		Vector3				mPosition;
		Quaternion			mRotation;
		Vector3				mScale;
		mutable AABB		mLocalAABB;
	};//class SpaceContent
	
}//namespace Blade


#endif //__Blade_SpaceNodeContent_h__