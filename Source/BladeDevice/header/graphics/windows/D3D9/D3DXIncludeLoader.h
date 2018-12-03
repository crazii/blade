/********************************************************************
	created:	2016/03/28
	filename: 	D3DXIncludeLoader.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_D3DXIncludeLoader_h__
#define __Blade_D3DXIncludeLoader_h__
#include "../D3DIncludeLoader.h"

namespace Blade
{

	class D3DXIncludeLoader : public ID3DXInclude
	{
	public:
		D3DXIncludeLoader()
		{
			mLoader.bind( BLADE_FACTORY_CREATE(D3DIncludeLoader, D3DIncludeLoader::getIncludeLoaderType() ) );
		}
		virtual ~D3DXIncludeLoader()	{}

		/** @brief  */
		STDMETHOD(Open)(D3DXINCLUDE_TYPE includeType,LPCSTR pFileName,LPCVOID pParentData,LPCVOID *ppData,UINT *pByteLen)
		{
			D3D_INCLUDE_TYPE incType = includeType == D3DXINC_LOCAL ? D3D_INCLUDE_LOCAL : D3D_INCLUDE_SYSTEM;
			return mLoader->Open(incType, pFileName, pParentData, ppData, pByteLen);
		}

		/** @brief  */
		STDMETHOD(Close)(LPCVOID pData)
		{
			return mLoader->Close(pData);
		}

		/** @brief  */
		inline void setPath(const TString& path)
		{
			mLoader->setPath(path);
		}
	protected:
		Handle<D3DIncludeLoader> mLoader;
	};
	
}//namespace Blade

#endif // __Blade_D3DXIncludeLoader_h__
