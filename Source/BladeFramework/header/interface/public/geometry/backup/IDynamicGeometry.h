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
		typedef enum EGeometrySpace
		{
			DGS_LOCAL,
			DGS_PARENT,
			DGS_WORLD,
		}SPACE;

	public:
		virtual ~IDynamicGeometry() = 0 {}

		/*
		@describe
		@param
		@return
		*/
		virtual void		attachTo(IDynamicGeometry* parent) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void		detach() = 0;


		//manipulation geometry data in parent's space
		//if no parent attached,it is equivalent to global method

		/*
		@describe
		@param
		@return
		*/
		virtual void				setLocalPosition(const Vector3& pos) = 0;

		inline void					setLocalPosition(scalar x,scalar y,scalar z)
		{
			setLocalPosition( Vector3(x,y,z) );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void				setLocalRotation(const Quaternion& rotation) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void				setLocallScale(const Vector3& scale) = 0;

		inline void					setLocalScale(scalar x,scalar y,scalar z)
		{
			setGlobalScale( Vector3(x,y,z) );
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getLocalPosition() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Quaternion&	getLocalRotation() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getLocalScale() const = 0;


	};//class IDynamicGeometry


	template class BLADE_FRAMEWORK_API	Handle<IDynamicGeometry>;
	typedef Handle<IDynamicGeometry>	HDYNAMICGEOM;

	
}//namespace Blade


#endif //__Blade_IDynamicGeometry_h__