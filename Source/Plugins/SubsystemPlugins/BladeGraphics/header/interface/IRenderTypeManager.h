/********************************************************************
	created:	2010/08/21
	filename: 	IRenderTypeManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderTypeManager_h__
#define __Blade_IRenderTypeManager_h__
#include <BladeGraphics.h>
#include <interface/InterfaceSingleton.h>
#include <RenderType.h>

namespace Blade
{
	class IRenderTypeManager : public InterfaceSingleton<IRenderTypeManager>
	{
	public:
		BLADE_GRAPHICS_API static const TString HELPER_RENDER_TYPE;
	public:
		virtual ~IRenderTypeManager()		{}

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getRenderTypeCount() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual RenderType*		getRenderType(index_t index) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			addRenderType(RenderType* type) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual RenderType*		getRenderType(const TString& name) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			removeRenderType(const TString& name) = 0;
		inline void		removeRenderType(const RenderType* type)
		{
			if( type != NULL )
				return this->removeRenderType(type->getName());
		}

	};//class IGraphicsTypeManager

	extern template class BLADE_GRAPHICS_API Factory<IRenderTypeManager>;
	
}//namespace Blade


#endif //__Blade_IGraphicsTypeManager_h__