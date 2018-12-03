/********************************************************************
	created:	2011/09/06
	filename: 	TerrainRelightTask.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/ITerrainConfigManager.h>
#include "TerrainRelightTask.h"
#include "TerrainRelightTool.h"
#include "TerrainEditable.h"

namespace Blade
{
	enum ETerrainNeighbor
	{
		TN_UP = 0,
		TN_LEFT,
		TN_DOWN,
		TN_RIGHT,
	};

	static const ITask::Type TASK_TYPE = BTString("CPU");

	//////////////////////////////////////////////////////////////////////////
	const ITask::Type&	TerrainRelightTask::getType() const
	{
		return TASK_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainRelightTask::run()
	{
		if( mEditables.empty() )
			return;

		typedef TempMap<int, NormalData>	XPosMap;
		typedef TempMap<int, XPosMap>		PosMap;
		typedef	TempVector<std::pair<Vector3*, NormalData> >	VertexList;
		typedef TempVector<VertexList>		VertexMatrix;

		size_t TerrainVertexSize = ITerrainConfigManager::getInterface().getTerrainInfo().mTileSize+1;

		TempBuffer buffer;
		VertexMatrix vMatrix;
		{
			PosMap map;
			for(size_t n = 0; n < mEditables.size(); ++n)
			{
				TerrainEditable* tEd = mEditables[n].editable;
				//now editable created for all pages on the first time initialization, some are not loaded, skipping
				if(mEditables[n].height == NULL || mEditables[n].normal == NULL)
					continue;
				int x = (int)tEd->getPosition().x;
				int z = (int)tEd->getPosition().z;
				map[z][x] = mEditables[n];
			}

			vMatrix.resize(map.size());
			size_t TerrainVertexCount = ITerrainConfigManager::getInterface().getTerrainInfo().mTileVertexCount;
			buffer.reserve( map.size()*map.size()*TerrainVertexCount*sizeof(Vector3) );
			Vector3* data = (Vector3*)buffer.getData();

			index_t x = 0,z = 0;
			for( PosMap::iterator i = map.begin(); i != map.end(); ++i)
			{
				XPosMap& xmap = i->second;
				assert(xmap.size() == map.size() );

				vMatrix[z].resize( xmap.size() );
				for(XPosMap::iterator n = xmap.begin(); n != xmap.end(); ++n )
				{
					TerrainEditable* tEd = n->second.editable;
					vMatrix[z][x].first = data;
					vMatrix[z][x].second= n->second;
					int16 const* pHeight = n->second.height;

					const Vector3 scale( ITerrainConfigManager::getInterface().getTerrainScale() );
					this->setupRawVertexData(tEd->getPosition(), scale, pHeight, data, TerrainVertexSize, tEd->needNormalUpdate() );
					data += TerrainVertexCount;
					++x;
				}
				x = 0;
				++z;
			}
		}


		for(size_t tz = 0; tz < vMatrix.size(); ++tz)
		{
			for(size_t tx = 0; tx < vMatrix.size(); ++tx)
			{
				TerrainEditable* tEd = vMatrix[tz][tx].second.editable;
				puint8	NormalBuffer = vMatrix[tz][tx].second.normal;
				const Vector3* vertex = vMatrix[tz][tx].first;

				if( tEd->needNormalUpdate() )
				{
					//if( ITerrainConfigManager::getInterface().isUseVertexNormal() )
					{
						const Vector3* neighbor[4];
						if( tz == 0 )
							neighbor[TN_UP] = NULL;
						else
							neighbor[TN_UP] = vMatrix[tz-1][tx].first;

						if( tz == vMatrix.size() - 1 )
							neighbor[TN_DOWN] = NULL;
						else
							neighbor[TN_DOWN] = vMatrix[tz+1][tx].first;


						if( tx == 0 )
							neighbor[TN_LEFT] = NULL;
						else
							neighbor[TN_LEFT] = vMatrix[tz][tx-1].first;

						if( tx == vMatrix[tz].size() - 1 )
							neighbor[TN_RIGHT] = NULL;
						else
							neighbor[TN_RIGHT] = vMatrix[tz][tx+1].first;

						this->upateTerrainNormalVector(NormalBuffer, vertex, TerrainVertexSize, neighbor);
					}
					//else
					//	this->updateTerrainNormalMap(NormalBuffer, vertex, TerrainVertexSize);

				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainRelightTask::onRemoved()
	{
		mEditables.clear();
		mToolBinding->onTaskDone();
	}

	//////////////////////////////////////////////////////////////////////////
	void	TerrainRelightTask::addTerrainForUpdate(TerrainEditable* te, const pint16 height, puint8 normal)
	{
		NormalData n = { te, height, normal };
		mEditables.push_back(n);
	}

	//////////////////////////////////////////////////////////////////////////
	void	TerrainRelightTask::setupRawVertexData(const Vector3& position,const Vector3& scale,const int16* height,Vector3* vertexData,size_t size,bool fullyUpdate)
	{
		const scalar	ModHeight = scalar( uint16(-1) )*scalar(0.5);

		if( fullyUpdate )
		{
			//update all vertices
			for(size_t i = 0; i < size; ++i )
			{
				for(size_t j = 0; j < size; ++j)
				{
					Vector3& vertex = *(vertexData++);

					vertex.y = (*(height++)/ModHeight)*scale.y;
					vertex.z = i*scale.z;
					vertex.x = j*scale.x;

					//note that the position is not needed for single terrain's normal calculation
					//but to calculate normal across terrains better have a position
					vertex += position;
				}
			}
		}
		else
		{
			//only update edges, which is needed for neighbors
			//top 2 rows
			
			for(size_t i = 0; i < 2; ++i )
			{
				for(size_t j = 0; j < size; ++j)
				{
					Vector3& vertex = *(vertexData + i*size + j);
					const int16 iHeight = *(height+ i*size+j);
					vertex.y = (iHeight/ModHeight)*scale.y;
					vertex.z = i*scale.z;
					vertex.x = j*scale.x;
					vertex += position;
				}
			}//for i

			//bottom 2 rows
			for(size_t i = size-2; i < size; ++i )
			{
				for(size_t j = 0; j < size; ++j)
				{
					Vector3& vertex = *(vertexData + i*size + j);

					const int16 iHeight = *(height+ i*size+j);
					vertex.y = (iHeight/ModHeight)*scale.y;
					vertex.z = i*scale.z;
					vertex.x = j*scale.x;
					vertex += position;
				}
			}//for i

			//left 2 cols
			for(size_t j = 0; j < 2; ++j)
			{
				for(size_t i = 2; i < size-2; ++i)
				{
					Vector3& vertex = *(vertexData + i*size + j);

					const int16 iHeight = *(height+ i*size+j);
					vertex.y = (iHeight/ModHeight)*scale.y;
					vertex.z = i*scale.z;
					vertex.x = j*scale.x;
					vertex += position;
				}
			}//for j

			//right 2 cols
			for(size_t j = size-2; j < size; ++j)
			{
				for(size_t i = 2; i < size-2; ++i)
				{
					Vector3& vertex = *(vertexData + i*size + j);

					const int16 iHeight = *(height+ i*size+j);
					vertex.y = (iHeight/ModHeight)*scale.y;
					vertex.z = i*scale.z;
					vertex.x = j*scale.x;
					vertex += position;
				}
			}//for j
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	TerrainRelightTask::upateTerrainNormalVector(uint8* NormalBuffer, const Vector3* vertexData, size_t size, const Vector3** neighborVertex)
	{
		size_t index = 0;
#define SHARED_EDGE (1)
		for( size_t i = 0; i < size; ++i )
		{
			for( size_t j = 0; j < size; ++j)
			{
				assert(index < size*size );
				Vector3 normal[4];
				Vector3 up,left,right,bottom;
				bool calc[4] = {true,true,true,true};

				if( index < size )
				{
					if( neighborVertex[TN_UP] == NULL )
						calc[0] = calc[3] = false;
					else
					{
						up = neighborVertex[TN_UP][ size*(size-1-SHARED_EDGE)+index ] - vertexData[index];
						up.normalize();
					}
				}
				else
				{
					up = vertexData[index - size] - vertexData[index];
					up.normalize();
				}

				if( index >= size*size - size )
				{
					if( neighborVertex[TN_DOWN] == NULL)
						calc[1] = calc[2] = false;
					else
					{
						bottom = neighborVertex[TN_DOWN][index-(size-1-SHARED_EDGE)*size] - vertexData[index];
						bottom.normalize();
					}
				}
				else
				{
					bottom = vertexData[index + size] - vertexData[index];
					bottom.normalize();
				}

				if( index%size == 0 )
				{
					if( neighborVertex[TN_LEFT] == NULL )
						calc[0] = calc[1] = false;
					else
					{
						left = neighborVertex[TN_LEFT][index+size-1-SHARED_EDGE] - vertexData[index];
						left.normalize();
					}
				}
				else
				{
					left = vertexData[index-1] - vertexData[index];
					left.normalize();
				}

				if( (index+1)%size == 0 /*&& index%size != 0*/ )
				{
					if( neighborVertex[TN_RIGHT] == NULL )
						calc[2] = calc[3] = false;
					else
					{
						right = neighborVertex[TN_RIGHT][index-size+1+SHARED_EDGE] - vertexData[index];
						right.normalize();
					}
				}
				else
				{
					right = vertexData[index+1] - vertexData[index];
					right.normalize();
				}

				assert(calc[0] || calc[1] || calc[2] || calc[3] );

				Vector3 n = Vector3::ZERO;
				scalar weight = 0;

				if( calc[0] )
				{
					normal[0] = up.crossProduct(left);
					normal[0].normalize();

					scalar normalWeight = ::acos( Math::Clamp(up.dotProduct(left),-1.f,1.f) );

					n += normal[0]*normalWeight;

					weight += normalWeight;
				}


				if( calc[1] )
				{
					normal[1] = left.crossProduct(bottom);
					normal[1].normalize();

					scalar normalWeight = ::acos( Math::Clamp(left.dotProduct(bottom),-1.f,1.f) );

					n += normal[1]*normalWeight;

					weight += normalWeight;
				}

				if( calc[2] )
				{
					normal[2] = bottom.crossProduct(right);
					normal[2].normalize();

					scalar normalWeight = ::acos( Math::Clamp(bottom.dotProduct(right),-1.f,1.f) );

					n += normal[2]*normalWeight;

					weight += normalWeight;
				}

				if( calc[3] )
				{
					normal[3] = right.crossProduct(up);
					normal[3].normalize();

					scalar normalWeight = ::acos( Math::Clamp(right.dotProduct(up),-1.f,1.f) );

					n += normal[3]*normalWeight;

					weight += normalWeight;
				}

				n /= weight;
				n.normalize();

				//[-1,1] ==> [0,255]
				NormalBuffer[index*3 + Color::R ] = (uint8)( (n.x + 1.0f)*0.5f * 255 + 0.5f); //last 0.5f means round()
				NormalBuffer[index*3 + Color::G ] = (uint8)( (n.y + 1.0f)*0.5f * 255 + 0.5f );
				NormalBuffer[index*3 + Color::B ] = (uint8)( (n.z + 1.0f)*0.5f * 255 + 0.5f );

				++index;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	TerrainRelightTask::updateTerrainNormalMap(uint8* NormalBuffer, const Vector3* vertexData, size_t size)
	{
		//
		//note: vertex count is  size*size,  i.e. (512+1)*(512+1)
		//when using vertex normal,  the normal buffer size is extractly the vertex size (513*513)
		//but when using normal map, the map data is per cell and not per vertex so, actually normal buffer is 512*512
		//there's a little bit data not used at the tail.

		//note 2: now we always compute normal using vertex normal method,
		//and the size mismatch for normal map mode will be scaled, at loading time.(513x513=>512x512)

		const size_t cellCount = size - 1;

		const Vector3* cellVertex = vertexData;

		/*
				+-A-+-C-+
				|\  |  /|
				B 1 | 2 D
				|  \|/  |
				+---+---+
				|  /|\  |
				| 3 | 4 |
				|/  |  \|
				+--+--+-+

				normal map 's normal is not per vertex ,but per cell (1x1 cell):calculate 2 face normal and average.

				normal 1: vector right down (A,1,B)
				normal 2: vector left down  (C,2,D)
				normal 3: vector left down
				normal 4: vector right down
		*/

		for(size_t i = 0; i < cellCount; ++i)
		{
			for( size_t  j = 0; j < cellCount; ++j)
			{
				Vector3 n = Vector3::ZERO;
				Vector3 normal[4];
				scalar  normalWeight[4];

				{
					Vector3 right = *(cellVertex+1) - *cellVertex;	//A
					Vector3 rightDown = *(cellVertex+size+1) - *cellVertex;
					Vector3 down = *(cellVertex+size) - *cellVertex;
					right.normalize();
					rightDown.normalize();
					down.normalize();

					normal[0] = rightDown.crossProduct( right );
					normal[0].normalize();
					normalWeight[0] = std::acos( Math::Clamp(rightDown.dotProduct(right),-1.f,1.f) );

					normal[1] = down.crossProduct( rightDown );
					normal[1].normalize();
					normalWeight[1] = std::acos( Math::Clamp(down.dotProduct(rightDown),-1.f,1.f) );
				}

				{
					Vector3 left = *cellVertex - *(cellVertex+1);	//A
					Vector3 leftDown = *(cellVertex+size) - *(cellVertex+1);
					Vector3 down = *(cellVertex+size+1) - *(cellVertex+1);
					left.normalize();
					leftDown.normalize();
					down.normalize();

					normal[2] = left.crossProduct( leftDown );
					normal[2].normalize();
					normalWeight[2] = std::acos( Math::Clamp(left.dotProduct(leftDown),-1.f,1.f) );

					normal[3] = leftDown.crossProduct( down );
					normal[3].normalize();
					normalWeight[3] = std::acos( Math::Clamp(leftDown.dotProduct(down),-1.f,1.f) );
				}

				scalar weight = 0.0f;
				for(int index = 0; index < 4; ++index)
				{
					n += normal[index]*normalWeight[index];
					weight += normalWeight[index];
				}
				n /= weight;
				n.normalize();


				//[-1,1] ==> [0,255]
				NormalBuffer[ Color::R ] = (uint8)( (n.x + 1.0f)*127 ); //last 0.5f means round()
				NormalBuffer[ Color::G ] = (uint8)( (n.y + 1.0f)*127 );
				NormalBuffer[ Color::B ] = (uint8)( (n.z + 1.0f)*127 );
				NormalBuffer += 3;

				++cellVertex;
			}//for
			++cellVertex;	//additional 1
		}//for

		assert( size_t(cellVertex - vertexData) == (size-1)*size );
	}
	

}//namespace Blade
