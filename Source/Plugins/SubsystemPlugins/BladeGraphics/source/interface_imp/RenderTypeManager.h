/********************************************************************
	created:	2010/06/28
	filename: 	RenderTypeManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderTypeManager_h__
#define __Blade_RenderTypeManager_h__
#include <BladeGraphics.h>
#include <Singleton.h>
#include <utility/BladeContainer.h>
#include <interface/IRenderTypeManager.h>

namespace Blade
{
	class RenderTypeManager : public IRenderTypeManager,public Singleton<RenderTypeManager>
	{
	public:
		RenderTypeManager();
		~RenderTypeManager();

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getRenderTypeCount() const;

		/*
		@describe
		@param
		@return
		*/
		virtual RenderType*		getRenderType(index_t index) const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			addRenderType(RenderType* type);

		/*
		@describe
		@param
		@return
		*/
		virtual RenderType*		getRenderType(const TString& name) const;

		/*
		@describe
		@param
		@return
		*/
		virtual void			removeRenderType(const TString& name);

	protected:

		/** @brief device ready handler */
		void		onDeviceReady(const Event& data);
		/** @brief device close handler */
		void		onDeviceClose(const Event& data);

		typedef Vector<RenderType*>			TypeList;
		typedef TStringMap<RenderType*>		TypeRegistry;

		TypeRegistry		mGraphicsTypeRegistry;
		TypeList			mGraphicsTypes;
	};//class RenderTypeManager
	
}//namespace Blade


#endif //__Blade_GraphicsTypesManager_h__