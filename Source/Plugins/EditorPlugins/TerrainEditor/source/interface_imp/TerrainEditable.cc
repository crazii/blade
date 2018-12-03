/********************************************************************
	created:	2011/05/22
	filename: 	TerrainEditable.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainEditable.h"
#include <interface/IResourceManager.h>
#include <interface/IStageConfig.h>
#include <interface/IEditorFramework.h>
#include <interface/ITerrainConfigManager.h>
#include <interface/IWorldEditableManager.h>
#include "TerrainEditorElement.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const TString TerrainEditable::TERRAIN_EDITABLE_NAME = BTString("TerrainEditable");

	//////////////////////////////////////////////////////////////////////////
	TerrainEditable::TerrainEditable()
		:EditableBase(TERRAIN_EDITABLE_NAME, TERRAIN_EDITABLE_NAME)
		,mGeometry(NULL)
		,mNeedNormalUpdate(false)
	{
		mFlag = EDF_SELECTABLE | EDF_PAGED;
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainEditable::TerrainEditable(IEntity* TileEntity, IGeometry* geometry, const TString& resourcePath)
		:EditableBase(TERRAIN_EDITABLE_NAME, TERRAIN_EDITABLE_NAME)
		,mGeometry(geometry)
		,mResourcePath(resourcePath)
		,mNeedNormalUpdate(false)
	{
		assert(TileEntity != NULL);
		mFlag = EDF_SELECTABLE;

		this->createEditorElement( TerrainEditorElement::TERRAIN_EDITOR_ELEMENT, TileEntity);
		this->setModified(true);

		//creation routine
		static_cast<TerrainEditorElement*>(mEditorElement)->initialzeInterface();
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainEditable::~TerrainEditable()
	{

	}

	/************************************************************************/
	/* IEditable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	TerrainEditable::initialize(EditorElement* element)
	{
		//loading routine
		this->setEditorElement(element);
		assert( mGeometry == NULL );
		static_cast<TerrainEditorElement*>(mEditorElement)->initialzeInterface();

		ILogicWorld* pWorld = IWorldEditableManager::getInterface().getWorld();

		IScene* scene = pWorld->getGeometryScene();
		IElement* geom = scene->createElement(GeomConsts::GEOMETRY_ELEMENT_READONLY);
		mGeometry = geom->getInterface(IID_GEOMETRY());
		//don't save geometry in editor
		element->getEntity()->addElement( IEditorFramework::EDITOR_ELEMENT_PREFIX + BTString("geometry"), HELEMENT(geom));

		mResourcePath = this->getTargetEntity()->getElementByType(TerrainConsts::TERRAIN_ELEMENT)->getResourcePath();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainEditable::save()
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		bool ret = true;
		TerrainEditorElement* element = static_cast<TerrainEditorElement*>(mEditorElement);

		if( element->isTerrainModifed() || this->isModified() )
		{
			assert(!mResourcePath.empty());
			ret = IResourceManager::getSingleton().saveResourceSync(mResourcePath, true);
			//assert(ret);
		}
		this->setModified(false);
		return ret;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const Vector3&			TerrainEditable::getPosition() const
	{
		return mGeometry->getGlobalPosition();
	}

	//////////////////////////////////////////////////////////////////////////
	const Vector3&			TerrainEditable::getScale() const
	{
		return mGeometry->getGlobalScale();
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&		TerrainEditable::getRotation() const
	{
		return mGeometry->getGlobalRotation();
	}

	//////////////////////////////////////////////////////////////////////////
	pint16					TerrainEditable::getHeightBuffer() const
	{
		TerrainEditorElement* tee = static_cast<TerrainEditorElement*>(mEditorElement);
		return tee->getTerrainHeightBuffer();
	}
	
	//////////////////////////////////////////////////////////////////////////
	void					TerrainEditable::updateHeightBuffer(size_t x,size_t z,size_t sizeX,size_t sizeZ)
	{
		TerrainEditorElement* tee = static_cast<TerrainEditorElement*>(mEditorElement);
		tee->updateTerrainHeightBuffer(x, z, sizeX, sizeZ);
		mNeedNormalUpdate = true;

		this->setModified(true);
	}

	//////////////////////////////////////////////////////////////////////////
	puint8					TerrainEditable::getNormalBuffer() const
	{
		TerrainEditorElement* tee = static_cast<TerrainEditorElement*>(mEditorElement);
		return tee->getTerrainNormalBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	void					TerrainEditable::updateNormalBuffer()
	{
		if( mNeedNormalUpdate )
		{
			TerrainEditorElement* tee = static_cast<TerrainEditorElement*>(mEditorElement);
			tee->updateTerrainNormal();
			mNeedNormalUpdate = false;

			this->setModified(true);
		}		
	}

	//////////////////////////////////////////////////////////////////////////
	const TStringList&		TerrainEditable::getTextureList() const
	{
		TerrainEditorElement* tee = static_cast<TerrainEditorElement*>(mEditorElement);
		return tee->getTextureList();
	}

	//////////////////////////////////////////////////////////////////////////
	TERRAIN_LAYER*	TerrainEditable::getLayerBuffer() const
	{
		TerrainEditorElement* tee = static_cast<TerrainEditorElement*>(mEditorElement);
		return tee->getLayerBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	uint8*					TerrainEditable::getBlendBuffer() const
	{
		TerrainEditorElement* tee = static_cast<TerrainEditorElement*>(mEditorElement);
		return tee->getBlendBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	void					TerrainEditable::updateBlockBlendBuffer(BLOCK_INDEX blockIndex,index_t StartX,index_t StartZ,
		index_t SizeX,size_t SizeZ,const TString& texture,bool updateTexture)
	{
		TerrainEditorElement* tee = static_cast<TerrainEditorElement*>(mEditorElement);
		tee->updateBlockBlendBuffer(blockIndex,StartX,StartZ,SizeX,SizeZ,texture,updateTexture);
		this->setModified(true);
	}

	//////////////////////////////////////////////////////////////////////////
	ITerrainInterface*	TerrainEditable::getInterface()
	{
		TerrainEditorElement* tee = static_cast<TerrainEditorElement*>(mEditorElement);
		return tee->getTerrainInterface();
	}

}//namespace Blade
