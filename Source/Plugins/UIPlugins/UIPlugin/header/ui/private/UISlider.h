/********************************************************************
	created:	2016/7/21
	filename: 	UISlider.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_UISlider_h__
#define __Blade_UISlider_h__

class CUISlider : public CSliderCtrl, public Blade::Allocatable
{
	DECLARE_MESSAGE_MAP()
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
public:
	CUISlider();
	virtual ~CUISlider();

	/** @brief  */
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
};


#endif//__Blade_UISlider_h__