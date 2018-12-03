/********************************************************************
	created:	2011/09/03
	filename: 	AtmosphereType.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AtmosphereType_h__
#define __Blade_AtmosphereType_h__
#include <RenderType.h>

namespace Blade
{

	class AtmosphereType : public RenderType
	{
	public:
		AtmosphereType();
		~AtmosphereType()	{}

		//////////////////////////////////////////////////////////////////////////
		virtual Material*				getMaterial() const;
	};
	

}//namespace Blade



#endif // __Blade_AtmosphereType_h__