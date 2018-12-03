/********************************************************************
	created:	2013/02/28
	filename: 	SerializerHelper.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "SerializerHelper.h"
#include <databinding/IDataSourceDataBase.h>
#include <interface/public/IScene.h>
#include <ConfigSerializer.h>
#include <ConfigTypes.h>
#include "Entity.h"

namespace Blade
{
	namespace SerializerHelper
	{
		const TString XML_TYPE = BTString("type");
		const TString XML_NAME = BTString("name");
		const TString XML_ENTITY = BTString("entity");
		const TString XML_ELEMENT = BTString("element");
		const TString ELEMENT_TYPE = XML_TYPE;
		const TString ELEMENT_NAME = XML_NAME;
		const TString ELEMENT_SCENE_TYPE = BTString("scene_type");
		const TString ELEMENT_REOURCE_PATH = BTString("resource");

		//////////////////////////////////////////////////////////////////////////
		IEntity*		loadEntityXML(IStage* stage, IEntity* entity, IXmlNode* entityNode,
			bool checkEntitySkip, bool checkElementSkip)
		{
			const TString& entityName = entity->getName();
			if( checkEntitySkip && SerializerHelper::checkEntitySkipping( entityName ) )
			{
				assert(entity == NULL);

				ILog::Information << BTString("Skip loading entity: \"")
					<< entityName
					<< BTString("\".")
					<<ILog::endLog;
				return NULL;
			}
			else if(false)
			{
				ILog::Information << BTString("Loading entity: \"")
					<< entityName
					<< BTString("\".")
					<<ILog::endLog;
			}

			IXmlNode::IXmlNodeList* elementList = entityNode->getChildList( SerializerHelper::XML_ELEMENT  );
			size_t elementCount = elementList->safeSize();

			for( uint32 n = 0; n < elementCount; ++n )
			{
				IXmlNode* elementNode = elementList->getAt(n);
				const TString* elemType = elementNode->getAttributeValue( SerializerHelper::ELEMENT_TYPE );
				const TString* elemName = elementNode->getAttributeValue( SerializerHelper::ELEMENT_NAME );
				const TString* sceneType = elementNode->getAttributeValue( SerializerHelper::ELEMENT_SCENE_TYPE );
				const TString* resourcePath = elementNode->getAttributeValue( SerializerHelper::ELEMENT_REOURCE_PATH );

				if( elemType == NULL || elemName == NULL || sceneType == NULL )
				{
					BLADE_LOG(Error, BTString("element incomplete information."));
					continue;
				}

				if( checkElementSkip && SerializerHelper::checkElementSkipping(*elemName) )
				{
					BLADE_LOG(Information, 
						BTString("Skip loading element: \"")
						<< *elemName
						<< BTString("\".") );
					continue;
				}
				else if(false)
				{
					BLADE_LOG(Information, BTString("loading element: \"")
						<< *elemName
						<< BTString("\"."));
				}

				IElement* element = entity->getElement( *elemName );
				if( element == NULL )
				{
					IScene* scene = stage->getScene( *sceneType );
					if( scene == NULL )
					{
						BLADE_LOG(Error, BTString("scene with the type \"")
							<< *sceneType + BTString("\" not created in the stage \"")
							<< stage->getName()
							<< BTString("\". skipping element."));
						continue;
					}

					element = scene->createElement( *elemType );
					if( element != NULL )
						entity->addElement(*elemName, HELEMENT(element));	//bind element first to init para data, then load config to load real data
					else
					{
						BLADE_LOG(Warning, BTString("element with the type '") << *elemType << BTString("' not created")
						<< BTString(", this may because the corresponding module is not loaded."));
						continue;
					}
				}

				DataBinder binder(*elemType, element);
				if( binder.isBound() )
				{
					if( !ConfigSerializer::readConfigXml( *binder.getConfig(), *elementNode) )
					{
						BLADE_LOG(Warning, BTString("element with the type \"")
							<< *elemType + BTString("\" loading failed."));
					}						
				}
				else
				{
					BLADE_LOG(Warning, BTString("element with the type \"")
						<< *elemType + BTString("\" has no serializer, element added without initialized."));
				}
				if( resourcePath != NULL )
					element->setResourcePath(*resourcePath);
				else
					element->setResourcePath( TString::EMPTY );
			}//elements

			return entity;
		}

		//////////////////////////////////////////////////////////////////////////
		bool		saveEntityXML(const IEntity* entity, IXmlNode* entityNode, bool checkEntitySkip, bool checkElementSkip)
		{
			const TString& name = entity->getName();
			if( checkEntitySkip && SerializerHelper::checkEntitySkipping(name) )
			{
				BLADE_LOG(Information, TEXT("skip saving entity \"")
					<< name
					<< TEXT("\"."));
				return false;
			}
			else
			{
				BLADE_LOG(Information, TEXT("saving entity \"")
					<< name
					<< TEXT("\"."));
			}

			if( entityNode->getName() != SerializerHelper::XML_ENTITY )
			{
				assert( entityNode->getName() == TString::EMPTY );
				entityNode->setName( SerializerHelper::XML_ENTITY);
			}

			const Entity::ElementMap& elems = static_cast<const Entity*>(entity)->getElements();
			for( Entity::ElementMap::const_iterator n = elems.begin(); n != elems.end(); ++n )
			{
				const TString& elemName = n->first;
				IElement* elem = n->second;
				const TString& elemType = elem->getType();
				const TString& sceneType = elem->getScene()->getSceneType();
				const TString& resourcePath = elem->getResourcePath();

				if( checkElementSkip && SerializerHelper::checkElementSkipping(elemName) )
				{
					BLADE_LOG(Information, TEXT("skip saving element \"")
						<< elemName
						<< TEXT("\"."));

					continue;
				}
				else
				{
					BLADE_LOG(Information, TEXT("saving element \"")
						<< elemName
						<< TEXT("\"."));
				}

				IXmlNode* elementNode = entityNode->addChild( SerializerHelper::XML_ELEMENT );
				elementNode->addAttribute( SerializerHelper::ELEMENT_TYPE, elemType );
				elementNode->addAttribute( SerializerHelper::ELEMENT_NAME, elemName );
				elementNode->addAttribute( SerializerHelper::ELEMENT_SCENE_TYPE, sceneType );
				elementNode->addAttribute( SerializerHelper::ELEMENT_REOURCE_PATH, resourcePath );

				//stage's element serializer has higher priority
				DataBinder binder(elemType, elem);
				if( binder.isBound() )
				{
					//elem->prepareSave(); entity preSave already called
					if( !ConfigSerializer::writeConfigXml(*binder.getConfig(), *elementNode ) )
					{
						BLADE_LOG(Warning, BTString("element with the type \"")
							<< elemType + BTString("\" saving failed ."));
					}
				}
				else
				{
					BLADE_LOG(Warning, BTString("element with the type \"")
						<< elemType + BTString("\" has no serializer, saving element data without actual content."));
				}
			}//for each element
			return true;
		}



		//////////////////////////////////////////////////////////////////////////
		IEntity*	loadEntityBinary(IStage* stage, IEntity* entity, IOBuffer& buffer,
			bool checkEntitySkip, bool checkElementSkip, const TString* strings/* = NULL*/, size_t strCount/* = 0*/)
		{
			uint16 elementCount = 0;
			if (!buffer.read(&elementCount))
			{
				assert(false);
				return NULL;
			}

			uint32 bytes = 0;
			if( !buffer.read(&bytes) )
			{
				assert(false);
				return NULL;
			}

			//read full block, so that no further seeking is needed on error exit.
			char* entityData = NULL;
			buffer.readDataPtr( (const void*&)entityData, (IStream::Size)bytes);

			if (entity == NULL)
				return entity;

			const TString entityName = entity->getName();
			if (checkEntitySkip && SerializerHelper::checkEntitySkipping(entityName))
			{
				BLADE_LOG(Information, BTString("Skip loading entity: \"") << entityName << BTString("\"."));
				return NULL;
			}
			else
				BLADE_LOG(Information, BTString("Loading entity: \"") << entityName << BTString("\"."));


			RefIOBuffer realBuffer(buffer, entityData, bytes);

			HCONFIG elementExtras = SerializerHelper::createElementExtraConfig();
			for( uint32 n = 0; n < elementCount; ++n )
			{
				uint32 elemBytes = 0;

				TString elemType, elemName, sceneType, resourcePath;
				ConfigSerializer::readConfigBinary(elementExtras, realBuffer, strings, strCount);

				elemType = elementExtras->getSubConfigByName(ELEMENT_TYPE)->getBinaryValue();
				elemName = elementExtras->getSubConfigByName(ELEMENT_NAME)->getBinaryValue();
				sceneType = elementExtras->getSubConfigByName(ELEMENT_SCENE_TYPE)->getBinaryValue();
				resourcePath = elementExtras->getSubConfigByName(ELEMENT_REOURCE_PATH)->getBinaryValue();

				if (!realBuffer.read(&elemBytes))
				{
					assert(false);
					return NULL;
				}

				RefIOBuffer elementBuffer(realBuffer, realBuffer.getCurrentData(), elemBytes);
				realBuffer.seekForward(elemBytes);

				if( checkElementSkip && SerializerHelper::checkElementSkipping(elemName) )
				{
					BLADE_LOG(Information, BTString("Skip loading element: \"") << elemName << BTString("\"."));
					continue;
				}
				else
					BLADE_LOG(Information, BTString("loading element: \"") << elemName << BTString("\"."));

				IElement* element = entity->getElement(elemName);

				if( element == NULL )
				{
					IScene* scene = stage->getScene( sceneType );
					if( scene == NULL )
					{
						BLADE_LOG(Warning, BTString("scene with the type \"")
							<< sceneType + BTString("\" not created in the stage \"")
							<< stage->getName()
							<< BTString("\". skipping element."));
						continue;
					}

					element = scene->createElement( elemType );
					if( element != NULL )
					{
						entity->addElement(elemName, HELEMENT(element));	//bind element first to init para data, then load config to load real data
					}
					else
					{
						BLADE_LOG(Warning, BTString("element with the type '") << elemType << BTString("' not created")
							<< BTString(", this may because the corresponding module is not loaded."));
						continue;
					}
				}

				DataBinder binder(elemType, element);
				if( !ConfigSerializer::readConfigBinary(binder.getConfig(), elementBuffer, strings, strCount) )
					BLADE_LOG(Warning, BTString("element with the type \"") << elemType + BTString("\" loading failed."));

				element->setResourcePath(resourcePath);
			}//elements
			return entity;
		}

		//////////////////////////////////////////////////////////////////////////
		size_t	saveEntityBinary(const IEntity* entity, IOBuffer& buffer, bool checkEntitySkip, bool checkElementSkip, TStringParam* strings/* = NULL*/)
		{
			const TString& name = entity->getName();

			if( checkEntitySkip && SerializerHelper::checkEntitySkipping(name) )
			{
				BLADE_LOG(Information, TEXT("skip saving entity \"")
					<< name
					<< TEXT("\"."));
				return 0;
			}
			else
			{
				BLADE_LOG(Information, TEXT("saving entity \"")
					<< name
					<< TEXT("\"."));
			}

			IOBuffer entityBuffer;
			const Entity::ElementMap& elems = static_cast<const Entity*>(entity)->getElements();

			uint16 elementCount = 0;

			for( Entity::ElementMap::const_iterator n = elems.begin(); n != elems.end(); ++n )
			{
				const TString& elemName = n->first;
				IElement* elem = n->second;

				if( checkElementSkip && SerializerHelper::checkElementSkipping(elemName) )
				{
					BLADE_LOG(Information, TEXT("skip saving element \"")
						<< elemName
						<< TEXT("\"."));
					continue;
				}
				else
				{
					BLADE_LOG(Information, TEXT("saving element \"")
						<< elemName
						<< TEXT("\"."));
				}
				const TString& elemType = elem->getType();

				//write builtin extra configs
				HCONFIG elementExtras = SerializerHelper::createElementExtraConfig(elem, elemType, elemName);
				if (!ConfigSerializer::writeConfigBinary(*elementExtras, entityBuffer, strings))
					BLADE_EXCEPT(EXC_FILE_WRITE, BTString("element with the type \"") + elemType + BTString("\" saving failed ."));

				IOBuffer elemBuffer;
				DataBinder binder(elemType, elem);
				ConfigGroup dummy(CAF_NONE);
				IConfig* config = binder.isBound() ? binder.getConfig() : &dummy;
				//elem->prepareSave();	entity preSave already called
				if( !ConfigSerializer::writeConfigBinary(*config, elemBuffer, strings) )
					BLADE_EXCEPT(EXC_FILE_WRITE, BTString("element with the type \"") + elemType + BTString("\" saving failed ."));

				uint32 elemBytes = (uint32)elemBuffer.tell();
				//element bytes & data
				entityBuffer.write(&elemBytes);
				entityBuffer.writeData(elemBuffer.getData(), elemBuffer.tell());

				++elementCount;
			}//for each element

			if (elementCount != 0)
			{
				buffer.write(&elementCount);
				buffer.writeAtom((uint32)entityBuffer.tell());
				buffer.writeData(entityBuffer.getData(), entityBuffer.tell());
				return (uint32)entityBuffer.tell();
			}
			return 0;
		}

		//////////////////////////////////////////////////////////////////////////
		HCONFIG	createElementExtraConfig(IElement* elem/* = NULL*/, const TString& elementType/* = TString::EMPTY*/, const TString& elementName/* = TString::EMPTY*/)
		{
			const TString& sceneType = elem != NULL ? elem->getScene()->getSceneType() : TString::EMPTY;
			const TString& resourcePath = elem != NULL ? elem->getResourcePath() : TString::EMPTY;

			//note: use config object to support dynamically adding/removing items.
			//adding or removing items will not break compatibility for existing resources,

			ConfigGroup* extras = BLADE_NEW ConfigGroup();

			ConfigAtom* sub = BLADE_NEW ConfigAtom(ELEMENT_TYPE);
			sub->setBinaryValue(elementType);
			extras->addSubConfigPtr(sub);

			sub = BLADE_NEW ConfigAtom(ELEMENT_NAME);
			sub->setBinaryValue(elementName);
			extras->addSubConfigPtr(sub);


			sub = BLADE_NEW ConfigAtom(ELEMENT_SCENE_TYPE);
			sub->setBinaryValue(sceneType);
			extras->addSubConfigPtr(sub);

			sub = BLADE_NEW ConfigAtom(ELEMENT_REOURCE_PATH);
			sub->setBinaryValue(resourcePath);
			extras->addSubConfigPtr(sub);
			//TODO: more items

			return HCONFIG(extras);
		}

	}//namespace SerializerHelper
	
}//namespace Blade