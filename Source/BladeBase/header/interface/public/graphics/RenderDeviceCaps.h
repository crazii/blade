/********************************************************************
	created:	2012/03/24
	filename: 	RenderDeviceCaps.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderDeviceCaps_h__
#define __Blade_RenderDeviceCaps_h__

namespace Blade
{

	typedef struct SRenderRenderDeviceCaps
	{
	public:

		///max texture can be bound
		size_t		mMaxTextures;

		///max render targets
		size_t		mMaxMRT;

		///max vertex streams / vertex attributes
		size_t		mMaxStreams;

	}RENDERDEV_CAPS;
	

}//namespace Blade


#endif //__Blade_RenderDeviceCaps_h__