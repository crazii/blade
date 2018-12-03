/********************************************************************
	created:	2010/05/07
	filename: 	IGraphicsSpaceCoordinator.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ISpaceCoordinator_h__
#define __Blade_ISpaceCoordinator_h__
#include <interface/IPage.h>
#include <interface/public/graphics/IGraphicsSpaceCoordinator.h>
#include <interface/ISpace.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	typedef struct SSpaceCoordinatorDesc
	{
		TString	type;
		TString	defaultSpace;
		uint32	visiblePage;
		uint32	pageSize;
	}COORDINATOR_DESC;

	//////////////////////////////////////////////////////////////////////////
	class BLADE_GRAPHICS_API ISpaceCoordinator : public IGraphicsSpaceCoordinator, public ISpace, public IPagingManager::IListener
	{
	public:
		virtual ~ISpaceCoordinator()	{}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool				initialize(ISpaceCoordinator* coorinator, const AABB& maxBound, uint32 partitionMask) = 0;

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const TString&		getCoordinatorType() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				initialize(const COORDINATOR_DESC* desc) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const COORDINATOR_DESC*	getDesc() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual size_t				getNumSpaces() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual ISpace*				getSpace(index_t index) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual ISpace*				getSpace(const TString& name) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual ISpace*				addSpace(const TString& type,const TString& name, const Vector3& maxSize, bool separated = false) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool				removeSpace(const TString& name) = 0;

		/**
		@describe x,z in range of [0,COORDINATOR_DESC.visiblePage)
		@param
		@return
		*/
		virtual ISpace*				getSpaceofPage(index_t x, index_t z) const = 0;

	};//class ISpaceCoordinator

	extern template class BLADE_GRAPHICS_API Factory<ISpaceCoordinator>;
	typedef Factory<ISpaceCoordinator> CoordinatorFactory;

}//namespace Blade


#endif //__Blade_ISpaceCoordinator_h__