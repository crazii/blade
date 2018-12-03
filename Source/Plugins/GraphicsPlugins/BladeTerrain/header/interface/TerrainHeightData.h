/********************************************************************
	created:	2010/05/04
	filename: 	TerrainHeightData.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainHeightData_h__
#define __Blade_TerrainHeightData_h__
#include <BladeTerrain.h>

namespace Blade
{

	class TerrainHeightData
	{
	public:
		inline TerrainHeightData()
			:mX(0),mZ(0),mSizeX(0),mSizeZ(0)				{}
		inline TerrainHeightData(size_t x,size_t z,size_t cx,size_t cz)
			:mX(x),mZ(z),mSizeX(cx),mSizeZ(cz)				{}
		inline ~TerrainHeightData()							{}

		/** @brief  */
		inline size_t	getX() const	{return mX;}

		/** @brief  */
		inline size_t	getZ() const	{return mZ;}

		/** @brief  */
		inline size_t	getSizeX() const	{return mSizeX;}

		/** @brief  */
		inline size_t	getSizeZ() const	{return mSizeZ;}

	protected:
		size_t	mX;
		size_t	mZ;
		size_t	mSizeX;
		size_t	mSizeZ;
	};//class TerrainHeightData
	
}//namespace Blade


#endif //__Blade_TerrainHeightData_h__
