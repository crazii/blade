/********************************************************************
	created:	2011/08/30
	filename: 	LightElement.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Element/LightElement.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>
#include <interface/IRenderScene.h>
#include "../interface_imp/Light.h"

namespace Blade
{
	const TString LightElement::LIGHT_TYPE = BXLang(BLANG_LIGHT_ELEMENT);

	//////////////////////////////////////////////////////////////////////////
	LightElement::LightElement()
		:GraphicsElement(LIGHT_TYPE)
		,mLight(NULL)
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	LightElement::~LightElement()
	{
		BLADE_DELETE mLight;
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	LightElement::postProcess(const ProgressNotifier& notifier)
	{
		GraphicsElement::postProcess(notifier);
	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				LightElement::onParallelStateChange(const IParaState& data)
	{
		//TODO : add data handling code
		//diffuse, specular
		GraphicsElement::onParallelStateChange(data);
	}

	/************************************************************************/
	/* GraphicsElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::showBoundingVolume(bool show, const Color& color/* = Color::WHITE*/)
	{
		if( mLight == NULL )
			return false;
		static_cast<Light*>(mLight)->enableVolumeHelper(show, color);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void				LightElement::onInitialize()
	{
		ILightManager* lightManager = this->getRenderScene()->getLightManager();

		Light* light = BLADE_NEW Light(lightManager);
		mLight = light;
		mContent = light;
		light->setElement(this);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	ILight*				LightElement::getLight() const
	{
		return mLight;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::setLightType(ELightType type)
	{
		if( mLight == NULL )
			return false;
		bool ret = static_cast<Light*>(mLight)->setType(type);
		//update bounding data
		if (ret)
			mBounding = static_cast<Light*>(mLight)->getLocalAABB();
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::setLightPosition(const Vector3& pos)
	{
		if( mLight == NULL )
			return false;
		static_cast<Light*>(mLight)->setPosition(pos);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::setLightDirection(const Vector3& dir)
	{
		if( mLight == NULL )
			return false;
		static_cast<Light*>(mLight)->setDirection(dir);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::setLightRange(scalar range)
	{
		if( mLight == NULL )
			return false;

		static_cast<Light*>(mLight)->setRange(range);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::setSpotLightInnerAngle(scalar inner)
	{
		if (mLight == NULL)
			return false;
		static_cast<Light*>(mLight)->setSpotLightInnerAngle(inner);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::setSpotLightOuterAngle(scalar outer)
	{
		if (mLight == NULL)
			return false;
		static_cast<Light*>(mLight)->setSpotLightOuterAngle(outer);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::setLightAttenuation(const scalar& attenuation)
	{
		if( mLight == NULL )
			return false;
		static_cast<Light*>(mLight)->setAttenuation(attenuation);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::setLightDiffuse(const Color& diff)
	{
		if( mLight == NULL )
			return false;
		static_cast<Light*>(mLight)->setDiffuse(diff);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightElement::setLightSpecular(const Color& spec)
	{
		if( mLight == NULL )
			return false;
		static_cast<Light*>(mLight)->setSpecular(spec);
		return true;
	}

}//namespace Blade
