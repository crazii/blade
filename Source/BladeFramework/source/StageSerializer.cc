/********************************************************************
	created:	2011/04/23
	filename: 	StageSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/MemoryStream.h>
#include <utility/Profiling.h>
#include <interface/IResourceManager.h>
#include <interface/public/IScene.h>
#include <interface/public/IElement.h>
#include <databinding/IDataSourceDataBase.h>
#include <ConfigSerializer.h>
#include <ConfigTypes.h>

#include "interface_imp/SerializerHelper.h"
#include "interface_imp/Entity.h"
#include "StageSerializer.h"
#include <utility/StringTable.h>
#include <utility/Profiling.h>


namespace Blade
{
	namespace StageSerializerString
	{
		const TString MAGIC = BTString("stage");
		const TString MAGIC_PAGE = BTString("page");
		const TString TYPE = BTString("type");
		const TString NAME = BTString("name");
		const TString STATIC_DATA = BTString("static_data");
		const TString SCENE_DATA = BTString("scene");
		const TString SCENE_TYPE = TYPE;
		const TString CONFIG_DATA = BTString("config_data");
	}

	static const scalar percent[] = 
	{
		0.1f/3.0f,	//static
		0.1f/3.0f,	//scene
		0.1f/3.0f,	//stage config object
		0.9f,		//entity
	};

	//////////////////////////////////////////////////////////////////////////
	//stage serializer
	//////////////////////////////////////////////////////////////////////////
#if ASYNC_PAGE_PROCESS
	//////////////////////////////////////////////////////////////////////////
	void			StageSerializer::postLoadEntities(IResource* res)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		BLADE_LW_PROFILING_FUNCTION();

		assert(res->getType() == STAGE_RESOURCE_TYPE);
		StageResource* resource = static_cast<StageResource*>(res);

		assert(mType == SST_PAGE);
		assert(mImpl != NULL);

		Page* page = static_cast<Page*>(resource->getPage());
		assert(page != NULL);
		for (size_t i = 0; i < mImpl->mLoadingEntities.size(); ++i)
		{
			mImpl->mLoadingEntities[i]->postLoad(true);
			page->addEntity(mImpl->mLoadingEntities[i], true);
		}
	}
#endif


	/************************************************************************/
	/* ISerializer interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	StageSerializerImpl_Binary::reprocessResource(IResource* resource)
	{
		HSTREAM hStream(BLADE_NEW MemoryStream(mBuffer));
		return this->loadStage(resource, hStream, *mNotifier);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	StageSerializerImpl_Binary::loadStage(IResource* res, const HSTREAM& stream, const ProgressNotifier& notifier)
	{
		assert( res->getType() == STAGE_RESOURCE_TYPE);
		StageResource* resource = static_cast<StageResource*>(res);
		IStage* stage = resource->getStage();
		Page* page = static_cast<Page*>(resource->getPage());
		if( stage == NULL )
		{
			assert(false);
			return false;
		}

		StringTable table;
		StringTable::readStrings(stream, table, Memory::getTemporaryPool());

		size_t stringCount = table.size();
		const TString* stringTable = stringCount > 0 ? &table[0] : &TString::EMPTY;

		IOBuffer buffer(stream);

		TString magic;
		buffer.readTString(magic);
		const TString& MAGIC = (mType == SST_STAGE) ? StageSerializerString::MAGIC : StageSerializerString::MAGIC_PAGE;
		if(MAGIC != magic )
		{
			assert(false);
			return false;
		}

		if (mType == SST_STAGE)
		{
			//always read stage's config
			DataBinder binder(STAGE_BINDING_CONFIG, stage);
			if (!binder.isBound() || !ConfigSerializer::readConfigBinary(binder.getConfig(), buffer, stringTable, stringCount))
				BLADE_EXCEPT(EXC_FILE_READ, BTString("failed loading stage data"));

			stage->instantProcess();

			BLADE_LOG(DebugOutput, TEXT("Loading stage file: \"")
				<< stream->getName()
				<< TEXT("\", stage title is:")
				<< stage->getTitle());
		}

		size_t LOADING_PHASE = size_t(-1);

		//////////////////////////////////////////////////////////////////////////
		//load stage static data
		++LOADING_PHASE;
		if (mType == SST_STAGE)
		{
			notifier.beginStep(percent[LOADING_PHASE]);
			BLADE_LW_PROFILING(STATIC_DATA);

			uint32 count = 0;
			buffer.read(&count);
			for( size_t i = 0; i < count; ++i)
			{
				TString dataType = TString::EMPTY;
				if (!ConfigSerializer::readStringBinary(dataType, buffer, stringTable, stringCount))
					BLADE_EXCEPT(EXC_FILE_READ, BTString("error reading stage file: static data."));

				const SerializableMap& map = IStageConfig::getSingleton().getSerializableMap();
				ISerializable* serializable = map.getSerializable(dataType);
				DataBinder binder(dataType, serializable);
				//always read data to make stream pointer correct
				if( !ConfigSerializer::readConfigBinary(binder.getConfig(), buffer, stringTable, stringCount) )
					BLADE_EXCEPT(EXC_FILE_READ, BTString("failed loading stage static data:") + dataType);

				notifier.onStep(i, count);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//load scenes
		++LOADING_PHASE;
		if (mType == SST_STAGE)
		{
			notifier.beginStep(percent[LOADING_PHASE]);
			BLADE_LW_PROFILING(SCENE);

			uint16 count = 0;
			buffer.read(&count);
			for(uint16 i = 0; i < count; ++i)
			{
				//try reading data type
				TString type = TString::EMPTY;
				if (!ConfigSerializer::readStringBinary(type, buffer, stringTable, stringCount))
					BLADE_EXCEPT(EXC_FILE_READ, BTString("error reading stage file: scene."));

				IScene* scene = NULL;
				if( !SerializerHelper::checkSceneSkipping(type) )
				{
					//scenes may be attached to stage already
					scene = stage->getScene(type);
					if (scene == NULL && SceneFactory::getSingleton().isClassRegistered(type) )
					{
						scene = BLADE_FACTORY_CREATE(IScene, type);
						stage->addScene(scene);
					}
				}

				//scene maybe NULL if plugin support this type is not installed
				DataBinder binder(type, scene);
				//always read data to make stream pointer correct
				if (!ConfigSerializer::readConfigBinary(binder.getConfig(), buffer, stringTable, stringCount))
					BLADE_EXCEPT(EXC_FILE_READ, BTString("failed loading scene:") + type);

				if(scene != NULL)
					scene->postProcess();
				notifier.onStep(i, count);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//load per stage serializables
		++LOADING_PHASE;
		if (mType == SST_STAGE)
		{
			notifier.beginStep(percent[LOADING_PHASE]);
			BLADE_LW_PROFILING(CONFIG_OBJECT);

			uint32 count = 0;
			buffer.read(&count);
			for(uint32 i = 0; i < count; ++i)
			{
				TString type = TString::EMPTY;
				if (!ConfigSerializer::readStringBinary(type, buffer, stringTable, stringCount))
					BLADE_EXCEPT(EXC_FILE_READ, BTString("error reading stage file: stage configs."));
				
				ISerializable* serializable = stage->getSerializableMap().getSerializable(type);
				bool create = (serializable == NULL);
				if(create)
					serializable = BLADE_FACTORY_CREATE(ISerializable, type);

				DataBinder binder(type, serializable);
				//always read data to make stream pointer correct
				if( !ConfigSerializer::readConfigBinary(binder.getConfig(), buffer, stringTable, stringCount) )
					BLADE_EXCEPT(EXC_FILE_READ, BTString("failed loading stage configurable:") + type);

				serializable->postProcess();

				if(create)
					stage->getSerializableMap().addSerializable(binder.getDataSource()->getName(), serializable);

				notifier.onStep(i, count);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//load entities
		BLADE_LW_PROFILING(ENTITY);
		uint16 entityCount = 0;
		if( !buffer.read(&entityCount) )
		{
			assert(false);
			return false;
		}

		++LOADING_PHASE;
		notifier.beginStep( (mType == SST_STAGE) ? percent[LOADING_PHASE] : 1.0f );

		if (mType == SST_PAGE)
			mLoadingEntities.reserve(entityCount);

		for( uint16 i = 0; i < entityCount; ++i )
		{
			Entity* entity = NULL;

			//load entity internal config
			DataBinder binder(ENTITY_BNDING_CONFIG, NULL);
			if (!ConfigSerializer::readConfigBinary(binder.getConfig(), buffer, stringTable, stringCount))
				BLADE_EXCEPT(EXC_FILE_WRITE, BTString("entity with the name \"") + entity->getName() + BTString("\" saving failed ."));

			const TString& name = binder.getConfig()->getSubConfigByName(ENTITY_NAME)->getBinaryValue();

			uint16 pages = 0;
			if (!SerializerHelper::checkEntitySkipping(name) || !(mType == SST_PAGE && page->getLoadStatus() != PS_LOADING) )
			{
				pages = binder.getConfig()->getSubConfigByName(ENTITY_PAGES)->getBinaryValue();
				if (mType == SST_PAGE && pages > 1 )
					entity = static_cast<Entity*>(stage->getEntity(name) );

				if(entity == NULL)
					entity = static_cast<Entity*>(stage->createEntity(name));
			}
			else
				BLADE_LOG(Information, BTString("Skip loading entity: \"") << name << BTString("\"."));

			if (entity != NULL)
				binder.getConfig()->synchronize(entity);

			//even if the entity is skipped, we still need continue to move the buffer pointer forward
			if(pages == 0)
			{
				notifier.beginStep(0.2f / entityCount);
				SerializerHelper::loadEntityBinary(stage, entity, buffer, false, true, stringTable, stringCount);
				notifier.onNotify(1.0);
			}
		
			if (entity != NULL)
			{
				entity->enablePaging(pages);

				bool doPostLoad = false;

				if (mType == SST_PAGE)
				{
					mLoadingEntities.push_back(entity);
#if !ASYNC_PAGE_PROCESS
					doPostLoad = true;
#endif
				}

				if (doPostLoad)
				{
					notifier.beginStep(0.2f / entityCount);
					entity->postLoad(notifier, true);
				}
				else
					notifier.advance(0.2f / entityCount);

				notifier.beginStep(0.6f / entityCount);
				if (mType == SST_PAGE)
				{
#if !ASYNC_PAGE_PROCESS
					page->addEntity(entity, true);
#endif
					notifier.onNotify(1.0);
				}
				else
					stage->loadEntitySync(entity, NULL, notifier);
			}
			else
				notifier.advance(0.8f / entityCount);

		}//entities

		if (mType == SST_STAGE)
			stage->postProcess();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	StageSerializerImpl_Binary::saveStage(const IResource* res, const HSTREAM& stream)
	{
		assert( res->getType() == STAGE_RESOURCE_TYPE);

		const StageResource* resource = static_cast<const StageResource*>(res);
		IStage* istage = resource->getStage();
		if( istage == NULL )
		{
			assert(false);
			return false;
		}

		TStringParam stringTable;

		Stage* stage = static_cast<Stage*>(istage);
		Page* page = static_cast<Page*>(resource->getPage());
		const EntitySet& entities = (mType == SST_STAGE) ? stage->getEntities() : page->getEntities();

		IOBuffer buffer;
		//magic
		const TString& magic = (mType == SST_STAGE) ? StageSerializerString::MAGIC : StageSerializerString::MAGIC_PAGE;
		buffer.writeTString(magic);
		if (mType == SST_STAGE)
		{
			DataBinder binder(STAGE_BINDING_CONFIG, stage);

			if (!binder.isBound() || !ConfigSerializer::writeConfigBinary(*binder.getConfig(), buffer, &stringTable))
				BLADE_EXCEPT(EXC_FILE_WRITE, BTString("failed saving stage data"));
		}

		//////////////////////////////////////////////////////////////////////////
		//write stage static data
		if (mType == SST_STAGE)
		{
			const SerializableMap& map = IStageConfig::getSingleton().getSerializableMap();
			uint32 count = (uint32)map.getCount();
			buffer.write(&count);
			for( size_t i = 0; i < count; ++i)
			{
				TString name;
				ISerializable* serializable = NULL;
				bool ret = map.getSerializable(i, name, serializable);
				assert(ret); BLADE_UNREFERENCED(ret);

				ConfigSerializer::writeStringBinary(name, buffer, &stringTable);

				DataBinder binder(name, serializable);
				ConfigGroup dummy(name, CAF_NONE);
				IConfig* config = binder.isBound() ? binder.getConfig() : &dummy;

				if (!ConfigSerializer::writeConfigBinary(*config, buffer, &stringTable))
					BLADE_EXCEPT(EXC_FILE_WRITE, BTString("failed saving stage static data:") + name);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//write scenes
		if (mType == SST_STAGE)
		{
			uint16 sceneCount = (uint16)stage->getSceneCount();
			IOBuffer sceneBuffer;
			for( size_t i = 0; i < sceneCount; ++i)
			{
				IScene* scene = stage->getScene(i);
				const TString& sceneType = scene->getSceneType();
				if (SerializerHelper::checkSceneSkipping(sceneType))
				{
					--sceneCount;
					continue;
				}

				ConfigSerializer::writeStringBinary(sceneType, sceneBuffer, &stringTable);

				DataBinder binder(sceneType, scene);			
				scene->prepareSave();
				//scenes may not have configs, but should be save in stage - always write scene type:
				ConfigGroup dummy(sceneType, CAF_NONE);
				IConfig* config = binder.isBound() ? binder.getConfig() : &dummy;

				if (!ConfigSerializer::writeConfigBinary(*config, sceneBuffer, &stringTable))
					BLADE_EXCEPT(EXC_FILE_WRITE, BTString("failed saving scene:") + sceneType);
			}
			buffer.write(&sceneCount);
			buffer.writeData(sceneBuffer.getData(), sceneBuffer.tell());
		}

		//////////////////////////////////////////////////////////////////////////
		//write stage serializables
		if (mType == SST_STAGE)
		{
			const SerializableMap& map = stage->getSerializableMap();
			uint32 count = (uint32)map.getCount();
			buffer.write(&count);

			for(uint32 i = 0; i < count; ++i)
			{
				TString type;
				ISerializable* serializable = NULL;
				bool ret = map.getSerializable(i, type, serializable);
				assert(ret);
				BLADE_UNREFERENCED(ret);

				ConfigSerializer::writeStringBinary(type, buffer, &stringTable);

				DataBinder binder(type, serializable);

				ConfigGroup dummy(type, CAF_NONE);
				IConfig* config = binder.isBound() ? binder.getConfig() : &dummy;

				serializable->prepareSave();
				if (!ConfigSerializer::writeConfigBinary(*config, buffer, &stringTable))
					BLADE_EXCEPT(EXC_FILE_WRITE, BTString("failed saving stage configurable:") + type);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//write entities
		{
			uint16 entityCount = (uint16)entities.size();
			IOBuffer entityBuffer;

			for (EntitySet::const_iterator i = entities.begin(); i != entities.end(); ++i)
			{
				Entity* entity = static_cast<Entity*>(*i);

				if (entity->isPagingEnabled() && mType != SST_PAGE)
				{
					--entityCount;
					continue;
				}

				entity->preSave();

				//write entity internal config
				DataBinder binder(ENTITY_BNDING_CONFIG, static_cast<Entity*>(entity));
				if (!binder.isBound() || !ConfigSerializer::writeConfigBinary(*binder.getConfig(), entityBuffer, &stringTable))
					BLADE_EXCEPT(EXC_FILE_WRITE, BTString("entity with the name \"") + entity->getName() + BTString("\" saving failed ."));

				size_t bytes = SerializerHelper::saveEntityBinary(entity, entityBuffer, true, true, &stringTable);
				if (bytes == 0)
					--entityCount;

				entity->postSave();
			}//for each entity

			buffer.write(&entityCount);
			buffer.writeData(entityBuffer.getData(), entityBuffer.tell());
		}

		StringTable::writeStrings(stream, stringTable);

		buffer.saveToStream(stream);
		stream->truncate(stream->tell());
		return true;
	}

	/************************************************************************/
	/* IStageSerializerImpl interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	StageSerializerImpl_XML::reprocessResource(IResource* resource)
	{
		HSTREAM hStream(BLADE_NEW MemoryStream(mBuffer));
		return this->loadStage(resource, hStream, *mNotifier);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	StageSerializerImpl_XML::loadStage(IResource* res,const HSTREAM& stream, const ProgressNotifier& notifier)
	{
		assert( res->getType() == STAGE_RESOURCE_TYPE);

		StageResource* resource = static_cast<StageResource*>(res);
		IStage* stage = resource->getStage();
		Page* page = static_cast<Page*>(resource->getPage());
		if( stage == NULL )
		{
			assert(false);
			return false;
		}

		XmlFile file;
		if( !file.open( stream ) )
		{
			assert(false);
			return false;
		}

		IXmlNode* root = file.getRootNode();
		const TString& MAGIC = (mType == SST_STAGE) ? StageSerializerString::MAGIC : StageSerializerString::MAGIC_PAGE;
		if(MAGIC != root->getName())
		{
			assert(false);
			return false;
		}

		//tittle
		if (mType == SST_STAGE)
		{
			DataBinder binder(STAGE_BINDING_CONFIG, stage);
			if (binder.isBound())
			{
				if (!ConfigSerializer::readConfigXml(*binder.getConfig(), *root))
					BLADE_LOG(Error, BTString("failed loading stage data"));
			}
			stage->instantProcess();

			BLADE_LOG(DebugOutput, TEXT("Loading stage file: \"")
				<< stream->getName()
				<< TEXT("\", stage title is:")
				<< stage->getTitle());
		}

		size_t LOADING_PHASE = size_t(-1);
		//////////////////////////////////////////////////////////////////////////
		//load stage static data
		++LOADING_PHASE;
		if (mType == SST_STAGE)
		{
			notifier.beginStep(percent[LOADING_PHASE]);
			BLADE_LW_PROFILING(STATIC_DATA);

			IXmlNode::IXmlNodeList* staticData = root->getChildList( StageSerializerString::STATIC_DATA );
			size_t count = staticData->safeSize();
			for(size_t i = 0; i < count; ++i)
			{
				notifier.onStep(i, count);

				IXmlNode* staticDataNode = staticData->getAt(i);

				const TString* dataType = staticDataNode->getAttributeValue( StageSerializerString::NAME);
				if( dataType == NULL )
				{
					assert(false);
					BLADE_LOG(Error, BTString("stage static data type missing."));
					continue;
				}

				const SerializableMap& map = IStageConfig::getSingleton().getSerializableMap();
				ISerializable* serializable = map.getSerializable(*dataType);
				DataBinder binder(*dataType, serializable);
				if( binder.isBound() )
				{
					if( !ConfigSerializer::readConfigXml( *binder.getConfig(), *staticDataNode) )
						BLADE_LOG(Error, BTString("failed loading stage static data:") << *dataType);
				}
				else
					BLADE_LOG(Error, BTString("stage data type not found:") << *dataType);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//load scenes
		++LOADING_PHASE;
		if (mType == SST_STAGE)
		{
			notifier.beginStep(percent[LOADING_PHASE]);
			BLADE_LW_PROFILING(SCENE);

			IXmlNode::IXmlNodeList* sceneData = root->getChildList( StageSerializerString::SCENE_DATA );
			size_t count = sceneData->safeSize();
			for(size_t i = 0; i < count; ++i)
			{
				notifier.onStep(i, count);

				IXmlNode* sceneNode = sceneData->getAt(i);
				const TString* type = sceneNode->getAttributeValue( StageSerializerString::SCENE_TYPE );
				if( type == NULL )
				{
					assert(false);
					BLADE_LOG(Error, BTString("scene type info missing."));
					continue;
				}

				if( SerializerHelper::checkSceneSkipping(*type) )
					continue;

				//scenes may be attached to stage already
				IScene* scene = scene = stage->getScene(*type);
				if( scene == NULL)
				{
					scene = BLADE_FACTORY_CREATE(IScene, *type);
					stage->addScene(scene);
				}

				DataBinder binder(*type, scene);
				if( binder.isBound() )
				{
					if( !ConfigSerializer::readConfigXml(*binder.getConfig(), *sceneNode) )
						BLADE_LOG(Error, BTString("failed loading scene:") << *type);
					scene->postProcess();
				}
				else
					BLADE_LOG(Warning, BTString("scene config not found.") << *type);
			}

		}

		//////////////////////////////////////////////////////////////////////////
		//load stage serializables
		++LOADING_PHASE;
		if (mType == SST_STAGE)
		{
			notifier.beginStep(percent[LOADING_PHASE]);
			BLADE_LW_PROFILING(CONFIG_OBJECT);

			IXmlNode::IXmlNodeList* serializingData = root->getChildList( StageSerializerString::CONFIG_DATA );
			size_t count = serializingData->safeSize();
			for(size_t i = 0; i < count; ++i)
			{
				notifier.onStep(i, count);

				IXmlNode* serializingNode = serializingData->getAt(i);
				const TString* type = serializingNode->getAttributeValue( StageSerializerString::NAME);
				if( type == NULL )
				{
					assert(false);
					BLADE_LOG(Error, BTString("configurable type missing."));
					continue;
				}

				ISerializable* serializable = stage->getSerializableMap().getSerializable(*type);
				bool create = serializable == NULL;
				if(create)
					serializable = BLADE_FACTORY_CREATE(ISerializable, *type);

				DataBinder binder(*type, serializable);
				if( binder.isBound() )
				{
					if( !ConfigSerializer::readConfigXml(*binder.getConfig(), *serializingNode) )
						BLADE_LOG(Error, BTString("failed loading stage configurable:") << *type);
					serializable->postProcess();
				}
				else
					BLADE_LOG(Error, BTString("config not found for configurable:") << *type);

				if(create)
					stage->getSerializableMap().addSerializable(*type, serializable);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		++LOADING_PHASE;
		notifier.beginStep((mType == SST_STAGE) ? percent[LOADING_PHASE] : 1.0f);
		BLADE_LW_PROFILING(ENTITY);

		IXmlNode::IXmlNodeList* entities = root->getChildList(ENTITY_BNDING_CONFIG);
		size_t entityCount = entities->safeSize();
		if (mType == SST_PAGE)
			mLoadingEntities.reserve(entityCount);
		if (entityCount == 0)
			notifier.onNotify(1.0f);
		else
		{
			for (uint32 i = 0; i < entityCount; ++i)
			{
				//BLADE_LW_PROFILING(ONE_ENTITY);

				IXmlNode* entityNode = entities->getAt(i);
				const TString* entityName = entityNode->getAttributeValue(ENTITY_NAME);
				assert(entityName != NULL);
				if (SerializerHelper::checkEntitySkipping(*entityName) || (mType == SST_PAGE && page->getLoadStatus() != PS_LOADING))
				{
					notifier.advance(1.0f / entityCount);
					continue;
				}

				Entity* entity = NULL;
				uint16 pages = 0;	//pages that this entity cross
				const TString* strPages = entityNode->getAttributeValue(ENTITY_PAGES);
				if (strPages != NULL)
					pages = (uint16)TStringHelper::toUInt(*strPages);
				//entity located on multiple pages, it may already exist
				if (pages > 1)
					entity = static_cast<Entity*>(stage->getEntity(*entityName));

				if (entity == NULL)
				{
					entity = static_cast<Entity*>(stage->createEntity(*entityName));

					notifier.beginStep(0.2f / entityCount);
					SerializerHelper::loadEntityXML(stage, entity, entityNode, false, true);
					notifier.onNotify(1.0f);

					entity->enablePaging(pages);

					bool doPostLoad = false;
					if (mType == SST_PAGE)
					{
						mLoadingEntities.push_back(entity);
#if !ASYNC_PAGE_PROCESS
						doPostLoad = true;
#endif
					}

					if (doPostLoad)
					{
						notifier.beginStep(0.2f / entityCount);
						entity->postLoad(notifier, true);
					}
					else
						notifier.advance(0.2f / entityCount);
				}
				else
				{
					notifier.advance(0.4f / entityCount);
					assert(entity->isPagingEnabled());
				}

				notifier.beginStep(0.6f / entityCount);

				if (mType == SST_PAGE)
				{
#if !ASYNC_PAGE_PROCESS
					page->addEntity(entity, true);
#endif
					notifier.onNotify(1.0f);
				}
				else
					stage->loadEntitySync(entity, NULL, notifier);
			}//entities

		}//if (entityCount == 0)

		if(mType == SST_STAGE)
			stage->postProcess();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	StageSerializerImpl_XML::saveStage(const IResource* res, const HSTREAM& stream)
	{
		assert( res->getType() == STAGE_RESOURCE_TYPE);

		const StageResource* resource = static_cast<const StageResource*>(res);
		IStage* istage = resource->getStage();
		if(  istage == NULL )
		{
			assert(false);
			return false;
		}

		Stage* stage = static_cast<Stage*>(istage);
		Page* page = static_cast<Page*>(resource->getPage());
		const EntitySet& entities = (mType == SST_STAGE) ? stage->getEntities() : page->getEntities();
		XmlFile file;
		//we need to disable writing BOM because there's a header string before this content
		file.setDestEncoding( TE_UTF8, false );
		file.open( stream );

		IXmlNode* root = IXmlNode::createNode();

		//magic & tittle
		const TString& magic = (mType == SST_STAGE) ? StageSerializerString::MAGIC : StageSerializerString::MAGIC_PAGE;
		root->setName(magic);
		if (mType == SST_STAGE)
		{
			DataBinder binder(STAGE_BINDING_CONFIG, stage);
			if (binder.isBound())
			{
				if (!ConfigSerializer::writeConfigXml(*binder.getConfig(), *root))
					BLADE_LOG(Error, BTString("failed saving stage data."));
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//write stage static data
		if (mType == SST_STAGE)
		{
			const SerializableMap& map = IStageConfig::getSingleton().getSerializableMap();
			uint32 count = (uint32)map.getCount();
			for(size_t i = 0; i < count; ++i)
			{
				TString name;
				ISerializable* serializable = NULL;
				bool ret = map.getSerializable(i, name, serializable);
				assert(ret);
				BLADE_UNREFERENCED(ret);
				DataBinder binder(name, serializable);
				IXmlNode* staticNode = root->addChild( StageSerializerString::STATIC_DATA );

				if( binder.isBound() )
				{
					if( !ConfigSerializer::writeConfigXml(*binder.getConfig(), *staticNode) )
						BLADE_LOG(Error, BTString("failed saving stage static data:") << name);
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//write scene config
		if (mType == SST_STAGE)
		{
			uint32 sceneCount = (uint32)stage->getSceneCount();
			for( size_t i = 0; i < sceneCount; ++i)
			{
				IScene* scene = stage->getScene(i);
				const TString& sceneType = scene->getSceneType();

				if( SerializerHelper::checkSceneSkipping(sceneType) )
					continue;

				IXmlNode* sceneNode = root->addChild( StageSerializerString::SCENE_DATA );
				sceneNode->addAttribute( StageSerializerString::SCENE_TYPE, sceneType);

				DataBinder binder(sceneType, scene );
				IOBuffer databuffer;
				//filter out none-exist scenes
				if( binder.isBound() )
				{
					scene->prepareSave();
					if( !ConfigSerializer::writeConfigXml(*binder.getConfig(), *sceneNode) )
						BLADE_LOG(Error, BTString("failed saving scene:") << sceneType);
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//write stage serializables
		if (mType == SST_STAGE)
		{
			const SerializableMap& map = stage->getSerializableMap();
			size_t count = map.getCount();
			for(size_t i = 0; i < count; ++i)
			{
				TString type;
				ISerializable* serializable = NULL;
				bool ret = map.getSerializable(i, type, serializable);
				assert(ret);
				BLADE_UNREFERENCED(ret);
				IXmlNode* configNode = root->addChild( StageSerializerString::CONFIG_DATA );
				DataBinder binder(type, serializable);
				if( binder.isBound() )
				{
					serializable->prepareSave();
					if( !ConfigSerializer::writeConfigXml(*binder.getConfig(), *configNode) )
						BLADE_LOG(Error, BTString("failed saving stage configurable:") << type);
				}
				else
					BLADE_LOG(Error, BTString("no config data found for configurable:") << type);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//entity
		for(EntitySet::const_iterator i = entities.begin(); i != entities.end(); ++i )
		{
			const Entity* entity = static_cast<const Entity*>(*i);

			if(entity->isPagingEnabled() && mType != SST_PAGE)
				continue;

			const TString& name = entity->getName();
			IXmlNode* entityNode = root->addChild(ENTITY_BNDING_CONFIG);

			entity->preSave();
			entityNode->addAttribute(ENTITY_NAME, name);
			entityNode->addAttribute(ENTITY_PAGES, TStringHelper::fromUInt(entity->getMaxPages()));

			if( !SerializerHelper::saveEntityXML(entity, entityNode, true, true) )
				root->removeChild(entityNode);

			entity->postSave();
		}//for each entity

		file.saveXml(root);
		BLADE_DELETE root;
		return true;
	}

}//namespace Blade