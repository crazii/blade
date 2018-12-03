/********************************************************************
	created:	2011/06/08
	filename: 	TerrainBrushRect.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainBrushRect_h__
#define __Blade_TerrainBrushRect_h__
#include <utility/BladeContainer.h>
#include <math/Box2.h>
#include "TerrainEditable.h"
#include "ITerrainBrushOperation.h"


namespace Blade
{

	typedef TempVector<TerrainEditable*>	TerrainEditableList;

	class TerrainBrushRect
	{
	public:
		TerrainBrushRect();
		~TerrainBrushRect();

		/*
		@describe 
		@param 
		@return 
		*/
		size_t		getSize() const		{return mSize;}

		/*
		@describe setup a rect region
		@param 
		@return 
		*/
		void		setup(const TerrainEditableList& list, size_t size, scalar left, scalar top);

		/*
		@describe process the setup rect
		@param 
		@return 
		*/
		void		process(ITerrainBrushOperation& operation, scalar deltaTime, bool inverse);

		/*
		@describe 
		@param 
		@return 
		*/
		void		clear();

		/*
		@describe 
		@param 
		@return 
		*/
		TerrainEditable*	transformPosition(index_t& x, index_t& z);


	protected:
		typedef TempVector<PosInfo>	InfoList;
		typedef TempVector<TerrainEditableList> TerrainArray;
		
		size_t		mSize;
		InfoList	mXInfo;
		InfoList	mZInfo;
		TerrainArray	mTerrains;
	};
	

}//namespace Blade



#endif // __Blade_TerrainBrushRect_h__