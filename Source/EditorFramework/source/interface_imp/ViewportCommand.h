/********************************************************************
	created:	2013/04/28
	filename: 	ViewportCommand.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ViewportCommand_h__
#define __Blade_ViewportCommand_h__
#include <interface/public/ui/IMenu.h>
#include <interface/public/ui/IViewport.h>
#include <interface/public/ui/uiwindows/IViewportWindow.h>
#include <BladeEditor_blang.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class ViewModeCommand : public IUICommand, public StaticAllocatable
	{
	public:
		typedef TStringMap<FILL_MODE>	NameViewModeMap;
		typedef Map<FILL_MODE,TString>	ViewModeNameMap;
		NameViewModeMap		mNamedViewModes;
		ViewModeNameMap		mViewModeNames;

		ViewModeCommand()
		{
			mNamedViewModes[BTString(BLANG_VIEW_VIEWSOLID)] = GFM_DEFAULT;
			mNamedViewModes[BTString(BLANG_VIEW_VIEWWIRE)] = GFM_WIREFRAME;
			mNamedViewModes[BTString(BLANG_VIEW_VIEWPOINT)] = GFM_POINT;
			mViewModeNames[GFM_DEFAULT] = BTString(BLANG_VIEW_VIEWSOLID);
			mViewModeNames[GFM_SOLID] = BTString(BLANG_VIEW_VIEWSOLID);
			mViewModeNames[GFM_WIREFRAME] = BTString(BLANG_VIEW_VIEWWIRE);
			mViewModeNames[GFM_POINT] = BTString(BLANG_VIEW_VIEWPOINT);
		}

		virtual void execute(ICommandUI* cui)
		{
			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();

			if( file != NULL )
			{
				IGraphicsView* view = file->getEditorWindow()->getView();
				if( view != NULL )
				{
					NameViewModeMap::const_iterator i = mNamedViewModes.find(cui->getName());
					if( i == mNamedViewModes.end() )
					{
						assert(false);
						return;
					}
					view->setFillMode( i->second );
				}
			}
		}

		virtual bool update(ICommandUI* cui, index_t instance)
		{
			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();

			if( file != NULL )
			{
				IUIWindow* window = file->getEditorWindow();
				IGraphicsView* view;
				if( instance == INVALID_INDEX || window->getWindowType() != IViewportWindow::VIEWPORT_WINDOW_TYPE )
					view = window->getView();
				else
					view = static_cast<IViewportWindow*>(window)->getViewportSet()->getViewport(instance)->getView();

				if (view != NULL)
				{
					ViewModeNameMap::const_iterator i = mViewModeNames.find(view->getFillMode());
					if (i == mViewModeNames.end())
					{
						assert(false);
						return false;
					}
					cui->setChecked(cui->getName() == i->second);
				}
				return true;
			}
			return false;
		}
	};


	//////////////////////////////////////////////////////////////////////////
	class MaximizeViewCommand : public IUICommand, public StaticAllocatable
	{
	public:
		virtual void execute(ICommandUI* /*cui*/)
		{
			IUIMainWindow* ui = EditorFramework::getSingleton().getEditorUI();
			if(ui == NULL )
				return;
			IUIWindow* window = NULL;
			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();
			if( file != NULL )
			{
				window = file->getEditorWindow();
				assert( ui->getLayoutManager()->getActiveWindow() == window );
			}
			else
				window = ui->getLayoutManager()->getActiveWindow();

			if( window != NULL && window->getWindowType() == IViewportWindow::VIEWPORT_WINDOW_TYPE )
			{
				static_cast<IViewportWindow*>(window)->maximizeViewport();
			}
		}

		virtual bool update(ICommandUI* cui, index_t instance)
		{
			IUIMainWindow* ui = EditorFramework::getSingleton().getEditorUI();
			if( ui == NULL || ui->getLayoutManager() == NULL)
				return false;
			IUIWindow* window = NULL;

			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();
			if( file != NULL )
			{
				window = file->getEditorWindow();
				//assert( ui->getLayoutManager()->getActiveWindow() == window );	//ui implementation may delay
			}
			else
				window = ui->getLayoutManager()->getActiveWindow();

			if( window != NULL && window->getWindowType() == IViewportWindow::VIEWPORT_WINDOW_TYPE)
			{
				//just use the active view port param, because when maximized, others are not visible
				cui->setEnabled(true);
				cui->setChecked(static_cast<IViewportWindow*>(window)->isViewportMaximized(instance) );
				return true;
			}
			return false;
		}
	};

	class RealtimeViewCommand : public IUICommand, public StaticAllocatable
	{
	public:
		virtual void execute(ICommandUI* /*cui*/)
		{
			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();

			if( file != NULL )
			{
				IUIWindow* window = file->getEditorWindow();
				if( window != NULL && window->getWindowType() == IViewportWindow::VIEWPORT_WINDOW_TYPE )
				{
					IViewportWindow* win = static_cast<IViewportWindow*>(window);
					bool realtime = win->getViewportSet()->getActiveViewport()->isRealtimeUpdate();
					win->getViewportSet()->getActiveViewport()->setRealtime( !realtime );
				}
			}
		}

		virtual bool update(ICommandUI* cui, index_t instance)
		{
			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();
			if( file != NULL )
			{
				IUIWindow* window = file->getEditorWindow();
				if( window != NULL && window->getWindowType() == IViewportWindow::VIEWPORT_WINDOW_TYPE )
				{
					IViewport* viewport;
					IViewportWindow* win = static_cast<IViewportWindow*>(window);
					if( instance == INVALID_INDEX )
						viewport = win->getViewportSet()->getActiveViewport();
					else
						viewport = win->getViewportSet()->getViewport(instance);
					if( viewport != NULL )
					{
						bool realtime = viewport->isRealtimeUpdate();
						cui->setEnabled(true);
						cui->setChecked(realtime);
						return true;
					}
				}
			}
			return false;
		}
	};


	class ViewportChannelCommand : public IUICommand, public StaticAllocatable
	{
	public:
		typedef TStringMap<FILL_MODE>	NameViewModeMap;
		typedef Map<FILL_MODE,TString>	ViewModeNameMap;
		NameViewModeMap		mNamedViewModes;
		ViewModeNameMap		mViewModeNames;

		ViewportChannelCommand()
		{
			mNamedViewModes[BTString(BLANG_VIEW_VIEWSOLID)] = GFM_DEFAULT;
			mNamedViewModes[BTString(BLANG_VIEW_VIEWWIRE)] = GFM_WIREFRAME;
			mNamedViewModes[BTString(BLANG_VIEW_VIEWPOINT)] = GFM_POINT;
			mViewModeNames[GFM_DEFAULT] = BTString(BLANG_VIEW_VIEWSOLID);
			mViewModeNames[GFM_SOLID] = BTString(BLANG_VIEW_VIEWSOLID);
			mViewModeNames[GFM_WIREFRAME] = BTString(BLANG_VIEW_VIEWWIRE);
			mViewModeNames[GFM_POINT] = BTString(BLANG_VIEW_VIEWPOINT);
		}

		virtual void execute(ICommandUI* cui)
		{
			ICommandUI* parent = cui->getParentCUI();
			if(parent == NULL)
				return;

			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();
			if( file != NULL )
			{
				IUIWindow* window = file->getEditorWindow();
				if( window != NULL && window->getWindowType() == IViewportWindow::VIEWPORT_WINDOW_TYPE )
				{
					IViewportWindow* win = static_cast<IViewportWindow*>(window);
					IGraphicsView* view = win->getViewportSet()->getActiveViewport()->getView();
					const TString& name = cui->getName();
					
					IGraphicsChannel* channel = view->getChannel(parent->getName());
					if(channel == NULL)
					{
						assert(false);
						return;
					}

					if(name == BTString(BLANG_VIEW_VIEWHIDE) )
					{
						channel->setEnable(false);
						return;
					}
					else
						channel->setEnable(true);

					NameViewModeMap::const_iterator i = mNamedViewModes.find(name);
					if( i == mNamedViewModes.end() )
					{
						assert(false);
						return;
					}
					channel->setFillMode( i->second );
				}
			}
		}

		virtual bool update(ICommandUI* cui, index_t instance)
		{
			ICommandUI* parent = cui->getParentCUI();
			if(parent == NULL)
				return false;

			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();
			if( file != NULL )
			{
				IUIWindow* window = file->getEditorWindow();
				IGraphicsView* view;
				if( instance == INVALID_INDEX || window->getWindowType() != IViewportWindow::VIEWPORT_WINDOW_TYPE )
					view = window->getView();
				else
				{
					IViewportWindow* win = static_cast<IViewportWindow*>(window);
					view = win->getViewportSet()->getViewport(instance)->getView();
				}

				if( view != NULL )
				{
					IGraphicsChannel* channel = view->getChannel( parent->getName() );
					if(channel == NULL)	
						return false;

					ViewModeNameMap::const_iterator i = mViewModeNames.find( channel->getFillMode() );
					if( i == mViewModeNames.end() )
					{
						assert(false);
						return false;
					}

					if (!channel->isEnabled())
					{
						cui->setEnabled(false);
						return true;
					}
					cui->setChecked( i->second == cui->getName() );
				}
				return true;
			}
			return false;
		}
	};



	class ViewportPhaseCommand : public IUICommand, public StaticAllocatable
	{
	public:
		ViewportPhaseCommand() :mTarget(NULL) {}

		virtual void execute(ICommandUI* /*cui*/)
		{

		}

		virtual bool update(ICommandUI* cui, index_t instance)
		{
			IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();
			if( file != NULL )
			{
				IUIWindow* window = file->getEditorWindow();
				if( window != NULL && window->getWindowType() == IViewportWindow::VIEWPORT_WINDOW_TYPE )
				{
					IViewportWindow* win = static_cast<IViewportWindow*>(window);
					IViewport* viewport;
					if( instance == INVALID_INDEX )
						viewport = win->getViewportSet()->getActiveViewport();
					else
						viewport = win->getViewportSet()->getViewport(instance);

					if (viewport != NULL)
					{
						IGraphicsView* view = viewport->getView();
						assert(view != NULL);
						size_t phaseCount = view->getPhaseCount();

						//TODO:
						if (mTarget == NULL)
							mTarget = static_cast<IMenu*>(cui);
						else
							assert(mTarget == cui);
						
						if (phaseCount != mTarget->getSubMenuCount())
						{
							mTarget->removeAllMenus();
							
							for (size_t i = 0; i < phaseCount; ++i)
							{
								IGraphicsPhase* phase = view->getPhase(i);
								mTarget->addCheckItem(phase->getName(), &mExec, false);
							}
						}
					}
				}
			}
			return true;
		}
	protected:

		class ExecuteCommand : public IUICommand
		{
		public:
			virtual void execute(ICommandUI* cui)
			{
				IEditorFile* file = EditorFramework::getSingleton().getCurrentFile();
				if (file != NULL)
				{
					IUIWindow* window = file->getEditorWindow();
					if (window != NULL && window->getWindowType() == IViewportWindow::VIEWPORT_WINDOW_TYPE)
					{
						IViewportWindow* win = static_cast<IViewportWindow*>(window);
						IGraphicsView* view = win->getViewportSet()->getActiveViewport()->getView();
						IGraphicsPhase* phase = view->getPhase(cui->getName());
						if (phase != NULL)
						{
							phase->setEnable(!phase->isEnabled());
							//TODO: set phase as final output 
						}
					}
				}
			}
		};

		//HACK:
		IMenu* mTarget;	//dedicated target
		ExecuteCommand	mExec;
	};
	
}//namespace Blade

#endif //  __Blade_ViewportCommand_h__