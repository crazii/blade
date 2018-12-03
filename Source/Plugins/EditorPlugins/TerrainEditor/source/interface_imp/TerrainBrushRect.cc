/********************************************************************
	created:	2011/06/11
	filename: 	TerrainBrushRect.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainBrushRect.h"
#include <interface/ITerrainConfigManager.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	TerrainBrushRect::TerrainBrushRect()
		:mXInfo(BLADE_TEMPCONT_INIT)
		,mZInfo(BLADE_TEMPCONT_INIT)
		,mTerrains(BLADE_TEMPCONT_INIT)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TerrainBrushRect::~TerrainBrushRect()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void		TerrainBrushRect::setup(const TerrainEditableList& list, size_t size, scalar fLeft, scalar fTop)
	{
		mSize = size;

		typedef TempMap<scalar,TerrainEditable*>	XGroup;
		typedef TempMap<scalar,XGroup>				ZGroup;
		ZGroup	sortingGroup;

		for( size_t i = 0; i < list.size(); ++i)
		{
			TerrainEditable* editable = list[i];
			const Vector3& pos = editable->getPosition();
			sortingGroup[ pos.z ][ pos.x ] = editable;
		}
		mTerrains.resize( sortingGroup.size() );

		//brush size range [0, size)
		//terrain size range [0, TerrainSize]
		size_t TerrainSize = ITerrainConfigManager::getInterface().getTerrainInfo().mTileSize;

		int left	= (int)std::roundf(fLeft);
		int top		= (int)std::roundf(fTop);

		int z = 0;
		index_t zStart = 0;
		for(ZGroup::iterator i = sortingGroup.begin(); i != sortingGroup.end(); ++i)
		{
			int tz = (int)std::roundf(i->first);

			XGroup& sub = i->second;
			mTerrains[(index_t)z].resize( sub.size() );

			//build up x bounds

			int x = 0;
			index_t xStart = 0;
			for( XGroup::iterator n = sub.begin(); n != sub.end(); ++n )
			{
				int tx = (int)std::roundf(n->first);

				PosInfo xpos;

				//calculate only once
				if( z == 0 )
				{
					//the first one?
					if( x == 0 )
					{
						int xDiff = tx - left;

						if( xDiff <= 0 )
						{
							xpos.mStart = 0;
							xpos.mTerranStart = index_t(-xDiff);
						}
						else
						{
							xpos.mStart = (index_t)xDiff;
							xpos.mTerranStart = 0;
						}
						xStart = xpos.mStart;
					}
					else
					{
						xpos.mStart = xStart;
						xpos.mTerranStart = 0;
					}

					//the last one?
					if( x == (int)sub.size() - 1 )
					{
						int rectEnd = left + (int)size-1;
						//ignore terrain scale, TODO: apply terrain scale
						int terrainEnd = tx + (int)TerrainSize;
						assert( terrainEnd >= left );
						if( rectEnd > terrainEnd )
							xpos.mEnd = (index_t)(terrainEnd - left);
						else
							xpos.mEnd = size-1;

						xpos.mTerranEnd = xpos.mTerranStart + (xpos.mEnd - xpos.mStart);
					}
					else
					{
						assert( left + size-1 >= tx + TerrainSize );	//on right border: also on next terrain left border

						xpos.mTerranEnd = TerrainSize;
						xpos.mEnd = xpos.mStart + (xpos.mTerranEnd - xpos.mTerranStart);
						//the edge are shared between terrains
						//so the brush edge is shared,too: next start is this end
						xStart += xpos.mEnd;
						
						assert(xpos.mEnd - xpos.mStart <= size-1);
						assert(xpos.mTerranEnd - xpos.mTerranStart <= TerrainSize);
					}

					mXInfo.push_back(xpos);
				}

				mTerrains[(index_t)z][(index_t)x++] = n->second;
			}

			//build up z bounds

			PosInfo zpos;
			//the first one?
			if( z == 0 )
			{
				int zDiff = tz - top;

				if( zDiff <= 0 )
				{
					zpos.mStart = 0;
					zpos.mTerranStart = index_t(-zDiff);
				}
				else
				{
					zpos.mStart = (index_t)zDiff;
					zpos.mTerranStart = 0;
				}
				zStart = zpos.mStart;
			}
			else
			{
				zpos.mStart = zStart;
				zpos.mTerranStart = 0;
			}

			//the last one?
			if( z == (int)sortingGroup.size() - 1 )
			{
				int rectEnd = top + (int)size-1;
				//ignore terrain scale, TODO: apply terrain scale
				int terrainEnd = tz + (int)TerrainSize;

				assert( terrainEnd >= top );
				if( rectEnd > terrainEnd )
					zpos.mEnd = (index_t)(terrainEnd - top);
				else
					zpos.mEnd = size-1;

				zpos.mTerranEnd = zpos.mTerranStart + zpos.mEnd - zpos.mStart;
			}
			else
			{
				assert( top + size-1 >= tz + TerrainSize );

				zpos.mTerranEnd = TerrainSize;
				zpos.mEnd = zpos.mStart + (zpos.mTerranEnd - zpos.mTerranStart);
				//the edge are shared between terrains
				//so the brush edge is shared,too: next start is this end
				zStart += zpos.mEnd;

				assert(zpos.mEnd - zpos.mStart <= size-1);
				assert(zpos.mTerranEnd - zpos.mTerranStart <= TerrainSize);
			}
			mZInfo.push_back(zpos);
			++z;
		}

	}

	//////////////////////////////////////////////////////////////////////////
	void		TerrainBrushRect::process(ITerrainBrushOperation& operation, scalar deltaTime, bool inverse)
	{
		for(index_t i = 0; i < mZInfo.size(); ++i )
		{
			assert( i < mTerrains.size() );
			for( index_t j = 0; j < mXInfo.size(); ++j )
			{
				assert( j < mTerrains[i].size() );
				TerrainEditable* te = mTerrains[i][j];
				operation.process(mXInfo[j], mZInfo[i], te, deltaTime, inverse);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		TerrainBrushRect::clear()
	{
		mTerrains.clear();
		mXInfo.clear();
		mZInfo.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainEditable*	TerrainBrushRect::transformPosition(index_t& x, index_t& z)
	{
		index_t tx,tz;
		for(tx = 0; tx < mXInfo.size(); ++tx )
		{
			if( x >= mXInfo[tx].mStart && x <= mXInfo[tx].mEnd )
				break;
		}

		assert( tx < mXInfo.size() );

		for(tz = 0; tz < mXInfo.size(); ++tz )
		{
			if( z >= mZInfo[tz].mStart && z <= mZInfo[tz].mEnd )
				break;
		}
		assert( tz < mZInfo.size() );

		size_t TerrainSize = ITerrainConfigManager::getInterface().getTerrainInfo().mTileSize;

		if( tx == 0 )
			x = TerrainSize - mXInfo[0].mEnd;
		else
			x -= (tx-1)*TerrainSize - mXInfo[0].mEnd;

		if( tz == 0 )
			z = TerrainSize - mZInfo[0].mEnd;
		else
			z -= (tz-1)*TerrainSize - mZInfo[0].mEnd;
		return mTerrains[z][x];
	}
	

}//namespace Blade
