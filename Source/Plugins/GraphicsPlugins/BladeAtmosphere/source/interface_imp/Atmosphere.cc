/********************************************************************
	created:	2011/09/03
	filename: 	Atmosphere.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Atmosphere.h"
#include "AtmosphereType.h"
#include <ConfigTypes.h>
#include <utility/MemoryStream.h>
#include <interface/public/IScene.h>
#include <databinding/DataSourceWriter.h>
#include <databinding/DataBindingHelper.h>
#include <BladeBase_blang.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>

#include <interface/ILightManager.h>
#include <interface/IRenderTypeManager.h>

#include <interface/IRenderScheme.h>
#include <BladeAtmosphere_blang.h>

namespace Blade
{
	namespace ConfigString
	{
		static const TString	CONFIG_NAME = BXLang(BLANG_ATMOSPHERE_CONFIG);

		static const TString	SKY_TYPE = BXLang(BLANG_TYPE);

		static const TString	SKY_TYPE_SPHERE = BXLang("Sky Sphere");
		static const TString	SKY_TYPE_BOX = BXLang("Sky Box");
	
		static const TString	SKY_RADIUS = BXLang(BLANG_SKY_RADIUS);
		static const TString	SKY_MESH_DENSITY = BXLang(BLANG_SKY_DENISTY);

		static const TString	SKY_TEXTURE = BXLang(BLANG_SKY_TEXTURE);
		static const TString	GLOBAL_AMBIENT = BXLang(BLANG_SKY_AMBIENT);
		static const TString	FIRST_DIFFUSE = BXLang(BLANG_SKY_DIFFUSE);
		static const TString	FIRST_SPECULAR = BXLang(BLANG_SKY_SPECULAR);

		static const TString	SECOND_LIGHT = BXLang(BLANG_SKY_SECONDARYLIGHT);
		static const TString	SECOND_DIFFUSE = BXLang(BLANG_SKY_SECONDDIFFUSE);
		static const TString	SECOND_SPECULAR = BXLang(BLANG_SKY_SECONDSPECULAR);
	
	}//namespace ConfigString
	

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Atmosphere::Atmosphere()
		:mAtmosphere(NULL)
		,mLightElement(NULL)
		,mSecondaryLight(NULL)
		,mInited(false)
	{
		mAmbient = Color::WHITE * 0.8f;
		mDensity = 60u;
		mDiffuse = Color::WHITE;
		mSpecular = Color::GRAY;
		mRadius = 768.0f;
		mSecondDiffuse = Color::WHITE;
		mSecondSpecular = Color::WHITE;
		mSecondLight = false;
		mType = ConfigString::SKY_TYPE_BOX;
		mSkyTexture = TString::EMPTY;

		{
			DataSourceWriter<Atmosphere> writer(AtmosphereConsts::ATMOSPHERE_STAGE_DATA_TYPE);
			writer.beginVersion( Version(0,1,'A') );
			//version 0.1
			{
				TStringParam typeStrings;
				typeStrings.push_back(ConfigString::SKY_TYPE_SPHERE);
				typeStrings.push_back(ConfigString::SKY_TYPE_BOX);
				DataSourceWriter<Atmosphere> typeWriter = (writer << ConfigString::SKY_TYPE << typeStrings << DS_GROUP<Atmosphere>(&Atmosphere::mType, &Atmosphere::notifyConfigChange));

				CONFIG_UIHINT hint(CUIH_IMAGE, AtmosphereConsts::ATMOSPHERE_SKY_TEXTURE_IMAGE);
				typeWriter[ConfigString::SKY_TYPE_SPHERE] << ConfigString::SKY_TEXTURE << hint << &Atmosphere::notifyConfigChange << &Atmosphere::mSkyTexture;
				typeWriter[ConfigString::SKY_TYPE_SPHERE] << ConfigString::SKY_RADIUS << ConfigAtom::Range(128.0f, 2048.0f, 128.0f) << &Atmosphere::notifyConfigChange << &Atmosphere::mRadius;
				typeWriter[ConfigString::SKY_TYPE_SPHERE] << ConfigString::SKY_MESH_DENSITY << ConfigAtom::Range(60u, 250u, 10u) << &Atmosphere::notifyConfigChange << &Atmosphere::mDensity;

				CONFIG_UIHINT hint2(CUIH_IMAGE, AtmosphereConsts::ATMOSPHERE_SKYBOX_TEXTURE_IMAGE);
				typeWriter[ConfigString::SKY_TYPE_BOX] << ConfigString::SKY_TEXTURE << hint2 << &Atmosphere::notifyConfigChange << &Atmosphere::mSkyTexture;
				typeWriter.endVersion();

				writer << ConfigString::GLOBAL_AMBIENT << &Atmosphere::notifyConfigChange << &Atmosphere::mAmbient;
				writer << ConfigString::FIRST_DIFFUSE << &Atmosphere::notifyConfigChange << &Atmosphere::mDiffuse;
				writer << ConfigString::FIRST_SPECULAR << &Atmosphere::notifyConfigChange << &Atmosphere::mSpecular;
				writer << ConfigString::SECOND_LIGHT << &Atmosphere::notifyConfigChange << &Atmosphere::mSecondLight;
				writer << ConfigString::SECOND_DIFFUSE << &Atmosphere::notifyConfigChange << &Atmosphere::mSecondDiffuse;
				writer << ConfigString::SECOND_SPECULAR << &Atmosphere::notifyConfigChange << &Atmosphere::mSecondSpecular;
			}
			writer.endVersion();
		}

		DataBinder binder(AtmosphereConsts::ATMOSPHERE_STAGE_DATA_TYPE, this, true);
		mConfig = binder.getConfig();
	}

	//////////////////////////////////////////////////////////////////////////
	Atmosphere::~Atmosphere()
	{
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&		Atmosphere::getAtmosphereConfig() const
	{
		return mConfig;
	}

	//////////////////////////////////////////////////////////////////////////
	static AtmosphereType lsType;
	IGraphicsType*		Atmosphere::getAtmosphereType() const
	{
		return &lsType;
	}

	//////////////////////////////////////////////////////////////////////////
	void				Atmosphere::initialize(IElement* elem)
	{
		if( mInited )
		{
			assert(false);
			return;
		}
		mInited = true;

		if( elem == NULL || elem->getType() != AtmosphereConsts::ATMOSPHERE_ELEMENT_TYPE )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("element missing or type mismatch."));

		mAtmosphere = static_cast<AtmosphereElement*>(elem);

		if( mLightElement == NULL )
			mLightElement = static_cast<LightElement*>( 
			mAtmosphere->getScene()->createElement( BTString(BLANG_LIGHT_ELEMENT) ) 
			);
		DataBindingHelper::setElementData(mLightElement, BTString(BLANG_TYPE), (uint32)LT_DIRECTIONAL);

		mLightElement->activateContent();
		this->applyConfig();
	}

	//////////////////////////////////////////////////////////////////////////
	void				Atmosphere::shutdown()
	{
		if( !mInited )
			return;
		mInited = false;

		BLADE_DELETE mLightElement;
		mLightElement = NULL;

		BLADE_DELETE mSecondaryLight;
		mSecondaryLight = NULL;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				Atmosphere::registerRenderType()
	{
		IRenderTypeManager::getSingleton().addRenderType( static_cast<AtmosphereType*>(this->getAtmosphereType()) );
	}

	//////////////////////////////////////////////////////////////////////////
	void				Atmosphere::unregisterRenderType()
	{
		IRenderTypeManager::getSingleton().removeRenderType( this->getAtmosphereType()->getName() );
	}


	//////////////////////////////////////////////////////////////////////////
	void				Atmosphere::notifyConfigChange(void* data)
	{
		if (mAtmosphere == NULL || mLightElement == NULL)
			return;

		if (data == &mRadius || data == &mDensity)
		{
			if(mType == ConfigString::SKY_TYPE_SPHERE)
				mAtmosphere->initSkySphere(mRadius, mDensity, mSkyTexture);
		}
		else if (data == &mSkyTexture/* || data == &mType*/)	//note: type change will cause texture reset later. if type changes only, texture may not match (2d/cube)
		{
			if (mType == ConfigString::SKY_TYPE_SPHERE)
				mAtmosphere->initSkySphere(mRadius, mDensity, mSkyTexture);
			else
				mAtmosphere->initSkyBox(mSkyTexture);
		}
		else if( data == &mAmbient )
			mLightElement->getLight()->getLightManager()->setGlobalAmbient(mAmbient);
		else if( data == &mDiffuse )
			mLightElement->setLightDiffuse(mDiffuse);
		else if( data == &mSpecular )
			mLightElement->setLightSpecular(mSpecular);
		else if( data == &mSecondLight)
		{
			if( mSecondLight )
			{
				if( mSecondaryLight == NULL )
				{
					mSecondaryLight = static_cast<LightElement*>(
						mAtmosphere->getScene()->createElement( BTString(BLANG_LIGHT_ELEMENT) )
						);
					mSecondaryLight->setLightType(LT_DIRECTIONAL);
					Vector3 dir(1,-1,1);
					dir.normalize();
					mSecondaryLight->setLightDirection( dir );
					mSecondaryLight->setLightDiffuse( mSecondDiffuse );
					mSecondaryLight->setLightSpecular( mSecondSpecular );
				}
				mSecondaryLight->activateContent();
			}
			else
			{
				mSecondaryLight->deactivateContent();
				BLADE_DELETE mSecondaryLight;
				mSecondaryLight = NULL;
			}
		}
		else if( data == &mSecondDiffuse )
		{
			if( mSecondaryLight != NULL )	//maybe on cancel
				mSecondaryLight->setLightDiffuse(mSecondDiffuse);
		}
		else if( data == &mSecondSpecular )
		{
			if( mSecondaryLight != NULL )	//maybe on cancel
				mSecondaryLight->setLightSpecular(mSecondSpecular);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				Atmosphere::applyConfig()
	{
		assert( mAtmosphere != NULL );
		if (mType == ConfigString::SKY_TYPE_SPHERE)
			mAtmosphere->initSkySphere(mRadius, mDensity, mSkyTexture);
		else
			mAtmosphere->initSkyBox(mSkyTexture);

		mLightElement->setLightDiffuse(mDiffuse);
		mLightElement->setLightSpecular(mSpecular);

		Vector3 dir(-1,-1,-1);
		dir.normalize();
		mLightElement->setLightDirection( dir );
		mLightElement->getLight()->getLightManager()->setGlobalAmbient( mAmbient );

		if( mSecondLight )
		{
			if(mSecondaryLight == NULL )
				mSecondaryLight = static_cast<LightElement*>(
				mAtmosphere->getScene()->createElement( BTString("LightElement") )
				);

			mSecondaryLight->setLightDiffuse( mSecondDiffuse );
			mSecondaryLight->setLightSpecular( mSecondSpecular );

			Vector3 dir2(1,-1,1);
			dir2.normalize();
			mSecondaryLight->setLightDirection( dir2 );
		}
		else
		{
			BLADE_DELETE mSecondaryLight;
			mSecondaryLight = NULL;
		}
	}

}//namespace Blade
