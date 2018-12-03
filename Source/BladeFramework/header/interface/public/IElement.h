/********************************************************************
	created:	2010/04/08
	filename: 	IElement.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IElement_h__
#define __Blade_IElement_h__
#include <BladeFramework.h>
#include <utility/Variant.h>
#include <interface/Interface.h>
#include <interface/public/IResource.h>
#include <interface/public/ISerializable.h>

namespace Blade
{
	class IEntity;
	class IScene;

	class IParaState;
	class ParaStateSet;

	typedef struct SElementDescriptor : public Allocatable
	{
		TString		mSytemType;		///main type,for all element in the same sub-system, this type should be the same
		TString		mType;			///element type
		TString		mResourcePath;	///element resource full path, could be EMTPY if no binding resource
		IEntity*	mEntity;
		IScene*		mScene;
		HRESOURCE	mResource;
		uint32		mLoadedCount;
		mutable Lock mSyncLock;
	}ELEMENT_DESC;
	
	class BLADE_FRAMEWORK_API IElement : public ISerializable
	{
	public:
		virtual ~IElement()		{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const ELEMENT_DESC&	getDesc() const = 0;

		/** @brief system type: same for all element in the same sub-system */
		inline const TString&	getSystemType() const	{return this->getDesc().mSytemType;}
		/** @brief element type */
		inline const TString&	getType() const			{return this->getDesc().mType;}
		inline const TString&	getResourcePath() const	{return this->getDesc().mResourcePath;}
		inline IEntity*			getEntity() const		{return this->getDesc().mEntity;}
		inline IScene*			getScene() const		{return this->getDesc().mScene;}
		inline const HRESOURCE&	getBoundResource() const	{return this->getDesc().mResource;}
		inline bool				isLoading() const		{return this->getDesc().mLoadedCount == 1;}
		inline bool				isLoaded() const		{return this->getDesc().mLoadedCount > 1;}

		/**
		@describe 
		@param
		@return
		*/
		virtual Interface*		getInterface(InterfaceName type) = 0;

		template<typename T>
		inline T* getInterface(InterfaceID<T>) { return static_cast<T*>(this->getInterface(InterfaceID<T>::getName())); }

		/**
		@describe set by framework when loading entity, or you can set your own
		@return 
		*/
		virtual void			setResourcePath(const TString& resourcePath) = 0;

		/**
		@describe set by framework when loading entity, or you set your custom resource
		@param 
		@return 
		*/
		virtual void			setBoundResource(const HRESOURCE& resource) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			getResourceParams(ParamList& outParams, TString& outResType, TString& outSerializer) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool			loadResource(bool forceAsync) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool			unloadResource(bool forceUnload = false) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const ParaStateSet& getParallelStates() const = 0;

	private:
		/**
		@describe
		@param
		@return
		*/
		virtual size_t			initParallelStates() = 0;

	public:
		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			onParallelStateChange(const IParaState& data) = 0;

	protected:

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setEntity(IEntity* entity) = 0;
		friend class IEntity;
	};//class IElement

	typedef Handle<IElement> HELEMENT;
	
}//namespace Blade


#endif //__Blade_IElement_h__