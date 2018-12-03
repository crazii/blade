/********************************************************************
	created:	2015/08/04
	filename: 	PlatformData.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#define BLADE_USE_PLATFORM_DATA_EXPLICIT
#include <platform/PlatformData.h>
#include "PlatformData_Private.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
//internalMessagePump
#pragma comment (lib, "user32.lib")
#endif

extern "C"
{
	extern struct android_app* android_app_instance;
}

namespace Blade
{
	template class Singleton<PlatformData>;
	template class Singleton<PlatformData_Private>;

	static const TString LANGUAGE_SETTINGS[] =
	{
		BTString("en_US"),
		BTString("zh_TW"),
		BTString("zh_CN"),

		//...
		BTString("")
	};


	//////////////////////////////////////////////////////////////////////////
	PlatformData::PlatformData()
	{
		mData.userData = NULL;
		mData.appData = NULL;

#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		mVersionCode = 0;
		mClassLoader = NULL;
		mLoadMethod = NULL;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	PlatformData::~PlatformData()
	{
		this->shutdown();
	}

	//////////////////////////////////////////////////////////////////////////
	bool PlatformData::initialize()
	{
		//init language setting: it never changes even if user changes system languages
		enum ELanguages
		{
			LANG_START,

			LANG_EN_US = LANG_START,
			LANG_ZH_TW,
			LANG_ZH_CN,
			//...

			LANG_COUNT,
		};

		ELanguages lang = LANG_EN_US;
		String CPString;

		/************************************************************************/
		/* Platform specific                                                                     */
		/************************************************************************/

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		//app data
		mData.appData = ::GetModuleHandle(NULL);

		//languages
		//https://msdn.microsoft.com/en-us/library/dd317756.aspx
		UINT codePage = ::GetACP();
		CPString = BString("CP") + StringHelper::fromUInt(codePage);

		switch(codePage)
		{
		case 936:	//gb2312
		case 10008: //x-mac-chinesesimp
		case 20936: //x-cp20936
		case 50227: //x-cp50227
		case 50935: //EBCDIC Simplified Chinese Extended and Simplified Chinese
		case 50936: //EBCDIC Simplified Chinese
		case 51936: //EUC Simplified Chinese
		case 52936:	//hz-gb-2312
		case 54936:	//GB18030
			lang = LANG_ZH_CN;
			break;
		case 950:	//big5
		case 10002:	//x-mac-chinesetrad
		case 20000:	//x-Chinese_CNS
		case 20001:	//TCA Taiwan
		case 20002:	//x_Chinese-Eten
		case 50229:	//ISO 2022 Traditional Chinese
		case 50937:	//EBCDIC US-Canada and Traditional Chinese
		case 51950:	//EUC Traditional Chinese
			lang = LANG_ZH_TW;
			break;
		default:
			lang = LANG_EN_US;
		}

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		//app data
		mData.appData = android_app_instance->activity;
		ANativeActivity* activity = mData.appData;
		if( activity == NULL )
		{
			assert(false);
			return false;
		}

		//languages
		JNIEnv* env = this->attachJNI();
		jclass clsActivity = env->GetObjectClass(activity->clazz);

		//java.nio.charset.Charset
		jclass clsCharset = env->FindClass("java/nio/charset/Charset");
		jmethodID midDefault = env->GetStaticMethodID(clsCharset, "defaultCharset", "()Ljava/nio/charset/Charset;");
		jobject objcharSet = env->CallStaticObjectMethod(clsCharset, midDefault);
		jmethodID midGetName = env->GetMethodID(clsCharset, "displayName", "()Ljava/lang/String;");
		jstring name = static_cast<jstring>( env->CallObjectMethod(objcharSet, midGetName) );
		const char* buffer = env->GetStringUTFChars(name, NULL);
		CPString = buffer;


		env->ReleaseStringUTFChars(name, buffer);
		env->DeleteLocalRef(name);
		env->DeleteLocalRef(objcharSet);
		env->DeleteLocalRef(clsCharset);

		union 
		{
			uint32	langInt;
			char	langStr[4];
		}langU = {0};

		//TODO: use Locale class in Java via JNI? (Locale.toString())
		::AConfiguration_getLanguage(android_app_instance->config, langU.langStr);
		switch(langU.langInt)
		{
		case BLADE_FCC('e','n', 0, 0):
			lang = LANG_EN_US;
			break;
		case BLADE_FCC('z','h', 0, 0):
			lang = LANG_ZH_TW;
			break;
		default:
			lang = LANG_EN_US;
		}

#else
#error not implemented.
#endif

		//init private data
		PlatformData_Private::getSingleton().mCodePage = CPString;

		if( lang >= LANG_START && lang < LANG_COUNT )
			PlatformData_Private::getSingleton().mLanguage = LANGUAGE_SETTINGS[lang];
		else
		{
			assert(false);
			PlatformData_Private::getSingleton().mLanguage = LANGUAGE_SETTINGS[LANG_START];
		}

		PlatformData_Private::getSingleton().mSupportedLanguages.insert(PlatformData_Private::getSingleton().mSupportedLanguages.end(),
			LANGUAGE_SETTINGS, LANGUAGE_SETTINGS+LANG_COUNT);


		//note: following initialization may need PlatformData_Private.
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		// Android Specific
		// get lib path
		//libpath: getContext().getApplicationInfo().nativeLibraryDir
		jmethodID midGetInfo = env->GetMethodID(clsActivity, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
		jobject info = env->CallObjectMethod(activity->clazz, midGetInfo);
		{
			jclass infoClass = env->GetObjectClass(info);
			jfieldID fidLibDir = env->GetFieldID(infoClass, "nativeLibraryDir", "Ljava/lang/String;");
			{
				jstring jstrLibPath = (jstring)env->GetObjectField(info, fidLibDir);
				const char* charBuff = env->GetStringUTFChars(jstrLibPath, NULL);
				mLibPath = StringConverter::UTF8StringToTString(charBuff);
				env->ReleaseStringUTFChars(jstrLibPath, charBuff);
				env->DeleteLocalRef(jstrLibPath);
			}
			env->DeleteLocalRef(infoClass);
		}
		env->DeleteLocalRef(info);

		//get package name: com.xxx.xxxx
		jmethodID midGetPackageName = env->GetMethodID(clsActivity, "getPackageName", "()Ljava/lang/String;");
		jstring jstrPackageName= (jstring)env->CallObjectMethod(activity->clazz, midGetPackageName);
		{

			const char* charBuff = env->GetStringUTFChars(jstrPackageName, NULL);
			mPackageName = StringConverter::UTF8StringToTString(charBuff);
			env->ReleaseStringUTFChars(jstrPackageName, charBuff);
		}

		//get version code
		jmethodID midGetPM = env->GetMethodID(clsActivity, "getPackageManager", "()Landroid/content/pm/PackageManager;");
		jobject packageManager = env->CallObjectMethod(activity->clazz, midGetPM);
		{
			jclass pmClass = env->GetObjectClass(packageManager);
			jmethodID midGetPackageInfo = env->GetMethodID(pmClass, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
			{
				jobject packageInfo = env->CallObjectMethod(packageManager, midGetPackageInfo, jstrPackageName, 0);
				jclass packageInfoClass = env->GetObjectClass(packageInfo);
				jfieldID fidVersion = env->GetFieldID(packageInfoClass, "versionCode", "I");
				mVersionCode = env->GetIntField(packageInfo, fidVersion);
				env->DeleteLocalRef(packageInfoClass);
				env->DeleteLocalRef(packageInfo);
			}
			env->DeleteLocalRef(pmClass);
		}
		env->DeleteLocalRef(packageManager);
		env->DeleteLocalRef(jstrPackageName);

		//get class loader
		jmethodID mtdGetClassLoader = env->GetMethodID(clsActivity, "getClassLoader", "()Ljava/lang/ClassLoader;");

		mClassLoader = env->CallObjectMethod(activity->clazz, mtdGetClassLoader);
		mClassLoader = env->NewGlobalRef(mClassLoader);

		jclass clsLoader = env->FindClass("java/lang/ClassLoader");
		mLoadMethod = env->GetMethodID(clsLoader, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
		env->DeleteLocalRef(clsLoader);

		env->DeleteLocalRef(clsActivity);
		this->detachJNI();
#endif

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool PlatformData::shutdown()
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		if( mData.appData != NULL )
		{
			JNIEnv* env = this->attachJNI();

			env->DeleteGlobalRef(mClassLoader);
			mClassLoader = NULL;
			mLoadMethod = NULL;

			this->detachJNI();
		}
#endif
		mData.userData = NULL;
		mData.appData = NULL;

		return true;
	}
	
	//////////////////////////////////////////////////////////////////////////
	const PLATFORM_DATA&		PlatformData::getPlatformData() const
	{
		return mData;
	}

	//////////////////////////////////////////////////////////////////////////
	void PlatformData::setPlatformUserData(PLATFORM_USER_DATA userData)
	{
		assert( mData.userData == NULL );

		mData.userData = userData;
	}

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

	//////////////////////////////////////////////////////////////////////////
	void			PlatformData::internalMessagePump() const
	{
		MSG msg;
		//update all other window / attached window msg
		while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != FALSE )
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

	//////////////////////////////////////////////////////////////////////////
	android_app*	PlatformData::getAndroidApp() const
	{
		return android_app_instance;
	}

	//////////////////////////////////////////////////////////////////////////
	static 	int8_t	readMessage(struct android_app* android_app)
	{
		int8_t cmd;
		if ( ::read(android_app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd) )
			return cmd;
		else
			LOGE("No data on command pipe!");
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	void			PlatformData::internalMessagePump() const
	{
		int ident;
		int events;
		struct android_poll_source* source;

		while((ident=::ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
		{
			if(source != NULL)
			{
				android_app* app = source->app;
				if( source->id == LOOPER_ID_MAIN )
				{
					//process main cmd
					source->cmd = readMessage(app);

					source->preprocess(app, source);

					source->process(app, source);

					source->postprocess(app, source);
				}
				else if( source->id == LOOPER_ID_INPUT )
				{
					//process input event
					while(::AInputQueue_getEvent(app->inputQueue, &(source->event) ) >= 0)
					{
						LOGV("New input event: type=%d\n", AInputEvent_getType(source->event));
						if( ::AInputQueue_preDispatchEvent(app->inputQueue, source->event) )
							continue;

						source->preprocess(app, source);

						source->process(app, source);

						source->postprocess(app, source);

						::AInputQueue_finishEvent(app->inputQueue, source->event, 1);
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	JNIEnv*		PlatformData::attachJNI()
	{
		if( mData.appData == NULL )
			return NULL;

		ANativeActivity* activity = mData.appData;
		JavaVM* vm = activity->vm;
		JNIEnv* env = NULL;
		if( vm->AttachCurrentThread(&env, NULL) < 0 )
			return NULL;

		//important: clear pending exceptions
		env->ExceptionClear();
		return env;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PlatformData::detachJNI()
	{
		if( mData.appData == NULL )
			return false;

		ANativeActivity* activity = mData.appData;
		JavaVM* vm = activity->vm;
		vm->DetachCurrentThread();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	jclass		PlatformData::loadClassJNI(JNIEnv* env, const TString& className)
	{
		assert( mData.appData != NULL );

		if( mClassLoader == NULL || mLoadMethod == NULL )
			return NULL;

		TempBuffer buffer;
		jstring classNameJNI = env->NewStringUTF(TSTRING2UTF8(buffer, className));
		if( classNameJNI == NULL )
			return NULL;

		jclass ret = static_cast<jclass>( env->CallObjectMethod(mClassLoader, mLoadMethod, classNameJNI) );

		env->DeleteLocalRef(classNameJNI);
		return ret;
	}

#endif
	
}//namespace Blade