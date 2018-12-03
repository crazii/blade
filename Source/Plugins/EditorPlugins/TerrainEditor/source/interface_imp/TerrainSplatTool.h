/********************************************************************
	created:	2011/05/25
	filename: 	TerrainSplatTool.h
	author:		Crazii
	purpose:	
	log:		TerrainSplatOperation(per tile operation) is disabled,
				and TerrainBlockSplatOperation is used(per block operation)
*********************************************************************/
#ifndef __Blade_TerrainSplatTool_h__
#define __Blade_TerrainSplatTool_h__
#include "TerrainToolBase.h"
#include <memory/BladeMemory.h>
#include <interface/public/graphics/Color.h>
#include <interface/TerrainLayer.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class TerrainSplatOperation : public ITerrainBrushOperation
	{
	public:
		TerrainSplatOperation(EditorBrushTool::BrushData& data)
			:mBrushDataRef(data)	{}
		virtual ~TerrainSplatOperation()	{}

		/** @brief  */
		void	setTexture(const TString& texture);

	protected:

		//
		LAYER_ID		getLayerID(const TStringList& textureList, bool& settingTexture);

		//get blend channel for one vertex
		size_t			getBlendChannel(LAYER_ID layer, TERRAIN_LAYER& vertexLayer);

		//set layer as the second layer - layer1 (the layer0 should always be base map 0), and erase all other layer info (set layer3,layer4 empty)
		bool			resetLayer(LAYER_ID layer, TERRAIN_LAYER& vertexLayer);

		// per terrain process
		//virtual bool	process(const PosInfo& x,const PosInfo& z,TerrainEditable* te,scalar deltaTime,bool inverse);

		EditorBrushTool::BrushData&	mBrushDataRef;
		TString	mTexture;
		TString	mTextureRealPath;
	private:
		TerrainSplatOperation&	operator=(const TerrainSplatOperation&);
	};//class TerrainSplatOperation


	//////////////////////////////////////////////////////////////////////////
	class TerrainBlockSplatOperation : public TerrainSplatOperation
	{
	public:
		TerrainBlockSplatOperation(EditorBrushTool::BrushData& data)
			:TerrainSplatOperation(data)	{}

	protected:

		// per terrain process
		virtual bool	process(const PosInfo& x,const PosInfo& z,TerrainEditable* te,scalar deltaTime,bool inverse);

	private:
		TerrainBlockSplatOperation&	operator=(const TerrainBlockSplatOperation&);
	};//class TerrainBlockSplatOperation


	//////////////////////////////////////////////////////////////////////////
	class TerrainSplatTool : public TerrainBrushBaseTool, public StaticAllocatable
	{
	public:
		TerrainSplatTool(IconIndex icon,HOTKEY hotkey);
		~TerrainSplatTool();

	protected:

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				onSelection();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				onEnable(bool bEnabled);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ITerrainBrushOperation&	getBrushOperation();

	protected:

		/** @brief  */
		void				setSplatTexture(void* data);
		/** @brief change brush image when texture list item on UI is selected  */
		void				onOptionHighLight(const Event& data);

		TString						mSplatTexture;
		TerrainBlockSplatOperation	mBlockBrushOP;
		bool				mStateRegistered;
	};
	

}//namespace Blade



#endif // __Blade_TerrainSplatTool_h__