/********************************************************************
	created:	2010/05/27
	filename: 	ConfigControlUtil.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ConfigControlUtil_h__
#define __Blade_ConfigControlUtil_h__
#include <interface/IConfig.h>
#include <utility/BladeContainer.h>
#include <ConfigDialog/OptionControls.h>

/** @brief  */
typedef	Blade::TStringParam					OptionValueList;
typedef Blade::EConfigUIHint				ListSubControlType;
typedef Blade::CONFIG_ACCESS				ListSubControlAccess;	

namespace ConfigControlUtil
{

	/*
	@describe
	@param
	@return
	*/
	void			ParseConfigOption(Blade::IConfig* config,OptionValueList& out_vector,int& currentValueIndex);

	/*
	@describe 
	@param 
	@return 
	*/
	ConfigControl*	CreateConfigControl(Blade::IConfig* config, CWnd* parent, const CRect& rect, UINT nID, Blade::ConfigUIHint* hintOverride = NULL, IControlNotify* notify = NULL);
	
}//namespace ConfigControlUtil



#endif //__Blade_ConfigControllMap_h__