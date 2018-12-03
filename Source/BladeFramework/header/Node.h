/********************************************************************
	created:	2010/04/25
	filename: 	Node.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Node_h__
#define __Blade_Node_h__
#include <RefCount.h>
#include <utility/Mask.h>
#include <math/Quaternion.h>
#include <interface/public/geometry/INode.h>


namespace Blade
{
	class Node;
	class INodeUpdater
	{
	public:
		virtual ~INodeUpdater()		{}

		/** @brief  */
		virtual void notifyNodeChange(Node* node) = 0;

		/** @brief  */
		virtual void updateNodes() = 0;
	};

	namespace Impl
	{
	
		class ChildNameMap;
		class ChildIndexMap;
	
	}//namespace Impl

	///map child space to parent
	const GEOM_SPACE CHILD_SPACE_TO_PARENT[3] = 
	{
		GS_OBJECT,	//dummy
		GS_OBJECT,
		GS_WORLD,
	};

	class BLADE_FRAMEWORK_API Node : public INode, public Allocatable
	{
	private:
		Node(const Node&);
		Node& operator=(const Node&);
	public:
		Node(INodeUpdater* updater,index_t depth,Node* parent = NULL);

		Node(INodeUpdater* updater,index_t depth,const TString& name,const Vector3& pos,const Vector3& scale,const Quaternion& rotation,Node* parent = NULL);

		virtual ~Node();

		/************************************************************************/
		/* INode interface                                                                     */
		/************************************************************************/

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&		getName() const;

		/**
		@describe get index ,the index is set by parent
		@param 
		@return 
		*/
		virtual index_t				getIndex() const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual index_t				getDepthLevel() const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual INode*				getParent() const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setParent(INode* parent);

		/**
		@describe
		@param
		@return
		*/
		virtual void				deatchFromParent();

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t				getChildCount() const;

		/**
		@describe
		@param
		@return
		*/
		virtual INode*				getChild(size_t index) const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				attachChild(INode* child);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual INode*				removeChild(const TString& name);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual INode*				removeChild(size_t index);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				hasChild(const INode* node) const;

		/**
		@describe 
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual const Quaternion&	getLocalRotation() const
		{
			return mRotation;
		}
		virtual const Quaternion&	getWorldRotation() const
		{
			return mWorldRotation;
		}

		/**
		@describe set rotation value
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual void				setLocalRotation(const Quaternion& rotation);
		virtual void				setWorldRotation(const Quaternion& rotation);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isRotationInherited() const	{return mMask.checkBitAtIndex(NM_INHERIT_ROTATION);}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setRotationInherited(bool inherit);

		/**
		@describe 
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual const Vector3&		getLocalPosition() const
		{
			return mPosition;
		}
		virtual const Vector3&		getWorldPosition() const
		{
			return mWorldPosition;
		}

		/**
		@describe 
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual void				setLocalPosition(const Vector3& position);
		virtual void				setWorldPosition(const Vector3& position);

		/**
		@describe
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual const Vector3&		getLocalScale() const
		{
			return mScale;
		}
		virtual const Vector3&		getWorldScale() const
		{
			return mWorldScale;
		}

		/**
		@describe 
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual void				setLocalScale(const Vector3& scale);
		virtual void				setWorldScale(const Vector3& scale);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isScaleInherited() const	{return mMask.checkBitAtIndex(NM_INHERIT_SCALE);}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setScaleInherited(bool inherit);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				scale(const Vector3& scale_factor);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				translate(const Vector3& trans,GEOM_SPACE tspace = GS_OBJECT);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				yaw(scalar fAngleRadians,GEOM_SPACE tspace = GS_OBJECT);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				pitch(scalar fAngleRadians,GEOM_SPACE tspace = GS_OBJECT);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				roll(scalar fAngleRadians,GEOM_SPACE tspace = GS_OBJECT);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				rotate(const Vector3& axis,scalar fAngleRadians,GEOM_SPACE tspace = GS_OBJECT);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				rotate(const Quaternion& rotation,GEOM_SPACE tspace = GS_OBJECT);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		Node*				createChild(const TString& name,const Vector3& translation = Vector3::ZERO,const Quaternion& rotation = Quaternion::IDENTITY,const Vector3& scale = Vector3::UNIT_ALL);

		/** @brief  */
		Node*				createChild(const Vector3& translation = Vector3::ZERO,const Quaternion& rotation = Quaternion::IDENTITY,const Vector3& scale = Vector3::UNIT_ALL);

		/** @brief  */
		void				destroyChild(const TString& name);

		/** @brief  */
		void				destroyChild(index_t index);

		/** @brief  */
		void				destroyChild(Node* node);

		/** @brief  */
		void				destroyAllChildren();

		/** @brief  */
		void				requestUpdate();

		/** @brief  */
		void				update();

		/** @brief transform world rotation to local space */
		inline Quaternion	transformRotationWorldToLocal(const Quaternion& Rotation) const
		{
			return mWorldRotation.getInversedQuaternion()*Rotation;
		}

		/** @brief transform local rotation to world space */
		inline Quaternion	transformRotationLocalToWorld(const Quaternion& LocalRotation) const
		{
			return mWorldRotation*LocalRotation;
		}

		/** @brief transform world position to local space */
		inline Vector3		transformPositionWorldToLocal(const Vector3& Pos) const
		{
			return Pos*mWorldRotation.getInversedQuaternion()/mWorldScale - mWorldPosition;
		}

		/** @brief transform local position to world space */
		inline Vector3		transformPositionLocalToWorld(const Vector3& LocalPos) const
		{
			return LocalPos*mWorldRotation*mWorldScale + mWorldPosition;
		}

		/** @brief transform world scale to local space */
		inline Vector3		transformScaleWorldToLocal(const Vector3& Scale) const
		{
			return Scale/mWorldScale;
		}

		/** @brief transform local scale to world space */
		inline Vector3		transformScaleLocalToWorld(const Vector3& LocalScale) const
		{
			return mWorldScale*LocalScale;
		}

		/*
		@describe reset current pos/scale/rotation to init state
		@param 
		@return 
		*/
		void				reset(const Vector3& posiotion = Vector3::ZERO, const Vector3& scale = Vector3::UNIT_ALL, const Quaternion& rotation = Quaternion::IDENTITY);

	protected:
		/** @brief  */
		void			setIndex(index_t index)		{mIndex = index;}

		/** @brief  */
		void			setDepthLevel(index_t depth)
		{
			if( mLevel != depth)
			{
				mLevel = depth;
				onDepthLevelChange();
			}
		}

		/************************************************************************/
		/* pre-defined interfaces for subclass                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		virtual Node*	createChildImp(const TString& name,const Vector3& translation,const Quaternion& rotation,const Vector3& scale) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual Node*	createChildImp(const Vector3& translation,const Quaternion& rotation,const Vector3& scale) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	destroyChildImp(Node* node) = 0;

		/** @brief  */
		virtual void	onDepthLevelChange() = 0;

		/** @brief  */
		virtual void	onAttachToParentNode() = 0;

		/** @brief  */
		virtual void	onDetachFromParentNode() = 0;

		/** @brief  */
		virtual void	onPositionChange() = 0;

		/** @brief  */
		virtual void	onRotationChange() = 0;

		/** @brief  */
		virtual void	onScaleChange() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		static void			generateName(TString& name);

		TString				mName;
		index_t				mIndex;

		///depth level
		index_t				mLevel;

		///direct access to updater,thus reduce useless update recursion
		///when depth is quite deep,this is useful
		INodeUpdater*		mUpdater;
		Node*				mParent;

		//local transform (relative to parent)
		Vector3				mPosition;
		Vector3				mScale;
		Quaternion			mRotation;
		//world transform
		Vector3				mWorldPosition;
		Vector3				mWorldScale;
		Quaternion			mWorldRotation;

		enum ENodeMask
		{
			//attributes
			NM_INHERIT_ROTATION,
			NM_INHERIT_SCALE,
			//update
			NM_UPDATE,
			//listener notify
			NM_ROTATION_DIRTY,
			NM_SCALE_DIRTY,
			NM_POSITION_DIRTY
		};
		Mask				mMask;
	private:
		static	RefCount	msNameCounter;
		Pimpl<Impl::ChildNameMap>		mChildMap;
		Pimpl<Impl::ChildIndexMap>		mChildList;
	};//class Node

}//namespace Blade


#endif //__Blade_Node_h__