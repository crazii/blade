/********************************************************************
	created:	2011/11/19
	filename: 	ConfigSerializer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ConfigSerializer_h__
#define __Blade_ConfigSerializer_h__
#include <BladeFramework.h>
#include <interface/public/file/IFileDevice.h>
#include <utility/Version.h>

namespace Blade
{
	class IXmlNode;
	class IOBuffer;

	class BLADE_FRAMEWORK_API ConfigSerializer : public NonConstructable
	{
	public:
		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		static bool	writeConfigBinary(const IConfig& config, const HSTREAM& hOStream, TStringParam* strings = NULL);

		static bool	writeStringBinary(const TString& str, const HSTREAM& hOStream, TStringParam* strings = NULL);

		/**
		@describe 
		@param 
		@return 
		*/
		static bool	readConfigBinary(IConfig* config, const HSTREAM& hIStream, const TString* strings = NULL, size_t strCount = 0);

		static bool readStringBinary(TString& outStr, const HSTREAM& hIStream, const TString* strings = NULL, size_t strCount = 0);

		/**
		@describe 
		@param 
		@return 
		*/
		static bool	writeConfigXml(const IConfig& config, const HSTREAM& hOStream);

		/**
		@describe 
		@param 
		@return 
		*/
		static bool	readConfigXml(IConfig& config, const HSTREAM& hIStream);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		static bool	writeConfigBinary(const IConfig& config, IOBuffer& obuffer, TStringParam* strings = NULL);

		static bool	writeStringBinary(const TString& str, IOBuffer& obuffer, TStringParam* strings = NULL);

		/**
		@describe 
		@param 
		@return 
		*/
		static bool	readConfigBinary(IConfig* config, IOBuffer& ibuffer, const TString* strings = NULL, size_t strCount = 0);

		static bool readStringBinary(TString& outString, IOBuffer& ibuffer, const TString* strings = NULL, size_t strCount = 0);

		/**
		@describe 
		@param 
		@return 
		*/
		static bool	writeConfigXml(const IConfig& config, IOBuffer& obuffer);

		/**
		@describe 
		@param 
		@return 
		*/
		static bool	readConfigXml(IConfig& config, IOBuffer& ibuffer);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/

		/**
		@describe read the config data from the XML node
		@param 
		@return 
		*/
		static bool	readConfigXml(IConfig& config, IXmlNode& node);

		/**
		@describe write the config data into the XML node
		@param 
		@return 
		*/
		static bool	writeConfigXml(IConfig& config, IXmlNode& node);

	};//class ConfigSerializer

	

}//namespace Blade



#endif // __Blade_ConfigSerializer_h__