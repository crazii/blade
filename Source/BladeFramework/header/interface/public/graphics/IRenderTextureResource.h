/********************************************************************
	created:	2010/05/21
	filename: 	IRenderTextureResource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderTextureResource_h__
#define __Blade_IRenderTextureResource_h__
#include <interface/public/IResource.h>
#include <interface/graphics/ITexture.h>

namespace Blade
{
	class IRenderTextureResource : public IResource
	{
	public:
		virtual ~IRenderTextureResource()	{}

		/**
		@describe
		@param
		@return
		*/
		const HTEXTURE&		getRenderTexture() const;


	};//class IRenderTextureResource
	
}//namespace Blade


#endif //__Blade_IRenderTextureResource_h__