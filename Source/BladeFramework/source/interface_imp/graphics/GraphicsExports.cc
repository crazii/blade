/********************************************************************
	created:	2013/11/26
	filename: 	GraphicsExports.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/GraphicsState.h>
#include <interface/public/graphics/CameraState.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/graphics/IGraphicsSpaceCoordinator.h>
#include <interface/public/graphics/GraphicsEffect.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>

namespace Blade
{
	template class ParaStateT<IGraphicsCamera*>;

	namespace GraphicsState
	{

		const TString SHOW_AABB = BTString("SHOW_AABB");
		const TString AABB_COLOR = BTString("AABB_COLOR");		

	}//namespace GraphicsState


	namespace CameraState
	{

		const TString CAMERA = BTString("CAMERA_OBJECT");

		const TString FAR_PLANE = BTString("FAR_PLANE");
		const TString NEAR_PLANE = BTString("NEAR_PLANE");
		const TString FIELD_OF_VIEW = BTString("FIELD_OF_VIEW");//horizontal FOV
		const TString ASPECT = BTString("ASPECT_RATIO");

	}//namespace CameraState
	
	 //graphics interfaces definition
	const TString RenderDeviceReadyEvent::NAME = BTString("Render::DeviceReady");
	const TString RenderDeviceCloseEvent::NAME = BTString("Render::DeviceClose");

}//namespace Blade