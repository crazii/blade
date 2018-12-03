/********************************************************************
	created:	2013/02/26
	filename: 	EntitySerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EntitySerializer.h"
#include <interface/public/IScene.h>
#include <ConfigSerializer.h>
#include <interface/IResourceManager.h>

#include "Stage.h"
#include "SerializerHelper.h"

namespace Blade
{
	namespace EntitySerializerString
	{
		const TString XML_NAME = BTString("name");
		const TString XML_TYPE = BTString("type");

		const TString XML_ENTITY = BTString("entity");
		const TString XML_ENTITY_NAME = XML_NAME;

		const TString XML_ELEMENT = BTString("element");
		const TString XML_ELEMENT_NAME = XML_NAME;
		const TString XML_ELEMENT_TYPE = XML_TYPE;
		const TString XML_ELEMENT_SCENE_TYPE = BTString("scene_type");

	}//namespace EntitySerializerString

	/************************************************************************/
	/* EntitySerializer                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			EntitySerializer::createResource(IResource* res, ParamList& params)
	{
		assert( res->getType() == IEntity::ENTITY_RESOURCE_TYPE );

		EntityResource* resource = static_cast<EntityResource*>(res);

		IEntity* entity = (IEntity*)(void*)params[BTString("ENTITY")];
		resource->setEntity(entity);
		return entity != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EntitySerializer::cacheEntity(const HSTREAM& stream, const ParamList& params)
	{
		this->createImpl(stream);
		Stage* stage = static_cast<Stage*>( (void*)params[BTString("STAGE")] );
		bool ret = mImpl->cacheEntity(stream, stage);

		mCachedEntity = (IEntity*)(void*)params[BTString("ENTITY")];
		ret = ret && mImpl->loadEntity(mCachedEntity);

		if(ret)
			static_cast<Entity*>(mCachedEntity)->instantLoad();

		assert(ret);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EntitySerializer::postLoadEntity(IResource* resource)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		assert( resource->getType() == IEntity::ENTITY_RESOURCE_TYPE );
		EntityResource* res = static_cast<EntityResource*>(resource);
		assert( res->getEntity() != NULL && res->getEntity() == mCachedEntity );
		BLADE_UNREFERENCED(res);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EntitySerializer::saveEntity(const IResource* resource, const HSTREAM& stream)
	{
		assert( resource->getType() == IEntity::ENTITY_RESOURCE_TYPE );
		const EntityResource* res = static_cast<const EntityResource*>(resource);

		mImpl = EntitySerializerFactory::getSingleton().createInstance( mImplClass );

		const String header = BString("<!--") + BString(" ") + StringConverter::TStringToUTF8String(mImplClass) +  BString(" ") + BString("-->\n");
		stream->write( header.c_str(), header.size() );

		bool ret = mImpl->saveEntity( res->getEntity(), stream);
		const EntitySaveDesc* desc = res->getSaveDesc();

		//save binding resources
		if (desc != NULL)
		{
			const Entity::ElementMap& elements = static_cast<Entity*>(res->getEntity())->getElements();
			for (Entity::ElementMap::const_iterator i = elements.begin(); i != elements.end(); ++i)
			{
				const HELEMENT& element = i->second;
				if (desc->saveAll || desc->elements->find(element->getType()) != INVALID_INDEX)
				{
					if (element->getBoundResource() != NULL)
					{
						assert(element->getResourcePath() != TString::EMPTY);
						IResourceManager::getSingleton().saveResourceSync(element->getBoundResource(), true);
					}

				}
			}
		}
		return ret;
	}

	/************************************************************************/
	/* EntitySerializerImpl_XML                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			EntitySerializerImpl_XML::loadEntity(IEntity* entity)
	{
		assert(entity != NULL);

		//////////////////////////////////////////////////////////////////////////
		IXmlNode* entityNode = mFile.getRootNode();
		assert( entityNode != NULL );
		entity = SerializerHelper::loadEntityXML(mStage, entity, entityNode, false, true );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EntitySerializerImpl_XML::saveEntity(const IEntity* entity,const HSTREAM& stream)
	{
		assert(entity != NULL);

		mFile.open(stream);

		IXmlNode* entityNode = IXmlNode::createNode();
		if( SerializerHelper::saveEntityXML(entity, entityNode, false, true) )
			mFile.saveXml(entityNode);

		BLADE_DELETE entityNode;
		return true;
	}

	/************************************************************************/
	/* EntitySerializerImpl_Binary                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			EntitySerializerImpl_Binary::loadEntity(IEntity* entity)
	{
		assert(entity != NULL);
		SerializerHelper::loadEntityBinary(mStage, entity, mBuffer, false, true);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			EntitySerializerImpl_Binary::saveEntity(const IEntity* entity,const HSTREAM& stream)
	{
		assert(entity != NULL);
		const Entity* entityImpl = static_cast<const Entity*>(entity);
		entityImpl->preSave();

		mBuffer.clear();
		if( SerializerHelper::saveEntityBinary(entity, mBuffer, false, true) > 0 )
			mBuffer.saveToStream(stream);
		mBuffer.clear();

		entityImpl->postSave();
		return true;
	}
}//namespace Blade