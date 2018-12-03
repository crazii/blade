/********************************************************************
	created:	2010/09/02
	filename: 	ShaderInstance.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderInstance_h__
#define __Blade_ShaderInstance_h__
#include <BladeGraphics.h>
#include <ResourceState.h>
#include <ShaderResource.h>

namespace Blade
{
	namespace Impl
	{
		class SubShaderSamplerDataImpl;
	}//namespace Impl

	class BLADE_GRAPHICS_API ShaderInstance : public SubResourceState , public Allocatable
	{
	public:
		ShaderInstance(const TString& profile);
		~ShaderInstance();

		ShaderInstance(const ShaderInstance& src);
		ShaderInstance& operator=(const ShaderInstance& rhs);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/*
		@describe
		@param
		@return
		*/
		const HSHADER&			getShader() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const ShaderResource*	getShaderResource() const;

		/*
		@describe 
		@param
		@return
		*/
		const TString&			getProfile() const
		{
			return mProfile;
		}

		/*
		@describe
		@param
		@return
		*/
		void				updateBultInVariable(EShaderVariableUsage usage, IRenderDevice* renderer) const;

		/*
		@describe
		@param
		@return
		*/
		void				updateCustomVariable(EShaderVariableUsage usage, IRenderDevice* renderer) const;

		/*
		@describe update texture binding according to sampler map info
		@param
		@return
		*/
		void				updateTexture(IRenderDevice* renderer, const TextureStateInstance state, const SamplerState& samplers) const;

		/*
		@describe map samplers to texture states
		@param
		@return
		*/
		bool				linkSampler(TStringList& uniforms, const SamplerState& samplers);

		/*
		@describe 
		@param 
		@return 
		*/
		inline void			updateShaderVariable(EShaderVariableUsage usage, IRenderDevice* renderer) const
		{
			this->updateBultInVariable(usage, renderer);
			this->updateCustomVariable(usage, renderer);
		}

		/*
		@describe per-shader custom variables (auto created)
		@param 
		@return 
		*/
		bool	setShaderVariable(const TString& variableName, EShaderConstantType ctype, size_t count, const void* pData)
		{
			if( mSubShader != NULL )
				return mSubShader->setShaderVariable(variableName, ctype, count, pData);
			else
			{
				assert(false);
				return false;
			}
		}

		/** @brief  */
		void				createAndLinkCustomInstanceVariable(InstanceVariableMap& variableData) const
		{
			if( mSubShader != NULL )
				mSubShader->createAndLinkCustomInstanceVariable(variableData);
			else
				assert(false);
		}

		/** @brief  */
		size_t				getInstanceVaraibleCount() const
		{
			return mSubShader != NULL ? mSubShader->getInstanceVariableCount() : 0;
		}

		/** @brief  */
		inline void		setOptionIndex(index_t index)			{mOptionIndex = index;}
		inline index_t	getOptionIndex() const					{return mOptionIndex;}
		inline void		setSubShaderName(const TString& name)	{mSubShaderName = name;}
		inline const TString& getSubShaderName() const			{return mSubShaderName;}

		/** @brief  */
		inline const ShaderVariableMap& getShaderVariables() const
		{
			return mSubShader != NULL ? mSubShader->getShaderVariables() : ShaderVariableMap::EMPTY;
		}

		/** @brief  */
		inline const PassVariableMap&	getPassShaderVariables() const
		{
			return mCustomPassVariables;
		}

		/** @brief  */
		inline bool	setPassShaderVariables(const TString& name, EShaderConstantType ctype, size_t count, const void* pData)
		{
			return mCustomPassVariables.writeVariable(name, ctype, count, pData);
		}

	protected:

		/************************************************************************/
		/* ResourceState interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			postLoad(const HRESOURCE& resource);

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onUnload();

	protected:
		index_t					mOptionIndex;
		SubShader*				mSubShader;
		TString					mProfile;
		TString					mSubShaderName;

		//custom per-pass shader variables
		//a pass has its unique shader instances, so it suitable to store per-pass variables
		//in shader instances, and this will also avoid name conflict for different sub shader within the same pass

		PassVariableMap	mCustomPassVariables;
		LazyPimpl<Impl::SubShaderSamplerDataImpl>	mSamplerData;
	};//class ShaderInstance

	typedef Handle<ShaderInstance> HSHADERINSTANCE;
	
}//namespace Blade


#endif //__Blade_ShaderInstance_h__