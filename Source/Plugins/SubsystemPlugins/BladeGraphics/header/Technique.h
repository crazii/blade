/********************************************************************
	created:	2010/05/22
	filename: 	Technique.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Technique_h__
#define __Blade_Technique_h__
#include <Pimpl.h>
#include <ResourceState.h>
#include <interface/public/graphics/IShader.h>
#include <BladeGraphics.h>
#include <ShaderVariableMap.h>
#include <MaterialLOD.h>
#include <ShaderGroup.h>
#include <MaterialCloneShare.h>

namespace Blade
{
	class Material;
	class Pass;

	namespace Impl
	{
		class PassList;
	}//namespace Impl

	class BLADE_GRAPHICS_API Technique : public ResourceStateGroup, public Allocatable
	{
	public:
		Technique(const TString& name, const TString& profile, Material* parent);
		~Technique();

		/** @brief  */
		inline Material*		getParent() const
		{
			return mParent;
		}

		/** @brief  */
		inline const TString&	getName() const
		{
			return mName;
		}

		/** @brief  */
		inline const TString&	getProfile() const
		{
			return mProfile;
		}

		/** @brief  */
		inline void				setParent(Material* parent)
		{
			mParent = parent;
		}

		/************************************************************************/
		/* Pass functions                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		size_t			getPassCount() const;

		/*
		@describe
		@param
		@return
		*/
		Pass*			getPass(index_t index) const;

		/*
		@describe
		@param
		@return
		*/
		Pass*			getPass(const TString& name) const;

		/*
		@describe add pass into technique
		@note the pass object is internal managed and should not be shared or delete outside
		@param
		@return
		*/
		bool			addPass(Pass* pass);

		/************************************************************************/
		/* SortGroup functions                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		const ShaderGroupList&getShaderGroupList() const				{return mGroups;}

		/*
		@describe prepare space for sort group this is optional, used for memory optimization
		@param 
		@return 
		*/
		bool				prepareShaderGroups(size_t count);

		/*
		@describe
		@param
		@return
		*/
		index_t				addShaderGroup(const ShaderGroup& group);
		
		/************************************************************************/
		/* LOD functions                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		bool				addLOD(MATERIALLOD::LOD_INDEX level, scalar distanceStartSQ, scalar distanceEndSQ, uint8 groupIndex);

		/*
		@describe
		@param
		@return
		*/
		inline const LODSetting&	getLODSetting() const	{return mLODSettings;}

		/************************************************************************/
		/* helpers                                                                     */
		/************************************************************************/
		/*
		@describe iterate all shader instances and try set per-pass custom variable
		@param 
		@return 
		*/
		bool	setPassVariable(index_t pass, const TString& variableName,EShaderConstantType type,size_t count,const void* pData);

		/*
		@describe iterate all shader and try set per-shader variable
		@param 
		@return 
		*/
		bool	setShaderVariable(const TString& variableName,EShaderConstantType type,size_t count,const void* pData);

		/**
		@describe 
		@param
		@return
		*/
		bool	getShaderVariableHandle(ShaderVariableMap::Handles& outHandle, const TString& variableName, EShaderConstantType type, size_t count);

		/*
		@describe
		@param
		@return
		*/
		virtual Technique*		clone(int method = MS_ALL) const;

	protected:
		Technique(const Technique& src, int shareMethod);

		Material*		mParent;
		TString			mName;
		TString			mProfile;
	private:
		Technique& operator=(const Technique&);

		LODSetting		mLODSettings;
		ShaderGroupList	mGroups;
		Pimpl<Impl::PassList>			mPasses;
	};//class Technique

}//namespace Blade


#endif //__Blade_Technique_h__