/********************************************************************
	created:	2015/03/25
	filename: 	Jni.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include "JniHelper.h"
#include <threading/Mutex.h>
#include <interface/public/window/IWindowMessagePump.h>

#include <unistd.h>
#include <android/native_activity.h>

#define BLADE_USE_PLATFORM_DATA_EXPLICIT
#include <platform/PlatformData.h>


namespace Blade
{
	static Mutex sThreadLock;
	static StaticLock sRaceLock;

	static volatile int sDialogRetCode = -1;
	static volatile bool sDialogInProgress = false;

	namespace Impl
	{
		extern "C" JNIEXPORT void JNICALL Java_com_blade_foundation_BladeBase_nativeOnSystemDialogResult(JNIEnv * env, jobject  object, int retCode)
		{
			ScopedLock lock(sRaceLock);
			sDialogRetCode = retCode;
			sDialogInProgress = false;
		}

		static volatile bool isDialogInProgress()
		{
			ScopedLock lock(sRaceLock);
			return sDialogInProgress;
		}

	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	static inline ANativeActivity* getActivity()
	{
		return PlatformData::getSingleton().getPlatformAppData();
	}

	namespace JniHelper
	{

		//////////////////////////////////////////////////////////////////////////
		IPlatformManager::MBRESULT
			showSystemDialog(const JavaU16String& u16Caption, const JavaU16String& u16Message, 
			const JavaU16String& u16Yes, const JavaU16String& u16No, const JavaU16String& u16Cancel, IPlatformManager::MBTYPE type)
		{
			MutexAutoLock lock(sThreadLock);
			{
				ScopedLock lock(sRaceLock);
				sDialogInProgress = true;
			}

			//hell, need JAVA to show a window
			JNIEnv* env = PlatformData::getSingleton().attachJNI();

			static jclass FOUNDATION_CLASS = NULL;
			static jmethodID DIALOG_METHOD = NULL;
			if( FOUNDATION_CLASS == NULL || DIALOG_METHOD == NULL )
			{
				jclass _class = PlatformData::getSingleton().loadClassJNI(env, BTString("com/blade/foundation/BladeBase") );
				FOUNDATION_CLASS = static_cast<jclass>( env->NewGlobalRef(_class) );
				env->DeleteLocalRef(_class);

				DIALOG_METHOD = env->GetStaticMethodID(FOUNDATION_CLASS, 
					"showDialog", 
					"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");

				assert( FOUNDATION_CLASS != NULL && DIALOG_METHOD != NULL);
			}

			jstring jCaption = env->NewString( &u16Caption[0], (int)u16Caption.size() );
			jstring jMessage = env->NewString( &u16Message[0], (int)u16Message.size() );
			jstring jYes = env->NewString( &u16Yes[0], (int)u16Yes.size() );
			jstring jNo = env->NewString( &u16No[0], (int)u16No.size() );
			jstring jCancel = env->NewString( &u16Cancel[0], (int)u16Cancel.size() );

			int count = 1;
			switch (type)
			{
			case IPlatformManager::MBT_OK:
				count = 1;
				break;
			case IPlatformManager::MBT_YESNO:
				count = 2;
				break;
			case IPlatformManager::MBT_YESNOCANCEL:
				count = 3;
				break;
			default:
				break;
			}

			env->CallStaticVoidMethod(FOUNDATION_CLASS, DIALOG_METHOD, jCaption, jMessage, jYes, jNo, jCancel, count);

			while(Impl::isDialogInProgress())
			{
				//now system dialog can be used before (window) plugin initialization
				//IWindowMessagePump::getSingleton().processMessage();
				PlatformData::getSingleton().internalMessagePump();
				::usleep(1000*100);
			}

			env->DeleteLocalRef(jCaption);
			env->DeleteLocalRef(jMessage);
			env->DeleteLocalRef(jYes);
			env->DeleteLocalRef(jNo);
			env->DeleteLocalRef(jCancel);

			int retCode = sDialogRetCode;
			sDialogRetCode = -1;

			PlatformData::getSingleton().detachJNI();

			assert( retCode == 1 || retCode == 0 || retCode == 2 );
			if( retCode == 1 )//yes
				return count == 1 ? IPlatformManager::MBR_OK : IPlatformManager::MBR_YES;
			else if( retCode == 0 )//no
				return IPlatformManager::MBR_NO;
			else
				return IPlatformManager::MBR_CANCEL;
		}
		
	}//namespace JniHelper

}//namespace Blade

#endif//BLADE_PLATFORM 