/********************************************************************
created:	2017/10/20
filename: 	GrassRenderType.h
author:		Crazii
purpose:	
*********************************************************************/
#ifndef __Blade_GrassRenderType_h__
#define __Blade_GrassRenderType_h__
#include <RenderType.h>


namespace Blade 
{
	class GrassRenderType : public RenderType
	{
	public:
		static const TString GRASS_RENDER_TYPE;
	public:
		GrassRenderType();
		~GrassRenderType();

		/************************************************************************/
		/*  IGraphicsType interface                                                                    */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual Material*		getMaterial() const;

		/*
		@describe
		@param
		@return
		*/
		virtual void			processRenderQueue(IRenderQueue* queue);

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

	protected:
		mutable Material*		mMaterial;
	};
	
}//namespace Blade 

#endif // __Blade_GrassRenderType_h__