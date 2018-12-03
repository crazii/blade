/********************************************************************
	created:	2013/01/20
	filename: 	UIEvents.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_UIEvents_h__
#define __Blade_UIEvents_h__
#include <interface/IConfig.h>
#include <interface/IMediaLibrary.h>
#include <math/Vector3.h>
#include <Event.h>

namespace Blade
{
	class IEntity;
	class Bindable;
	
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	///dispatch this state to set the target of UI 'media view' panel
	class MediaViewEvent : public Event, public NonAssignable
	{
	public:
		static BLADE_FRAMEWORK_API const TString NAME;

		const TString&	mMediaFileRef;
		index_t			mMediaType;

		inline MediaViewEvent(const TString& mediaFile, index_t mediaType) 
			:mMediaFileRef(mediaFile)
			,mMediaType(mediaType)
		{
			mName = NAME;
		}

		inline MediaViewEvent(const TString& mediaFile, const TString& uiHintString)
			:mMediaFileRef(mediaFile)
			,mMediaType( IMediaLibrary::getSingleton().getMediaType(uiHintString).mIndex ) 
		{
			mName = NAME;
		}

		inline MediaViewEvent(const MediaViewEvent& src)
			:mMediaFileRef(src.mMediaFileRef)
			,mMediaType(src.mMediaType)
		{
			mName = NAME;
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	///@deprecated dispatch this state if an entity is selected. used to update the property grid 
	class EntitySlectedEvent : public Event
	{
	public:
		static BLADE_FRAMEWORK_API const TString NAME;
		const IEntity* mEntity;

		EntitySlectedEvent(const IEntity* entity) :mEntity(entity)
		{
			mName = NAME;
		}
	};

	///dispatch this state if configurable(scene,etc) is selected. used to update the property grid 
	class ConfigurableSlectedEvent : public Event
	{
	public:
		static BLADE_FRAMEWORK_API const TString NAME;
		Bindable*			mTarget;
		IConfig*			mTargetConfig;

		ConfigurableSlectedEvent(Bindable* configurable,IConfig* config)
			:mTarget(configurable)
			,mTargetConfig(config)
		{
			mName = NAME;
		}
	};

	///state when option item in property grid is high lighted, i.e. selected by user
	class OptionHighLightEvent : public Event, public NonAssignable
	{
	public:
		static BLADE_FRAMEWORK_API const TString NAME;
		const Bindable*	mTarget;
		const TString&	mOptionValue;
		const TString&	mUIHint;
		OptionHighLightEvent(const Bindable* _target, const TString& val, const TString& uiHintString)
			:mTarget(_target),mOptionValue(val),mUIHint(uiHintString)
		{
			mName = NAME;
		}

		OptionHighLightEvent(const OptionHighLightEvent& src)
			:mTarget(src.mTarget)
			,mOptionValue(src.mOptionValue)
			,mUIHint(src.mUIHint)
		{
		}
	};

	///(built-in) view port camera config change
	class ViewportCameraConfigEvent : public Event
	{
	public:
		static BLADE_FRAMEWORK_API const TString	NAME;
		int mViewportIndex;
		scalar	mCameraSpeed;	//move speed
		scalar	mCameraRotateSpeed;	//rotate speed

		ViewportCameraConfigEvent(int index, scalar speed, scalar rotateSpeed)
			:mViewportIndex(index), mCameraSpeed(speed), mCameraRotateSpeed(rotateSpeed)
		{
			mName = NAME;
		}
	};

}//namespace Blade

#endif //  __Blade_UIEvents_h__