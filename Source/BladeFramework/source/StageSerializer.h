/********************************************************************
	created:	2011/04/23
	filename: 	StageSerializer.h
	author:		Crazii
	purpose:	stage serializer, used for loading stage's pages too.
*********************************************************************/
#ifndef __Blade_StageSerializer_h__
#define __Blade_StageSerializer_h__
#include <utility/StringList.h>
#include <utility/IOBuffer.h>
#include <utility/BladeContainer.h>
#include <interface/public/ISerializer.h>
#include <interface/IEntity.h>
#include <interface/public/IResource.h>
#include "StageResource.h"
#include "interface_imp/Page.h"

#define ASYNC_PAGE_PROCESS 1	//async creating entities & elements

namespace Blade
{
	enum EStageSerializeType
	{
		SST_STAGE,
		SST_PAGE,		//full loading of page
	};

	class IStageSerializerImpl : public TempAllocatable
	{
	public:
		IStageSerializerImpl() : mType(SST_STAGE), mNotifier(&ProgressNotifier::EMPTY) {}
		virtual ~IStageSerializerImpl() {}

		/** @brief  */
		virtual bool			reloadResource(const HSTREAM& stream, const ParamList& params) = 0;
		/** @brief  */
		virtual bool			reprocessResource(IResource* resource) = 0;
		/** @brief  */
		virtual bool			loadStage(IResource* res,const HSTREAM& stream, const ProgressNotifier& notifier) = 0;
		/** @brief  */
		virtual bool			saveStage(const IResource* res,const HSTREAM& stream) = 0;

	public:
		EStageSerializeType		mType;
		TempVector<Entity*>		mLoadingEntities;
		const ProgressNotifier* mNotifier;
	};

	class StageSerializer : public ISerializer, public TempAllocatable, public NonAssignable, public NonCopyable
	{
	public:
		typedef Factory<IStageSerializerImpl> StageSerializerFactory;
	public:
		StageSerializer(const TString& factoryClass = STAGE_SERIALIZER_TYPE_BINARY, EStageSerializeType type = SST_STAGE)
			:mImplClass(factoryClass)
		{
			mImpl = NULL;
			mType = type;
		}
		~StageSerializer()
		{
			BLADE_DELETE mImpl;
		}

		/************************************************************************/
		/* ISerializer interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* res,const HSTREAM& stream, const ParamList& params)
		{
			IStage* stage = static_cast<IStage*>( (void*)params[BTString("STAGE")] );
			IPage* page = NULL;
			const ProgressNotifier* notifier = &ProgressNotifier::EMPTY;
			if(params[BTString("PAGE")].isValid())
				page = static_cast<IPage*>((void*)params[BTString("PAGE")]);
			if(params[BTString("PROGRESS")].isValid())
				notifier = static_cast<ProgressNotifier*>((void*)params[BTString("PROGRESS")]);

			assert(res->getType() == STAGE_RESOURCE_TYPE);
			StageResource* resource = static_cast<StageResource*>(res);
			resource->setStage(stage);
			resource->setPage(page);

			this->createImpl(stream);
			bool ret = mImpl != NULL;
			if (ret)
			{
				mImpl->mNotifier = notifier;
				ret = mImpl->reloadResource(stream, params);
			}
#if ASYNC_PAGE_PROCESS
			if (mType == SST_PAGE)
				ret = ret && this->loadStage(resource, stream, *notifier);
#endif
			return ret;
		}

		/*
		@describe load resource in main thread,if success,return true \n
		and then the resource manager will not load it again in loading thread.
		i.e. when load a texture that already exist,the serializer just check ,and return true \n
		if the texture not exist(not loaded yet) ,then return false
		@param
		@return
		*/
		//virtual bool	preLoadResource(IResource* res);

		/*
		@describe process resource.like preLoadResource, this will be called in main thread.\n
		i.e.TextureResource need to be loaded into graphics card.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource)
		{
			assert(mImpl != NULL);
#if ASYNC_PAGE_PROCESS
			if (mType == SST_PAGE)
			{
				this->postLoadEntities(resource);
				return;
			}
#endif
			bool ret = mImpl->reprocessResource(resource);
			assert(ret); BLADE_UNREFERENCED(ret);
		}
		
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveResource(const IResource* res, const HSTREAM& stream)
		{
			bool ret = this->saveStage(res,stream);
			return ret;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* res, ParamList& params)
		{
			assert( res->getType() == STAGE_RESOURCE_TYPE);
			StageResource* resource = static_cast<StageResource*>(res);
			IStage* stage = static_cast<IStage*>( (void*)params[BTString("STAGE")] );
			resource->setStage(stage);

			IPage* page = NULL;
			if (params[BTString("PAGE")].isValid())
				page = static_cast<IPage*>((void*)params[BTString("PAGE")]);
			resource->setPage(page);
			return stage != NULL;
		}

		/*
		@describe this method is called when resource is reloaded,
		the serializer hold responsibility to cache the loaded data for resource,
		then in main thread ISerializer::reprocessResource() is called to fill the existing resource with new data.\n
		this mechanism is used for reloading existing resource for multi-thread,
		the existing resource is updated in main thread(synchronizing state),
		to ensure that the data is changed only when it is not used in another thread.
		@param
		@return
		*/
		virtual bool	reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			const ProgressNotifier* notifier = &ProgressNotifier::EMPTY;
			if (params[BTString("PROGRESS")].isValid())
				notifier = static_cast<ProgressNotifier*>((void*)params[BTString("PROGRESS")]);
			this->createImpl(stream);
			mImpl->mNotifier = notifier;
			bool ret = mImpl->reloadResource(stream, params);

#if ASYNC_PAGE_PROCESS
			if (mType == SST_PAGE)
				ret = ret && this->loadStage(resource, stream, *notifier);
#else
			BLADE_UNREFERENCED(resource);
#endif
			return ret;
		}

		/*
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource)
		{
			assert(mImpl != NULL);
#if ASYNC_PAGE_PROCESS
			if (mType == SST_PAGE)
			{
				this->postLoadEntities(resource);
				return true;
			}
#endif
			bool ret = mImpl->reprocessResource(resource);
			return ret;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool			loadStage(IResource* res,const HSTREAM& stream, const ProgressNotifier& notifier)
		{
#if !ASYNC_PAGE_PROCESS
			this->createImpl(stream);
#endif
			return mImpl->loadStage(res, stream, notifier);
		}

		/** @brief  */
		bool			saveStage(const IResource* res,const HSTREAM& stream)
		{
			mImpl = StageSerializerFactory::getSingleton().createInstance( mImplClass );
			mImpl->mType = mType;

			const String header = BString("<!--") + BString(" ") + StringConverter::TStringToUTF8String(mImplClass) +  BString(" ") + BString("-->\n");
			stream->write( header.c_str(), header.size() );
			return mImpl->saveStage(res, stream);
		}

#if ASYNC_PAGE_PROCESS
		/** @brief  */
		void			postLoadEntities(IResource* res);
#endif

	private:

		/** @brief  */
		void	createImpl(const HSTREAM& stream)
		{
			assert(mImpl == NULL);
			TString header;
			{
				TempVector<tchar> tempString;
				char c = char();
				while( c != '\n' )
				{
					stream->read(&c,1);

					if( c == '\0')
						break;
					tempString.push_back( (tchar)c );
				}
				tempString.push_back( tchar() ) ;
				header = &tempString[0];
			}

			header = TStringHelper::trim(header);
			if( !TStringHelper::isStartWith(header, BTString("<!--"))
				|| !TStringHelper::isEndWith(header, BTString("-->")) 
				)
			{
				assert(false);
				return;
			}

			header = TStringHelper::trim( header.substr_nocopy(4, header.size()-7) );
			mImpl = StageSerializerFactory::getSingleton().createInstance(header);
			mImpl->mType = mType;
		}

		const TString&			mImplClass;
		IStageSerializerImpl*	mImpl;
		EStageSerializeType		mType;
	};//StageSerializer

	//binary serializer for stage
	class StageSerializerImpl_Binary : public IStageSerializerImpl
	{
	public:
		virtual ~StageSerializerImpl_Binary()	{}

		/************************************************************************/
		/* IStageSerializerImpl interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	reloadResource(const HSTREAM& stream, const ParamList& /*params*/)
		{
			return mBuffer.loadFromStream( stream );
		}

		/** @brief  */
		virtual bool	reprocessResource(IResource* resource);

		/** @brief  */
		virtual bool	loadStage(IResource* res,const HSTREAM& stream, const ProgressNotifier& notifier);
		
		/** @brief  */
		virtual bool	saveStage(const IResource* res,const HSTREAM& stream);

	public:
		
	protected:

		IOBuffer	mBuffer;
	};//class StageSerializerImpl_Binary

	//text serializer for stage
	class StageSerializerImpl_XML : public IStageSerializerImpl
	{
	public:
		virtual ~StageSerializerImpl_XML()	{}

		/************************************************************************/
		/* IStageSerializerImpl interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	reloadResource(const HSTREAM& stream, const ParamList& /*params*/)
		{
			return mBuffer.loadFromStream( stream );
		}

		/** @brief  */
		virtual bool	reprocessResource(IResource* resource);

		/** @brief  */
		virtual bool	loadStage(IResource* res,const HSTREAM& stream, const ProgressNotifier& notifier);
		
		/** @brief  */
		virtual bool	saveStage(const IResource* res,const HSTREAM& stream);

	protected:

		IOBuffer	mBuffer;
	};//class StageSerializerImpl_XML


	//////////////////////////////////////////////////////////////////////////
	class StageSerializerBinary : public StageSerializer
	{
	public:
		StageSerializerBinary()
			:StageSerializer(STAGE_SERIALIZER_TYPE_BINARY, SST_STAGE)
		{
		}
	};

	class StageSerializerXML : public StageSerializer
	{
	public:
		StageSerializerXML()
			:StageSerializer(STAGE_SERIALIZER_TYPE_XML, SST_STAGE)
		{
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class PageSerializer : public StageSerializer
	{
	public:
		PageSerializer()
			:StageSerializer(STAGE_SERIALIZER_TYPE_BINARY, SST_PAGE)
		{
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class PageSerializerBinary : public StageSerializer
	{
	public:
		PageSerializerBinary()
			:StageSerializer(STAGE_SERIALIZER_TYPE_BINARY, SST_PAGE)
		{
		}
	};

	class PageSerializerXML : public StageSerializer
	{
	public:
		PageSerializerXML()
			:StageSerializer(STAGE_SERIALIZER_TYPE_XML, SST_PAGE)
		{
		}
	};

	static const TString PAGE_SERIALIZER_TYPE = BTString("Page Resource");
	static const TString PAGE_SERIALIZER_TYPE_XML = BTString("Text Page");
	static const TString PAGE_SERIALIZER_TYPE_BINARY = BTString("Binary Page");

}//namespace Blade



#endif // __Blade_StageSerializer_h__