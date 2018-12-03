/********************************************************************
	created:	2015/08/09
	filename: 	ITableWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ITableWindow_h__
#define __Blade_ITableWindow_h__
#include <BladeFramework.h>
#include <interface/public/input/IMouse.h>
#include <interface/public/ui/UIWindowBase.h>
#include <interface/IConfig.h>

namespace Blade
{
	class BLADE_FRAMEWORK_API ITableWindow : public UIWindowBase
	{
	public:
		static const TString TABLE_WINDOW_TYPE;
		enum EStyle
		{
			TES_NONE					= 0x0000,
			TES_FULL_ROW_SELECTION		= 0x0001,
			TES_MULTI_SELECTION			= 0x0002,
			TES_GRIDVIEW				= 0x0004,
			TES_SHOW_ROWNO				= 0x0008,	///show row number
			TES_ROWEDIT					= 0x0010,	///provide popup menu to add/delete row
			TES_ROWEDIT_HELPER			= 0x0020,	///easy/quick access helper/buttons for row editing, only takes effect with TES_ROWEDIT
			TES_CONTEXT_MENU			= 0x0040,	///context menu for row editing, only takes effect with TES_ROWEDIT
		};

		///cell position (row, column index)
		typedef struct SCellPosition
		{
			index_t col;
			index_t row;
		}CELLPOS;

		typedef enum ETableNavigation
		{
			TN_NONE,		///reinit: un-select all and navigate to start
			TN_LEFT,
			TN_RIGHT,
			TN_UP,
			TN_DOWN,

			TN_ROWSTART,	///first row
			TN_ROWEND,		///last row
			TN_COLSTART,	///first col
			TN_COLEND,		///last col

			TN_PREVPAGE,
			TN_NEXTPAGE,

			TN_START,
			TN_END,
		}ITEMNAV;

		///data listener
		class BLADE_FRAMEWORK_API IListener
		{
		public:
			virtual ~IListener()	{}
			/** @brief you can use it for change notification or get the modified data
			return false to discard change */
			virtual bool onItemChange(index_t row, index_t col, const Variant& val) = 0;
		};

		///view controller: controlling the function keys, i.e. LEFT/RIGHT/TAB, & customized keys
		///by default the table window has its own controller suitable for most situations
		///you don't need to implement your own controller unless you need advanced input/navigation control
		///when you want override part of the functionality, you can perform default behavior for the other part, using
		///window->getDefaultController() to access the default controller functions.
		///or, just return false, so that the default routine is called
		class BLADE_FRAMEWORK_API IController
		{
		public:
			/** @brief  */
			virtual bool onKeyDown(ITableWindow* window, EKeyCode kc, uint32 modifierMask)
			{
				BLADE_UNREFERENCED(window);BLADE_UNREFERENCED(kc);BLADE_UNREFERENCED(modifierMask);
				return false;
			}

			/** @brief  */
			virtual bool onMouseClick(ITableWindow* window, MSBUTTON button, const POINT2I& pos)
			{
				BLADE_UNREFERENCED(window);BLADE_UNREFERENCED(button);BLADE_UNREFERENCED(pos);
				return false;
			}

			/** @brief  */
			virtual bool onMouseDBClick(ITableWindow* window, MSBUTTON button, const POINT2I& pos)
			{
				BLADE_UNREFERENCED(window);BLADE_UNREFERENCED(button);BLADE_UNREFERENCED(pos);
				return false;
			}

			/** @brief customize navigation */
			virtual bool onSelectItem(ITableWindow* window, ITEMNAV nav, index_t* row, index_t* col)
			{
				BLADE_UNREFERENCED(window);BLADE_UNREFERENCED(nav);
				BLADE_UNREFERENCED(row);BLADE_UNREFERENCED(col);return false;
			}
		};

	public:
		ITableWindow() :UIWindowBase(ITableWindow::TABLE_WINDOW_TYPE)	{}
		virtual ~ITableWindow()	{}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	initialize(const HCONFIG* columnDescList, size_t count, int style = TES_NONE, IListener* listener = NULL, IController* controller = NULL) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual IController*	getDefaultController() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual int		getStyle() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	activate() = 0;
		
		/**
		@describe 
		@param
		@return
		*/
		virtual size_t	getColumnCount() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const HCONFIG&	getColumnDesc(index_t index) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual index_t	findColumn(const TString& columnName) const = 0;

		/**
		@describe add one column, all item for this column is initialized with config's default value.
		@param
		@return
		*/
		virtual bool	addColumn(index_t index, HCONFIG columnDesc) = 0;

		inline bool appendColumn(HCONFIG columnDesc)
		{
			return this->addColumn( this->getColumnCount(), columnDesc);
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	refreshColumn(index_t index) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	deleteColumn(index_t index) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	clearColumns() = 0;

		/**
		@describe row count
		@param
		@return
		*/
		virtual size_t	getRowCount() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual Variant*	getRow(index_t index) = 0;
		virtual const Variant*	getRow(index_t index) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	beginAddRow(size_t totalRows = 0) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	endAddRow() = 0;

		/**
		@describe insert one item. colCount must equal to current column count of the window.
		@param
		@return
		*/
		virtual bool	addRow(size_t index, const Variant* cols, size_t colCount, IconIndex icon = INVALID_ICON_INDEX) = 0;

		inline bool	appendRow(const Variant* cols, size_t colCount, IconIndex icon = INVALID_ICON_INDEX)
		{
			return this->addRow(this->getRowCount(), cols, colCount, icon);
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	deleteRow(index_t index) = 0;

		/**
		@describe delete all rows
		@param
		@return
		*/
		virtual bool	clearRows() = 0;

		/**
		@describe get the visible row count according to current window size
		@param
		@return
		*/
		virtual size_t	getVisibleRowCount() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual Variant& getCell(index_t row, index_t col) = 0;
		virtual const Variant& getCell(index_t row, index_t col) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t	getSelectedCount() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t	getSelectedCells(CELLPOS* items, size_t inputCount = 1) const = 0;

		/**
		@describe 
		@param columnIndex: specify the column in which to find. INVALID_INDEX means find in all colmuns
		@return
		*/
		class FindContext
		{
		public:
			const ITableWindow* window;
			index_t	row;
			index_t col;
		};
		typedef bool (*FnCellCompare)(const Variant& v, const FindContext& context);
		virtual bool	findCell(FnCellCompare cmpFunc, CELLPOS& outPos, index_t columnIndex = INVALID_INDEX) const = 0;

		/**
		@describe 
		@param [in] edit whether to edit current cell
		@param [in] if edit is false and current cell is being edited, whether to apply editing change.\n
		if edit is true, this parameter is ignored.
		@return
		*/
		virtual bool	editCurrentCell(bool edit, bool editupdate) = 0;

		/**
		@describe hit test
		@param [in] pos: position in window coordinate
		@return
		*/
		virtual bool	hitCell(const POINT2I& pos, CELLPOS& outCell) const = 0;

		/**
		@describe 
		@param [in] deselect: whether to deselect previous selection. if not, toggle new selection to existing
		selections if TES_MULTI_SELECTION style is set.
		@return
		*/
		virtual bool	navigateTo(const CELLPOS& pos, bool edit, bool deselect) = 0;
		inline bool	navigateTo(index_t row, index_t col, bool edit, bool deselect)
		{
			CELLPOS pos = {row, col};
			return this->navigateTo(pos, edit, deselect);
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	navigateTo(ITEMNAV nav, bool edit, bool deselect) = 0;
	};


	/************************************************************************/
	/* default behavior of table controller                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	class TableDefController : public ITableWindow::IController
	{
	protected:
		ITableWindow* mWindow;
		ITableWindow::IController* mController;
	public:
		TableDefController()
		{
			mWindow = NULL;
			mController = this;
		}

		/** @brief  */
		inline void initController(ITableWindow* window, ITableWindow::IController* controller)
		{
			mWindow = window;
			assert(mWindow != NULL);
			if(controller != NULL )
				mController = controller;
			else
				mController = this;
		}

		/************************************************************************/
		/* ITableWindow::IController                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	onKeyDown(ITableWindow* window, EKeyCode kc, uint32 modifierMask)
		{
			if(kc == KC_F2)
			{
				window->editCurrentCell(true, false);
				return true;
			}
			else if( kc == KC_ESCAPE )
			{
				window->editCurrentCell(false, false);
				return true;
			}

			ITableWindow::ITEMNAV nav = ITableWindow::TN_NONE;
			bool shift = (modifierMask & KM_SHIFT) != 0;
			bool control = (modifierMask & KM_CTRL) != 0;

			if( (kc == KC_LEFT && !control) || (kc == KC_TAB && shift) )
				nav = ITableWindow::TN_LEFT;
			else if( (kc == KC_RIGHT && !control) || (kc == KC_TAB && !shift) )
				nav = ITableWindow::TN_RIGHT;
			else if( (kc == KC_UP && !control) || (kc == KC_ENTER && shift) )
				nav = ITableWindow::TN_UP;
			else if( (kc == KC_DOWN && !control) || (kc == KC_ENTER && !shift) )
				nav = ITableWindow::TN_DOWN;
			else if( (kc == KC_HOME && !control)  || (kc == KC_LEFT && control) )
				nav = ITableWindow::TN_COLSTART;
			else if( (kc == KC_END && !control) || (kc == KC_RIGHT && control) )
				nav = ITableWindow::TN_COLEND;
			else if( kc == KC_UP && control )
				nav = ITableWindow::TN_ROWSTART;
			else if( kc == KC_DOWN && control )
				nav = ITableWindow::TN_ROWEND;
			else if( kc == KC_HOME && control )
				nav = ITableWindow::TN_START;
			else if( kc == KC_END && control )
				nav = ITableWindow::TN_END;
			else if( kc == KC_PAGEUP )
				nav = ITableWindow::TN_PREVPAGE;
			else if( kc == KC_PAGEDOWN )
				nav = ITableWindow::TN_NEXTPAGE;

			if( nav != ITableWindow::TN_NONE )
			{
				window->editCurrentCell(false, true);
				bool deSelectPrev = (kc == KC_ENTER) || (kc == KC_TAB) || !shift;
				window->navigateTo(nav, false, deSelectPrev);
			}
			return true;
		}

		/** @brief  */
		virtual bool onMouseClick(ITableWindow* window, MSBUTTON button, const POINT2I& pos)
		{
			ITableWindow::CELLPOS cellPos;
			bool deselect = (window->getStyle()&ITableWindow::TES_MULTI_SELECTION) == 0;

			if( window->hitCell(pos, cellPos) )
			{
				ITableWindow::CELLPOS activeCell;
				if( window->getSelectedCells(&activeCell) >= 1 && activeCell.row == cellPos.row && activeCell.col == cellPos.col )
					window->navigateTo(cellPos, true, deselect);
				else
				{
					window->editCurrentCell(false, true);
					window->navigateTo(cellPos, false, deselect);
				}
			}
			return button == MSB_LEFT;
		}

		/** @brief  */
		virtual bool onMouseDBClick(ITableWindow* window, MSBUTTON button, const POINT2I& pos)
		{
			if( button != MSB_LEFT )
				return true;

			bool deselect = (window->getStyle()&ITableWindow::TES_MULTI_SELECTION) == 0;
			ITableWindow::CELLPOS cellPos;
			if( window->hitCell(pos, cellPos) )
			{
				window->editCurrentCell(false, true);
				window->navigateTo(cellPos, true, deselect);
			}
			return true;
		}

		/** @brief  */
		virtual bool onSelectItem(ITableWindow* window, ITableWindow::ITEMNAV nav, index_t* row, index_t* col)
		{
			if( mController != this && mController->onSelectItem(window, nav, row, col) )
				return true;

			size_t rowCount = window->getRowCount();
			size_t colCount = window->getColumnCount();

			assert(*row < rowCount);
			assert(*col < colCount);
			switch(nav)
			{
			case ITableWindow::TN_LEFT:
				if( --(*col) == INVALID_INDEX)
					*col = 0;
				break;
			case ITableWindow::TN_RIGHT:
				if(++(*col) == colCount)
					*col = colCount - 1;
				break;
			case ITableWindow::TN_UP:
				if(--(*row) == INVALID_INDEX)
					*row = 0;
				break;
			case ITableWindow::TN_DOWN:
				if(++(*row) == rowCount)
					*row = rowCount - 1;
				break;
			case ITableWindow::TN_COLSTART:
				*col = 0;
				break;
			case ITableWindow::TN_COLEND:
				*col = colCount - 1;
				break;
			case ITableWindow::TN_ROWSTART:
				*row = 0;
				break;
			case ITableWindow::TN_ROWEND:
				*row = rowCount - 1;
				break;
			case ITableWindow::TN_START:
				*row = 0; *col = 0;
				break;
			case ITableWindow::TN_END:
				*row = rowCount - 1;
				*col = colCount - 1;
				break;
				//get the row count in client area
				//note: do we need to keep the selected cell at the same screen location?
				//-No, the default MFC CListCtrl has the same behavior like this
			case ITableWindow::TN_PREVPAGE:
				{
					size_t rows = window->getVisibleRowCount();
					if( *row < rows )
						*row = 0;
					else
						*row -= rows;
				}
				break;
			case ITableWindow::TN_NEXTPAGE:
				{
					size_t rows = window->getVisibleRowCount();
					if( *row + rows >= rowCount )
						*row = rowCount -1;
					else
						*row += rows;
				}
				break;
			default:
				assert(false);
				break;
			}
			return true;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool	onTableKeyDown(EKeyCode kc, uint32 modifierMask)
		{
			if( mController == this || !mController->onKeyDown(mWindow, kc, modifierMask) )
				this->onKeyDown(mWindow, kc, modifierMask);
			return true;
		}

		/** @brief x,y: window coordinates */
		bool	onTableMouseButtonDown(MSBUTTON button, const POINT2I& pos)
		{
			//we generate our own mouse click event in Win32MouseDevice,
			//but Win32 List Ctrl eat mouse up message, and we cannot detect the click event sometimes,
			//so we have to use button down as click

			//beside the problem above, Win32MouseDevice's SetCapture conflicts with List Ctrl.
			//now we use the MFC event callback directly, called by TableView
			//the ITableWindow's mouse object will no longer available.
			//http://stackoverflow.com/questions/30204761/wm-lbuttonup-is-not-received-in-my-subclass-procedure

			//use window coordinates, instead of table view's
			//this is because we need coordinates compatible  with IUIWindow::getWindow(),
			//so that client code can use popup menus etc correctly.

			bool ret = mController->onMouseClick(mWindow, button, pos);
			if( mController != this && !ret )
				ret = this->onMouseClick(mWindow, button, pos);
			return ret;
		}

		/** @brief x,y: window coordinates */
		bool	onTableMouseButtonDBClick(MSBUTTON button, const POINT2I& pos)
		{
			if( mController == this || !mController->onMouseDBClick(mWindow, button, pos) )
				return this->onMouseDBClick(mWindow, button, pos);
			return true;
		}

	};//class TableDefController
	
}//namespace Blade


#endif // __Blade_ITableWindow_h__