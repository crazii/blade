/********************************************************************
	created:	2013/04/28
	filename: 	ViewportOption.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ViewportOption_h__
#define __Blade_ViewportOption_h__
#include <ui/private/UITabDialog.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class LayoutPage;
	class ViewButton : public CButton, public TempAllocatable
	{
	public:
		using TempAllocatable::operator new;
		using TempAllocatable::operator delete;
	public:
		ViewButton(LayoutPage* parent, index_t layout, bool drawText);
		virtual BOOL	Create(CWnd* pParentWnd);
		virtual void	DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
		afx_msg BOOL	OnClicked();
		/** @brief  */
		void	setLayout(index_t layout)	{mIndex = layout; this->Invalidate();}

		LayoutPage* mParent;
		index_t mIndex;
		bool	mDrawText;

		DECLARE_MESSAGE_MAP()
	};

	class LayoutPage : public ITabPage, public TempAllocatable
	{
	public:
		LayoutPage();
		~LayoutPage();
		/** @brief  */
		virtual CWnd*	createWindow(CWnd* parent);
		/** @brief  */
		virtual CWnd*	getWindow();
		/** @brief  */
		virtual bool	onOK();
		/** @brief  */
		virtual bool	onCancel();
		/** @brief  */
		virtual bool	onApply();

		/** @brief  */
		void	setCurrentLayout(index_t layout);
		/** @brief  */
		index_t	getCurrentLayout() const	{return mCurrentLayout;}
		/** @brief  */
		void	onWindowResize();

		CWnd			mWnd;
		index_t			mCurrentLayout;

		typedef TempVector<ViewButton*> ViewButtonList;
		ViewButton*		mLargePreview;

		ViewButtonList	mButtons;
	};
	
}//namespace Blade

#endif//__Blade_ViewportOption_h__