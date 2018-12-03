/********************************************************************
	created:	2010/06/28
	filename: 	TerrainType.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainType.h"
#include <interface/public/IRenderable.h>
#include <interface/IMaterialManager.h>
#include <BladeTerrain_blang.h>
#include "TerrainConfigManager.h"
#include <Material.h>
#include <Technique.h>

namespace Blade
{
	
	//////////////////////////////////////////////////////////////////////////
	TerrainType::TerrainType()
		:RenderType( BTString(BLANG_TERRAIN) )
		,mMaterial(NULL)
	{

	}


	//////////////////////////////////////////////////////////////////////////
	TerrainType::~TerrainType()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Material*			TerrainType::getMaterial() const
	{
		if( mMaterial == NULL )
			mMaterial = IMaterialManager::getSingleton().getMaterial( BTString("terrain") );
		return mMaterial;
	}

	//////////////////////////////////////////////////////////////////////////
	Material*				TerrainType::updateProfile(const TString& profile, bool& updated)
	{
		Material* mat = this->TerrainType::getMaterial();
		updated = false;

		Technique* tech = mat->getActiveTechnique();

		if( profile == BTString("2_0") || 
			(profile != BTString("3_0") &&
				TerrainConfigManager::getSingleton().getTextureQuality() == TerrainConsts::TEXTURE_QUALITY_LOW ) )
		{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4640)
#endif
			static Technique* legacy = mat->getTechnique(BTString("legacy"));

			if( tech != legacy )
			{
				mat->setActiveTechnique(legacy);
				updated = true;
			}
			return mat;
		}

		index_t matching = mat->getTechniqueByProfile(profile);
		if( matching != INVALID_INDEX )
		{
			Technique* newTech = mat->getTechnique(matching);
			if( tech == NULL || newTech->getProfile() > tech->getProfile() )
			{
				mat->setActiveTechnique(matching);
				updated = true;
			}
		}
		return mat;
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainType::processRenderQueue(IRenderQueue* queue)
	{
		TerrainConfigManager::getSingleton().getBatchCombiner()->processQueue(queue);
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainType::onRenderDeviceReady()
	{
		RenderType::onRenderDeviceReady();
		//BLADE_LW_PROFILING( VETEX_DECL_INIT );

		mVertexDecl = IGraphicsResourceManager::getSingleton().createVertexDeclaration();

		//h pos
		mVertexDecl->addElement(TSI_POSITION_XZ,0, TERRAIN_POSITION_XZ_FORMAT, VU_POSITION,0);
		//v pos
		mVertexDecl->addElement(TSI_POSITION_Y,0, TERRAIN_POSITION_Y_FORMAT, VU_POSITION,1);

		//normal
		mVertexDecl->addElement(TSI_NORMAL, 0, VET_UBYTE4N, VU_NORMAL,0);
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainType::onRenderDeviceClose()
	{
		RenderType::onRenderDeviceClose();
		mVertexDecl.clear();
	}

}//namespace Blade