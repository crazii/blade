/********************************************************************
	created:	2010/06/28
	filename: 	TerrainType.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainType_h__
#define __Blade_TerrainType_h__
#include <RenderType.h>
#include <interface/public/graphics/IVertexDeclaration.h>

namespace Blade
{
	enum ETerrainStreamIndex
	{
		TSI_POSITION_XZ		= 0,
		TSI_POSITION_Y,

		TSI_NORMAL,			//put last because it is optional, avoid gap
		TSI_COUNT,
	};

	class TerrainType : public RenderType
	{
	public:
		TerrainType();
		~TerrainType();

		/************************************************************************/
		/*  IGraphicsType interface                                                                    */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual Material*			getMaterial() const;

		/*
		@describe update the material's profile. return a non NULL pointer indicates that
		the profile has been updated, otherwise the profile remains unchanged
		@param
		@return
		*/
		virtual Material*			updateProfile(const TString& profile, bool& updated);

		/*
		@describe
		@param
		@return
		*/
		virtual void				processRenderQueue(IRenderQueue* queue);

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceReady();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceClose();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		const HVDECL&			getVertexDeclaration()	{return mVertexDecl;}

	protected:
		HVDECL					mVertexDecl;
		mutable Material*		mMaterial;
	};//class TerrainType
	
}//namespace Blade


#endif //__Blade_TerrainType_h__
