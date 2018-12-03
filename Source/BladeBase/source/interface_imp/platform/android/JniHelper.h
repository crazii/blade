/********************************************************************
	created:	2015/03/25
	filename: 	JniHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_JniHelper_h__
#define __Blade_JniHelper_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <utility/String.h>
#include <utility/BladeContainer.h>
#include <utility/CodeConverter.h>
#include <interface/IPlatformManager.h>
#include <jni.h>

namespace Blade
{

	namespace JniHelper
	{
		typedef TempVector<jchar>	JavaU16String;

		//DO NOT pass UTF8 string as input.
		/** @brief  */
		template<typename CharT>
		inline void convertString(const CharT* str, JavaU16String& outJString)
		{
			if( str == NULL )
			{
				//outJString[0] must be valid
				outJString.resize(1, jchar(0));
				return;
			}
			size_t size = Char_Traits<CharT>::length(str);
			outJString.resize( size );

			//android(/linux) wchar_t default size is 4 (UTF32)
			if( sizeof(CharT) == 4 )
				CodeConverter::CodeConvert(TE_UTF32, TE_UTF16, (const char*)str, size, (char*)&outJString[0], outJString.size()*sizeof(jchar) );
			else if( sizeof(CharT) == 1 )
				CodeConverter::CodeConvert(TE_MBCS, TE_UTF16, (const char*)str, size, (char*)&outJString[0], outJString.size()*sizeof(jchar) );
			else
				assert(false);
		}

		/** @brief  */
		extern IPlatformManager::MBRESULT
			showSystemDialog(const JavaU16String& u16Caption, const JavaU16String& u16Message, 
			const JavaU16String& u16Yes, const JavaU16String& u16No, const JavaU16String& u16Cancel, IPlatformManager::MBTYPE type);
		
	}//namespace JniHelper
	
}//namespace Blade


#endif//BLADE_PLATFORM
#endif // __Blade_JniHelper_h__