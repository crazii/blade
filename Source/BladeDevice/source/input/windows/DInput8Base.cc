/********************************************************************
	created:	2011/05/14
	filename: 	DInput8Base.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#include <input/windows/DInput8Base.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

namespace Blade
{
	LPDIRECTINPUT8	DInput8Base::msDInput8 = NULL;
	RefCount		DInput8Base::msInstanceCount(0);

	//////////////////////////////////////////////////////////////////////////
	DInput8Base::DInput8Base()
		:mDI8Device(NULL)
	{
		msInstanceCount.increment();
		DInput8Base::initDInput8();
	}

	//////////////////////////////////////////////////////////////////////////
	DInput8Base::~DInput8Base()
	{
		this->releaseDI8Device();

		if( msInstanceCount.decrement() == 0 )
		{
			DInput8Base::closeDInput8();
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				DInput8Base::createDI8Keyboard()
	{
		this->releaseDI8Device();

		HRESULT hr = msDInput8->CreateDevice(GUID_SysKeyboard, &mDI8Device, NULL); 

		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}
		else
			return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				DInput8Base::createDI8Mouse()
	{
		this->releaseDI8Device();

		HRESULT hr = msDInput8->CreateDevice(GUID_SysMouse, &mDI8Device, NULL); 

		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}
		else
			return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				DInput8Base::createDI8JoyStick()
	{
		this->releaseDI8Device();

		HRESULT hr = msDInput8->CreateDevice(GUID_Joystick, &mDI8Device, NULL); 

		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}
		else
			return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void				DInput8Base::releaseDI8Device()
	{
		if(msDInput8 != NULL && mDI8Device != NULL )
		{
			mDI8Device->Unacquire();
			mDI8Device->Release();
			mDI8Device = NULL;
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			DInput8Base::initDInput8()
	{
		if( msDInput8 != NULL )
			return true;

		HRESULT hr; 
		// Create the DirectInput object. 
		hr = DirectInput8Create(::GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
			IID_IDirectInput8, (void**)&msDInput8, NULL); 

		if(	FAILED(hr) )
			return false;
		else
			return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			DInput8Base::closeDInput8()
	{
		if( msDInput8 != NULL )
		{
			msDInput8->Release();
			msDInput8 = NULL;
		}
	}
	

}//namespace Blade



#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS