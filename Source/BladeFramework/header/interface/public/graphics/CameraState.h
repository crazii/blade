/********************************************************************
	created:	2011/09/10
	filename: 	CameraState.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_CameraState_h__
#define __Blade_CameraState_h__
#include <BladeFramework.h>
#include <parallel/ParaStateDef.h>

namespace Blade
{
	class IGraphicsCamera;

	extern template class BLADE_FRAMEWORK_API ParaStateT<IGraphicsCamera*>;
	typedef ParaStateT<IGraphicsCamera*> ParaCameraData;

	namespace CameraState
	{
		//extern const BLADE_FRAMEWORK_API TString NEW_VIEW;
		//extern const BLADE_FRAMEWORK_API TString VIEW_TO_DELETE;
		extern const BLADE_FRAMEWORK_API TString CAMERA;

		extern const BLADE_FRAMEWORK_API TString FAR_PLANE;
		extern const BLADE_FRAMEWORK_API TString NEAR_PLANE;
		extern const BLADE_FRAMEWORK_API TString FIELD_OF_VIEW;	///@note: horizontal fov, not fovY
		extern const BLADE_FRAMEWORK_API TString ASPECT;
	
	}//namespace CameraState
	
	
	

}//namespace Blade



#endif // __Blade_CameraState_h__