/********************************************************************
	created:	2013/12/28
	filename: 	AndroidWindowMessagePump.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <unistd.h>

#include <window/android/AndroidWindowMessagePump.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	AndroidMessagePump::AndroidMessagePump()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	AndroidMessagePump::~AndroidMessagePump()
	{

	}

	/************************************************************************/
	/* IWindowMessagePump interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	AndroidMessagePump::processMessage()
	{
		std::for_each(mMessageListeners.begin(),mMessageListeners.end(), std::mem_fun(&IMessageListener::preProcessMessages) );

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
					source->cmd = AndroidMessagePump::androidThreadMessage(app);

					source->preprocess(app, source);

					std::for_each(mMessageListeners.begin(),mMessageListeners.end(),
						std::bind2nd( std::mem_fun(&IMessageListener::preProcessMessage), source) );

					source->process(app, source);

					std::for_each(mMessageListeners.begin(),mMessageListeners.end(),
						std::bind2nd( std::mem_fun(&IMessageListener::postProcessMessage), source) );

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

						std::for_each(mMessageListeners.begin(),mMessageListeners.end(),
							std::bind2nd( std::mem_fun(&IMessageListener::preProcessMessage), source) );

						source->process(app, source);

						std::for_each(mMessageListeners.begin(),mMessageListeners.end(),
							std::bind2nd( std::mem_fun(&IMessageListener::postProcessMessage), source) );

						source->postprocess(app, source);
						::AInputQueue_finishEvent(app->inputQueue, source->event, 1);
					}
				}
			}
		}

		std::for_each(mMessageListeners.begin(),mMessageListeners.end(), std::mem_fun(&IMessageListener::postProcessMessages) );
	}

	//////////////////////////////////////////////////////////////////////////
	bool	AndroidMessagePump::addMessageListener(IMessageListener* listener)
	{
		if( listener != NULL )
			return mMessageListeners.insert(listener).second;
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	AndroidMessagePump::removeMessageListener(IMessageListener* listener)
	{
		if( listener != NULL )
			return mMessageListeners.erase(listener) == 1;
		else
			return false;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	int8_t AndroidMessagePump::androidThreadMessage(struct android_app* android_app)
	{
		int8_t cmd;
		if ( ::read(android_app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd) )
			return cmd;
		else
			LOGE("No data on command pipe!");
		return -1;
	}
	
}//namespace Blade

#endif//BLADE_PLATFORM