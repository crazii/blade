/********************************************************************
	created:	2013/02/26
	filename: 	EntityResource.h
	author:		Crazii
	purpose:	entity resource definition
*********************************************************************/
#ifndef __Blade_EntityResource_h__
#define __Blade_EntityResource_h__
#include <interface/public/IResource.h>
#include <interface/IEntity.h>

namespace Blade
{
	//desc for saving elements
	struct EntitySaveDesc
	{
		const TStringParam* elements;
		bool		saveAll;
	};

	class EntityResource : public IResource, public Allocatable
	{
	public:
		static const TString ENTITY_SERIALIZER_TYPE_XML;
		static const TString ENTITY_SERIALIZER_TYPE_BINARY;
	public:
		EntityResource();
		~EntityResource();

		/************************************************************************/
		/* IResource interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const
		{
			return IEntity::ENTITY_RESOURCE_TYPE;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline IEntity*		getEntity() const
		{
			return mEntity;
		}

		/** @brief  */
		inline void			setEntity(IEntity* entity)
		{
			assert(mEntity == NULL);
			mEntity = entity;
		}

		/** @brief  */
		inline void setSaveDesc(EntitySaveDesc* desc)
		{
			mSaveDesc = desc;
		}
		const EntitySaveDesc*		getSaveDesc() const
		{
			return mSaveDesc;
		}

	protected:
		IEntity*		mEntity;
		EntitySaveDesc*	mSaveDesc;
	};//class EntityResource
	
}//namespace Blade

#endif //  __Blade_EntityResource_h__