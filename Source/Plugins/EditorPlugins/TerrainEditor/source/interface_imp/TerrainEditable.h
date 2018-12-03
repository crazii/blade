/********************************************************************
	created:	2011/05/22
	filename: 	TerrainEditable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainEditable_h__
#define __Blade_TerrainEditable_h__
#include <interface/public/IEditable.h>
#include <interface/public/EditableBase.h>
#include <interface/ITerrainInterface.h>

namespace Blade
{
	class IGeometry;

	class TerrainEditable : public EditableBase , public Allocatable
	{
	public:
		static const TString TERRAIN_EDITABLE_NAME;
	public:
		TerrainEditable();
		TerrainEditable(IEntity* TileEntity, IGeometry* geometry, const TString& resourcePath);
		~TerrainEditable();

		/************************************************************************/
		/* IEditable interface                                                                     */
		/************************************************************************/
		/*
		@describe run time creation
		@param 
		@return 
		*/
		virtual bool			initialize(const IConfig*,const ParamList* )	{return true;}

		/*
		@describe serialization: loading
		@param 
		@return 
		*/
		virtual bool			initialize(EditorElement* element);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			save();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		const Vector3&			getPosition() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const Vector3&			getScale() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const Quaternion&		getRotation() const;

		/*
		@describe 
		@param 
		@return 
		*/
		pint16					getHeightBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void					updateHeightBuffer(size_t x,size_t z,size_t sizeX,size_t sizeZ);

		/*
		@describe 
		@param 
		@return 
		*/
		puint8					getNormalBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		inline bool				needNormalUpdate() const	{return mNeedNormalUpdate;}

		/*
		@describe 
		@param 
		@return 
		*/
		void					updateNormalBuffer();

		/*
		@describe 
		@param 
		@return 
		*/
		const TStringList&		getTextureList() const;

		/*
		@describe 
		@param 
		@return 
		*/
		TERRAIN_LAYER*			getLayerBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		uint8*					getBlendBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void					updateBlockBlendBuffer(BLOCK_INDEX blockIndex,index_t StartX,index_t StartZ,
			index_t SizeX,size_t SizeZ,const TString& texture,bool updateTexture);

		//change access
		using EditableBase::setModified;

		/**
		@describe 
		@param
		@return
		*/
		ITerrainInterface*	getInterface();


	protected:
		IGeometry*	mGeometry;
		TString		mResourcePath;
		bool		mNeedNormalUpdate;
	};//class TerrainEditable

	

}//namespace Blade



#endif // __Blade_TerrainEditable_h__