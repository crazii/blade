/********************************************************************
	created:	2013/04/22
	filename: 	IViewport.h
	author:		Crazii
	purpose:	built-in data abstraction of UI view port, the UI module will 
				create view port UI base on this info
*********************************************************************/
#ifndef __Blade_IViewport_h__
#define __Blade_IViewport_h__
#include <utility/Bindable.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <interface/public/input/IMouse.h>
#include <interface/public/logic/CameraActor.h>

namespace Blade
{
	class IWindow;
	class IUIWindow;
	class ILogicWorld;

	static const int MAX_VIEWPORT_COUNT = 4;

	enum EViewportType
	{
		VT_TOP = 0,
		VT_FONT,
		VT_LEFT,
		VT_PERSPECTIVE,

		//
		VT_COUNT,
	};

	typedef struct SViewportDesc : public Bindable
	{
		scalar				mCameraSpeed;
		scalar				mCameraRotateSpeed;
		//size in percent of parent dimension
		scalar				mLeft;
		scalar				mRight;
		scalar				mTop;
		scalar				mBottom;

		EViewportType		mType;
		FILL_MODE			mFillMode;
		///real time update
		///one view port will auto-update at real time when it is active AND this real-time switch is on.
		mutable bool		mRealtime;
		mutable bool		mMaximized;
		///active (focused)
		mutable bool		mActive;
	}VIEWPORT_DESC;

	typedef struct SViewportLayout
	{
		///view port count
		size_t			count;
		VIEWPORT_DESC	layouts[MAX_VIEWPORT_COUNT];
	}VIEWPORT_LAYOUT;


	class IViewport
	{
	public:
		virtual ~IViewport() {}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual index_t				getIndex() const = 0;

		/**
		@describe get the controllable camera instance of the view port
		@param 
		@return 
		*/
		virtual CameraActor*		getCameraActor() const = 0;

		inline IGraphicsCamera*		getCamera() const
		{
			return this->getCameraActor()->getCamera();
		}

		/**
		@describe the the view of the view port
		@param 
		@return 
		*/
		virtual IGraphicsView*		getView() const = 0;

		/**
		@describe set target window. TODO: use render target which is more general.
		@param 
		@return 
		*/
		virtual void				setWindow(IWindow* window) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IWindow*			getWindow() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const VIEWPORT_DESC&	getDesc() const = 0;

		/** @brief  */
		inline EViewportType	getViewportType() const
		{
			return this->getDesc().mType;
		}
		/** @brief  */
		inline scalar			getCameraSpeed() const
		{
			return this->getDesc().mCameraSpeed;
		}
		/** @brief  */
		inline bool				isRealtimeUpdate() const
		{
			return this->getDesc().mRealtime;
		}
		/** @brief  */
		inline bool				isMaximized() const
		{
			return this->getDesc().mMaximized;
		}

		/** @brief  */
		inline bool				isActive() const
		{
			return this->getDesc().mActive;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		setRealtime(bool realtime) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void		setMaximized(bool maximized) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		drawViewport() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		activate() = 0;
	};

	//per window view-ports instance
	class IViewportSet
	{
	public:
		virtual ~IViewportSet() {}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		initialize(ILogicWorld* world, const TString& cameraNamePrefix) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getViewportCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IViewport*	getViewport(index_t index) const = 0;

		/**
		@describe get the only one active view port
		the active view port is update (drawing) in real time, if isRealtimeUpdate() is true.
		@param 
		@return 
		*/
		virtual IViewport*	getActiveViewport() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual index_t		getActiveViewportIndex() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		setActiveViewport(index_t index) = 0;

		/**
		@describe reset/update when layout changes
		@param 
		@return 
		*/
		virtual bool		resetLayout(const VIEWPORT_LAYOUT* layout = NULL) = 0;

		/**
		@describe manually draw the view port
		@param 
		@return false if no view port found with the index, or the view port is auto-updated (active & real-time)
		*/
		virtual bool		drawViewport(index_t index) = 0;
		/** @brief  */
		inline void			drawAllViewport()
		{
			size_t count = this->getViewportCount();
			for(size_t i = 0; i < count; ++i)
			{
				bool ret = this->drawViewport(i);
				assert(ret);
				BLADE_UNREFERENCED(ret);
			}
		}
	};
	
}//namespace Blade

#endif //  __Blade_IViewport_h__