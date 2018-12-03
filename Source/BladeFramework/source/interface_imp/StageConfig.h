/********************************************************************
	created:	2011/04/29
	filename: 	StageConfig.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_StageConfig_h__
#define __Blade_StageConfig_h__
#include <interface/IStageConfig.h>
#include <Singleton.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class StageConfig : public IStageConfig, public Singleton<StageConfig>
	{
	public:
		struct PageLayerConfig
		{
			TStringList elements;	//valid elements in layer
			int			priority;	//sub page loading priority
			size_t		size;		//sub page count for layer
			scalar		range;		//loading radius for layer

			PageLayerConfig() :size(0){}
		};
		typedef TStringMap<PageLayerConfig> LayerConfigMap;

		using Singleton<StageConfig>::getSingleton;
		using Singleton<StageConfig>::getSingletonPtr;

	public:
		StageConfig();
		~StageConfig();

		/************************************************************************/
		/* IStageConfig interface                                                                     */
		/************************************************************************/
		/*
		@describe entity names with this prefix will be skipped on loading/saving
		@param 
		@return 
		*/
		virtual const TString&	getEntitySkippingPrefix() const;

		/*
		@describe reset the prefix
		@param 
		@return 
		*/
		virtual void			setEntitySkippingPrefix(const TString& prefix);

		/*
		@describe element names with this prefix will be skipped on loading/saving
		@param 
		@return 
		*/
		virtual const TString&	getElementSkippingPrefix() const;

		/*
		@describe reset the prefix
		@param 
		@return 
		*/
		virtual void			setElementSkippingPrefix(const TString& prefix);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getSceneTypeSkippingPrefix() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setSceneTypeSkippingPrefix(const TString& prefix);

		/*
		@describe get common serializable objects bound to all stages
		@param 
		@return 
		@note: common serializable objects are usually created on stage serialization, or added by user for the first time.
		*/
		virtual SerializableMap&getSerializableMap()				{return mSerializables;}

		/**
		@describe add layer info for paging system, info shared by all stages' paging manager
		@param
		@return
		*/
		virtual bool			addPageLayer(const TString& name, const TStringParam& elementTypes, size_t subPageSize, scalar range, int priority = IPage::DEFAULT_PRIORITY);

		virtual bool			addPageLayer(const TString& name, const TString& elementType, size_t subPageSize, scalar range, int priority = IPage::DEFAULT_PRIORITY)
		{
			TStringParam types;
			types.push_back(elementType);
			return this->addPageLayer(name, types, subPageSize, range, priority);
		}

		/**
		@describe add extra element for a layer. the added order will control the element loading order: later added element will be loaded after the elements added before.
		@param 
		@return 
		*/
		virtual bool			addLayerElement(const TString& name, const TString& element);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&	getStageConfig();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		const LayerConfigMap&	getLayerConfig() const { return mLayerConfig; }

	protected:
		TString			mSkippingEntityNamePrefix;
		TString			mSkippingElementNamePrefix;
		TString			mSkippingSceneTypePrefix;
		SerializableMap	mSerializables;
		LayerConfigMap	mLayerConfig;
	};

}//namespace Blade



#endif // __Blade_StageConfig_h__