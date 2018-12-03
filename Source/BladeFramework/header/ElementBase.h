/********************************************************************
	created:	2011/04/09
	filename: 	ElementBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ElementBase_h__
#define __Blade_ElementBase_h__
#include <interface/public/IElement.h>
#include <parallel/ParaStateSet.h>

namespace Blade
{

	class BLADE_FRAMEWORK_API ElementBase : public IElement
	{
	public:
		ElementBase(const TString& systemType, const TString& type, EParaPriority priority);
		ElementBase(const ElementBase& src);
		ElementBase& operator=(const ElementBase& rhs);
		virtual ~ElementBase();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data */
		inline virtual void			postProcess(const ProgressNotifier& notifier)
		{
			notifier.onNotify(1.0f);
			ScopedLock lck(mDesc->mSyncLock);
			++mDesc->mLoadedCount;
		}

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		inline virtual const ELEMENT_DESC& getDesc() const
		{
			return *mDesc;
		}

		/**
		@describe
		@param
		@return
		*/
		virtual Interface*		getInterface(InterfaceName /*type*/) { return NULL; }

		/**
		@describe 
		@param 
		@return 
		*/
		inline virtual void			setEntity(IEntity* entity)
		{
			if( mDesc->mEntity != entity )
			{
				mDesc->mEntity = entity;
				this->onEntityChange();
			}
		}

		/**
		@describe 
		@param unload : unload existing resource
		@return 
		*/
		inline virtual void			setResourcePath(const TString& resourcePath)
		{
			if( mDesc->mResourcePath != resourcePath )
				mDesc->mResourcePath = resourcePath;
		}

		/**
		@describe
		@param
		@return
		*/
		inline virtual void			setBoundResource(const HRESOURCE& resource)
		{
			assert(mDesc->mResource == NULL || mDesc->mResource == resource);
			mDesc->mResource = resource;
		}

		/**
		@describe
		@param
		@return
		*/
		inline virtual void			getResourceParams(ParamList& /*outParams*/, TString& /*outResType*/, TString& /*outSerializer*/) {}

		/**
		@describe
		@param
		@return
		*/
		virtual bool			loadResource(bool forceAsync);

		/**
		@describe
		@param
		@return
		*/
		virtual bool			unloadResource(bool forceUnload = false);

		/**
		@describe 
		@param 
		@return 
		*/
		inline virtual const ParaStateSet& getParallelStates() const
		{
			return mParallelStates;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
	protected:
		/** @brief  */
		virtual void onEntityChange()	{}
		/** @brief called before resource unloaded  */
		virtual void onResourceUnload() {}

		/** @brief  */
		inline void	setScene(IScene* scene)
		{
			mDesc->mScene = scene;
		}

		ELEMENT_DESC*	mDesc;
		ParaStateSet	mParallelStates;
	};//class ElementBase
	

}//namespace Blade



#endif // __Blade_ElementBase_h__