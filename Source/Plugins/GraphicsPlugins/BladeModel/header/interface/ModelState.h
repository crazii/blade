/********************************************************************
	created:	2014/09/25
	filename: 	ModelState.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelState_h__
#define __Blade_ModelState_h__
#include <BladeModel.h>
#include <parallel/ParaStateDef.h>
#include <utility/StringList.h>

namespace Blade
{
	namespace ModelState
	{

		static const TString ANIMATION_LIST = BTString("ParaSate::Model::AnimationList");
		typedef ParaStateT<TStringList>		ParaAnimationList;

		static const TString CURRENT_ANIMAION = BTString("ParaSate::Model::CurrentAnimation");

	}//namespace ModelState

}//namespace Blade

#endif //__Blade_ModelState_h__