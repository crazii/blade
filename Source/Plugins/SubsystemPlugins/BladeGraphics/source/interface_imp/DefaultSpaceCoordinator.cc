/********************************************************************
	created:	2010/04/30
	filename: 	DefaultSpaceCoordinator.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "DefaultSpaceCoordinator.h"
#include <interface/public/graphics/GraphicsInterface_blang.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	DefaultSpaceCoordinator::DefaultSpaceCoordinator()
		:SpaceCoordinatorBase( BTString(BLANG_DEFAULT_COORDINATOR) )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DefaultSpaceCoordinator::~DefaultSpaceCoordinator()
	{

	}


	///************************************************************************/
	///* IGraphicsSpaceCoordinator interface                                                                      */
	///************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t				DefaultSpaceCoordinator::getNumSpaces() const
	{
		return mSpaceList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	ISpace*		DefaultSpaceCoordinator::getSpace(index_t index) const
	{
		if( index < mSpaceList.size() )
		{
			SpaceList::const_iterator i = mSpaceList.begin();
			std::advance(i, index);
			return i->second;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	ISpace*		DefaultSpaceCoordinator::getSpace(const TString& name) const
	{
		SpaceList::const_iterator i = mSpaceList.find(name);
		if( i == mSpaceList.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	ISpace*		DefaultSpaceCoordinator::addSpace(const TString& type,const TString& name, const Vector3& maxSize, bool separated/* = false*/)
	{
		if( name == NULL || type == NULL )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid space type or name.") );
		
		SpaceList& list = mSpaceList;
		SpaceHandle& handle = list[name];
		if( handle != NULL )
		{
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("same space already added.") );
		}

		//ISpace* man = SpaceFactory::getSingleton().createInstance(type);
		ISpace* space = BLADE_FACTORY_CREATE(ISpace,type);

		AABB aab(Vector3::ZERO, maxSize);
		space->initialize(this, aab, 0);
		if(separated)
			space->raiseSpaceFlag(SF_SEPARATED);

		handle.bind(space);
		return space;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					DefaultSpaceCoordinator::removeSpace(const TString& name)
	{
		if( name != NULL )
		{
			size_t n = mSpaceList.erase( name );
			if( n != 1 )
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("cannot remove space: not found") );
			
			return n == 1;
		}
		return false;
	}


}//namespace Blade