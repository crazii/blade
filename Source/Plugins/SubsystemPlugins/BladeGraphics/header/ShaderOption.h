/********************************************************************
	created:	2012/02/25
	filename: 	ShaderOption.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderOption_h__
#define __Blade_ShaderOption_h__
#include <BladeGraphics.h>
#include <ResourceState.h>
#include <ShaderInstance.h>
#include <MaterialLOD.h>

namespace Blade
{
	extern template class BLADE_GRAPHICS_API FixedVector<uint8, MATERIALLOD::MAX_LOD>;

	class BLADE_GRAPHICS_API ShaderOption : public ResourceStateGroup, public Allocatable
	{
	public:
		//an array of sort group index to addressing sort groups
		typedef FixedVector<uint8, MATERIALLOD::MAX_LOD> GroupListLink;

	public:
		ShaderOption(const TString& name);
		ShaderOption(const ShaderOption& src, int method);
		~ShaderOption();

		/************************************************************************/
		/* shader                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		ShaderInstance*			createShader(const TString& profile);

		/*
		@describe
		@param
		@return
		*/
		ShaderInstance*			getShaderInstance() const;

		/*
		@describe
		@param
		@return
		*/
		const GroupListLink&	getGroupLink() const	{return mGroupLink;}

		/*
		@describe
		@param
		@return
		*/
		void					addGroupLink(uint8 groupIndex)	{mGroupLink.push_back(groupIndex);}

		/** @brief  */
		void					addGroupLinks(uint8 start, uint8 end)
		{
			end = end > MATERIALLOD::MAX_LOD ? MATERIALLOD::MAX_LOD : end;
			for(uint8 i = start; i < end; ++i)
				mGroupLink.push_back(i);
		}

		/*
		@describe 
		@param 
		@return 
		*/
		void					setOptionIndex(index_t index);

		/** @brief  */
		const HSHADER&			getProgram() const
		{
			return mProgram != NULL ? mProgram->getShader() : HSHADER::EMPTY;
		}

		/** @brief  */
		const TString&			getName() const { return mName; }

	protected:
		HSHADERINSTANCE			mProgram;
		GroupListLink			mGroupLink;
		TString					mName;
	};//class ShaderOption

	typedef Handle<ShaderOption> HSHADEROPTION;

}//namespace Blade


#endif //__Blade_ShaderOption_h__