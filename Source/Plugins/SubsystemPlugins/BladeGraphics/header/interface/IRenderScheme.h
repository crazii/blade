/********************************************************************
	created:	2010/05/06
	filename: 	IRenderScheme.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderScheme_h__
#define __Blade_IRenderScheme_h__
#include <utility/Bindable.h>
#include <BladeGraphics.h>
#include <interface/public/graphics/ITexture.h>
#include <RenderType.h>
#include <interface/IRenderPipeline.h>
#include <interface/IRenderPhase.h>

namespace Blade
{
	class IRenderTarget;
	class IRenderDevice;
	class IGraphicsView;
	class ICamera;
	class ILightManager;
	class ICameraManager;
	class Material;

	class IRenderScheme
	{
	public:
		virtual ~IRenderScheme()	{}

		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const = 0;

		/**
		@describe get sorted profile list, in ascending order
		@param
		@return
		*/
		virtual const TStringList& getProfiles() const = 0;

		/*
		@describe 
		@param
		@return
		*/
		virtual const TString&	getProfile() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			setProfile(index_t index) = 0;

		inline bool				setProfile(const TString& profile)
		{
			index_t index = this->getProfiles().find(profile);
			return index != INVALID_INDEX ? this->setProfile(index) : false;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getBufferCount() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const HTEXTURE&getBuffer(index_t index) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const HTEXTURE&getBuffer(const TString& name) const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			updateCulling(ICamera* mainCamera = NULL) const = 0;

		/*
		@describe change of render target, camera, view for rendering, to gain more flexibility
		@param
		@return
		*/
		virtual void			render(IRenderDevice* device, bool swapBuffer,
			IRenderTarget* target = NULL, IGraphicsView* view = NULL, ICamera* camera = NULL) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			setupRender(IRenderDevice* device, IRenderTarget* target, IGraphicsView* view, ICamera* camera,
			ILightManager* lightManager,ICameraManager* cameraManager) = 0;

		/************************************************************************/
		/* render/output control                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual IRenderPipeline*	createPipeline() = 0;

	};//class IRenderScheme

	class BLADE_GRAPHICS_API IRenderSchemeOperation : public Bindable
	{
	public:
		virtual ~IRenderSchemeOperation() {}

		/** @brief  */
		void setTargetPrefix(const TString& prefix) { mPrefix = prefix; }

		/** @brief get prefix for buffer names, output names which need to be operate on */
		virtual const TString& getTargetPrefix() const { return mPrefix; }

		/** @brief gather outputs */
		virtual void initialize(ICameraManager* cameraManager, ILightManager* lightManager, IRenderPhase** phases, size_t phaseCount) = 0;

		/** @brief  */
		virtual void shutdown(ICameraManager* cameraManager, ILightManager* lightManager) = 0;

		/** @brief adjust buffer before creating buffers for scheme */
		virtual void adjustBuffer(const TString& name, size_t& width, size_t& height, PixelFormat& format, Sampler& sampler)
		{
			BLADE_UNREFERENCED(name); BLADE_UNREFERENCED(width); BLADE_UNREFERENCED(height); BLADE_UNREFERENCED(format); BLADE_UNREFERENCED(sampler);
		}

		/** @brief  */
		virtual void preCull(ICamera* sceneCamera, ICameraManager* cameraManager, ILightManager* lightManager)
		{
			BLADE_UNREFERENCED(sceneCamera); BLADE_UNREFERENCED(cameraManager); BLADE_UNREFERENCED(lightManager);
		}

		/** @brief  */
		virtual void postCull(ICamera* sceneCamera, ICameraManager* cameraManager, ILightManager* lightManager)
		{
			BLADE_UNREFERENCED(sceneCamera); BLADE_UNREFERENCED(cameraManager); BLADE_UNREFERENCED(lightManager);
		}

		/** @brief  */
		virtual void preRender(ICamera* /*sceneCamera*/) {}

		/** @brief  */
		virtual void postRender(ICamera* /*sceneCamera*/) {}
	private:
		TString	mPrefix;
	};
	
}//namespace Blade


#endif //__Blade_IRenderScheme_h__