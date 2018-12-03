/********************************************************************
	created:	2011/05/26
	filename: 	TerrainToolManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainToolManager_h__
#define __Blade_TerrainToolManager_h__
#include <utility/BladeContainer.h>
#include <interface/IFileUINotify.h>
#include "TerrainToolBase.h"

namespace Blade
{
	class TerrainEditable;

	class TerrainToolManager : public Singleton<TerrainToolManager>
	{
	public:
		TerrainToolManager();
		~TerrainToolManager();
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		void	initalize();

		/*
		@describe 
		@param
		@return
		*/
		void	shutdown();

		/** @brief none-DLL interface */
		typedef Vector<TerrainEditable*>	TerrainEditableList;
		/** @brief  */
		const TerrainEditableList&	getEditableList() const							{return mEditableList;}
		/** @brief  */
		void						addTerrainEditable(TerrainEditable* editable)	{mEditableList.push_back(editable);}
		/** @brief  */
		bool						removeTerrainEditable(TerrainEditable* editable)
		{
			TerrainEditableList::iterator it = std::find(mEditableList.begin(), mEditableList.end(), editable);
			if (it != mEditableList.end())
			{
				mEditableList.erase(it);
				return true;
			}
			return false;
		}
		/** @brief  */
		void						clearTerrainEditables()							{mEditableList.clear();}

	protected:
		typedef StaticVector<TerrainBrushBaseTool*>		TerrainBrushList;
		typedef StaticVector<TerrainCommandBaseTool*>	TerrainCommandList;

		TerrainBrushList	mBrushTools;
		TerrainCommandList	mCmdTools;
		TerrainEditableList	mEditableList;
	};
	

}//namespace Blade



#endif // __Blade_TerrainToolManager_h__