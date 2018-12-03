/********************************************************************
	created:	2011/12/21
	filename: 	RenderProperty.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/RenderProperty.h>
#include <interface/public/graphics/ITexture.h>

namespace Blade
{
	template class FixedArray<HRENDERPROPERTY,RPT_COUNT>;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IRenderProperty* IRenderProperty::createProperty(RENDER_PROPERTY eProp)
	{
		switch(eProp)
		{
		case RPT_COLOR:
			return BLADE_NEW ColorProperty();
		case RPT_COLORWIRTE:
			return BLADE_NEW ColorWriteProperty();
		case RPT_FOG:
			return BLADE_NEW FogProperty();
		case RPT_ALPHABLEND:
			return BLADE_NEW AlphaBlendProperty();
		case RPT_DEPTH:
			return BLADE_NEW DepthProperty();
		case RPT_STENCIL:
			return BLADE_NEW StencilProperty();
		case RPT_SCISSOR:
			return BLADE_NEW ScissorProperty();
		default:
			break;
		}
		assert(false);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderProperty* IRenderProperty::cloneProperty(IRenderProperty* prop)
	{
		if(prop == NULL)
			return NULL;
		switch(prop->getType())
		{
		case RPT_COLOR:
			return BLADE_NEW ColorProperty( *static_cast<ColorProperty*>(prop) );
		case RPT_COLORWIRTE:
			return BLADE_NEW ColorWriteProperty( *static_cast<ColorWriteProperty*>(prop) );
		case RPT_FOG:
			return BLADE_NEW FogProperty( *static_cast<FogProperty*>(prop) );
		case RPT_ALPHABLEND:
			return BLADE_NEW AlphaBlendProperty( *static_cast<AlphaBlendProperty*>(prop) );
		case RPT_DEPTH:
			return BLADE_NEW DepthProperty( *static_cast<DepthProperty*>(prop) );
		case RPT_STENCIL:
			return BLADE_NEW StencilProperty( *static_cast<StencilProperty*>(prop) );
		case RPT_SCISSOR:
			return BLADE_NEW ScissorProperty( *static_cast<ScissorProperty*>(prop) );
		default:
			break;
		}
		assert(false);
		return NULL;
	}
	
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				RenderPropertySet::addProperty(const HRENDERPROPERTY& hProp)
	{
		if( hProp == NULL || this->hasProperty(hProp->getType()))
			return false;

		RENDER_PROPERTY eRP = hProp->getType();
		if( eRP < RPT_COUNT )
		{
			HRENDERPROPERTY& hTarget = mPropertyList[eRP];
			assert(hTarget == NULL );
			hTarget = hProp;
			mPropertyMask.raiseBitAtIndex(eRP);
			return true;
		}
		else
			return false;

	}

	//////////////////////////////////////////////////////////////////////////
	bool				RenderPropertySet::setProperty(const HRENDERPROPERTY& hProp)
	{
		if( hProp == NULL )
			return false;
		RENDER_PROPERTY eRP = hProp->getType();

		if( eRP < RPT_COUNT )
		{
			mPropertyList[eRP] = hProp;
			mPropertyMask.raiseBitAtIndex(eRP);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				RenderPropertySet::removeProperty(RENDER_PROPERTY eRP)
	{
		if( eRP < RPT_COUNT )
		{
			HRENDERPROPERTY& hProp = mPropertyList[eRP];
			hProp.clear();
			mPropertyMask.clearBitAtIndex(eRP);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void				RenderPropertySet::mergeProperties(const RenderPropertySet& mergeSource)
	{
		for(int i = RPT_BEGIN; i < RPT_COUNT; ++i)
		{
			RENDER_PROPERTY eProp = (RENDER_PROPERTY)i;
			if( mergeSource.hasProperty(eProp) )
				this->setProperty(mergeSource.getProperty(eProp));
		}
		//mCullMode = mergeSource.mCullMode;	//FIXME:
	}

	//////////////////////////////////////////////////////////////////////////
	const HRENDERPROPTYSET&	RenderPropertySet::getDefaultRenderProperty()
	{
		static HRENDERPROPTYSET DEFAULT_PROPERTY;
		if( DEFAULT_PROPERTY == NULL )
		{
			DEFAULT_PROPERTY.lock();
			DEFAULT_PROPERTY.bind( BLADE_NEW RenderPropertySet() );
			Lock::memoryBarrier();
			DEFAULT_PROPERTY.unlock();
		}
		return DEFAULT_PROPERTY;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	const Sampler Sampler::DEFAULT;
	const Sampler Sampler::DEFAULT_RTT(TAM_CLAMP, TAM_CLAMP, TAM_CLAMP, TFM_LINEAR, TFM_LINEAR, TFM_LINEAR, 0.0f, 0.0f);
	const Sampler Sampler::DEFAULT_RTT_DEPTH(TAM_CLAMP, TAM_CLAMP, TAM_CLAMP, TFM_POINT, TFM_POINT, TFM_POINT, 0.0f, 0.0f);


}//namespace Blade
