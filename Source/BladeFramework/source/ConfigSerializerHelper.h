/********************************************************************
	created:	2011/11/26
	filename: 	ConfigSerializerHelper.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ConfigSerializerHelper_h__
#define __Blade_ConfigSerializerHelper_h__
#include <BladePCH.h>
#include <utility/Variant.h>
#include <utility/IOBuffer.h>
#include <utility/XmlFile.h>
#include <utility/StringHelper.h>
#include <utility/CodeConverter.h>
#include <interface/IConfig.h>

namespace Blade
{
	namespace ConfigUtil
	{
		/************************************************************************/
		/* XML utility function(recursive)                                                                     */
		/************************************************************************/
		static const TString XML_NODE_NAME = BTString("config");
		static const TString ATTRIB_VERSION = BTString("version");
		static const TString ATTRIB_NAME = BTString("name");
		static const TString ATTRIB_VALUE = BTString("value");
		//////////////////////////////////////////////////////////////////////////
		static bool loadConfigXMLRecursive(IXmlNode* node, IConfig* config)
		{
			const TString* Val = node->getAttributeValue( ATTRIB_VALUE );
			if( Val != NULL )
				config->setValue(*Val, CAF_SERIALIZE);
			else
				assert(false);

			const IConfig::IList* list;
			const TString* tsVer = node->getAttributeValue( ATTRIB_VERSION );
			if( tsVer == NULL )
			{
				//list = config->getSubConfigs();
				//assert( list == NULL );
				return true;
			}
			else
			{
				list = config->getSubConfigs( Version(*tsVer) );
				//assert(list != NULL);
				if( list == NULL )
					return true;
			}

			IXmlNode::IXmlNodeList* nodes = node->getChildList(XML_NODE_NAME);

			if (config->getDataHandler() != NULL && config->getDataHandler()->getTarget() != NULL && config->getBinaryValue().isCollection())
				config->getDataHandler()->prepareData(nodes->safeSize());
			
			for( size_t i = 0; i <  nodes->safeSize(); ++i)
			{
				IXmlNode* child = nodes->safeAt(i);
				const TString* subname = child->getAttributeValue( ATTRIB_NAME );
				IConfig* subConfig = list->getConfig(*subname);
				if( subConfig != NULL )
					ConfigUtil::loadConfigXMLRecursive(child,subConfig);
			}
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		static void saveConfigXMLRecursive(IXmlNode* node, IConfig* config)
		{
			node->addAttribute( ATTRIB_NAME,config->getName() );
			config->updateData(false, true);
			node->addAttribute( ATTRIB_VALUE,config->getValue() );
			const IConfig::IList* list = config->getSubConfigs();
			const Version* version = config->getLatestVersion();
			size_t count = 0;
			if( list == NULL )
			{
				assert(version == NULL);
				const Variant& var = config->getBinaryValue();
				if( !var.isCollection() )
					return;	
			}
			else
			{
				assert(version != NULL);
				count = list->getCount();
			}
			//always save the latest version
			if( version != NULL )
				node->addAttribute( ATTRIB_VERSION, version->getVersionString() );

			for( size_t i = 0; i < count; ++i )
			{
				IConfig* subConfig = list->getConfig(i);
				if( (subConfig->getAccess()&CAF_SERIALIZE) )
				{
					IXmlNode* subNode = node->addChild( XML_NODE_NAME );
					ConfigUtil::saveConfigXMLRecursive(subNode, subConfig );
				}
			}
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		//////////////////////////////////////////////////////////////////////////
		static bool loadConfigXml(XmlFile& xmlfile, IConfig* config)
		{
			IXmlNode* node = xmlfile.getRootNode();
			if( node == NULL )
				return false;
			const TString* tsVer = node->getAttributeValue( ATTRIB_VERSION );
			if( tsVer == NULL || node->getName() == TString::EMPTY || node->getName() != config->getName() )
				return false;

			Version ver(*tsVer);
			const IConfig::IList* list = config->getSubConfigs(ver);
			if( list == NULL )
			{
				assert(false);
				return false;
			}

			for( size_t i = 0;i < node->getChildCount();++i)
			{
				IXmlNode* child = node->getChild(i);
				const TString* name = child->getAttributeValue( ATTRIB_NAME );
				if( name == NULL )
				{
					assert(false);
					return false;
				}
				IConfig* subconfig = list->getConfig( *name );
				if( subconfig != NULL )
				{
					if( !ConfigUtil::loadConfigXMLRecursive(child,subconfig) )
						return false;
				}
			}
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		static bool saveConfigXml(XmlFile& xmlfile, const IConfig* config)
		{
			if( config->getName() == TString::EMPTY || !(config->getAccess()&CAF_SERIALIZE) )
			{
				assert(false);
				return false;
			}
			IXmlNode* node = IXmlNode::createNode();
			node->setName( config->getName() );
			node->addAttribute( ATTRIB_VALUE, config->getValue() );
			const IConfig::IList* list = config->getSubConfigs();
			const Version* version = config->getLatestVersion();
			if( list == NULL )
			{
				assert( version == NULL );
				return true;
			}
			else
				assert( version != NULL);
			node->addAttribute( ATTRIB_VERSION, version->getVersionString() );

			for( size_t i = 0; i < list->getCount(); ++i)
			{
				IConfig* subConfig = list->getConfig(i);
				if( (subConfig->getAccess()&CAF_SERIALIZE) )
				{
					IXmlNode* subNode = node->addChild( XML_NODE_NAME );
					ConfigUtil::saveConfigXMLRecursive(subNode, subConfig);
				}
			}
			xmlfile.saveXml(node);
			BLADE_DELETE node;
			return true;
		}

		/************************************************************************/
		/* Binary utility function(recursive)                                                                     */
		/************************************************************************/
		static const uint16 CONFIG_SAFETAG_BEGIN = 0xCFBEu;
		static const uint16 CONFIG_SAFETAG_END = 0xCFEDu;
		static const uint16 CONFIG_MAGIC = 0xB1CF;

		//value mask bits
		static const uint16 VALUE_SET = 0x01u;	//value should be written
		static const uint16 VALUE_STR = 0x02u;	//value is string
		static const uint16 VALUE_NULL = 0x04u;	//value is undefined

		//////////////////////////////////////////////////////////////////////////
		static bool loadConfigBinaryRecursive(IOBuffer& iBuffer, IConfig* config, const TString* strings = NULL, size_t strCount = 0)
		{
			//!begin
			uint16 safetag = 0;
			iBuffer.read(&safetag);
			if( safetag != CONFIG_SAFETAG_BEGIN )
			{
				assert(false);
				return false;
			}

			//!value type id
			uint16 id;
			iBuffer.read(&id);
			bool hasVal = (id & VALUE_SET) != 0;
			bool isString = (id & VALUE_STR) != 0;
			bool nullVal = (id & VALUE_NULL) != 0;

			if (hasVal)
			{
				//!version
				Version version = Version::MAX;
				iBuffer.read(&version);
				//!sub count
				uint16 count = 0;
				iBuffer.read(&count);

				//!value
				bool nulValDest = (config == NULL || config->getBinaryValue().getType() == VI_UNDEFINED);
				Variant var = !nulValDest ? config->getBinaryValue() : Variant();
				if (isString)
				{
					TString tstr;
					uint16 index = 0;
					if (strings != NULL)
					{
						iBuffer.read(&index);
						assert(index < strCount);
						tstr = index < strCount ? strings[index] : TString::EMPTY;
						tstr.make_const_temp();
					}
					else
						iBuffer.readTString(tstr);

					if(!nulValDest)
						var = tstr;
				}
				else if(!nullVal)
				{
					const void* source = NULL;
					uint16 bytes16 = 0;
					//verify size
					iBuffer.read(&bytes16);
					iBuffer.readDataPtr(source, (IStream::Size)bytes16);

					if (!nulValDest)
					{
						size_t bytes = config->getBinaryValue().getSize();
						assert(bytes16 == bytes);
						if(bytes16 == bytes)
							std::memcpy(var.getBinaryData(), source, bytes);
					}
				}

				//optionally set value if access is valid (access may change in code after saving data)
				if (!nulValDest && (config->getAccess() & CAF_SERIALIZE))
					config->setBinaryValue(var, CAF_SERIALIZE);

				//!subs
				if (count != 0)
				{
					const IConfig::IList* list = config != NULL ? config->getSubConfigs(version) : NULL;
					for (index_t i = 0; i < count; ++i)
					{
						TString name;
						if (strings != NULL)
						{
							uint16 index = 0;
							iBuffer.read(&index);
							assert(index < strCount);
							name = index < strCount ? strings[index] : TString::EMPTY;
						}
						else
							iBuffer.readTString(name);

						IConfig* subConfig = (list != NULL) ? list->getConfig(name) : (IConfig*)NULL;
						ConfigUtil::loadConfigBinaryRecursive(iBuffer, subConfig, strings, strCount);
					}
				}
			}

			//!end
			iBuffer.read(&safetag);
			if( safetag != CONFIG_SAFETAG_END )
			{
				assert(false);
				return false;
			}
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		static uint32 saveConfigBinaryRecursive(IOBuffer& oBuffer,const IConfig* config, TStringParam* strings)
		{
			assert(config != NULL);

			IStream::Off pos = oBuffer.tell();

			const IConfig::IList* list = config->getSubConfigs();

			//!begin
			oBuffer.write(&CONFIG_SAFETAG_BEGIN);

			//!type id
			size_t type = config->getBinaryValue().getType();

			bool isString = (type == VI_STRING || type == VI_WSTRING);
			bool hasVal = (config->getAccess()&CAF_SERIALIZE) != 0;
			bool nullVal = type == VI_UNDEFINED;
			uint16 id = (isString ? VALUE_STR : 0u) | (hasVal ? VALUE_SET : 0u) | (nullVal ? VALUE_NULL : 0u);
			oBuffer.write(&id);

			if (hasVal)
			{
				uint16 count = 0;
				//!version
				if (list == NULL)
					oBuffer.write(&Version::MIN);
				else
				{
					count = (uint16)list->getCount();
					oBuffer.write(config->getLatestVersion());
				}
				//!sub count
				oBuffer.write(&count);

				//!value
				if (isString)
				{
					if (strings != NULL)
					{
						index_t index = strings->find(config->getValue());
						if (index == INVALID_INDEX)
						{
							index = strings->size();
							strings->push_back(config->getValue());
						}
						oBuffer.writeAtom((uint16)index);
					}
					else
						oBuffer.writeTString(config->getValue());
				}
				else if(!nullVal)
				{
					size_t bytes = config->getBinaryValue().getSize();
					oBuffer.writeAtom((uint16)bytes);
					oBuffer.writeData(config->getBinaryValue().getBinaryData(), (IStream::Size)bytes);
				}

				//!subs
				if(count > 0)
				{
					IOBuffer subBuffer;
					for (index_t i = 0; i < count; ++i)
					{
						IConfig* subConfig = list->getConfig(i);
						if (strings != NULL)
						{
							index_t index = strings->find(subConfig->getName());
							if (index == INVALID_INDEX)
							{
								index = strings->size();
								strings->push_back(subConfig->getName());
							}
							subBuffer.writeAtom((uint16)index);
						}
						else
							subBuffer.writeTString(subConfig->getName());

						ConfigUtil::saveConfigBinaryRecursive(subBuffer, subConfig, strings);
					}
					oBuffer.writeData(subBuffer.getData(), subBuffer.tell());
				}
			}

			//!end
			oBuffer.write(&CONFIG_SAFETAG_END);

			return uint32(oBuffer.tell() - pos);
		}


		//////////////////////////////////////////////////////////////////////////
		static bool	loadConfigBinary(const HSTREAM& stream, IConfig* config, const TString* strings = NULL, size_t strCount = 0)
		{
			IOBuffer buffer(stream);

			uint16 magic = 0;
			if (!buffer.read(&magic))
			{
				assert(false);
				return false;
			}
			if (magic != CONFIG_MAGIC)
			{
				buffer.seekBack();
				assert(false);
				return false;
			}

			uint32 bytes = 0;
			buffer.read(&bytes);

			bool ret = true;
			if (config == NULL)
				buffer.seekForward(bytes);
			else
				ret = ConfigUtil::loadConfigBinaryRecursive(buffer, config, strings, strCount);

			//synchronize position
			stream->seek( IStream::SP_BEGIN, buffer.tell());
			return ret;
		}
		
		//////////////////////////////////////////////////////////////////////////
		static void saveConfigBinary(const HSTREAM& stream,const IConfig* config, TStringParam* strings = NULL)
		{
			IOBuffer buffer;
			ConfigUtil::saveConfigBinaryRecursive(buffer, config, strings);
			uint32 bytes = (uint32)buffer.tell();

			stream->write(&CONFIG_MAGIC);
			stream->write(&bytes);
			stream->writeData(buffer.getData(), buffer.tell());
		}

	}//namespace ConfigUtil

}//namespace Blade



#endif // __Blade_ConfigSerializerHelper_h__