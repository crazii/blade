/********************************************************************
	created:	2010/05/27
	filename: 	ConfigControlUtil.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ConfigDialog/ConfigControlUtil.h>
#include <ConfigDialog/TextSlider.h>
#include <ConfigDialog/FilePicker.h>
#include <ConfigDialog/ColorPicker.h>
#include <ConfigDialog/SliderInput.h>
#include <ConfigDialog/ImagePicker.h>


using namespace Blade;

namespace ConfigControlUtil
{

	//////////////////////////////////////////////////////////////////////////
	void			ParseConfigOption(IConfig* config, OptionValueList& out_vector, int& currentValueIndex)
	{
		assert( config != NULL );
		if( config == NULL )
			return;

		for( size_t i = 0; i < config->getOptions().size(); ++i )
			out_vector.push_back( config->getOptions()[i] );

		for(currentValueIndex = 0; currentValueIndex < (int)out_vector.size(); ++currentValueIndex )
		{
			if( config->getValue() == out_vector[(index_t)currentValueIndex] )
				break;
		}
		
		if( currentValueIndex == (int)out_vector.size() )
			currentValueIndex = -1;
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigControl*	CreateConfigControl(IConfig* config, CWnd* parent, const CRect& rect, UINT nID, 
		ConfigUIHint* hintOverride/* = NULL*/, IControlNotify* notify/* = NULL*/)
	{
		assert(config != NULL && parent != NULL );
		ConfigUIHint hint = config->getUIHint();
		if( hintOverride != NULL )
			hint = *hintOverride;

		ConfigControl* newControl = NULL;
		switch( hint )
		{
		case CUIH_NONE:
			break;
		case CUIH_INPUT:
		case CUIH_INPUT_RANGE:
			{
				CSliderInput* pInput = BLADE_NEW CSliderInput();
				pInput->Create(rect, parent, nID, config, false);
				newControl = pInput;
			}
			break;
		case CUIH_RANGE:
			{
				CTextSlider *pSliderShow = BLADE_NEW CTextSlider();
				pSliderShow->Create(WS_TABSTOP, rect, parent, nID, config);
				newControl = pSliderShow;
			}
			break;
		case CUIH_LIST:
		case CUIH_INDEX:
			{
				DropDownControl* pDropDown = BLADE_NEW DropDownControl();
				pDropDown->Create(rect, parent, nID, config);
				newControl = pDropDown;
			}
			break;
		case CUIH_CHECK:
			{
				CheckControl* pButton = BLADE_NEW CheckControl();
				pButton->Create(rect, parent, nID, config);
				newControl = pButton;
			}
			break;
		case CUIH_FILE: case CUIH_PATH: case CUIH_CUSTOM:
			{
				CFilePicker* pFilePicker = BLADE_NEW CFilePicker();
				pFilePicker->Create(rect, parent, nID, config);
				newControl = pFilePicker;
			}
			break;

		case CUIH_COLOR:
			{
				CColorPicker* pColorPicker = BLADE_NEW CColorPicker();
				pColorPicker->Create(rect, parent, nID, config);
				newControl = pColorPicker;
			}
			break;
		case CUIH_IMAGE:
			{
				CImagePicker* pImagePicker = BLADE_NEW CImagePicker();
				pImagePicker->Create(rect, parent, nID, config);
				newControl = pImagePicker;
			}
			break;
		default:
			break;
		}//switch

		if( newControl != NULL )
		{
			newControl->SetNotify(notify);
			newControl->GetWindow()->SetFont( parent->GetFont() );
		}

		return newControl;
	}

}//namespace ConfigControlUtil