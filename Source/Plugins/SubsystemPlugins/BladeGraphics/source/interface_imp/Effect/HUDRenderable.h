/********************************************************************
	created:	2015/12/30
	filename: 	HUDRenderable.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_HUDRenderable_h__
#define __Blade_HUDRenderable_h__
#include <interface/public/IRenderable.h>
#include <interface/IRenderQueue.h>
#include <SpaceContent.h>


namespace Blade
{

	class HUDRenderable : public SpaceContent, public IRenderable, public Allocatable
	{
	public:
		HUDRenderable();
		~HUDRenderable();

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual RenderType&				getRenderType() const;

		/**
		@describe 
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const;

		/**
		@describe 
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const	{return mMaterial;}

		/**
		@describe 
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const	{return mWorldTransform;}

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const { return const_cast<ISpaceContent*>(static_cast< const ISpaceContent*>(this)); }

		/************************************************************************/
		/* SpaceConent interface                                                                    */
		/************************************************************************/
		/**
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void			updateRender(IRenderQueue* queue)
		{
			if(mVisible && mMaterial->isLoaded() )
				queue->addRenderable(this);
		}

		/** @brief  */
		virtual	uint32			getAppFlag() const
		{
			return mVisible ? this->getRenderType().getAppFlag().getMask() : 0;
		}

		/** @brief  */
		virtual void			notifyPositionChange()
		{
			mWorldTransform.setTranslation(this->getPosition());
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void			setPixelSize(size_t x, size_t y);
		/** @brief  */
		void			setTexture(index_t index, const TString& texture);
		/** @brief  */
		void			setHighLight(bool highlit);
		/** @brief  */
		inline void		setVisible(bool visible)	{mVisible = visible;}

	protected:
		HMATERIALINSTANCE	mMaterial;
		Matrix44			mWorldTransform;
		const GraphicsGeometry* mQuadGeometry;
		bool				mVisible;
	};
	

}//namespace Blade

#endif // __Blade_HUDRenderable_h__
