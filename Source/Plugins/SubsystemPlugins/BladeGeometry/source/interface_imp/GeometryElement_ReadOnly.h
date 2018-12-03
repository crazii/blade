/********************************************************************
	created:	2010/05/07
	filename: 	GeometryElement_ReadOnly.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GeometryElement_ReadOnly_h__
#define __Blade_GeometryElement_ReadOnly_h__
#include <BladeGeometry.h>
#include <ElementBase.h>
#include <GeometryNode.h>
#include <parallel/ParaStateDef.h>

namespace Blade
{
	class GeometryElement_ReadOnly : public IGeometry, public GeometryElementBase, public Allocatable
	{
	public:
		GeometryElement_ReadOnly();
		~GeometryElement_ReadOnly();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data */
		virtual void			postProcess(const ProgressNotifier& notifier);
		/** @brief  */
		virtual Interface*		getInterface(InterfaceName type)
		{
			CHECK_RETURN_INTERFACE(IGeometry, type, this);
			return NULL;
		}

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual size_t			initParallelStates();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onParallelStateChange(const IParaState& data);

		/************************************************************************/
		/* IGeometry interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void				setGlobalPosition(const Vector3& /*pos*/) {}

		/*
		@describe
		@param
		@return
		*/
		virtual void				setGlobalRotation(const Quaternion& /*rotation*/) {}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setGlobalDirection(const Vector3& /*targetDir*/, const Vector3& /*DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z*/)
		{
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void				setGlobalScale(const Vector3& /*scale*/) {}

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getGlobalPosition() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const Quaternion&	getGlobalRotation() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getGlobalScale() const;

		/*
		@describe 
		@param
		@return
		*/
		virtual const AABB&	getLocalBounding() const;

		/**
		@describe
		@param
		@return
		*/
		virtual INode*				getGeometryNode() const { return NULL; }


		/**
		@describe
		@param
		@return
		*/
		virtual Vector3				getDirection(const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z, GEOM_SPACE gspace = GS_WORLD) const
		{
			BLADE_UNREFERENCED(gspace);
			return DirDefinitionVec * mRotation;
		}

		/**
		@describe
		@param
		@return
		*/
		virtual void				setDirection(const Vector3& /*newDirection*/, GEOM_SPACE /*gspace = GS_WORLD*/, const Vector3& /*DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z*/)
		{

		}

		/**
		@describe look at a specific point
		@param
		@return
		*/
		virtual void				lookAt(const POINT3& /*target*/, GEOM_SPACE /*gspace = GS_WORLD*/, const Vector3& /*DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z*/)
		{

		}

		/**
		@describe
		@param
		@return
		*/
		virtual void				setFixedYawAxis(bool /*useFixedAxis*/, const Vector3& /*FixedYawAxis = Vector3::UNIT_Y*/) {}

		/**
		@describe
		@param
		@return
		*/
		virtual bool				getFixedYawAxis(Vector3& /*axis*/) const { return false; }
		
		/************************************************************************/
		/* GeometryElementBase interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual void				initialize(IScene* scene, ParaStateQueue* queue, GeometryUpdater* updater);

	protected:

		Vector3		mPositionData;
		Vector3		mScaleData;
		Quaternion	mRotationData;
		AABB		mAABData;

		ParaVec3	mPosition;
		ParaVec3	mScale;
		ParaQuat	mRotation;
		ParaAAB		mBounding;

		//config binding
		friend class GeometryPlugin;
	};//class GeometryElement_ReadOnly
	
}//namespace Blade



#endif //__Blade_GeometryElement_ReadOnly_h__