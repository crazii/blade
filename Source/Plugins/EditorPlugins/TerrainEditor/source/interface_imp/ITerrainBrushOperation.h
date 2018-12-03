/********************************************************************
	created:	2011/08/12
	filename: 	ITerrainBrushOperation.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ITerrainBrushOperation_h__
#define __Blade_ITerrainBrushOperation_h__


namespace Blade
{
	//fwd declaration
	class TerrainEditable;

	struct PosInfo
	{
		//rect local bounds
		index_t mStart;
		index_t mEnd;
		//terrain space bounds
		index_t	mTerranStart;
		index_t mTerranEnd;
	};

	class ITerrainBrushOperation
	{
	public:

		// per terrain process
		virtual bool	process(const PosInfo& x, const PosInfo& z, TerrainEditable* te, scalar deltaTime, bool inverse) = 0;

	};//class ITerrainBrushOperation
	

}//namespace Blade



#endif // __Blade_ITerrainBrushOperation_h__