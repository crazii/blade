/********************************************************************
	created:	2009/02/07
	filename: 	BladePCH.h
	author:		Crazii
	
	purpose:	foundation-layer precompiled header
*********************************************************************/
#ifndef __Blade_BladePCH_h__
#define __Blade_BladePCH_h__

//platform dependency & types
#include <BladePlatform.h>
#include <BladeTypes.h>

//memory interfaces
#include <memory/BladeMemory.h>

//memory STL compatible allocators
#include <memory/SizeAllocator.h>
#include <memory/TempAllocator.h>
#include <memory/StaticAllocator.h>

//blade containers
#include <utility/FixedArray.h>
#include <utility/TList.h>

//Strings
#include <utility/String.h>
#include <utility/StringList.h>
#include <utility/StringHelper.h>
#include <utility/StringStream.h>

//basic
#include <Lock.h>
#include <Handle.h>
#include <StaticHandle.h>	//unique handle
#include <BladeException.h>

//patterns
#include <Singleton.h>
#include <Factory.h>
#include <interface/InterfaceSingleton.h>

//template functors
#include <utility/Functors.h>

//version
#include <utility/Version.h>

//mask
#include <utility/Mask.h>

//common utilities
#include <utility/Variant.h>
#include <utility/CodeConverter.h>

//buffer utility
#include <utility/Buffer.h>
#include <utility/IOBuffer.h>

//string extension/converter
#include <utility/StringHelperEx.h>

//IO
#include <interface/public/file/IStream.h>
#include <interface/public/file/IFileDevice.h>

//time
#include <interface/public/time/Time.h>
#include <interface/public/time/ITimeDevice.h>

//log
#include <interface/ILog.h>

//multi-language
#include <interface/ILangTable.h>

//file utility
#include <utility/TextFile.h>
#include <utility/ConfigFile.h>
#include <utility/XmlFile.h>

//math
#include <math/BladeSIMD.h>
#include <math/Half.h>
#include <math/BladeMath.h>
#include <math/Vector2i.h>
#include <math/Vector3i.h>
#include <math/Box2i.h>
#include <math/Box3i.h>

#include <math/BladeSIMD.h>
#include <math/Vector2.h>
#include <math/Box2.h>
#include <math/Line2.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Quaternion.h>
#include <math/DualQuaternion.h>
#include <math/Matrix33.h>
#include <math/Matrix44.h>
#include <math/AxisAlignedBox.h>
#include <math/Ray.h>
#include <math/Plane.h>
#include <math/Sphere.h>

//graphics
#include <interface/public/graphics/Color.h>
#include <interface/public/graphics/PixelFormat.h>
#include <interface/public/graphics/IImage.h>
#include <interface/public/graphics/IImageManager.h>

//////////////////////////////////////////////////////////////////////////
//containers (SHOULD NOT BE USED IN PUBLIC HEADERS)
//////////////////////////////////////////////////////////////////////////
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4820)
#endif
#include <map>
#include <set>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <utility/BladeContainer.h>
//std misc
#include <limits>
#include <algorithm>
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif //__Blade_BladePCH_h__