/********************************************************************
	created:	2010/04/30
	filename: 	Material.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Material_h__
#define __Blade_Material_h__
#include <BladeGraphics.h>
#include <ResourceState.h>
#include <Pimpl.h>
#include <GraphicsResource.h>
#include <MaterialCloneShare.h>
#include <utility/StringList.h>

namespace Blade
{
	namespace Impl
	{
		class MaterialDataImpl;
	}//namespace Impl

	class Pass;
	class Technique;

	class BLADE_GRAPHICS_API Material : public GraphicsResource, public ResourceStateGroup
	{
	public:
		static const TString MATERIAL_RESOURCE_TYPE;

		class IListener
		{
		public:
			/** @brief  */
			virtual void	onActivateTechnique(Technique* old, Technique* now) = 0;
		};

	public:
		Material(const TString& name);
		virtual ~Material();

		/*
		@describe
		@param
		@return
		*/
		inline const TString&	getName() const
		{
			return mName;
		}

		/*
		@describe
		@param
		@return
		*/
		size_t					getTechniqueCount() const;

		/*
		@describe
		@param
		@return
		*/
		Technique*				getTechnique(index_t index) const;

		/*
		@describe
		@param
		@return
		*/
		Technique*				getTechnique(const TString& name) const;

		/*
		@describe get technique by profile, if exact matching not found, use lower profile instead
		@param
		@return
		*/
		index_t					getTechniqueByProfile(const TString& name) const;

		/*
		@describe add technique into technique
		@note the technique object is internal managed and should not be shared or delete outside
		@param
		@return
		*/
		bool					addTechnique(Technique* tech);

		/*
		@describe
		@param
		@return
		*/
		Technique*				setActiveTechnique(index_t index);

		/*
		@describe
		@param
		@return
		*/
		Technique*				setActiveTechnique(const TString& techName);

		/*
		@describe 
		@param
		@return
		*/
		bool					setActiveTechnique(Technique* tech);

		/*
		@describe
		@param
		@return
		*/
		Technique*				getActiveTechnique() const;

		/*
		@describe 
		@param
		@return
		*/
		size_t					getMaxGroupCount() const;

		/*
		@describe 
		@param
		@return
		*/
		bool					addListener(IListener* listener);

		/*
		@describe 
		@param
		@return
		*/
		bool					removeListener(IListener* listener);

		inline const TStringList&getTextureUniforms() const	{return mTextureUniforms;}

		/**
		@describe set property to all techniques & passes
		@param 
		@return 
		*/
		void					setRenderProperty(const HRENDERPROPERTY& prop);

		/** @brief  */
		inline bool				hasCustomInstanceShaderVariables() const	{return mHasCustomInstanceShaderConstants;}
	
		/*
		@describe
		@param
		@return
		*/
		virtual Handle<Material>	clone(const TString& name, int method = MS_ALL) const = 0;

	protected:
		/**
		@describe set uniforms
		@param
		@return
		*/
		void					setTextureUniforms(const TStringParam& uniforms);

		/*
		@describe
		@param
		@return
		*/
		void					removeAllTechniques();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onAllReady();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onUnload();

	protected:

		TString					mName;
		Technique*				mActiveTech;
		TStringList				mTextureUniforms;
		uint32					mFlags;
		bool					mNeedUpdate;
		bool					mHasCustomInstanceShaderConstants;

	private:
		Material&			operator=(const Material&);
		Pimpl<Impl::MaterialDataImpl>	mData;
	};//Material

	typedef Handle<Material>	HMATERIAL;
	
}//namespace Blade


#endif //__Blade_Material_h__