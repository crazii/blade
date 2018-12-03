/********************************************************************
	created:	2013/03/23
	filename: 	BladeModel.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeModel_h__
#define __Blade_BladeModel_h__
#include <utility/String.h>
#include <interface/IModel.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_MODEL_EXPORTS
#			define BLADE_MODEL_API __declspec( dllexport )
#		else
#			define BLADE_MODEL_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_MODEL_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_MODEL_API	//static link lib

#endif

namespace Blade
{
	namespace ModelConsts
	{
		//note: those constants are not exported,
		//they will be constructed in the client DLL wherever being used.
		//this can reduce the DLL linkage if possible
		static const TString MODEL_ELEMENT_TYPE = BTString("Model Element");
		static const TString MODEL_ANIMATION_ITEM_HINT = BTString("Model Animation");
		static const TString MODEL_BONE_ITEM_HINT = BTString("Model Bone");

		static const TString MODEL_RESOURCE_TYPE = BTString("model resource");
		static const TString MODEL_FILE_EXT = BTString("blm");
		static const TString MODEL_SERIALIZER_BINARY = BTString("binary model");
		static const TString MODEL_SERIALIZER_XML = BTString("text model");

		static const TString MODEL_PARAM_SOFTMODE = BTString("SoftWareMode");	//software buffer instead of hardware video buffer

		static const TString SKELETON_RESOURCE_TYPE = BTString("skeleton resource");
		static const TString SKELETON_FILE_EXT = BTString("bls");
		static const TString SKELETON_SERIALIZER_BINARY = BTString("binary skeleton");
		static const TString SKELETON_SERIALIZER_XML = BTString("text skeleton");

		//paging layer
		static const TString MODEL_PAGE_LAYER = BTString("models");

		//IK configuration
		static const TString IK_NONE = BTString("Model::IK::None");			//disabled
		static const TString IK_FOOT = BTString("Model::IK::Foot");			//foot: simplest, separated fee IK chains
		static const TString IK_HAND = BTString("Model::IK::Hand");			//hand + foot, separated fee/hands IK chains
		static const TString IK_FULLBODY = BTString("Model::IK::FullBody");	//full body, unique chain of skeleton

		//IK effector types
		static const TString EFFECTOR_HAND = BTString("hand");
		static const TString EFFECTOR_FOOT = BTString("foot");
	}

}//namespace Blade

#endif //  __Blade_BladeModel_h__