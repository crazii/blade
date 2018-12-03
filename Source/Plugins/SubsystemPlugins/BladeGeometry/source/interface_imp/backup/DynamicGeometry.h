/********************************************************************
	created:	2010/05/07
	filename: 	DynamicGeometry.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DynamicGeometry_h__
#define __Blade_DynamicGeometry_h__
#include <interface/geometry/IDynamicGeometry.h>
#include <GeometryNode.h>
#include "StaticGeometry.h"

namespace Blade
{
	class DynamicGeometry : public IDynamicGeometry
	{
	public:
		DynamicGeometry();
		~DynamicGeometry();

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const ParaStateMap& getSubjects();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const ParaStateMap& getObservers();

		/************************************************************************/
		/* IStaticGeometry interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void				setGlobalPosition(const Vector3& pos);

		/*
		@describe
		@param
		@return
		*/
		virtual void				setGlobalRotation(const Quaternion& rotation);

		/*
		@describe
		@param
		@return
		*/
		virtual void				setGlobalScale(const Vector3& scale);

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

		/************************************************************************/
		/* IDynamicGeometry interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void		attachTo(IDynamicGeometry* parent);

		/*
		@describe
		@param
		@return
		*/
		virtual void		detach();


		//manipulation geometry data in parent's space
		//if no parent attached,it is equivalent to global method

		/*
		@describe
		@param
		@return
		*/
		virtual void				setLocalPosition(const Vector3& pos);

		/*
		@describe
		@param
		@return
		*/
		virtual void				setLocalRotation(const Quaternion& rotation);

		/*
		@describe
		@param
		@return
		*/
		virtual void				setLocallScale(const Vector3& scale);

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getLocalPosition() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const Quaternion&	getLocalRotation() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const Vector3&		getLocalScale() const;

		/************************************************************************/
		/* custom interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		void						initialize(IGeometryStateQueue& queue);

		/*
		@describe
		@param
		@return
		*/
		GeometryNode*				getNode() const;

		protected:
			StaticGeometry		mStaticState;
			GeometryNode*		mNode;
	};//class DynamicGeometry
	
}//namespace Blade



#endif //__Blade_DynamicGeometry_h__