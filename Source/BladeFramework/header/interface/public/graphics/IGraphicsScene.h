/********************************************************************
	created:	2010/05/05
	filename: 	IGraphicsScene.h
	author:		Crazii
	purpose:	public interface of graphics scene. all methods should be 
				called at TS_MAIN_SYNC state.
*********************************************************************/
#ifndef __Blade_IGraphicsScene_h__
#define __Blade_IGraphicsScene_h__
#include <math/BladeMath.h>
#include <interface/public/IScene.h>
#include <interface/public/ITask.h>
#include <interface/public/IElement.h>

namespace Blade
{
	class Vector3;

	class IWindow;
	class IRenderTarget;
	class IRenderWindow;
	class IGraphicsView;
	class IGraphicsCamera;
	class IGraphicsInterface;
	class IGraphicsSpaceCoordinator;
	class IGraphicsEffectManager;
	class IImage;
	class SpaceQuery;

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4263) //member function does not override base class virtual member function
#pragma warning(disable:4264) //no override available for virtual virtual member function from base
#endif

	class BLADE_FRAMEWORK_API IGraphicsScene : public IScene
	{
	public:
		virtual ~IGraphicsScene()	{}

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual IGraphicsSpaceCoordinator*	getSpaceCoordinator() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsEffectManager*	getEffectManager() const = 0;

		/**
		@describe the space should be setup before calling this
		@param coordinator: space coordinator type to use. EMTPY will use default setting. if scene is loaded from file, coordinator is already loaded from file and this parameter is ignored
		@param defSpace: default space type of coordinator. EMTPY will use default setting. if scene is loaded from file, space type is already loaded from file and this parameter is ignored
		@return
		*/
		virtual	bool			initialize(IGraphicsView* mainView, const TString& coordinator = TString::EMPTY, const TString& defSpace = TString::EMPTY) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsView*	getMainView() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsCamera*	getMainCamera() const = 0;

		/**
		@describe get the default render window of the scene
		@param 
		@return 
		*/
		virtual IRenderWindow*	getDefaultRenderWindow() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IRenderWindow*	getActiveRenderWindow() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		setActiveRenderWindow(IRenderWindow* window = NULL, IGraphicsCamera* camera = NULL, IGraphicsView* view = NULL) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual HELEMENT	createGraphicsElement(const TString& elemType, const TString& spaceName = TString::EMPTY ) = 0;

		/**
		@describe
		@param [in] separated create the space as separated, so cameras in other spaces cannot see object it it
		and cameras in this space cannot see objects in other spaces
		@return
		*/
		virtual bool		createSpace(const TString& InstanceName,const TString& spaceType,
			const Vector3& maxSize, bool separated = true) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool		removeSpace(const TString& InstanceName) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual HTASK		createExtraTask(const TString& name, IRenderTarget* target, IGraphicsView* mainView,IGraphicsCamera* mainCamera,
			ITask::ETaskPriority priorityGroup = ITask::TP_HIGH,index_t priorityID = -1) = 0;

		/**
		@describe whether this scene is drawing on each frame automatically
		@param 
		@return 
		*/
		virtual void		setRealtimeDrawing(bool realtime) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		isRealtimeDrawing() const = 0;

		/**
		@describe redraw the scene at once and only once, mainly used for windowing style paint/refresh based on window event
		@param the camera & view override, specify NULL to use scene default
		@return 
		*/
		virtual bool		drawScene(IWindow* window = NULL, IGraphicsCamera* camera = NULL, IGraphicsView* view = NULL) = 0;

		/**
		@describe draw the scene synchronously for once
		@param 
		@return 
		*/
		virtual bool		drawScene(IRenderTarget* target, IGraphicsCamera* camera = NULL, IGraphicsView* view = NULL) = 0;

		/**
		@describe draw scene synchronously for once, to image
		@param
		@return
		*/
		virtual bool		drawScene(IImage* target, IGraphicsCamera* camera = NULL, IGraphicsView* view = NULL) = 0;

		/************************************************************************/
		/* space related methods
		like scene query,etc.
		this could be replace by physics system, if the framework has installed a physics system
		*/
		/************************************************************************/
		/**
		@describe query the all elements, using implementation defined query method, i.e. ray casting, volume testing etc.
		@param [in] spaceName: the space inside which to perform a query.\n
		default:EMPTY means all spaces within the scene's coordinator
		"Default" means only the coordinator itself, no other sub spaces
		@return
		*/
		virtual bool		queryElements(SpaceQuery& query, const TString& spaceName = TString::EMPTY) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IElement*	queryNearestElement(SpaceQuery& query, const TString& spaceName = TString::EMPTY) const = 0;

	};//class IGraphicsScene


#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
	
}//namespace Blade


#endif //__Blade_IGraphicsScene_h__