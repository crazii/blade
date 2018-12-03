/********************************************************************
	created:	2011/08/26
	filename: 	TerrainEditorElement.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainEditorElement_h__
#define __Blade_TerrainEditorElement_h__
#include <interface/EditorElement.h>
#include <utility/StringList.h>
#include <utility/BladeContainer.h>
#include <interface/ITerrainInterface.h>

namespace Blade
{
	class TerrainEditorElement : public EditorElement
	{
	public:
		static const TString TERRAIN_EDITOR_ELEMENT;
	public:
		TerrainEditorElement();
		virtual ~TerrainEditorElement();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data in main synchronous state */
		virtual void	postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		pint16				getTerrainHeightBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void				updateTerrainHeightBuffer(size_t x, size_t z, size_t sizeX, size_t sizeZ);

		/*
		@describe 
		@param 
		@return 
		*/
		puint8				getTerrainNormalBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void				updateTerrainNormal();

		/*
		@describe 
		@param 
		@return 
		*/
		const TStringList&	getTextureList() const;

		/*
		@describe 
		@param 
		@return 
		*/
		TERRAIN_LAYER*		getLayerBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		uint8*				getBlendBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void				updateBlockBlendBuffer(BLOCK_INDEX blockIndex,index_t StartX,index_t StartZ,
			index_t SizeX,size_t SizeZ,const TString& texture,bool updateTexture);

		/*
		@describe 
		@param 
		@return 
		*/
		bool				isTerrainModifed() const;

		/**
		@describe 
		@param
		@return
		*/
		void				initialzeInterface();

		/** @brief  */
		inline ITerrainInterface*	getTerrainInterface() const { return mInterface; }

	protected:

		/** @brief  */
		virtual void		onInitialize() {}

		ITerrainInterface*	mInterface;
	};
	

}//namespace Blade



#endif // __Blade_TerrainEditorElement_h__