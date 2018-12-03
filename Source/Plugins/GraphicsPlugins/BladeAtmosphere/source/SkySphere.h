/********************************************************************
	created:	2011/09/03
	filename: 	SkySphere.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_SkySphere_h__
#define __Blade_SkySphere_h__
#include <SpaceContent.h>
#include <math/Matrix44.h>
#include <interface/public/IRenderable.h>
#include <interface/IRenderQueue.h>
#include <interface/IAtmosphere.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/public/graphics/IVertexSource.h>
#include <MaterialInstance.h>

namespace Blade
{

	class SkySphere : public SpaceContent , public IRenderable
	{
	public:
		SkySphere();
		~SkySphere();

		/************************************************************************/
		/* SpaceContent interfaces                                                                     */
		/************************************************************************/
		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void					updateRender(IRenderQueue* queue)	{ queue->addRenderable(this);}

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual RenderType&				getRenderType() const			{return *static_cast<RenderType*>( IAtmosphere::getSingleton().getAtmosphereType() );}

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const				{return mGeometry;}

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const				{return mMaterial;}

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const	{return Matrix44::IDENTITY;}

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const { return const_cast<ISpaceContent*>(static_cast< const ISpaceContent*>(this)); }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		void		initialize(scalar radius,size_t density,const TString& texture = TString::EMPTY);

		/*
		@describe 
		@param 
		@return 
		*/
		scalar		getRadius() const		{return mRadius;}

		/*
		@describe 
		@param 
		@return 
		*/
		size_t		getDensity() const		{return mDensity;}

		/*
		@describe 
		@param 
		@return 
		*/
		const TString&	getTexture() const;


	protected:

		scalar				mRadius;
		size_t				mDensity;

		HMATERIALINSTANCE	mMaterial;
		GraphicsGeometry	mGeometry;
		HIBUFFER			mIndexBuffer;
		HVERTEXSOURCE		mVertexSource;
	};
	

	class SkyBox : public SpaceContent, public IRenderable
	{
	public:
		SkyBox();
		~SkyBox();

		/************************************************************************/
		/* SpaceContent interfaces                                                                     */
		/************************************************************************/
		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void					updateRender(IRenderQueue* queue) { queue->addRenderable(this); }

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual RenderType&				getRenderType() const { return *static_cast<RenderType*>(IAtmosphere::getSingleton().getAtmosphereType()); }

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const { return mMaterial; }

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const { return Matrix44::IDENTITY; }

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const { return const_cast<ISpaceContent*>(static_cast< const ISpaceContent*>(this)); }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		void		initialize(const TString& texture);

		/*
		@describe
		@param
		@return
		*/
		const TString& getTexture() const;


	protected:
		HMATERIALINSTANCE	mMaterial;
		const GraphicsGeometry*	mGeometry;
	};

}//namespace Blade



#endif // __Blade_SkySphere_h__