/********************************************************************
	created:	2010/05/07
	filename: 	IDynamicGeometry.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IDynamicGeometry_h__
#define __Blade_IDynamicGeometry_h__
#include <interface/geometry/IStaticGeometry.h>

namespace Blade
{
	class IDynamicGeometry : public IStaticGeometry
	{
	public:
		virtual ~IDynamicGeometry() = 0 {}

		/*
		@describe
		@param
		@return
		*/
		virtual void				attachTo(IDynamicGeometry* parent) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void				detach() = 0;


		//manipulation geometry data in parent's space
		//if no parent attached,it is equivalent to global method

		/*
		@describe
		@param
		@return
		*/
		virtual void				setPosition(const Vector3& pos,GEOM_SPACE gspace = GS_WORLD) = 0;

		inline void					setPosition(scalar x,scalar y,scalar z,GEOM_SPACE gspace = GS_WORLD)
		{
			setPosition( Vector3(x,y,z) ,gspace );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void				setRotation(const Quaternion& rotation,GEOM_SPACE gspace = GS_WORLD) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void				setScale(const Vector3& scale,GEOM_SPACE gspace = GS_WORLD) = 0;

		inline void					setScale(scalar x,scalar y,scalar z,GEOM_SPACE gspace = GS_WORLD)
		{
			setScale( Vector3(x,y,z),gspace );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getPosition(GEOM_SPACE gspace = GS_WORLD) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Quaternion&	getRotation(GEOM_SPACE gspace = GS_WORLD) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getScale(GEOM_SPACE gspace = GS_WORLD) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getDirection(const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z,GEOM_SPACE gspace = GS_WORLD) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void				setDirection(const Vector3& newDirection,const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z,GEOM_SPACE gspace = GS_WORLD) = 0;

		inline void					setDirection(scalar x,scalar y,scalar z,const Vector3& DirDefinitionVec = Vector3::NEGATIVE_UNIT_Z,GEOM_SPACE gspace = GS_WORLD)
		{
			return setDirection( Vector3(x,y,z),DirDefinitionVec,gspace );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void				lookAt(const Vector3& target,GEOM_SPACE gspace = GS_WORLD) = 0;

		inline void					lookAt(scalar x,scalar y,scalar z,GEOM_SPACE gspace = GS_WORLD)
		{
			return lookAt( Vector3(x,y,z) );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual

		/*
		@describe
		@param
		@return
		*/
		virtual void				yaw(scalar fYawAngleRadian,GEOM_SPACE gspace = GS_PARENT) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void				pitch(scalar fYawAngleRadian,GEOM_SPACE gspace = GS_PARENT) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void				roll(scalar fYawAngleRadian,GEOM_SPACE gspace = GS_PARENT) = 0;



	};//class IDynamicGeometry


	template class BLADE_FRAMEWORK_API	Handle<IDynamicGeometry>;
	typedef Handle<IDynamicGeometry>	HDYNAMICGEOM;

	
}//namespace Blade


#endif //__Blade_IDynamicGeometry_h__