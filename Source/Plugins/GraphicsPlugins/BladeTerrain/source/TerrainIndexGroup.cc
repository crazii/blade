/********************************************************************
	created:	2011/05/31
	filename: 	IndexGroup.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "terrain_interface/TerrainIndexGroup.h"
#include "TerrainConfigManager.h"

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TerrainIndexGroup::TerrainIndexGroup()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TerrainIndexGroup::~TerrainIndexGroup()
	{

	}
	
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TerrainQueryIndexGroup::TerrainQueryIndexGroup(index_t LODLevel)
	{
		//the top 2 level is simple and could use the Hardware index buffer instead,
		//so we don't build query buffer for the highest 2 levels

		index_t Depth = LODLevel - 1;
		mQueryQuads.resize(LODLevel+1);

		for(index_t i = 0; i < LODLevel+1; ++i)
		{
			mQueryQuads[i] = BLADE_NEW QueryIndexQuad();
			this->initQuad(mQueryQuads[i], Depth-i );
		}
	}


	//////////////////////////////////////////////////////////////////////////
	TerrainQueryIndexGroup::~TerrainQueryIndexGroup()
	{
		for(size_t i = 0; i < mQueryQuads.size(); ++i)
		{
			BLADE_DELETE mQueryQuads[i];
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/** @brief  */
	const QueryIndexQuad*	TerrainQueryIndexGroup::getQueryIndexbyLOD(index_t LODLevel) const
	{
		if(LODLevel >= mQueryQuads.size() )
		{
			assert(false);
			return NULL;
		}
		return mQueryQuads[LODLevel];
	}

	//////////////////////////////////////////////////////////////////////////
	void		TerrainQueryIndexGroup::initQuad(QueryIndexQuad* quad,index_t depth)
	{
		assert( quad != NULL );

		if( (int)depth <= 0 )
		{
			quad->mSubQuad = NULL;
			return;
		}

		quad->mSubQuad = BLADE_NEW QueryIndexQuad[4];
		for(size_t i = 0; i < 4; ++i )
			this->initQuad( &(quad->mSubQuad[i]),depth-1);
	}

}//namespace Blade
