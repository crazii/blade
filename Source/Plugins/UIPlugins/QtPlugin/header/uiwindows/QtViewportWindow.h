/********************************************************************
	created:	2016/7/28
	filename: 	QtViewportWindow.h
	author:		Crazii
	purpose:	
*********************************************************************/
#include <interface/public/ui/uiwindows/IViewportWindow.h>
#include <QtWindow.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class QtViewPortContent : public QGraphicsView, public Allocatable
	{
		typedef QGraphicsView Base;
	public:
		QtViewPortContent() :mViewport(NULL) 
		{
			this->setStyleSheet("background-color: transparent;");
			this->setAttribute(Qt::WA_TranslucentBackground, true);
			this->setAttribute(Qt::WA_DontCreateNativeAncestors, true);
			this->setAttribute(Qt::WA_NativeWindow, true);
			this->setAttribute(Qt::WA_DeleteOnClose, true);
			this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
		}
		/** @brief  */
		virtual QPaintEngine* paintEngine() const { return NULL; }
		/** @brief  */
		virtual void paintEvent(QPaintEvent *evt) { this->paint();}
		/** @brief  */
		inline void setViewport(IViewport* viewport) {mViewport = viewport;}
		/** @brief  */
		inline void activate() { this->Base::setFocus();mViewport->activate(); }
		/** @brief  */
		virtual void mousePressEvent(QMouseEvent *evt)
		{
			this->activate();
			Base::mousePressEvent(evt);
		}
		/** @brief  */
		virtual void mouseDoubleClickEvent(QMouseEvent *evt)
		{
			this->activate();
			Base::mouseDoubleClickEvent(evt);
		}
		/** @brief  */
		virtual void mouseMoveEvent(QMouseEvent *evt)
		{
			Base::mouseMoveEvent(evt);
			this->paint();
		}
		/** @brief  */
		virtual void keyPressEvent(QKeyEvent *evt)
		{
			Base::keyPressEvent(evt);
			this->paint();
		}
		/** @brief  */
		virtual void resizeEvent(QResizeEvent *evt)
		{
			Base::resizeEvent(evt);
			this->setSceneRect(0, 0, evt->size().width(), evt->size().height());
			this->paint();
		}
		/** @brief  */
		virtual void showEvent(QShowEvent *evt)
		{
			Base::showEvent(evt);
			this->paint();
		}
		/** @brief  */
		void paint() {if(mViewport != NULL) mViewport->drawViewport();}
	protected:
		IViewport* mViewport;
	};

	class QtFrame : public QFrame
	{
	public:
		QtFrame()
		{
			//this->setAttribute(Qt::WA_TranslucentBackground, true);
			this->setStyleSheet("background-color: transparent;");
		}
		/** @brief  */
		virtual QPaintEngine* paintEngine() const { return NULL; }
		/** @brief  */
		virtual void paintEvent(QPaintEvent *evt) { }
	};

	//////////////////////////////////////////////////////////////////////////
	class QtViewPort : public QtWindow
	{
	public:
		QtViewPort() :mIndex(0)
		{
			mFilterToolboxButton = true;
			this->setAttribute(Qt::WA_TranslucentBackground, true);
			//this->setAttribute(Qt::WA_PaintOnScreen, true);
			this->setAttribute(Qt::WA_NoSystemBackground, true);
		}
		/** @brief  */
		virtual QPaintEngine* paintEngine() const { return NULL; }
		/** @brief  */
		virtual index_t getIndex() const {return mIndex;}
		/** @brief  */
		void	setIndex(index_t index)	{mIndex = index;}
		/** @brief  */
		void activate();

	protected:
		/** @brief  */
		virtual bool eventFilter(QObject *watched, QEvent *evt)
		{
			if (evt->type() ==  QEvent::MouseButtonPress )
				this->activate();
			return QtWindow::eventFilter(watched, evt);
		}
		/** @brief  */
		virtual void mousePressEvent(QMouseEvent *evt)
		{
			this->activate();
			QtWindow::mousePressEvent(evt);
		}
		/** @brief  */
		virtual void mouseDoubleClickEvent(QMouseEvent *evt)
		{
			this->activate();
			QtWindow::mouseDoubleClickEvent(evt);
		}
		index_t mIndex;
	};

	class QtViewportWindow : public QWidget, public IViewportWindow, public Allocatable
	{
	public:
		QtViewportWindow();
		~QtViewportWindow();

		/************************************************************************/
		/* IUIWindow interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool		initWindow(uintptr_t parentWindowImpl, IconIndex icon, const TString& caption = TString::EMPTY);

		/** @brief  */
		virtual IKeyboard*	getKeyboard() const
		{
			if (mGlobalKeyboard != NULL)
				return mGlobalKeyboard;
			else
				return mKeyboard[mViewportSet->getActiveViewportIndex()];
		}

		/** @brief  */
		virtual IMouse*		getMouse() const
		{
			return mMouse[mViewportSet->getActiveViewportIndex()];
		}

		/** @brief  */
		virtual IUIToolBox*	createToolBox(const TString& name, ICONSIZE is);

		/** @brief  */
		virtual IUIToolBox*	getToolBox(const TString& name) const;

		/************************************************************************/
		/* IViewportWindow interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual index_t		getCurrentViewportLayout() const { return mViewportLayout; }

		/**
		@describe
		@param
		@return
		*/
		virtual bool		setCurrentViewportLayout(index_t index);

		/**
		@describe get window's view port set, DO NOT holding this pointer since it may change
		@param
		@return
		*/
		virtual IViewportSet*	getViewportSet() const { return mViewportSet; }

		/**
		@describe maximize/restore the active view port
		@param
		@return
		*/
		virtual bool		maximizeViewport();

		/**
		@describe whether active view port is maximized
		@param
		@return
		*/
		virtual bool		isViewportMaximized(index_t index) const;

		/**
		@describe get mouse for specific view port
		@param
		@return
		*/
		virtual IMouse*		getViewportMouse(index_t index) const
		{
			assert(index < mViewportSet->getViewportCount());
			return mMouse[index];
		}

		/**
		@describe
		@param
		@return
		*/
		virtual IKeyboard*	getViewportKeyboard(index_t index) const
		{
			assert(index < mViewportSet->getViewportCount());
			return mGlobalKeyboard != NULL ? mGlobalKeyboard : mKeyboard[index];
		}

		/**
		@describe
		@param
		@return
		*/
		virtual void		updateViewportLayout();

		/**
		@describe 
		@param
		@return
		*/
		virtual uintptr_t	getHandle() const {return (uintptr_t)static_cast<const QWidget*>(this);}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void updateViewportBorder(index_t activeIndex);

	protected:
		/** @brief  */
		virtual void resizeEvent(QResizeEvent *evt)	{ QWidget::resizeEvent(evt); this->updateViewportLayout();}

		QtViewPort		mViewport[MAX_VIEWPORT_COUNT];		//frame with toolbar
		QtFrame			mFrames[MAX_VIEWPORT_COUNT];		//border: IMPORTANT: child declared after parent
		QtViewPortContent*	mContent[MAX_VIEWPORT_COUNT];	//real content
		IWindow*		mViewportWindow[MAX_VIEWPORT_COUNT];
		IMouse*			mMouse[MAX_VIEWPORT_COUNT];
		IKeyboard*		mKeyboard[MAX_VIEWPORT_COUNT];
		IKeyboard*		mGlobalKeyboard;
		IViewportSet*	mViewportSet;
		index_t			mViewportLayout;

		typedef List<IUIToolBox*> ToolBoxList;
		ToolBoxList		mToolBoxes;
	};

	class QtViewportToolBox : public IViewportToolboxSet, public Allocatable
	{
	public:
		QtViewportToolBox(const TString& name, ICONSIZE is, QtViewPort viewPort[MAX_VIEWPORT_COUNT]);
		~QtViewportToolBox();

		/************************************************************************/
		/* IUIToolBox interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const { return mName; }

		/**
		@describe
		@param
		@return
		*/
		virtual bool			addButton(IUICommand* cmd, const TString& name, IconIndex icon, int style = TBS_NONE)
		{
			bool ret = true;
			for (int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
				ret = mViewportToolBox[i]->addButton(cmd, name, icon, style) && ret;
			return ret;
		}

		/*
		@describe
		@param [in] groupStyle set the style for none-embedded grouped menu
		@return
		*/
		virtual bool			addButton(IMenu* menu, int style = TBS_CHECKGROUP)
		{
			bool ret = true;
			for (int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
				ret = mViewportToolBox[i]->addButton(menu, style) && ret;
			return ret;
		}

		/*
		@describe
		@param [in] width: width of the tool, -1 means using default
		@return
		*/
		virtual bool			addTool(IConfig* config, int width = -1)
		{
			bool ret = true;
			for (int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
				ret = mViewportToolBox[i]->addTool(config, width) && ret;
			return ret;
		}

		/*
		@describe
		@param [in] config: group config with specified config list
		@return
		*/
		virtual bool			addToolSet(IConfig* config, IconIndex icon)
		{
			bool ret = true;
			for (int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
				ret = mViewportToolBox[i]->addToolSet(config, icon) && ret;
			return ret;
		}

		/*
		@describe update tool data from config
		@param
		@return
		*/
		virtual void			updateTools()
		{
			for (int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
				mViewportToolBox[i]->updateTools();
		}

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getButtonCount() const
		{
			return mViewportToolBox[0]->getIconSize();
		}

		/*
		@describe
		@param
		@return
		*/
		virtual ICONSIZE		getIconSize()	const
		{
			return mViewportToolBox[0]->getIconSize();
		}

		/*
		@describe
		@param
		@return
		*/
		virtual bool			setIconSize(ICONSIZE is)
		{
			bool ret = true;
			for (int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
				ret = mViewportToolBox[i]->setIconSize(is) && ret;
			return ret;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual IUIToolBox*	getViewportToolbox(index_t index) const { return index < MAX_VIEWPORT_COUNT ? mViewportToolBox[index] : NULL; }

	protected:
		//view port tool bars
		IUIToolBox*	mViewportToolBox[MAX_VIEWPORT_COUNT];
		TString	mName;
	};//class QtViewportToolBox
}//namespace Blade