/********************************************************************
	created:	2011/04/23
	filename: 	IStageConfig.h
	author:		Crazii
	purpose:	global serialization config for stages
*********************************************************************/
#ifndef __Blade_IStageConfig_h__
#define __Blade_IStageConfig_h__
#include <BladeFramework.h>
#include <Factory.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/ISerializable.h>
#include <interface/IPage.h>

namespace Blade
{
	class IStageConfig : public InterfaceSingleton<IStageConfig>
	{
	public:
		BLADE_FRAMEWORK_API static const TString SCENE_TYPE_SKIP_PREFIX;
		BLADE_FRAMEWORK_API static const TString ENTITY_SKIP_PREFIX;
		BLADE_FRAMEWORK_API static const TString ELEMENT_SKIP_PREFIX;

		virtual ~IStageConfig()		{}

		/**
		@describe entity names with this prefix will be skipped on loading/saving
		@param 
		@return 
		@note this function's mainly purpose is to 
		help some manufacture app (such as editor/tools) to save small amount of data, \n
		and could load data again, but the real Client-Oriented app will not load the data
		*/
		virtual const TString&	getEntitySkippingPrefix() const = 0;

		/**
		@describe reset the prefix, this is not for Client-Oriented app
		@param 
		@return 
		*/
		virtual void			setEntitySkippingPrefix(const TString& prefix) = 0;


		/**
		@describe element names with this prefix will be skipped on loading/saving
		@param 
		@return 
		*/
		virtual const TString&	getElementSkippingPrefix() const = 0;

		/**
		@describe reset the prefix
		@param 
		@return 
		*/
		virtual void			setElementSkippingPrefix(const TString& prefix) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getSceneTypeSkippingPrefix() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setSceneTypeSkippingPrefix(const TString& prefix) = 0;

		/**
		@describe get common serializable objects bound to all stages
		@param 
		@return 
		@note: common serializable objects are usually created on stage serialization, or added by user for the first time.
		*/
		virtual SerializableMap&getSerializableMap() = 0;

		/**
		@describe add layer info for paging system, shared by all stages' paging manager
		@param name: specify layer name
		@param elmentTypes: list of element types managed by this layer. managed layer will be loaded/unloaded by paging system
		@param subPageSize: layers are divided to sub pages. minimal sub page size is MIN_SUBPAGE_SIZE
		@param range: loading range, sub pages will be loaded if they pass range intersection, unloaded if they out of range. there is a buffer zone to avoid frequent loads/unloads.
		@param priority: loading order of the layer, layer with high priority value will loaded first
		@return
		*/
		virtual bool			addPageLayer(const TString& name, const TStringParam& elementTypes, size_t subPageSize, scalar range, int priority = IPage::DEFAULT_PRIORITY) = 0;
		virtual bool			addPageLayer(const TString& name, const TString& elementType, size_t subPageSize, scalar range, int priority = IPage::DEFAULT_PRIORITY) = 0;

		/**
		@describe  add extra element for a layer. the added order will control the element loading order: later added element will be loaded after the elements added before.
		@param 
		@return 
		*/
		virtual bool			addLayerElement(const TString& name, const TString& element) = 0;

		/**
		@describe config bound to stage
		@param 
		@return 
		*/
		virtual const HCONFIG&	getStageConfig() = 0;

	};//class IStageConfig

	extern template class BLADE_FRAMEWORK_API Factory<IStageConfig>;

}//namespace Blade



#endif // __Blade_IStageConfig_h__