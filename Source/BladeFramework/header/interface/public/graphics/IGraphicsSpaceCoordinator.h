/********************************************************************
	created:	2010/05/06
	filename: 	IGraphicsSpaceCoordinator.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IGraphicsSpaceCoordinator_h__
#define __Blade_IGraphicsSpaceCoordinator_h__
#include <math/Vector3.h>
#include <Event.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	class BLADE_FRAMEWORK_API IGraphicsSpaceCoordinator
	{
	public:
		class IPositionReference
		{
		public:
			/** @brief source position (view point or something like that.i.e.player position) in space (not logical) coordinates */
			virtual const Vector3&	getPosition() const = 0;
		};

	public:
		virtual ~IGraphicsSpaceCoordinator()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual bool				setPositionReference(IPositionReference* treference) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IPositionReference*	getPositionReference() const = 0;

	};//class IGraphicsSpaceCoordinator
	
}//namespace Blade


#endif //__Blade_IGraphicsSpaceCoordinator_h__