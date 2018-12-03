/********************************************************************
	created:	2011/06/02
	filename: 	CustomEffect.h
	author:		Crazii
	purpose:	
	log:		this file changed from CustomGeometry to CustomEffect on 12/29/2012
				combine custom geometry & effect
*********************************************************************/
#ifndef __Blade_CustomEffect_h__
#define __Blade_CustomEffect_h__
#include <math/Matrix44.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/IGraphicsEffect.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/IEventManager.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <SpaceContent.h>
#include <interface/public/IRenderable.h>
#include <MaterialInstance.h>

namespace Blade
{
	//manager for custom buffers
	class EffectManager;

	class CustomEffect : public IGraphicsEffect, public IResource::IListener,
		public SpaceContentBase, public IRenderable, public Allocatable
	{
	public:
		static const TString CUSTOM_EFFECT_TYPE;
		CustomEffect(Material* material, bool _dynamic, bool independent);
		~CustomEffect();

		/************************************************************************/
		/* IGraphicsEffect interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	isReady() const {return this->getSpace() != NULL &&  this->isMaterialLoaded();}
		/** @brief  */
		virtual bool	onAttach();
		/** @brief  */
		virtual bool	onDetach();

		/************************************************************************/
		/* IResource::IListener interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	onReady()
		{
			this->setVisible(true);
			return true;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	onUnload()
		{
			this->setVisible(false);
		}

		/************************************************************************/
		/* SpaceContent interface                                                                     */
		/************************************************************************/
		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void			updateRender(IRenderQueue* queue);

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual RenderType&				getRenderType() const;

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
		virtual MaterialInstance*		getMaterial() const				{return mMaterial;}

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const		{return Matrix44::IDENTITY;}

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
		bool					isMaterialLoaded() const
		{
			return mMaterial == NULL || mMaterial->isLoaded();
		}

		/*
		@describe 
		@param 
		@return 
		*/
		void					loadMaterial()
		{
			if(mMaterial != NULL)
				mMaterial->load();
		}

		/*
		@describe 
		@param 
		@return 
		*/
		bool					isUsingIndex() const			{return mIndexBuffer != NULL;}

		/*
		@describe 
		@param 
		@return 
		*/
		const HVBUFFER&			getVertexBuffer() const	{return mVertexBuffer;}

		/*
		@describe 
		@param 
		@return 
		*/
		void					setImmediateBuffer(const Vector3* vb, size_t vbCount, const uint32* ib, size_t ibCount, bool buildBounding);

		/*
		@describe 
		@param 
		@return 
		*/
		void					clearImmediateBuffer(const Event&);

		/*
		@describe 
		@param 
		@return 
		*/
		const HMATERIALINSTANCE&setCustomMaterial(Material* material);

		/** @brief  */
		MaterialInstance*		getMaterialInstance() const			{return mMaterial;}

		/** @brief  */
		inline void				setListenMaterialLoad(bool needListen)	{mNeedListenMaterial = needListen;}

	protected:
		HMATERIALINSTANCE	mMaterial;
		HVBUFFER			mVertexBuffer;
		HIBUFFER			mIndexBuffer;
		HVERTEXSOURCE		mVertexSource;

		GraphicsGeometry	mGeometry;

		bool				mNeedListenMaterial;
		bool				mListeningMaterialLoading;
		//already in space before attach
		bool				mInSpace;

		static HVDECL	msVertexDeclaration;
	};
	

}//namespace Blade



#endif // __Blade_CustomEffect_h__