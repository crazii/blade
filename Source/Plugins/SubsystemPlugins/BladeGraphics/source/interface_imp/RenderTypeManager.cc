/********************************************************************
	created:	2010/06/28
	filename: 	RenderTypeManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderTypeManager.h"

#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <interface/IEventManager.h>

#include <BladeGraphics_blang.h>
#include <Material.h>
#include "Material/MaterialManager.h"

namespace Blade
{
	const TString IRenderTypeManager::HELPER_RENDER_TYPE = BTString(BLANG_RENDER_HELPER);

	//////////////////////////////////////////////////////////////////////////
	RenderTypeManager::RenderTypeManager()
	{
		IEventManager::getSingleton().addEventHandler( RenderDeviceReadyEvent::NAME, EventDelegate(this, &RenderTypeManager::onDeviceReady) );
		IEventManager::getSingleton().addEventHandler( RenderDeviceCloseEvent::NAME, EventDelegate(this, &RenderTypeManager::onDeviceClose) );	
	}

	//////////////////////////////////////////////////////////////////////////
	RenderTypeManager::~RenderTypeManager()
	{
		IEventManager::getSingleton().removeEventHandlers(RenderDeviceReadyEvent::NAME, this);
		IEventManager::getSingleton().removeEventHandlers(RenderDeviceCloseEvent::NAME, this);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			RenderTypeManager::getRenderTypeCount() const
	{
		return mGraphicsTypes.size();
	}

	//////////////////////////////////////////////////////////////////////////
	RenderType*		RenderTypeManager::getRenderType(index_t index) const
	{
		if( index < mGraphicsTypes.size() )
			return mGraphicsTypes[index];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderTypeManager::addRenderType(RenderType* type)
	{
		if( type == NULL )
			return false;

		RenderType*& empty = mGraphicsTypeRegistry[type->getName()];
		if( empty == NULL )
		{
			empty = type;
			type->setTypeID((uint32)mGraphicsTypes.size());
			mGraphicsTypes.push_back(type);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	RenderType*	RenderTypeManager::getRenderType(const TString& name) const
	{
		TypeRegistry::const_iterator i = mGraphicsTypeRegistry.find(name);
		if( i == mGraphicsTypeRegistry.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderTypeManager::removeRenderType(const TString& name)
	{
		TypeRegistry::iterator i = mGraphicsTypeRegistry.find(name);
		if( i == mGraphicsTypeRegistry.end() )
			return;
		mGraphicsTypes.erase( std::find(mGraphicsTypes.begin(),mGraphicsTypes.end(),i->second) );
		mGraphicsTypeRegistry.erase(i);
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderTypeManager::onDeviceReady(const Event& /*data*/)
	{
		//pre-load all materials on device ready
		for(size_t i = 0; i < mGraphicsTypes.size(); ++i)
			mGraphicsTypes[i]->onRenderDeviceReady();
	}

	//////////////////////////////////////////////////////////////////////////
	void		RenderTypeManager::onDeviceClose(const Event& /*data*/)
	{
		//un-load all materials on device close
		for(size_t i = 0; i < mGraphicsTypes.size(); ++i)
			mGraphicsTypes[i]->onRenderDeviceClose();
	}

}//namespace Blade