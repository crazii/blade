/********************************************************************
	created:	2016/7/28
	filename: 	QtViewportWindow.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <interface/public/ui/IViewportManager.h>
#include <uiwindows/QtViewportWindow.h>
#include <QtIconManager.h>
#include <QtToolbox.h>

namespace Blade
{
	class QtGraphicsScene : public QGraphicsScene, public Allocatable
	{
	public:
		QtGraphicsScene(QObject* parent = NULL) :QGraphicsScene(parent) {}
		virtual void drawBackground(QPainter *painter, const QRectF &rect) {}
		virtual void drawForeground(QPainter *painter, const QRectF &rect) {}
		virtual void drawItems(QPainter *painter, int numItems,
			QGraphicsItem *items[],
			const QStyleOptionGraphicsItem options[],
			QWidget *widget = Q_NULLPTR) {}
	};

	class QtgraphicsViewport : public QWidget
	{
	public:
		QtgraphicsViewport()
		{
			this->setAttribute(Qt::WA_TranslucentBackground, true);
			this->setAttribute(Qt::WA_DontCreateNativeAncestors, true);
			this->setAttribute(Qt::WA_NativeWindow);
			this->setAttribute(Qt::WA_DeleteOnClose, true);
			this->setStyleSheet("background-color: transparent");
		}
		/** @brief  */
		virtual QPaintEngine* paintEngine() const { return NULL; }
		/** @brief  */
		virtual void paintEvent(QPaintEvent *evt) { }
	};

	//////////////////////////////////////////////////////////////////////////
	void QtViewPort::activate()
	{
		static_cast<QtViewportWindow*>(this->parent())->updateViewportBorder(mIndex);

		QLayout* lyt = this->centralWidget()->layout();
		QtViewPortContent* contet = static_cast<QtViewPortContent*>(lyt->itemAt(0)->widget());
		if(contet != NULL)
			contet->activate();
	}

	//////////////////////////////////////////////////////////////////////////
	QtViewportWindow::QtViewportWindow()
		:mGlobalKeyboard(NULL)
		,mViewportSet(NULL)
	{
		this->setAttribute(Qt::WA_DeleteOnClose, true);
		this->setAttribute(Qt::WA_TranslucentBackground, true);
		this->setAttribute(Qt::WA_DontCreateNativeAncestors);
		this->setAttribute(Qt::WA_NativeWindow);

		for (int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			mViewportWindow[i] = NULL;
			mMouse[i] = NULL;
			mKeyboard[i] = NULL;
			mContent[i] = BLADE_NEW QtViewPortContent();
			QFrame* frame = &mFrames[i];
			frame->setFrameShape(QFrame::StyledPanel);
			frame->setLineWidth(1);
			frame->setFrameShadow(QFrame::Plain);

			mViewport[i].setIndex((index_t)i);
			mViewport[i].setParent(this);
			mViewport[i].setCentralWidget(frame);
			mViewport[i].setContextMenuPolicy(Qt::NoContextMenu);

			mContent[i]->setAttribute(Qt::WA_PaintOnScreen, true);

			//mContent[i]->QGraphicsView::setViewport(QT_NEW QtgraphicsViewport());
			//mContent[i]->viewport()->setAttribute(Qt::WA_PaintOnScreen, true);
			//mContent[i]->viewport()->setAttribute(Qt::WA_DontCreateNativeAncestors, true);
			//mContent[i]->viewport()->setAttribute(Qt::WA_NativeWindow);
			//mContent[i]->viewport()->setAttribute(Qt::WA_DeleteOnClose, true);
			mContent[i]->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
			mContent[i]->viewport()->setAutoFillBackground(false);
			mContent[i]->viewport()->setStyleSheet("background-color: transparent");
			////mContent[i]->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);

			//mContent[i]->setUpdatesEnabled(false);
			QGraphicsScene* scene = QT_NEW QtGraphicsScene(mContent[i]);
			mContent[i]->setScene( scene );
			//mContent[i]->setVisible(false);

			QBoxLayout* layout = QT_NEW QBoxLayout(QBoxLayout::Down);
			layout->setContentsMargins(0,0,0,0);
			layout->setSpacing(0);
			frame->setLayout(layout);
			layout->addWidget(mContent[i]);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	QtViewportWindow::~QtViewportWindow()
	{
		if (mViewportSet != NULL)
			IViewportManager::getSingleton().destroyViewportSet(mViewportSet);

		for (size_t i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			BLADE_DELETE mContent[i];
			IWindowService::getSingleton().destroyWindow(mViewportWindow[i]->getUniqueName());
		}

		for(ToolBoxList::iterator i = mToolBoxes.begin(); i != mToolBoxes.end(); ++i)
			BLADE_DELETE *i;
	}

	/************************************************************************/
	/* IUIWindow interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		QtViewportWindow::initWindow(uintptr_t parentWindowImpl, IconIndex icon, const TString& caption/* = TString::EMPTY*/)
	{
		QWidget* parent = (QWidget*)(void*)parentWindowImpl;
		this->setParent(parent);
		QIcon* qicon = QtIconManager::getSingleton().getIcon(icon);
		if (qicon != NULL)
			this->setWindowIcon(*qicon);

		mGlobalKeyboard = IUIService::getSingleton().getGlobalKeyboard();
		for (size_t i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			IWindow* ui_window = IWindowService::getSingleton().attachWindow( (uintptr_t)static_cast<QWidget*>(mContent[i]/*->viewport()*/) );
			mViewportWindow[i] = ui_window;

			mMouse[i] = IUIService::getSingleton().getInputMouse(ui_window);
			if (mMouse[i] == NULL)
				mMouse[i] = IUIService::getSingleton().createInputMouse(ui_window);
			if (mGlobalKeyboard == NULL)
			{
				mKeyboard[i] = IUIService::getSingleton().getInputKeyboard(ui_window);
				if (mKeyboard[i] == NULL)
					mKeyboard[i] = IUIService::getSingleton().createInputKeyboard(ui_window);
			}
		}

		this->activateWindow();
		this->init((uintptr_t)static_cast<QWidget*>(this), icon, caption, false, false);

		return this->setCurrentViewportLayout(IViewportManager::getSingleton().getCurrentLayout());;
	}

	//////////////////////////////////////////////////////////////////////////
	IUIToolBox*	QtViewportWindow::createToolBox(const TString& name, ICONSIZE is)
	{
		if (this->getToolBox(name) != NULL)
		{
			assert(false);
			return NULL;
		}
		QtViewportToolBox* toolbox = BLADE_NEW QtViewportToolBox(name, is, mViewport);
		mToolBoxes.push_back(toolbox);
		return toolbox;
	}

	//////////////////////////////////////////////////////////////////////////
	IUIToolBox*	QtViewportWindow::getToolBox(const TString& name) const
	{
		for (ToolBoxList::const_iterator i = mToolBoxes.begin(); i != mToolBoxes.end(); ++i)
		{
			QtViewportToolBox* tb = static_cast<QtViewportToolBox*>(*i);
			if (tb->getName() == name)
				return tb;
		}
		return NULL;
	}

	/************************************************************************/
	/* IViewportWindow interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		QtViewportWindow::setCurrentViewportLayout(index_t index)
	{
		if (mViewportLayout == index && mViewportSet != NULL)
			return false;
		mViewportLayout = index;

		if (mViewportLayout >= IViewportManager::getSingleton().getLayoutCount())
		{
			assert(false);
			return false;
		}
		const VIEWPORT_LAYOUT& layout = IViewportManager::getSingleton().getLayout(mViewportLayout);
		if (mViewportSet == NULL)
			mViewportSet = IViewportManager::getSingleton().createViewportSet(layout);
		else
			mViewportSet->resetLayout(&layout);

		index_t maximizedIndex = INVALID_INDEX;
		index_t activeIndex = INVALID_INDEX;
		for (size_t i = 0; i < mViewportSet->getViewportCount(); ++i)
		{
			IViewport* viewport = mViewportSet->getViewport(i);
			mContent[i]->setViewport(viewport);
			viewport->setWindow(mViewportWindow[i]);

			//only find the first one
			if (viewport->isMaximized() && maximizedIndex == INVALID_INDEX)
				maximizedIndex = i;
			mViewport[i].setVisible(true);

			if (viewport->isActive())
				activeIndex = i;
		}

		for (size_t i = mViewportSet->getViewportCount(); i < MAX_VIEWPORT_COUNT; ++i)
		{
			mViewport[i].setVisible(false);
			mViewport[i].setEnabled(false);
			mContent[i]->setViewport(NULL);
		}

		if (activeIndex != INVALID_INDEX)
		{
			if (maximizedIndex != INVALID_INDEX)
			{
				assert(maximizedIndex == activeIndex);
				//maximizeViewport is a toggle operation, disable it first, and then toggle it
				mViewportSet->getViewport(maximizedIndex)->setMaximized(false);
				this->maximizeViewport();
			}
			this->updateViewportLayout();
			this->updateViewportBorder(activeIndex);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtViewportWindow::maximizeViewport()
	{
		size_t count = mViewportSet->getViewportCount();
		index_t index = mViewportSet->getActiveViewportIndex();
		assert(index < count);
		bool maximized = !mViewportSet->getViewport(index)->isMaximized();
		mViewportSet->getViewport(index)->setMaximized(maximized);
		this->updateViewportLayout();

		for (index_t i = 0; i < count; ++i)
		{
			if (i == index)
				mViewport[i].setVisible(true);
			else
			{
				mViewportSet->getViewport(i)->setMaximized(false);
				mViewport[i].setVisible(maximized ? false : true);
			}
			if(mViewport[i].isVisible())
				mContent[i]->paint();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtViewportWindow::isViewportMaximized(index_t index) const
	{
		if (index < mViewportSet->getViewportCount())
			return mViewportSet->getViewport(index)->isMaximized();
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void		QtViewportWindow::updateViewportLayout()
	{
		////qt event happens before init
		//if (mViewportSet == NULL)
		//	return;
		QSize sz = this->size();
		for (size_t i = 0; i < mViewportSet->getViewportCount(); ++i)
		{
			IViewport* viewPort = mViewportSet->getViewport(i);
			if (viewPort->isMaximized())
			{
				if (viewPort->getView() != NULL)
				{
					viewPort->getView()->setSizePosition(0.0f, 0.0f, 1.0f, 1.0f);
				}
				mViewport[i].move(0, 0);
				mViewport[i].resize(sz.width(), sz.height());
			}
			else
			{
				if (viewPort->getView() != NULL)
				{
					const VIEWPORT_DESC& desc = viewPort->getDesc();
					viewPort->getView()->setSizePosition(desc.mLeft, desc.mTop, desc.mRight, desc.mBottom);
				}

				const VIEWPORT_DESC& desc = viewPort->getDesc();
				int left = (int)(sz.width()*desc.mLeft);
				int right = (int)(sz.width()*desc.mRight);
				int top = (int)(sz.height()*desc.mTop);
				int bottom = (int)(sz.height()*desc.mBottom);

				mViewport[i].move(left, top);
				mViewport[i].resize(right - left, bottom - top);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtViewportWindow::updateViewportBorder(index_t activeIndex)
	{
		for(index_t i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			QFrame* frame = static_cast<QFrame*>(mViewport[i].centralWidget());
			if (activeIndex == i)
				frame->setStyleSheet("background-color: transparent; border: 1px solid red;");
			else
				frame->setStyleSheet("background-color: transparent;");
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	QtViewportToolBox::QtViewportToolBox(const TString& name, ICONSIZE is, QtViewPort viewPort[MAX_VIEWPORT_COUNT])
	{
		for (int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			QtToolbox* toolBox = viewPort[i].createQtToolBox(name, is, Qt::TopToolBarArea, false);
			mViewportToolBox[i] = toolBox;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	QtViewportToolBox::~QtViewportToolBox()
	{

	}
	
}//namespace Blade