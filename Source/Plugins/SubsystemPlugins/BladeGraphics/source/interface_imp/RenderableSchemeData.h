/********************************************************************
	created:	2017/01/19
	filename: 	RenderableSchemeData.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderableSchemeData_h__
#define __Blade_RenderableSchemeData_h__
#include <BladeGraphics.h>
#include <interface/public/IRenderable.h>
#include <LoopContainers.h>
#include <LoopDataTypes.h>

namespace Blade
{
	class ILight;

	class RenderSchemeData : public Impl::IRenderData, public Allocatable
	{
	public:
		typedef LoopVector<ILight*>	LightList;
		typedef Vector<ILight*> FixedLightList;

		RenderSchemeData()
		{
			mLocalLightCaclulated = false;
			mFixedLocalLightCalculated = false;
		}

		//cache local lights each frame, shared across multiple cameras, to remove redundant calculations
		//i.e. rendering planar reflections
		LoopFlag	mLocalLightCaclulated;
		LightList	mLocalLights;

		//for static objects affected by static lights, those data only update once
		bool		mFixedLocalLightCalculated;
		FixedLightList	mFixedLocalLights;
	};

	namespace Impl
	{
		inline const HRENDERDATA&  ensureRenderData(const IRenderable* renderable)
		{
			if (renderable->getRenderData() == NULL)
			{
				HRENDERDATA data;
				data.constructInstance<RenderSchemeData>();
				renderable->setRenderData(data);
			}
			return renderable->getRenderData();
		}

	}//namespace Impl
	
}//namespace Blade

#endif // __Blade_RenderableSchemeData_h__