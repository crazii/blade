/********************************************************************
	created:	2010/04/30
	filename: 	DefaultSpaceCoordinator.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DefaultSpaceCoordinator_h__
#define __Blade_DefaultSpaceCoordinator_h__
#include <SpaceCoordinatorBase.h>
#include <utility/BladeContainer.h>
#include <StaticHandle.h>

namespace Blade
{
	class ISpace;

	class DefaultSpaceCoordinator : public SpaceCoordinatorBase
	{
	public:
		DefaultSpaceCoordinator();
		~DefaultSpaceCoordinator();

		/************************************************************************/
		/* ISpaceCoordinator interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual size_t		getNumSpaces() const;

		/*
		@describe
		@param
		@return
		*/
		virtual ISpace*		getSpace(index_t index) const;

		/*
		@describe
		@param
		@return
		*/
		virtual ISpace*		getSpace(const TString& name) const;

		/*
		@describe
		@param
		@return
		*/
		virtual ISpace*		addSpace(const TString& type,const TString& name, const Vector3& maxSize, bool separated = false);

		/*
		@describe
		@param
		@return
		*/
		virtual bool		removeSpace(const TString& name);

	protected:
		typedef StaticHandle<ISpace>	SpaceHandle;
		typedef TStringMap<SpaceHandle>	SpaceList;
		SpaceList	mSpaceList;
	};


}//namespace Blade


#endif //__Blade_DefaultSpaceCoordinator_h__