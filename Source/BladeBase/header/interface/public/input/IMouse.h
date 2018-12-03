/********************************************************************
	created:	2010/04/30
	filename: 	IMouse.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IMouse_h__
#define __Blade_IMouse_h__
#include <math/Vector2i.h>
#include <math/Vector3i.h>
#include <math/Vector2.h>
#include <math/Vector3.h>

namespace Blade
{

	typedef enum EMouseButton
	{
		MSB_LEFT = 0,
		MSB_RIGHT,
		MSB_MIDDLE,

		//add here if new buttons

		MSB_COUNT,
		MSB_BEGIN = MSB_LEFT,
	}MSBUTTON;

	typedef enum EMouseButtonState
	{
		MBS_UP,
		MBS_DOWN,
	}MBSTATE;

	class IMouse
	{
	public:
		class IListener
		{
		public:
			/** @brief return false to filter out the data(skip data recording) */
			virtual bool	onMouseButtonDown(MSBUTTON button) = 0;

			/** @brief  */
			virtual bool	onMouseButtonUp(MSBUTTON button) = 0;

			/** @brief  */
			virtual void	onMouseButtonClick(MSBUTTON button) = 0;

			/** @brief  */
			virtual void	onMouseButtonDBClick(MSBUTTON button) = 0;
		};

	public:
		virtual ~IMouse()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual MBSTATE			getButtonState(MSBUTTON button) const = 0;


		/** @brief  */
		inline	bool			isButtonDown(MSBUTTON button) const
		{
			return this->getButtonState(button) == MBS_DOWN;
		}

		/** @brief  */
		inline	bool			isButtonUp(MSBUTTON button) const
		{
			return this->getButtonState(button) == MBS_UP;
		}

		/**
		@describe get mouse position, in screen coordinates (0~screen_width,0~screen_height)
		@param 
		@return 
		*/
		virtual POINT2I			getGlobalMousePosition() const = 0;

		/**
		@describe get mouse position, in window coordinates (0~window_width,0~window_height).
		pos may out of window coordinates (negative or larger than window size)
		@param 
		@return 
		*/
		virtual POINT2I			getMousePosition() const = 0;

		/**
		@describe get normalized mouse position (0~1,0~1), in window coordinates
		pos may out of window coordinates (< 0 or > 1)
		@param 
		@return 
		*/
		virtual POINT2			getNormalizedMousePosition() const = 0;

		/**
		@describe set mouse position in screen coordinates
		@param 
		@return 
		*/
		virtual void			setGlobalMousePosition(POINT2I pos) = 0;

		/**
		@describe return false if already hide
		@param 
		@return 
		*/
		virtual bool			hide() = 0;

		/**
		@describe return false if already show
		@param 
		@return 
		*/
		virtual bool			show() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual POINT3I			getMouseMovement() const = 0;

		/**
		@describe 
		@param 
		@return 
		@note: z movement is not normalized
		*/
		virtual POINT3			getNormalizedMovement() const = 0;

	};//class IMouse
	
}//namespace Blade


#endif //__Blade_IMouse_h__