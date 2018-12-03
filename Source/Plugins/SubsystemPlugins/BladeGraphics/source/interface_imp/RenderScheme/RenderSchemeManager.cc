/********************************************************************
	created:	2012/02/16
	filename: 	RenderSchemeManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderSchemeManager.h"
#include "DefaultRenderQueue.h"
#include "RenderScheme.h"
#include "RenderSchemeParser.h"
#include "ImageEffectOutput.h"

#include <interface/public/graphics/IRenderDevice.h>
#include <interface/IResourceManager.h>
#include <interface/IGraphicsSystem.h>
#include <interface/IPlatformManager.h>

#include <interface/IRenderTypeManager.h>
#include <Pass.h>
#include <Technique.h>
#include <Material.h>
#include <ConfigTypes.h>
#include "../RenderSorter.h"
#include "RenderSchemeConfig.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)
#endif

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	RenderSchemeManager::RenderSchemeManager()
	{
		mLastProperty = NULL;
		mLastTextureStates = NULL;
		mLastShaderProgram = NULL;
		mLastShaderInst = NULL;
		mLastSampler = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	RenderSchemeManager::~RenderSchemeManager()
	{
		this->shutdown();
	}

	/************************************************************************/
	/* IRenderSchemeManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			RenderSchemeManager::initialize(bool sharedOutputBuffer)
	{
		const TString path = BTString("media:renderscheme");
		HFILEDEVICE hFolder = IResourceManager::getSingleton().loadFolder( path );

		TStringParam fileList;
		hFolder->findFile(fileList, BTString("*.scheme"), IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);

		SchemeParser parser;
		for(size_t i = 0; i < fileList.size(); ++i )
		{
			HSTREAM stream = hFolder->openFile( fileList.at(i) );
			if( stream != NULL )
			{
				SCHEME_DESC* desc = BLADE_NEW SCHEME_DESC();
				if( parser.parseFile(stream, *desc) )
					mSchemeMap[desc->mName] = desc;
				else
				{
					//delete output
					size_t outputCount = desc->mOutputList.size();
					for(size_t j = 0; j < outputCount; ++j)
					{
						const OUTPUT_DESC* outputDesc = desc->mOutputList[j];

						//delete step
						size_t stepCount = outputDesc->mStepDescList.size();
						for(size_t n = 0; n < stepCount; ++n)
							BLADE_DELETE outputDesc->mStepDescList[n];

						BLADE_DELETE outputDesc;
					}

					BLADE_DELETE desc;
				}
			}
		}

		RenderScheme::setUseSharedBuffer(sharedOutputBuffer);

		RenderSchemeConfig::getSingleton().initConfig();
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderSchemeManager::shutdown()
	{
		for( SchemeInfoMap::iterator iter = mSchemeMap.begin(); iter != mSchemeMap.end(); ++iter)
		{
			SCHEME_DESC* desc = iter->second;

			//delete output
			size_t outputCount = desc->mOutputList.size();
			for(size_t i = 0; i < outputCount; ++i)
			{
				const OUTPUT_DESC* outputDesc = desc->mOutputList[i];

				//delete step
				size_t stepCount = outputDesc->mStepDescList.size();
				for(size_t n = 0; n < stepCount; ++n)
					BLADE_DELETE outputDesc->mStepDescList[n];

				BLADE_DELETE outputDesc;
			}

			BLADE_DELETE desc;
		}
		mSchemeMap.clear();
		RenderScheme::clearSharedBuffers();
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderQueue*	RenderSchemeManager::createRenderQueue()
	{
		DefaultRenderQueue* queue = BLADE_NEW DefaultRenderQueue();
		return queue;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderScheme*	RenderSchemeManager::createRenderScheme(const TString& name) const
	{
		SchemeInfoMap::const_iterator i = mSchemeMap.find( name );
		if( i != mSchemeMap.end() )
		{
			const SCHEME_DESC& desc = *i->second;
			return this->createScheme( desc );
		}
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderSchemeManager::destroyRenderScheme(IRenderScheme* scheme) const
	{
		if( scheme != NULL )
		{
			mActiveSchemes.erase(scheme);
			BLADE_DELETE static_cast<RenderScheme*>(scheme);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			RenderSchemeManager::getSchemeCount() const
	{
		return mSchemeMap.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	RenderSchemeManager::getScheme(index_t index) const
	{
		if( index >= mSchemeMap.size() )
			return TString::EMPTY;

		SchemeInfoMap::const_iterator i = mSchemeMap.begin();
		std::advance(i, index);
		return i->first;
	}

	//////////////////////////////////////////////////////////////////////////
	const TStringList*	RenderSchemeManager::getSchemeProfiles(index_t index) const
	{
		if( index < mSchemeMap.size() )
		{
			SchemeInfoMap::const_iterator i = mSchemeMap.begin();
			std::advance(i, index);
			return &(i->second->mProfileList);
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const TStringList*	RenderSchemeManager::getSchemeProfiles(const TString& schemeName) const
	{
		SchemeInfoMap::const_iterator i = mSchemeMap.find(schemeName);
		if( i != mSchemeMap.end() )
			return &(i->second->mProfileList);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			RenderSchemeManager::getActiveSchemes(TPointerParam<IRenderScheme>& schemeList) const
	{
		schemeList.clear();
		for(SchemeSet::const_iterator i = mActiveSchemes.begin(); i != mActiveSchemes.end(); ++i)
			schemeList.push_back(*i);
		return schemeList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderSchemeManager::applyRenderSettings(IRenderDevice* device, const ShaderOption* shaderOpt, const TextureStateInstance textures, const SamplerState* samplers)
	{
		this->applyGlobalShaderSetting(device, shaderOpt);
		this->applyInstanceShaderSetting(device, shaderOpt);
		this->applyTextureSetting(device, shaderOpt, textures, samplers);
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderSchemeManager::applyRenderProperty(IRenderDevice* device, const RenderPropertySet& prop,FILL_MODE fm/* = GFM_DEFAULT*/)
	{
		// property content may change even for the same property object
		//if( mLastProperty != prop )
		{
			if( fm != GFM_DEFAULT && fm != prop.getFillMode() )
			{
				FILL_MODE oldFM = prop.getFillMode();
				prop.setFillMode(fm);
				device->setRenderProperty( prop );
				prop.setFillMode(oldFM);
			}
			else
				device->setRenderProperty( prop );
			mLastProperty = &prop;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderSchemeManager::applyGlobalShaderSetting(IRenderDevice* device, const ShaderOption* shaderOpt)
	{
		if(shaderOpt == NULL)
		{
			mLastShaderInst = NULL;
			mLastShaderProgram = NULL;
			return;
		}

		const ShaderInstance* shaderInst = shaderOpt->getShaderInstance();
		if( shaderInst != mLastShaderInst)
		{
			mLastShaderInst = shaderInst;

			const HSHADER& program = shaderInst->getShader();
			if( mLastShaderProgram != program)
			{
				mLastShaderProgram = program;
				device->setShader(program);
				shaderInst->updateShaderVariable(SVU_GLOBAL, device);
				shaderInst->updateShaderVariable(SVU_SHADER, device);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderSchemeManager::applyInstanceShaderSetting(IRenderDevice* device, const ShaderOption* shaderOpt)
	{
		const ShaderInstance* shaderInst = shaderOpt != NULL ? shaderOpt->getShaderInstance() : NULL;
		if( shaderInst != NULL )
			shaderInst->updateShaderVariable(SVU_INSTANCE, device);
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderSchemeManager::applyTextureSetting(IRenderDevice* device, const ShaderOption* shaderOpt, const TextureStateInstance textures, const SamplerState* samplers)
	{
		if (shaderOpt == NULL || textures.state == NULL || samplers == NULL)
		{
			mLastTexture = TextureStateInstance();
			mLastSampler = NULL;
			return;
		}

		const ShaderInstance* shaderInst = shaderOpt->getShaderInstance();
		if (mLastTexture != textures || mLastSampler != samplers)
		{
			mLastTexture = textures;
			mLastSampler = samplers;
			shaderInst->updateTexture(device, textures, *samplers);
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IRenderSorter*	RenderSchemeManager::createRenderSorter(const TStringList& sorters)
	{
		IRenderSorter* sorter = NULL;
		size_t sorterCount = sorters.size();
		if( sorterCount == 1 )
		{
			//note: some sorters are registered by plugin, if plugin not installed
			//skip creating
			if( Factory<IRenderSorter>::getSingleton().isClassRegistered(sorters[0]))
				sorter = BLADE_FACTORY_CREATE(IRenderSorter, sorters[0]);
		}
		else if( sorterCount != 0 )
		{
			CombinedRenderSorter* combinedSorter = BLADE_NEW CombinedRenderSorter();
			combinedSorter->reserve(sorterCount);
			for(size_t i = 0; i < sorterCount; ++i)
			{
				if( Factory<IRenderSorter>::getSingleton().isClassRegistered(sorters[i]))
					combinedSorter->push_back( BLADE_FACTORY_CREATE(IRenderSorter, sorters[i]) );
			}
			if( combinedSorter->size() == 0 )
			{
				BLADE_DELETE combinedSorter;
				combinedSorter = NULL;
			}
			sorter = combinedSorter;
		}
		return sorter;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderScheme*	RenderSchemeManager::createScheme(const SCHEME_DESC& desc) const
	{
		RenderScheme* scheme = BLADE_NEW RenderScheme(&desc);
		mActiveSchemes.insert(scheme);
		return scheme;
	}


}//namespace Blade