/********************************************************************
	created:	2013/02/28
	filename: 	SerializerHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_SerializerHelper_h__
#define __Blade_SerializerHelper_h__
#include <utility/StringHelper.h>
#include <utility/XmlFile.h>

#include <interface/IStageConfig.h>
#include <interface/IEntity.h>
#include <interface/IStage.h>


namespace Blade
{
	namespace SerializerHelper
	{

		/** @brief  */
		inline bool	checkEntitySkipping(const TString& entityName)
		{
			//TODO: check per stage skipping
			const TString& skipPrefix = IStageConfig::getSingleton().getEntitySkippingPrefix();
			return TStringHelper::isStartWith( entityName, skipPrefix );
		}

		/** @brief  */
		inline bool	checkElementSkipping(const TString& elementName)
		{
			//TODO: check per stage skipping
			const TString& skipPrefix = IStageConfig::getSingleton().getElementSkippingPrefix();
			return TStringHelper::isStartWith( elementName, skipPrefix );
		}

		/** @brief  */
		inline bool checkSceneSkipping(const TString& sceneType)
		{
			const TString& skipPrefix = IStageConfig::getSingleton().getSceneTypeSkippingPrefix();
			return TStringHelper::isStartWith( sceneType, skipPrefix );
		}

		/** @brief  */
		IEntity*	loadEntityXML(IStage* stage, IEntity* entity, IXmlNode* entityNode, 
			bool checkEntitySkip, bool checkElementSkip);
		/** @brief  */
		bool		saveEntityXML(const IEntity* entity, IXmlNode* entityNode, bool checkEntitySkip, bool checkElementSkip);

		/** @brief  */
		IEntity*	loadEntityBinary(IStage* stage, IEntity* entity, IOBuffer& buffer,
			bool checkEntitySkip, bool checkElementSkip, const TString* strings = NULL, size_t strCount = 0);

		/** @brief  */
		size_t		saveEntityBinary(const IEntity* entity, IOBuffer& buffer, bool checkEntitySkip, bool checkElementSkip, TStringParam* strings = NULL);

		/** @brief  */
		HCONFIG		createElementExtraConfig(IElement* elem = NULL, const TString& elementType = TString::EMPTY, const TString& elementName = TString::EMPTY);
		
	}//namespace SerializerHelper
	
}//namespace Blade

#endif//__Blade_SerializerHelper_h__