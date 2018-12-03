/********************************************************************
	created:	2013/10/23
	filename: 	GraphicsDefs.h
	author:		Crazii
	purpose:	public shared enumeration type for graphics systems
*********************************************************************/
#ifndef __Blade_GraphicsDefs_h__
#define __Blade_GraphicsDefs_h__
#include <utility/Mask.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>

namespace Blade
{
	/** @brief camera projection type */
	typedef enum EProjectionType
	{
		PT_ORTHOGRAPHIC,
		PT_PERSPECTIVE,
	}PROJECTION;

	/** @brief gizmo helper type */
	enum EGizmoType
	{
		GT_MOVE,
		GT_SCALE,
		GT_ROTATE,

		GT_COUNT,
		GT_BEGIN = GT_MOVE,
	};

	/** @brief axis type */
	enum EAxis
	{
		GA_NONE,
		GA_X		= 0x01,
		GA_Y		= 0x02,
		GA_Z		= 0x04,
		GA_NEG_X	= 0x08,
		GA_NEG_Y	= 0x10,
		GA_NEG_Z	= 0x20,

		GA_VIEWMASK	= 0xF00,
		GA_VIEWX	= 0x100,
		GA_VIEWY	= 0x200,
		GA_VIEWZ	= 0x400,
		GA_XY		= GA_X|GA_Y,
		GA_XZ		= GA_X|GA_Z,
		GA_YZ		= GA_Y|GA_Z,
		GA_XYZ		= GA_X|GA_Y|GA_Z,

		GA_VIEWXY	= GA_VIEWX|GA_VIEWY,
		GA_VIEWXZ	= GA_VIEWX|GA_VIEWZ,
		GA_VIEWYZ	= GA_VIEWY|GA_VIEWZ,
		GA_VIEWXYZ	= GA_VIEWX|GA_VIEWY|GA_VIEWZ,
	};

	enum ELightType
	{
		LT_DIRECTIONAL = 0,
		LT_POINT,
		LT_SPOT,
		//TODO: add more here

		LT_COUNT,
	};


#define LARGE_TYPE_FLAGS 0	//up to 64 types

#if (LARGE_TYPE_FLAGS)
	typedef Mask64 AppFlag;
	static const uint64 INVALID_APPFLAG = 0;
	static const uint64 FULL_APPFLAG = uint64(-1);
#else
	typedef Mask AppFlag;
	static const uint32 INVALID_APPFLAG = 0;
	static const uint32 FULL_APPFLAG = uint32(-1);
#endif

	namespace GraphicsConsts
	{
		
		//system type(main  type of all graphics elements)
		static const TString GRAPHICS_SYSTEM_TYPE = BXLang(BLANG_GRAPHICS_SYSTEM_ELEMENT);
		static const TString EMPTY_ELEMENT_TYPE = BXLang(BLANG_GRAPHICS_ELEMENT);

		static const TString CONFIG_SPACE_MASK = BTString("Space Mask");
		static const TString LOCAL_BOUNDING = BTString("Local Bounding");
		static const TString DYNAMIC_TAG = BTString("Dynamic Moving");

	}//namespace GraphicsConsts
	
}//namespace Blade

#endif //  __Blade_GraphicsDefs_h__