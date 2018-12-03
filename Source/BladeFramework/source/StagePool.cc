/********************************************************************
	created:	2012/12/24
	filename: 	StagePool.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <StagePool.h>

namespace Blade
{
	
	SPOOL StagePool::msPool = SPOOL( SPOOL::Static() );
	RefCount StagePool::msCount = RefCount(0);

	//////////////////////////////////////////////////////////////////////////
	void StagePool::initialize(const TString& type/* = BTString("Resource") */)
	{
		if( msCount.safeIncrement() == 1 )
		{
			//TODO: double check lock
			msPool.bind( BLADE_FACTORY_CREATE(IPool, type) );
			msPool->initialize();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void StagePool::shutdown()
	{
		if( msCount.safeDecrement() == 0 )
			msPool.clear();
	}


}//namespace Blade