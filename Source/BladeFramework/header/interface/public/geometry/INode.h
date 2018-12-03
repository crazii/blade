/********************************************************************
	created:	2010/05/08
	filename: 	INode.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_INode_h__
#define __Blade_INode_h__
#include <BladeFramework.h>
#include <utility/String.h>
#include <math/Vector3.h>
#include <interface/public/geometry/GeomTypes.h>

namespace Blade
{
	class Quaternion;

	class BLADE_FRAMEWORK_API INode
	{
	public:
		virtual ~INode()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&		getName() const = 0;

		/**
		@describe get index ,the index is set by parent
		@param 
		@return 
		*/
		virtual index_t				getIndex() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual index_t				getDepthLevel()const = 0 ;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual INode*				getParent() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setParent(INode* parent) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void				deatchFromParent() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t				getChildCount() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual INode*				getChild(size_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				attachChild(INode* child) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual INode*				removeChild(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual INode*				removeChild(size_t index) = 0;

		inline INode*				removeChild(INode* child)
		{
			if( child != NULL )
				return this->removeChild( child->getName() );
			return NULL;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				hasChild(const INode* node) const = 0;

		/**
		@describe 
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual const Quaternion&	getLocalRotation() const = 0;
		virtual const Quaternion&	getWorldRotation() const = 0;

		/**
		@describe set rotation
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual void				setLocalRotation(const Quaternion& rotation) = 0;
		virtual void				setWorldRotation(const Quaternion& rotation) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isRotationInherited() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setRotationInherited(bool inherit) = 0;

		/**
		@describe 
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual const Vector3&		getLocalPosition() const = 0;
		virtual const Vector3&		getWorldPosition() const = 0;

		/**
		@describe 
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual void				setLocalPosition(const Vector3& position) = 0;
		inline void					setLocalPosition(scalar x,scalar y,scalar z)
		{
			this->setLocalPosition( Vector3(x,y,z) );
		}
		
		virtual void				setWorldPosition(const Vector3& position) = 0;
		inline void					setWorldPosition(scalar x,scalar y,scalar z)
		{
			this->setWorldPosition( Vector3(x,y,z) );
		}

		/**
		@describe 
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual const Vector3&		getLocalScale() const = 0;
		virtual const Vector3&		getWorldScale() const = 0;

		/**
		@describe
		@param 
		@return 
		@note transforms relative to local space is meaningless, so "local transform" is actually relative to parent
		*/
		virtual void				setLocalScale(const Vector3& scale) = 0;
		inline void					setLocalScale(scalar x,scalar y,scalar z)
		{
			this->setLocalScale( Vector3(x,y,z) );
		}

		virtual void				setWorldScale(const Vector3& scale) = 0;
		inline void					setWorldScale(scalar x,scalar y,scalar z)
		{
			this->setWorldScale( Vector3(x,y,z) );
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isScaleInherited() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setScaleInherited(bool inherit) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				scale(const Vector3& scale_factor) = 0;

		inline void					scale(scalar x,scalar y,scalar z)
		{
			this->scale( Vector3(x,y,z) );
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				translate(const Vector3& trans,GEOM_SPACE tspace = GS_OBJECT) = 0;

		inline void					translate(scalar x,scalar y,scalar z,GEOM_SPACE tspace = GS_OBJECT)
		{
			this->translate( Vector3(x,y,z), tspace );
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				yaw(scalar fAngleRadians,GEOM_SPACE tspace = GS_OBJECT) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				pitch(scalar fAngleRadians,GEOM_SPACE tspace = GS_OBJECT) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				roll(scalar fAngleRadians,GEOM_SPACE tspace = GS_OBJECT) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				rotate(const Vector3& axis,scalar fAngleRandians,GEOM_SPACE tspace = GS_OBJECT) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				rotate(const Quaternion& rotation,GEOM_SPACE tspace = GS_OBJECT) = 0;

	};//class INode
	
}//namespace Blade


#endif //__Blade_INode_h__