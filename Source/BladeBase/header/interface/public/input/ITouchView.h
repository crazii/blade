/********************************************************************
	created:	2013/12/28
	filename: 	ITouchView.h
	author:		Crazii
	purpose:	touch screen desc:
*********************************************************************/
#ifndef __Blade_ITouchView_h__
#define __Blade_ITouchView_h__
#include <math/Vector2i.h>
#include <math/Vector2.h>

namespace Blade
{

	class ITouchView
	{
	public:

		typedef enum ETouchState
		{
			TS_DOWN,
			TS_UP,
		}TOUCH_STATE;

		/** @brief describe a touch view dimension */
		typedef struct STouchViewDesc
		{
			size_t	mLeft;
			size_t	mTop;
			size_t	mWidth;
			size_t	mHeight;
			ITouchView* mParent;
			size_t	mChildCount;
		}TOUCH_DESC;

		/** @brief describe touch move */
		typedef struct STouchMoveData
		{
			POINT2		mPoint;		///pos in pixels
			POINT2		mNPoint;	///normalized pos [0-1.0]
		}TOUCH_DATA;

		class IListener
		{
		public:
			/** @brief  */
			virtual bool	onTouchDown(const TOUCH_DATA& touchData) = 0;
			/** @brief  */
			virtual bool	onTouchUp(const TOUCH_DATA& touchData) = 0;
			/** @brief  */
			virtual void	onTouchMove(const TOUCH_DATA& touchData) = 0;
			/** @brief  */
			virtual void	onTouchCancel(const TOUCH_DATA& touchData) = 0;
		};

	public:
		virtual ~ITouchView()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TOUCH_DESC&	getDesc() const = 0;

		/**
		@describe get current touch state
		@param 
		@return 
		*/
		virtual TOUCH_STATE			getState() const = 0;

		/**
		@describe get current touch position, success only current touch state is down
		@param 
		@return 
		*/
		virtual const TOUCH_DATA&	getPosition() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ITouchView*		createChildView(const TOUCH_DESC& desc) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteChildView(ITouchView* view) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ITouchView*		getChildView(index_t index) const = 0;
	};

	//TODO: touch input should be pure
	//gesture system better be implemented on logic(game) framework
	/*
	//pre-defined gesture, will available on buffered mode
	typedef enum ETouchGesture
	{
	TC_TAP,		//tap on one point
	TC_DTAP,	//double tap
	TG_LINE,	//single line with direction
	TG_CIRCLE,	//circle
	TC_CROSS,	//cross(too non-intersected line with acceptable timeout)
	TC_HOOK,	//hook representing right/OK or other meanings - two connected lines with certain angles
	TC_LINELIST,//line list	
	}GESTURE;

	//limit for TC_LINELIST
	static const int MAX_LINES = 4;

	//data to describe gesture
	typedef struct SGestureData
	{
	GESTURE mGesture;
	union
	{
	TOUCH_DATA	mTap;
	TOUCH_DATA	mDtap;
	TOUCH_DATA	mLine[2];
	struct
	{
	TOUCH_DATA	mCenter;
	TOUCH_DATA	mRadius
	};
	TOUCH_DATA	mCross[4];

	//for botch hook & line list
	struct
	{
	int			mLineCount;	// always > 1
	TOUCH_DATA	mLineList[MAX_LINES+1];
	};
	};
	}GESTURE_DATA;
	*/
	
}//namespace Blade

#endif //  __Blade_ITouchView_h__