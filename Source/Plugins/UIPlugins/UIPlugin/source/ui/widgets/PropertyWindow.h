/********************************************************************
	created:	2011/05/12
	filename: 	PropertyWindow.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_PropertyWindow_h__
#define __Blade_PropertyWindow_h__
#include <interface/IFramework.h>
#include <databinding/IDataSourceDataBase.h>
#include <ConfigDialog/OptionList.h>
#include <ui/DockWindow.h>
#include <interface/public/ui/widgets/IUIPropertyGrid.h>

namespace Blade
{

	class PropertyWindow : public DockWindowBase<IUIPropertyGrid>, public IFramework::IMainLoopUpdater, public Allocatable
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		using Allocatable::operator new;
		using Allocatable::operator delete;
		using Allocatable::operator new[];
		using Allocatable::operator delete[];

	public:
		PropertyWindow();
		~PropertyWindow();

		/************************************************************************/
		/* IFramework::IMainLoopUpdater interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void	update();

		/** @brief  */
		virtual BOOL Create(LPCTSTR lpszText, DWORD dwStyle,
			const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);

		/** @brief  */
		void		ShowProperty(IConfig* config = NULL);

	protected:
		DECLARE_MESSAGE_MAP()

		afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	protected:
		/** @brief  */
		void		onBindProperty(const Event& data);
		/** @brief  */
		void		clearBinding();
		/** @brief  */
		virtual bool bindEntity(const IEntity* entity);
		/** @brief  */
		virtual const IEntity* getBoundEntity() const { return mTargetEntity; }

		CStatic			mTitle;
		COptionList		mPropertyGrid;
		HCONFIG	mRootEntityConfig;
		HCONFIG	mRootConfig;
		const IEntity*	mTargetEntity;
		ITimeDevice*	mTimer;

		typedef List<DataBinder*> BinderList;

		BinderList mConfigBinder;	//long time binding
	};

	
}//namespace Blade

#endif // __Blade_PropertyWindow_h__