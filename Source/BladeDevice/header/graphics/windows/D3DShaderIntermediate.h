/********************************************************************
	created:	2010/08/23
	filename: 	D3DShaderIntermediate.h
	author:		Crazii
	purpose:	
*********************************************************************/
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#ifndef __Blade_D3DShaderIntermediate_h__
#define __Blade_D3DShaderIntermediate_h__
#include <graphics/windows/D3D9/D3D9Header.h>

#include <interface/public/graphics/IShader.h>
#include <graphics/ShaderIntermediate.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <utility/IOBuffer.h>
#include <utility/BladeContainer.h>
#include <graphics/windows/D3DIncludeLoader.h>

namespace Blade
{
	class D3DShaderIntermediate : public ShaderIntermediate
	{
	public:
		static const TString HLSL_ID;
	public:
		D3DShaderIntermediate(const TString& name);
		~D3DShaderIntermediate();

		/************************************************************************/
		/* IShaderIntermediate interface                                                                     */
		/************************************************************************/
		/** @brief safe id through different shader framework, such as HLSL & CG,etc */
		virtual const TString&		getShaderID() const	{return HLSL_ID;}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		IShader::INFO_TABLE&getConstantTable()	{return mInfoTable;}
		/** @brief  */
		DWORD				getCompilingFlags(EShaderType type) const;
		/** @brief  */
		void				setCompilingFlags(EShaderType type, DWORD flags);

		/** @brief  */
		static const TString&		getD3DShaderID()	{return HLSL_ID;}
	protected:
		IShader::INFO_TABLE	mInfoTable;
		DWORD		mFlags[SHADER_TYPE_COUNT];
	};//class D3DShaderIntermediate

	
}//namespace Blade


#endif //__Blade_D3DShaderIntermediate_h__

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
