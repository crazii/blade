/********************************************************************
	created:	2010/05/07
	filename: 	GeometryNode.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GeometryNode_h__
#define __Blade_GeometryNode_h__
#include <Node.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class GeometryNode : public Node
	{
	public:
		class IListener
		{
		public:
			/** @brief  */
			virtual void notifyPositionChange(GeometryNode* node) = 0;
			/** @brief  */
			virtual void notifyScaleChange(GeometryNode* node) = 0;
			/** @brief  */
			virtual void notifyRotationChange(GeometryNode* node) = 0;
		};


	public:
		GeometryNode(INodeUpdater* updater,index_t depth,Node* parent = NULL);
		GeometryNode(INodeUpdater* updater,index_t depth,const TString& name,const Vector3& pos,const Vector3& scale,const Quaternion& rotation,Node* parent = NULL);

		virtual ~GeometryNode();


		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		void				addListener(IListener* listener);

		/*
		@describe
		@param
		@return
		*/
		void				removeListener(IListener* listener);


		/*
		@describe
		@param
		@return
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		void				setDirection(const Vector3& direction,GEOM_SPACE tspace = GS_OBJECT,const Vector3& originDir = Vector3::NEGATIVE_UNIT_Z);

		/*
		@describe
		@param
		@return
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		void				lookAt(const POINT3& target,GEOM_SPACE tspace = GS_OBJECT,const Vector3& originDir = Vector3::NEGATIVE_UNIT_Z);

		/*
		@describe
		@param
		@return
		*/
		void				setFixedYawAxis(bool useFixedYawAxis,const Vector3& axis = Vector3::UNIT_Y);

		/*
		@describe 
		@param 
		@return 
		*/
		bool				getFixedYawAxis(Vector3& axis) const;

		/*
		@describe 
		@param
		@return
		@note difference from setGlobalPosition: no listener dispatch
		*/
		void				setGlobalPositionDirect(const Vector3& position);

	protected:
		/************************************************************************/
		/* Node Interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual Node*	createChildImp(const TString& name,const Vector3& translation,const Quaternion& rotation,const Vector3& scale);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual Node*	createChildImp(const Vector3& translation,const Quaternion& rotation,const Vector3& scale);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	destroyChildImp(Node* node);

		//notification

		/** @brief  */
		virtual void	onDepthLevelChange();

		/** @brief  */
		virtual void	onAttachToParentNode();

		/** @brief  */
		virtual void	onDetachFromParentNode();

		/** @brief  */
		virtual void	onPositionChange();

		/** @brief  */
		virtual void	onRotationChange();

		/** @brief  */
		virtual void	onScaleChange();


		typedef Set<IListener*>	ListenerList;

		ListenerList	mListenerList;
		bool			mUseFixedYawAxis;
		Vector3			mFixedYawAxis;
	};
	
}//namespace Blade


#endif //__Blade_GeometryNode_h__
