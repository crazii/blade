/********************************************************************
	created:	2012/01/19
	filename: 	MaterialManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/IResourceManager.h>
#include <interface/IEnvironmentManager.h>

#include <interface/IShaderVariableSource.h>

#include "MaterialManager.h"
#include "DefaultMaterial.h"


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	MaterialManager::MaterialManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	MaterialManager::~MaterialManager()
	{

	}

	/************************************************************************/
	/* IMaterialManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			MaterialManager::initialize()
	{
		if( IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvString::WORKING_MODE) == BTString("tool") )
			return;

		const TString path = BTString("media:material");
		HFILEDEVICE hFolder = IResourceManager::getSingleton().loadFolder( path );

		if( hFolder != NULL )
		{
			TStringParam fileList;
			hFolder->findFile(fileList, BTString("*.bmt"), IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);
			for(size_t i = 0; i < fileList.size(); ++i)
			{
				const TString& file = fileList.at(i);
				HSTREAM stream = hFolder->openFile( file );
				HRESOURCE hRes = IResourceManager::getSingleton().loadResourceSync(stream);
				if( hRes != NULL )
				{
					HMATERIAL hMaterial = hRes;
					mMaterialList[hMaterial->getName()] = hMaterial;
				}
				else
					ILog::Error << BTString("load material failed: ") << file << ILog::endLog;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			MaterialManager::shutdown()
	{
		for( MaterialList::iterator i = mMaterialList.begin(); i != mMaterialList.end(); ++i )
		{
			HMATERIAL& mat = i->second;
			DefaultMaterial* material = static_cast<DefaultMaterial*>(mat);
			bool cloned = material->isCloned();

			mat->unload();
			HRESOURCE hRes = mat;
			mat.clear();

			if( cloned )
				continue;
			IResourceManager::getSingleton().unloadResource( hRes );
		}
		mMaterialList.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	Material*		MaterialManager::getMaterial(const TString& name) const
	{
		MaterialList::const_iterator i = mMaterialList.find(name);
		if( i == mMaterialList.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MaterialManager::destroyMaterial(const TString& name)
	{
		return mMaterialList.erase( name ) == 1;
	}

	////////////////////////////////////////////////////////////////////////////
	Material*		MaterialManager::createMaterial(const TString& name)
	{
		Material* mat = mMaterialList[name];
		if( mat != NULL )
		{
			assert(false);
			BLADE_EXCEPT(EXC_REDUPLICATE, BTString("material with a name '") + name + BTString("' already exist."));
		}
		mat = BLADE_NEW DefaultMaterial(name);
		return mat;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	////////////////////////////////////////////////////////////////////////
	bool			MaterialManager::addMaterial(const HMATERIAL& hMaterial)
	{
		if( hMaterial == NULL )
			return false;

		HMATERIAL& slot = mMaterialList[hMaterial->getName()];
		if(slot != NULL )
			return false;

		slot = hMaterial;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	HMATERIAL&				MaterialManager::getOrAddMaterialRef(const TString& name)
	{
		return mMaterialList[name];
	}

}//namespace Blade
