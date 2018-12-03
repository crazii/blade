/********************************************************************
	created:	2016/07/25
	filename: 	QtWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtWindow_h__
#define __Blade_QtWindow_h__
#include <QtToolbox.h>

namespace Blade
{

	class QtWindow : public QMainWindow
	{
	public:
		QtWindow();
		~QtWindow();

		/** @brief  */
		virtual index_t getIndex() const {return 0;}

		/** @brief  */
		void	updateUI();

		/** @brief  */
		QtToolbox*		createQtToolBox(const TString& name, ICONSIZE is, Qt::ToolBarAreas areas = Qt::TopToolBarArea|Qt::LeftToolBarArea, bool allowMove = true);

		/** @brief  */
		QtToolbox*		getQtToolBox(const TString& name) const;

		/** @brief  */
		void setMenuBar(QMenuBar *menubar);

		/** @brief  */
		bool needFilterToolButton() const { return mFilterToolboxButton; }

	protected:
		/** @brief  */
		bool eventFilter(QObject *watched, QEvent *evt);

		typedef List<QtToolbox*> ToolBoxList;
		ToolBoxList		mToolBoxes;
		bool			mFilterToolboxButton;

		typedef List<QtWindow*> WindowList;
		static WindowList msWindows;
	};
	
}//namespace Blade

#endif // __Blade_QtWindow_h__