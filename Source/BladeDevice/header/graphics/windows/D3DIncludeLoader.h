/********************************************************************
	created:	2011/08/13
	filename: 	D3DIncludeLoader.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_D3DIncludeLoader_h__
#define __Blade_D3DIncludeLoader_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <d3dcommon.h>
#include <BladeDevice.h>
#include <utility/IOBuffer.h>
#include <utility/BladeContainer.h>
#include <utility/StringHelper.h>
#include <utility/StringList.h>

namespace Blade
{

	class BLADE_DEVICE_API D3DIncludeLoader : public ID3DInclude, public TempAllocatable
	{
	public:
		D3DIncludeLoader()	{}
		virtual ~D3DIncludeLoader() { for(BufferList::iterator i = mHeaderBuffer.begin(); i != mHeaderBuffer.end(); ++i) { BLADE_DELETE i->second;}}

		/** @brief  */
		STDMETHOD(Open)(D3D_INCLUDE_TYPE IncludeType,LPCSTR pFileName,LPCVOID pParentData,LPCVOID *ppData,UINT *pByteLen);

		/** @brief  */
		STDMETHOD(Close)(LPCVOID pData)
		{
			BufferList::iterator i = mHeaderBuffer.find(pData);
			if(i == mHeaderBuffer.end() )
				return S_FALSE;

			BLADE_DELETE i->second;
			mHeaderBuffer.erase(i);
			return S_OK;
		}

		/** @brief  */
		inline void setPath(const TString& path)
		{
			mPathList.push_back( TStringHelper::standardizePath(path) );
		}

		/*
		@describe set the include loader, this allows expanding for different loaders
		@remark the default loader exists as "Default" ,the loader is created internally, by reading the type here
		@param 
		@return 
		*/
		static inline void		setIncludeLoaderType(const TString& loaderType)
		{
			msLoaderType = loaderType;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		static const TString&	getIncludeLoaderType()
		{
			return msLoaderType;
		}

	protected:
		class IncludeBuffer : public IOBuffer, public TempAllocatable
		{
		public:
			using TempAllocatable::operator new;
			using TempAllocatable::operator delete;
		};
		typedef Map<const void*, IncludeBuffer*>	BufferList;

		TStringParam	mPathList;
		BufferList		mHeaderBuffer;
	private:
		static TString	msLoaderType;
	};//class D3DIncludeLoader


}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#endif // __Blade_D3DIncludeLoader_h__