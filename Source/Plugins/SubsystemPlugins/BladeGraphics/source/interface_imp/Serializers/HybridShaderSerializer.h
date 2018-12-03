/********************************************************************
	created:	2014/12/29
	filename: 	HybridShaderSerializer.h
	author:		Crazii
	purpose:	hybrid serializer to save text content and other binary informations(semantics, etc)
*********************************************************************/
#ifndef __Blade_HybridShaderSerializer_h__
#define __Blade_HybridShaderSerializer_h__
#include <ShaderResource.h>
#include "BinaryShaderSerializer.h"

namespace Blade
{

	class HybridShaderSerializer : public BinaryShaderSerializer
	{
	public:
		HybridShaderSerializer();
		~HybridShaderSerializer();

		/************************************************************************/
		/* BinaryShaderSerializer overrides                                                                     */
		/************************************************************************/
		/*@brief prepare content for saving */
		virtual bool	getContentBuffer(SubShader* subShader, Buffer& buffer);

		/*@brief loading intermediate from content buffer */
		virtual HSHADERINTMEDIA	loadContentBuffer(const TString& name, const void* data, size_t bytes,
			const TString& profile, const TString entrypoint[SHADER_TYPE_COUNT]);
	protected:

	};

}//namespace Blade

#endif//__Blade_HybridShaderSerializer_h__