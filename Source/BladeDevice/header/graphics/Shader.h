/********************************************************************
	created:	2014/12/22
	filename: 	Shader.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Shader_h__
#define __Blade_Shader_h__
#include <interface/public/graphics/IShader.h>
#include <interface/public/graphics/ShaderInfoTable.h>

namespace Blade
{
	class ShaderIntermediate;
	class Shader : public IShader
	{
	public:
		Shader(const ShaderIntermediate* intermediate);
		virtual ~Shader();

		/************************************************************************/
		/* IShader interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const INFO_TABLE& getConstantTable() const	{return mInfoTable;}

		/*
		@describe
		@param
		@return
		*/
		virtual const TString& getProfileString(EShaderType type) const;
		
		/*
		@describe 
		@param
		@return
		*/
		virtual const TString&	getEntry(EShaderType type) const;

	protected:
		INFO_TABLE	mInfoTable;
		TString		mProfile[SHADER_TYPE_COUNT];
		TString		mEntryPoint[SHADER_TYPE_COUNT];
	};
	
}//namespace Blade


#endif // __Blade_Shader_h__