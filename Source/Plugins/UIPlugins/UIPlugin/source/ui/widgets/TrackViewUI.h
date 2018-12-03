#pragma once
#include "resource.h"
#include <interface/IFramework.h>
#include <interface/public/ITrack.h>
#include <interface/public/time/Time.h>
#include <ui/private/UIDialog.h>
#include <ui/private/UISlider.h>

// CTrackViewUI dialog

class CTrackViewUI : public CUIDialog, public Blade::IFramework::IMainLoopUpdater
{
public:
	CTrackViewUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrackViewUI();

// Dialog Data
	enum { IDD = IDD_TRACKVIEW };

public:

	/** @brief Blade::IFramework::IMainLoopUpdater */
	virtual void	update();

	/** @brief  */
	void			updateUI(bool force);

	/** @brief  */
	bool	setTrack(Blade::ITrack* track);

	/** @brief  */
	Blade::ITrack*	getTrack() const		{return mTrackData.getTrack();}

protected:
	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	/** @brief  */
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int	 OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedTrackviewBegin();
	afx_msg void OnBnClickedTrackviewPrev();
	afx_msg void OnBnClickedTrackviewPlaypause();
	afx_msg void OnBnClickedTrackviewNext();
	afx_msg void OnBnClickedTrackviewEnd();
	afx_msg void OnBnClickedTimeSequence();

	DECLARE_MESSAGE_MAP()

public:
	/** @brief  */
	inline void		setToolTip(CToolTipCtrl* tooltip)	{mTooltip = tooltip;}

protected:
	CToolTipCtrl*	mTooltip;
	CUISlider		mSlider;
	Blade::Timer	mTimer;
	HBITMAP			mPlayIcon;
	HBITMAP			mPauseIcon;
	Blade::TrackData mTrackData;
};
