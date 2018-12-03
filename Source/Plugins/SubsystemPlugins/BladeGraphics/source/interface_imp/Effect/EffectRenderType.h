/********************************************************************
	created:	2011/06/02
	filename: 	EffectRenderType.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EffectRenderType_h__
#define __Blade_EffectRenderType_h__

#include <RenderType.h>
#include <BladeGraphics_blang.h>

namespace Blade
{
	class EffectRenderType : public RenderType
	{
	public:
		EffectRenderType()	:RenderType(BXLang(BLANG_EFFECT)){}
		~EffectRenderType()	{}

		/*
		@describe
		@param
		@return
		*/
		virtual Material*				getMaterial() const
		{
			return NULL;
		}
	};//class EffectRenderType
	

}//namespace Blade



#endif // __Blade_EffectRenderType_h__