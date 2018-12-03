/********************************************************************
	created:	2013/02/26
	filename: 	EntitySerializer.h
	author:		Crazii
	purpose:	serializer of an entity (resource) using the resource framework
*********************************************************************/
#ifndef __Blade_EntitySerializer_h__
#define __Blade_EntitySerializer_h__
#include <memory/TempAllocator.h>
#include <utility/StringList.h>
#include <utility/IOBuffer.h>
#include <utility/StringHelper.h>
#include <utility/BladeContainer.h>
#include <utility/XmlFile.h>

#include <interface/public/ISerializer.h>
#include <interface/IStageConfig.h>
#include <interface/IEntity.h>
#include "Entity.h"
#include "EntityResource.h"

namespace Blade
{
	class Stage;

	//binary /text abstraction
	class IEntitySerializerImpl : public TempAllocatable
	{
	public:
		/** @brief  */
		virtual ~IEntitySerializerImpl() {}

		/** @brief asynchronous loading */
		virtual bool			cacheEntity(const HSTREAM& stream, Stage* stage) = 0;
		/** @brief synchronous state */
		virtual bool			loadEntity(IEntity* entity) = 0;
		/** @brief  */
		virtual bool			saveEntity(const IEntity* resource, const HSTREAM& stream) = 0;
	};

	//serializer wrap
	class EntitySerializer : public ISerializer, public TempAllocatable
	{
	public:
		typedef Factory<IEntitySerializerImpl> EntitySerializerFactory;

		EntitySerializer(const TString& factoryClass = EntityResource::ENTITY_SERIALIZER_TYPE_XML)
			:mImplClass(factoryClass)
		{
			mImpl = NULL;
			mCachedEntity = NULL;
		}
		~EntitySerializer()
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
		virtual bool	loadResource(IResource* res, const HSTREAM& stream, const ParamList& params)
		{
			assert( res->getType() == IEntity::ENTITY_RESOURCE_TYPE );
			EntityResource* resource = static_cast<EntityResource*>(res);
			assert( resource->getEntity() == NULL );

			IEntity* entity = (IEntity*)(void*)params[BTString("ENTITY")];
			resource->setEntity( entity );
			return this->cacheEntity(stream, params);
		}

		/*
		@describe process resource.like preLoadResource, this will be called in main thread.\n
		i.e.TextureResource need to be loaded into graphics card.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource)
		{
			assert( resource->getType() == IEntity::ENTITY_RESOURCE_TYPE );
			EntityResource* res = static_cast<EntityResource*>(resource);
			IEntity* entity = res->getEntity();
			assert( entity != NULL );
			bool ret= this->postLoadEntity(res);
			assert(ret);

			BLADE_UNREFERENCED(entity);
			BLADE_UNREFERENCED(ret);
		}
		
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveResource(const IResource* res, const HSTREAM& stream)
		{
			assert( res->getType() == IEntity::ENTITY_RESOURCE_TYPE );
			return this->saveEntity(res,stream);
		}

		/*
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* res, ParamList& params);

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
		virtual bool	reloadResource(IResource* /*resource*/, const HSTREAM& stream, const ParamList& params)
		{
			return this->cacheEntity(stream, params);
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool			cacheEntity(const HSTREAM& stream, const ParamList& params);
		/** @brief  */
		bool			postLoadEntity(IResource* resource);
		/** @brief  */
		bool			saveEntity(const IResource* resource, const HSTREAM& stream);

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
			mImpl = EntitySerializerFactory::getSingleton().createInstance(header);
		}

	protected:
		const TString&			mImplClass;
		IEntitySerializerImpl*	mImpl;
		IEntity*				mCachedEntity;
	private:
		EntitySerializer(const EntitySerializer&);
		EntitySerializer& operator=(const EntitySerializer&);
	};//class EntitySerializer

	class EntitySerializerImpl_XML : public IEntitySerializerImpl
	{
	public:
		EntitySerializerImpl_XML()	{mStage = NULL;}
		~EntitySerializerImpl_XML()	{}
		/** @brief asynchronous loading */
		virtual bool			cacheEntity(const HSTREAM& stream, Stage* stage)
		{
			mStage = stage;
			assert(mStage != NULL);
			return mFile.open( stream );
		}
		/** @brief synchronous state */
		virtual bool			loadEntity(IEntity* entity);
		/** @brief  */
		virtual bool			saveEntity(const IEntity* entity,const HSTREAM& stream);
	protected:
		XmlFile		mFile;
		Stage*		mStage;
	};//class EntitySerializerImpl_XML

	class EntitySerializerImpl_Binary : public IEntitySerializerImpl
	{
	public:
		EntitySerializerImpl_Binary()	{mStage = NULL;}
		~EntitySerializerImpl_Binary()	{}
		/** @brief asynchronous loading */
		virtual bool			cacheEntity(const HSTREAM& stream, Stage* stage)
		{
			mStage = stage;
			assert(mStage != NULL);
			return mBuffer.loadFromStream(stream);
		}
		/** @brief synchronous state */
		virtual bool			loadEntity(IEntity* entity);
		/** @brief  */
		virtual bool			saveEntity(const IEntity* entity,const HSTREAM& stream);
	protected:
		IOBuffer	mBuffer;
		Stage*		mStage;
	};//class EntitySerializerImpl_XML


	class EntitySerializerBinary : public EntitySerializer
	{
	public:
		EntitySerializerBinary()
			:EntitySerializer( EntityResource::ENTITY_SERIALIZER_TYPE_BINARY )
		{
		}
	};

	class EntitySerializerXML : public EntitySerializer
	{
	public:
		EntitySerializerXML()
			:EntitySerializer( EntityResource::ENTITY_SERIALIZER_TYPE_XML )
		{

		}
	};
	
}//namespace Blade

#endif //  __Blade_EntitySerializer_h__