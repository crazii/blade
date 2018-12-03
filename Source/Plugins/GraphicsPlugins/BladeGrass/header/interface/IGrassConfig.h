/********************************************************************
	created:	2018/05/27
	filename: 	IGrassConfig.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IGrassConfig_h__
#define __Blade_IGrassConfig_h__
#include <BladeGrass.h>
#include <interface/InterfaceSingleton.h>

namespace Blade
{
	class IGrassConfig : public InterfaceSingleton<IGrassConfig>
	{
	public:
		/** @brief  */
		virtual IGraphicsType* getGraphicsType() const = 0;
	};

	extern template class BLADE_GRASS_API Factory<IGrassConfig>;
	
}//namespace Blade


#endif // __Blade_IGrassConfig_h__