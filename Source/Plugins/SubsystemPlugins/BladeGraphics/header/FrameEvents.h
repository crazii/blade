/********************************************************************
	created:	2018/11/18
	filename: 	FrameEvents.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_FrameEvents_h__
#define __Blade_FrameEvents_h__
#include <BladeGraphics.h>
#include <Event.h>

namespace Blade
{
	class IRenderTarget;

	//////////////////////////////////////////////////////////////////////////
	///dispatched on render target start/end rendering
	struct FrameEvent : public Event
	{
	protected:
		FrameEvent() {}
		FrameEvent(IRenderTarget* target)
			:mFinalTarget(target)
		{
		}

		IRenderTarget*	mFinalTarget;
	};//struct RenderTargetState

	///make ease for dispatching
	struct FrameBeginEvent : public FrameEvent
	{
		static const BLADE_GRAPHICS_API TString	NAME;

		FrameBeginEvent(IRenderTarget* target)
			:FrameEvent(target)
		{
			mName = NAME;
		}
	};
	struct FrameEndEvent : public FrameEvent
	{
		static const BLADE_GRAPHICS_API TString	NAME;

		FrameEndEvent(IRenderTarget* target)
			:FrameEvent(target)
		{
			mName = NAME;
		}
	};

	///the LoopFlag class is a wrap for bool type,
	///and it is auto-reseted to false on each begin of the frame
	///note: this differs from LoopFlag since multiple frames maybe drawn during one loop
	class FrameFlag
	{
	public:
		BLADE_GRAPHICS_API FrameFlag();
		BLADE_GRAPHICS_API explicit FrameFlag(bool true_false);
		BLADE_GRAPHICS_API ~FrameFlag();

		/** @brief  */
		inline operator bool() const
		{
			return mFlag[0] == 1;
		}

		/** @brief  */
		inline bool operator!() const
		{
			return mFlag[0] == 0;
		}

		/** @brief  */
		inline bool operator[](index_t i) const
		{
			return mFlag[i] == 1;
		}

		/** @brief  */
		inline uint8& operator[](index_t i)
		{
			return mFlag[i];
		}

		/** @brief  */
		inline bool	operator==(bool rhs) const
		{
			return (mFlag[0] != 0) == rhs;
		}

		/** @brief  */
		FrameFlag&	operator=(bool rhs)
		{
			mFlag[0] = rhs ? 1u : 0u;
			return *this;
		}

	protected:
		/** @brief  */
		BLADE_GRAPHICS_API void			onFrameBegin(const Event& data);

		uint8 mFlag[4];
	};//FrameFlag
	
}//namespace Blade


#endif // __Blade_FrameEvents_h__