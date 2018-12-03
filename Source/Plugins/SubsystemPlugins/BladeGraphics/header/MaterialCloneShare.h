/********************************************************************
	created:	2012/03/17
	filename: 	MaterialCloneShare.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MaterialCloneShare_h__
#define __Blade_MaterialCloneShare_h__
#include <interface/public/graphics/RenderProperty.h>

namespace Blade
{

	enum EMaterialShare
	{
		MS_DEEP			= 0x00,	//no share
		MS_SHADERS		= 0x01,
		MS_PROPERTY		= 0x02,
		MS_SAMPLER		= 0x04,
		MS_ALL			= MS_SHADERS|MS_PROPERTY|MS_SAMPLER,
	};
	
	enum EMaterialInstanceShare
	{
		MIS_NONE		= 0x00,	//no share
		MIS_COLOR		= 1 << RPT_COLOR,
		MIS_COLORWIRTE	= 1 << RPT_COLORWIRTE,
		MIS_FOG			= 1 << RPT_FOG,
		MIS_ALPHABLEND	= 1 << RPT_ALPHABLEND,
		MIS_DEPTH		= 1 << RPT_DEPTH,
		MIS_STENCIL		= 1 << RPT_STENCIL,
		MIS_SCISSOR		= 1 << RPT_SCISSOR,

		MIS_TEXTURE		= 1 << RPT_COUNT,
		MIS_PROJCAM		= 1 << (RPT_COUNT+1),

		MIS_ALL			= 0xFFFFFFFFl,
	};

}//namespace Blade


#endif //__Blade_MaterialCloneShare_h__