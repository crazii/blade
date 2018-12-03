/********************************************************************
	created:	2010/09/05
	filename: 	IShaderVariableSource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IShaderVariableSource_h__
#define __Blade_IShaderVariableSource_h__
#include <BladeGraphics.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/graphics/IRenderView.h>
#include <interface/public/graphics/IShader.h>
#include <interface/public/graphics/ITexture.h>
#include <AutoShaderVariable.h>

namespace Blade
{
	class ICamera;
	class ILightManager;
	class IGlobalLightVariables;
	class IGraphicsView;

	class RenderPropertySet;
	struct RenderOperation;

	class IShaderVariableSource : public InterfaceSingleton<IShaderVariableSource>
	{
	public:
		virtual ~IShaderVariableSource()	{}

		/**
		@describe 
		@param
		@return
		*/
		virtual void					onShaderSwitch(bool dynamicSwithing) = 0;

		/*
		@describe get engine built-in semantics, maybe global or per-instance
		@param
		@return
		*/
		virtual AutoShaderVariable*		getBuiltInVariable(const TString& semantic) = 0;

		/*
		@describe create/get global custom shader variables, this type of variable is shared among all shaders
		this is usually auto called automatically by framework, but you can create it manually
		you can set global custom variable by calling IShaderVariableSource::setGlobalCustomVariable
		@param 
		@return 
		@remarks create variable if it not exist, otherwise get the existing variable
		*/
		virtual const HAUTOSHADERVARIABLE&	createGlobalCustomVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HAUTOSHADERVARIABLE&	getGlobalCustomVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool					setGlobalCustomVariable(const HAUTOSHADERVARIABLE& variable, const void* source, size_t bytes) = 0;

		inline bool setGlobalCustomVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count, const void* source, size_t bytes)
		{
			const HAUTOSHADERVARIABLE& var = this->getGlobalCustomVariable(name, type, count);
			if (var == NULL || var->getRealSize() < bytes)
				return false;
			return this->setGlobalCustomVariable(var, source, bytes);
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void					setCamera(const ICamera* camera) = 0;
		inline const ICamera*			getCamera() const
		{
			return mCamera;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual void					setView(const IRenderView* view) = 0;
		inline const IRenderView*		getView() const
		{
			return mView;
		}
		
		/*
		@describe instance data source
		@param
		@return
		*/
		virtual void					setRenderOperation(const RenderOperation* rop) = 0;
		inline const RenderOperation*	getRenderOperation() const
		{
			return mROP;
		}

		/*
		@describe set render properties. multiple property set will merged into new set
		@param 
		@return 
		*/
		virtual bool					setRenderProperties(const RenderPropertySet* passProperty,
			const RenderPropertySet* customPassProperty, const RenderPropertySet* instanceProperty) = 0;

		/** @brief get probably merged property set */
		inline const RenderPropertySet*		getRenderProperty() const 
		{
			return mPropertySet;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void					setLightManager(ILightManager* lm) = 0;
		inline ILightManager*			getLightManager() const
		{
			return mLightManager;
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual void		beginFrame() = 0;

	protected:

		const ICamera*		mCamera;
		const IRenderView*	mView;
		const RenderOperation* mROP;
		const RenderPropertySet*	mPropertySet;
		ILightManager*		mLightManager;
	};//class IShaderVariableSource

	extern template class BLADE_GRAPHICS_API Factory<IShaderVariableSource>;
	
}//namespace Blade


#endif //__Blade_IShaderVariableSource_h__