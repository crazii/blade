/********************************************************************
	created:	2015/11/17
	filename: 	ImporterParams.h
	author:		
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ImporterParams_h__
#define __Blade_ImporterParams_h__
#include <utility/String.h>

namespace Blade
{

	//model imports params
	static const TString IMPORT_MODEL_PATH				= BTString("MIMP::TString::PATH");
	static const TString IMPORT_MODEL_SAMPLING_RATE		= BTString("MIMP::scalar::SAMPLING_RATE");
	static const TString IMPORT_MODEL_POS_ERROR			= BTString("MIMP::scalar::POS_ERROR");
	static const TString IMPORT_MODEL_ANGLE_ERROR		= BTString("MIMP::scalar::ANGLE_ERROR");
	static const TString IMPORT_MODEL_EXTERNAL_TEXTURE	= BTString("MIMP::bool::EXTERNAL_TEXTURE");
	static const TString IMPORT_MODEL_SKELETON_FULLPATH	= BTString("MIMP::bool::SKELETON_FULLPATH");
	static const TString IMPORT_MODEL_ENABLE_SKELETON	= BTString("MIMP::bool::ENABLE_SKELETON");
	static const TString IMPORT_MODEL_ENABLE_MESH		= BTString("MIMP::bool::ENABLE_MESH");


	//other params
	
}//namespace Blade


#endif // __Blade_ImporterParams_h__