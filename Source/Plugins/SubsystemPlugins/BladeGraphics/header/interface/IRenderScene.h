/********************************************************************
	created:	2012/03/25
	filename: 	IRenderScene.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderScene_h__
#define __Blade_IRenderScene_h__
#include <interface/public/graphics/IGraphicsScene.h>
#include <BladeGraphics.h>

namespace Blade
{
	class IRenderDevice;
	class IRenderScheme;
	class ParaStateQueue;
	class ILightManager;
	class ICameraManager;
	class IAABBRenderer;
	class IGraphicsUpdater;
	class IMaterialLODUpdater;
	class ICamera;
	class ISpaceCoordinator;

	class BLADE_GRAPHICS_API IRenderScene : public IGraphicsScene
	{
	public:
		static const TString GRAPHICS_SCENE_TYPE;
	public:
		/*
		@describe
		@param
		@return
		*/
		virtual bool			setupRenderDevice(IRenderDevice* device) = 0;

		/*
		@describe synchronous update
		@param 
		@return 
		*/
		virtual void			onMainLoopUpdate() = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			isRenderTargetUsed(IRenderTarget* target) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			useRenderTarget(IRenderTarget* target) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			unuseRenderTarget(IRenderTarget* target) = 0;

		/*
		@describe get current render scheme for this scene (per scene)
		@param 
		@return 
		*/
		virtual IRenderScheme*	getRenderScheme() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual IStage*			getStage() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual ParaStateQueue*	getStateQueue() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsUpdater*getUpdater() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual IAABBRenderer*	getAABBRenderer() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual ILightManager*	getLightManager() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual ICameraManager*	getCameraManager() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual IMaterialLODUpdater*	getMaterialLODUpdater() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ICamera*		getMainCameraImpl() const = 0;

		/** @brief internal method without state check */
		virtual ISpaceCoordinator*			getSpaceCoordinatorImpl() const = 0;
	};
	
	extern template class BLADE_GRAPHICS_API Factory<IRenderScene>;
	typedef Factory<IRenderScene> RenderSceneFactory;


}//namespace Blade


#endif //__Blade_IRenderScene_h__