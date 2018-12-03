/********************************************************************
	created:	2010/05/07
	filename: 	IGeometry.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IGeometry_h__
#define __Blade_IGeometry_h__
#include <interface/public/geometry/INode.h>

namespace Blade
{

	class IGeometry : public Interface
	{
	public:
		virtual ~IGeometry()		{}

		/*
		@describe
		@param
		@return
		*/
		virtual void				setGlobalPosition(const Vector3& pos) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void				setGlobalRotation(const Quaternion& rotation) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void				setGlobalDirection(const Vector3& targetDir, const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void				setGlobalScale(const Vector3& scale) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getGlobalPosition() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Quaternion&	getGlobalRotation() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getGlobalScale() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const AABB&	getLocalBounding() const = 0;

		/**
		@describe geometry node to set local position, etc.
		@param
		@return
		*/
		virtual INode*				getGeometryNode() const = 0;


		/**
		@describe
		@param
		@return
		*/
		virtual Vector3				getDirection(const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z,GEOM_SPACE gspace = GS_WORLD) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void				setDirection(const Vector3& newDirection, GEOM_SPACE gspace = GS_WORLD,const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z) = 0;

		inline void					setDirection(scalar x,scalar y,scalar z, GEOM_SPACE gspace = GS_WORLD,const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z)
		{
			return this->setDirection(Vector3(x,y,z), gspace, DirDefinitionVec);
		}

		/**
		@describe look at a specific point
		@param
		@return
		*/
		virtual void				lookAt(const POINT3& target,GEOM_SPACE gspace = GS_WORLD,const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z) = 0;

		inline void					lookAt(scalar x,scalar y,scalar z,GEOM_SPACE gspace = GS_WORLD,const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z)
		{
			return this->lookAt(POINT3(x,y,z), gspace, DirDefinitionVec);
		}

		/**
		@describe
		@param
		@return
		*/
		virtual void				setFixedYawAxis(bool useFixedAxis,const Vector3& FixedYawAxis = Vector3::UNIT_Y) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				getFixedYawAxis(Vector3& axis) const = 0;

	};//class IGeometry

	typedef InterfaceID<IGeometry> IID_GEOMETRY;
	
}//namespace Blade


#endif //__Blade_IGeometry_h__