// TrackViewUI.cpp : implementation file
//
#include "stdafx.h"
#include <utility/Mask.h>
#include <interface/ITimeService.h>
#include <ui/IconManager.h>
#include "BladeUIPlugin.h"
#include "TrackViewUI.h"
#include "TrackViewRangeDialog.h"

using namespace Blade;
// CTrackViewUI dialog
//////////////////////////////////////////////////////////////////////////
CTrackViewUI::CTrackViewUI(CWnd* pParent /*=NULL*/)
	:CUIDialog(CTrackViewUI::IDD, pParent)
	,mTooltip(NULL)
{
	IFramework::getSingleton().addUpdater( this );
	mTimer.reset();
}

//////////////////////////////////////////////////////////////////////////
CTrackViewUI::~CTrackViewUI()
{
	if( this->GetSafeHwnd() != NULL )
	{
		CButton* button = static_cast<CButton*>(this->GetDlgItem( IDC_TRACKVIEW_PLAYPAUSE ) );
		if( button != NULL && button->GetSafeHwnd() != NULL )
			button->SetBitmap(NULL);
	}

	::DeleteObject( mPlayIcon );
	::DeleteObject( mPauseIcon );

	IFramework::getSingleton().removeUpdater( this );
}

//////////////////////////////////////////////////////////////////////////
void	CTrackViewUI::update()
{
	mTrackData.update();
	this->updateUI(false);
}

//////////////////////////////////////////////////////////////////////////
void	CTrackViewUI::updateUI(bool force)
{
	if( mTrackData.getTrack() == NULL || mTrackData.getTrack()->getFrameTime() < Math::HIGH_EPSILON || mTrackData.getTrack()->getDuration() < Math::HIGH_EPSILON )
		return;

	mTimer.update();
	if( mTimer.getSeconds() > mTrackData.getTrack()->getFrameTime() )
		mTimer.reset();
	else if( !force )
		return;

	scalar frameTime = mTrackData.getTrack()->getFrameTime();

	AFX_MANAGE_STATE(::AfxGetStaticModuleState());
	if( this->IsWindowVisible() )
	{
		scalar position = mTrackData.getTimeLine();
		int sliderPos = (int)(position/frameTime);
		CSliderCtrl* slider = static_cast<CSliderCtrl*>( this->GetDlgItem(IDC_TRACKVIEW_TRACK) );
		int oldPos = slider->GetPos();
		//const tchar* animation = mTrack->getCurrentAnimation();
		if( oldPos != sliderPos || force)
		{
			slider->SetPos(sliderPos);
			CStatic* timeInfo = static_cast<CStatic*>(this->GetDlgItem( IDC_TRACKVIEW_TIME_SEQUENCE) );
			TStringStream stream;

			scalar duration = mTrackData.getTrack()->getDuration();
			//animation changed, update slider range.
			slider->SetRange(0, mTrackData.getFrameCount());	//frame count
			slider->SetSelection(mTrackData.getStartFrame(), mTrackData.getEndFrame());

			stream.format(TEXT("%.3f/%.3f\n%d/%d"), position, duration, sliderPos, mTrackData.getFrameCount() );
			TString s = stream.str();
			timeInfo->SetWindowText( s.c_str() );
		}

		CButton* button = static_cast<CButton*>(this->GetDlgItem( IDC_TRACKVIEW_PLAYPAUSE ) );
		if( mTrackData.isPlaying() && button->GetCheck() == BST_UNCHECKED )
		{
			button->SetCheck( BST_CHECKED );
			button->SetBitmap( mPauseIcon );
		}
		else if( !mTrackData.isPlaying() && button->GetCheck() == BST_CHECKED )
		{
			button->SetCheck( BST_UNCHECKED );
			button->SetBitmap( mPlayIcon );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool	CTrackViewUI::setTrack(Blade::ITrack* track)
{
	if( mTrackData.getTrack() != track )
	{
		mTrackData.setTrack(track);
		if( track != NULL )
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			//update track UI according to track
			CSliderCtrl* slider = static_cast<CSliderCtrl*>( this->GetDlgItem(IDC_TRACKVIEW_TRACK) );

			Mask mask = (uint32)track->getFeatures();
			if( mask.checkBits(ITrack::TF_SEEK) )
				slider->EnableWindow(TRUE);

			this->updateUI(true);
		}
		else
		{
			CSliderCtrl* slider = static_cast<CSliderCtrl*>( this->GetDlgItem(IDC_TRACKVIEW_TRACK) );
			slider->EnableWindow(FALSE);
			slider->SetPos(0);
			CStatic* timeInfo = static_cast<CStatic*>(this->GetDlgItem( IDC_TRACKVIEW_TIME_SEQUENCE) );
			timeInfo->SetWindowText(TEXT("0.0/1.0\n0/1"));
		}
	}
	return true;
}


/************************************************************************/
/* MFC related                                                                     */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
void CTrackViewUI::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRACKVIEW_TRACK, mSlider);
}

//////////////////////////////////////////////////////////////////////////
BOOL CTrackViewUI::OnInitDialog()
{
	BOOL result = __super::OnInitDialog();
	if( result )
	{
		this->EnableToolTips(TRUE);

		//position buttons
		struct 
		{
			int		id;
			TString icon;
			TString tip;
		}buttonList[] = 
		{
			IDC_TRACKVIEW_BEGIN,	BTString("start.png"),		BTLang(BLANG_TRACK_BEGIN),
			IDC_TRACKVIEW_PREV,		BTString("previous.png"),	BTLang(BLANG_TRACK_PREV),
			IDC_TRACKVIEW_PLAYPAUSE,BTString("play.png"),		BTLang(BLANG_TRACK_PLAY),
			IDC_TRACKVIEW_NEXT,		BTString("next.png"),		BTLang(BLANG_TRACK_NEXT),
			IDC_TRACKVIEW_END,		BTString("end.png"),		BTLang(BLANG_TRACK_END),
		};

		size_t size = 32;
		size_t offset = 0;
		const size_t spacing = 2;
		const TString& iconPath = IconManager::getSingleton().getIconPath();
		for(int i = 0; i < sizeof(buttonList)/sizeof(buttonList[0]); ++i)
		{
			CButton* button = static_cast<CButton*>(this->GetDlgItem( buttonList[i].id ) );
			HSTREAM stream = IResourceManager::getSingleton().loadStream(iconPath + buttonList[i].icon );
			HIMAGE img = IImageManager::getSingleton().loadImage( stream, IP_TEMPORARY);

			button->SetWindowPos(NULL, (int)offset, 0, (int)size, (int)size, SWP_NOZORDER);
			img = IImageManager::getSingleton().scaleImage(img, SIZE2I((int)size, (int)size), IP_TEMPORARY);

			HBITMAP hBitmap = IconManager::getSingleton().createBitmap( img );
			button->SetBitmap(hBitmap);
			if( mTooltip != NULL )
			{
				CRect rect;
				button->GetWindowRect(&rect);
				this->ScreenToClient(&rect);
				mTooltip->AddTool( button, buttonList[i].tip.c_str() );
			}
			offset += size + spacing;
		}

		//position texts
		CStatic* timeInfo = static_cast<CStatic*>(this->GetDlgItem( IDC_TRACKVIEW_TIME_SEQUENCE) );
		timeInfo->SetWindowPos(NULL, (int)offset, 0, (int)size*3, (int)size, SWP_NOZORDER);
		offset += size*3 + spacing;

		//position slider
		CWnd* slider = this->GetDlgItem(IDC_TRACKVIEW_TRACK);
		slider->SetWindowPos(NULL, (int)offset, 0, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

		//
		this->GetDlgItem(IDC_TRACKVIEW_TRACK)->EnableWindow(FALSE);
		CButton* button = static_cast<CButton*>(this->GetDlgItem( IDC_TRACKVIEW_PLAYPAUSE ) );
		mPlayIcon = button->GetBitmap();

		HSTREAM stream = IResourceManager::getSingleton().loadStream(IconManager::getSingleton().getIconPath() + BTString("pause.png"));
		HIMAGE img = IImageManager::getSingleton().loadImage( stream, IP_TEMPORARY);
		img = IImageManager::getSingleton().scaleImage(img, SIZE2I((int)size, (int)size), IP_TEMPORARY);
		mPauseIcon = IconManager::getSingleton().createBitmap( img );
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////
BOOL CTrackViewUI::PreTranslateMessage(MSG* pMsg)
{
	if( mTooltip != NULL )
		mTooltip->RelayEvent(pMsg);
	return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CTrackViewUI, CUIDialog)
	ON_WM_SIZE()
	ON_WM_MOUSEACTIVATE()
	ON_WM_HSCROLL()	//it's pretty tricky & dirty for MFC uses HSCROLL for slider events
	ON_BN_CLICKED(IDC_TRACKVIEW_BEGIN, &CTrackViewUI::OnBnClickedTrackviewBegin)
	ON_BN_CLICKED(IDC_TRACKVIEW_PREV, &CTrackViewUI::OnBnClickedTrackviewPrev)
	ON_BN_CLICKED(IDC_TRACKVIEW_PLAYPAUSE, &CTrackViewUI::OnBnClickedTrackviewPlaypause)
	ON_BN_CLICKED(IDC_TRACKVIEW_NEXT, &CTrackViewUI::OnBnClickedTrackviewNext)
	ON_BN_CLICKED(IDC_TRACKVIEW_END, &CTrackViewUI::OnBnClickedTrackviewEnd)

	ON_BN_DOUBLECLICKED(IDC_TRACKVIEW_BEGIN, &CTrackViewUI::OnBnClickedTrackviewBegin)
	ON_BN_DOUBLECLICKED(IDC_TRACKVIEW_PREV, &CTrackViewUI::OnBnClickedTrackviewPrev)
	ON_BN_DOUBLECLICKED(IDC_TRACKVIEW_PLAYPAUSE, &CTrackViewUI::OnBnClickedTrackviewPlaypause)
	ON_BN_DOUBLECLICKED(IDC_TRACKVIEW_NEXT, &CTrackViewUI::OnBnClickedTrackviewNext)
	ON_BN_DOUBLECLICKED(IDC_TRACKVIEW_END, &CTrackViewUI::OnBnClickedTrackviewEnd)

	ON_STN_DBLCLK(IDC_TRACKVIEW_TIME_SEQUENCE, &CTrackViewUI::OnBnClickedTimeSequence)
END_MESSAGE_MAP()


// CTrackViewUI message handlers
//////////////////////////////////////////////////////////////////////////
void CTrackViewUI::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	CRect rect;
	this->GetClientRect(&rect);

	//layout slider
	CWnd* slider = this->GetDlgItem(IDC_TRACKVIEW_TRACK);
	if( slider != NULL && slider->GetSafeHwnd() != NULL )
	{
		CRect rcChid;
		slider->GetWindowRect(&rcChid);
		this->ScreenToClient(&rcChid);
		slider->SetWindowPos(NULL, 0, 0, rect.Width()-rcChid.left, rect.Height(), SWP_NOZORDER|SWP_NOMOVE);
	}
}

//////////////////////////////////////////////////////////////////////////
int CTrackViewUI::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	//sometimes mouse move directly to button and this not activated,
	//the button event won't be generated, so activate this first
	this->SetFocus();
	return __super::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

//////////////////////////////////////////////////////////////////////////
//http://msdn.microsoft.com/en-us/library/ekx9yz55.aspx
void CTrackViewUI::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar)
{
	if( mTrackData.getTrack() != NULL && (mTrackData.getTrack()->getFeatures()&ITrack::TF_SEEK) )
	{
		CSliderCtrl* slider = reinterpret_cast<CSliderCtrl*>( pScrollBar );
		assert( slider == this->GetDlgItem(IDC_TRACKVIEW_TRACK) );	//this is the only slider/scrollbar we have.
		if( slider != NULL )
		{
			int min, max;
			slider->GetRange(min, max);

			int pos = slider->GetPos();
			mTrackData.play(false);
			//force align end to duration
			scalar timeLine = pos == max ? mTrackData.getTrack()->getDuration() : (scalar)pos * mTrackData.getTrack()->getFrameTime();
			mTrackData.setTimeLine(timeLine);
			this->updateUI(true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewUI::OnBnClickedTrackviewBegin()
{
	if( mTrackData.getTrack() != NULL && (mTrackData.getTrack()->getFeatures()&ITrack::TF_SEEK) )
	{
		mTrackData.play(false);
		mTrackData.setTimeLine( mTrackData.getStartTime() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewUI::OnBnClickedTrackviewPrev()
{
	if( mTrackData.getTrack() != NULL && (mTrackData.getTrack()->getFeatures()&ITrack::TF_SEEK) )
	{
		mTrackData.gotoPrevFrame();
		mTrackData.play(false);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewUI::OnBnClickedTrackviewPlaypause()
{
	if( mTrackData.getTrack() != NULL)
	{
		CButton* button = static_cast<CButton*>(this->GetDlgItem( IDC_TRACKVIEW_PLAYPAUSE ) );
		if( mTrackData.isPlaying() )
		{
			mTrackData.play(false);
			button->SetBitmap( mPlayIcon );
			button->SetCheck(BST_UNCHECKED);
		}
		else
		{
			mTrackData.play(true);
			button->SetBitmap( mPauseIcon );
			button->SetCheck(BST_CHECKED);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewUI::OnBnClickedTrackviewNext()
{
	if( mTrackData.getTrack() != NULL && (mTrackData.getTrack()->getFeatures()&ITrack::TF_SEEK) )
	{
		mTrackData.play(false);
		mTrackData.gotoNextFrame();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewUI::OnBnClickedTrackviewEnd()
{
	if( mTrackData.getTrack() != NULL && (mTrackData.getTrack()->getFeatures()&ITrack::TF_SEEK) )
	{
		mTrackData.play(false);
		mTrackData.gotoEndFrame();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewUI::OnBnClickedTimeSequence()
{
	if( mTrackData.getTrack() != NULL )
	{
		CTrackViewRangeDialog dlg(0, mTrackData.getFrameCount(), mTrackData.getStartFrame(), mTrackData.getEndFrame(), ::AfxGetMainWnd());
		if( dlg.DoModal() == IDOK )
		{
			mTrackData.setStartFrame( dlg.getRangeStart() );
			mTrackData.setEndFrame( dlg.getRangeEnd() );
			CSliderCtrl* slider = static_cast<CSliderCtrl*>( this->GetDlgItem(IDC_TRACKVIEW_TRACK) );
			slider->SetSelection(mTrackData.getStartFrame(), mTrackData.getEndFrame());
		}
	}
}
