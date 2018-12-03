/********************************************************************
	created:	2011/11/19
	filename: 	ConfigSerializer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ConfigSerializer.h>
#include "ConfigSerializerHelper.h"
#include <utility/MemoryStream.h>
#include <ConfigTypes.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::writeConfigBinary(const IConfig& config, const HSTREAM& hOStream, TStringParam* strings/* = NULL*/)
	{
		if( hOStream == NULL || !(hOStream->getAccesMode()&IStream::AM_WRITE) )
			return false;
		ConfigUtil::saveConfigBinary(hOStream, &config, strings);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::writeStringBinary(const TString& str, const HSTREAM& hOStream, TStringParam* strings/* = NULL*/)
	{
		if (strings != NULL)
		{
			index_t index = strings->find(str);
			if (index == INVALID_INDEX)
			{
				index = strings->size();
				strings->push_back(str);
			}
			return hOStream->writeAtom((uint16)index) == sizeof(uint16);
		}
		else
			return hOStream->write(&str) > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::readConfigBinary(IConfig* config, const HSTREAM& hIStream, const TString* strings/* = NULL*/, size_t strCount/* = 0*/)
	{
		if( hIStream == NULL || !(hIStream->getAccesMode()&IStream::AM_READ) )
			return false;
		return ConfigUtil::loadConfigBinary(hIStream, config, strings, strCount);
	}

	//////////////////////////////////////////////////////////////////////////
	bool ConfigSerializer::readStringBinary(TString& outString, const HSTREAM& hIStream, const TString* strings/* = NULL*/, size_t strCount/* = 0*/)
	{
		if (hIStream == NULL || !(hIStream->getAccesMode()&IStream::AM_READ))
			return false;

		if (strings != NULL)
		{
			uint16 index = 0;
			bool ret = hIStream->read(&index) == sizeof(index);
			if (ret)
			{
				assert(index < strCount);
				outString = index < strCount ? strings[index] : TString::EMPTY;
				outString.make_const_temp();
			}
			return ret;
		}
		else
			return hIStream->read(&outString) > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::writeConfigXml(const IConfig& config, const HSTREAM& hOStream)
	{
		XmlFile xmlfile;
		if( hOStream == NULL || !(hOStream->getAccesMode()&IStream::AM_WRITE) || !xmlfile.open(hOStream) )
		{
			return false;
		}

		ConfigUtil::saveConfigXml(xmlfile, &config);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::readConfigXml(IConfig& config, const HSTREAM& hIStream)
	{
		XmlFile xmlfile;
		if( hIStream == NULL || !(hIStream->getAccesMode()&IStream::AM_READ) || !xmlfile.open(hIStream) )
		{
			return false;
		}
		return ConfigUtil::loadConfigXml(xmlfile, &config);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::writeConfigBinary(const IConfig& config, IOBuffer& obuffer, TStringParam* strings/* = NULL*/)
	{
		HSTREAM hStream(BLADE_NEW MemoryStream(obuffer, IStream::AM_READWRITE));
		bool ret = ConfigSerializer::writeConfigBinary(config, hStream, strings);
		obuffer.seek(IStream::SP_BEGIN, hStream->tell());
		return ret;
	}

	bool	ConfigSerializer::writeStringBinary(const TString& str, IOBuffer& obuffer, TStringParam* strings/* = NULL*/)
	{
		HSTREAM hStream(BLADE_NEW MemoryStream(obuffer, IStream::AM_READWRITE));
		bool ret = ConfigSerializer::writeStringBinary(str, hStream, strings);
		obuffer.seek(IStream::SP_BEGIN, hStream->tell());
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::readConfigBinary(IConfig* config, IOBuffer& ibuffer, const TString* strings/* = NULL*/, size_t strCount/* = 0*/)
	{
		HSTREAM hStream( BLADE_NEW MemoryStream(ibuffer, IStream::AM_READ) );
		bool ret = ConfigSerializer::readConfigBinary(config, hStream, strings, strCount);
		ibuffer.seek(IStream::SP_BEGIN, hStream->tell());
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::readStringBinary(TString& outString, IOBuffer& ibuffer, const TString* strings/* = NULL*/, size_t strCount/* = 0*/)
	{
		HSTREAM hStream(BLADE_NEW MemoryStream(ibuffer, IStream::AM_READ));
		bool ret = ConfigSerializer::readStringBinary(outString, hStream, strings, strCount);
		ibuffer.seek(IStream::SP_BEGIN, hStream->tell());
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::writeConfigXml(const IConfig& config, IOBuffer& obuffer)
	{
		HSTREAM hStream( BLADE_NEW MemoryStream(obuffer,IStream::AM_READWRITE) );
		return ConfigSerializer::writeConfigXml(config,hStream);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::readConfigXml(IConfig& config, IOBuffer& ibuffer)
	{
		HSTREAM hStream( BLADE_NEW MemoryStream(ibuffer,IStream::AM_READ) );
		return ConfigSerializer::readConfigXml(config,hStream);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::readConfigXml(IConfig& config, IXmlNode& node)
	{
		ConfigUtil::loadConfigXMLRecursive(&node,&config);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigSerializer::writeConfigXml(IConfig& config, IXmlNode& node)
	{
		ConfigUtil::saveConfigXMLRecursive(&node,&config);
		return true;
	}

}//namespace Blade
