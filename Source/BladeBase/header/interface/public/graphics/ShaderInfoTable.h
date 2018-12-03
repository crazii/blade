/********************************************************************
	created:	2010/08/30
	filename: 	ShaderInfoTable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderInfoTable_h__
#define __Blade_ShaderInfoTable_h__
#include <utility/String.h>
#include "IShader.h"

namespace Blade
{
	namespace Impl
	{
		class ShaderInfoImpl;
	}//namespace Impl

	class BLADE_BASE_API ShaderInfoTable
	{
	public:
		ShaderInfoTable();
		~ShaderInfoTable();

		/** @brief  */
		const IShader::CONSTANT_INFO*	getConstantInfo(EShaderType shaderType, index_t index, SHADER_CONSTANT_TYPE type) const;

		/** @brief  */
		const IShader::CONSTANT_INFO*	getConstantInfoByName(EShaderType shaderType, const TString& name) const;

		/** @brief  */
		bool					addConstantInfo(const IShader::CONSTANT_INFO& cinfo);

		/** @brief  */
		size_t					getCount() const;

		/** @brief  */
		const IShader::CONSTANT_INFO*	getAt(index_t index) const;

		/** @brief  */
		inline const IShader::CONSTANT_INFO* operator[](index_t index) const
		{
			return this->getAt(index);
		}

	private:
		RefPimpl<Impl::ShaderInfoImpl>	mInfoData;
	};//class ShaderInfoTable
	
}//namespace Blade


#endif //__Blade_ShaderConstantTable_h__