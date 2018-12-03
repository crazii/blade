/********************************************************************
	created:	2010/05/08
	filename: 	GameEntity.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GameEntity_h__
#define __Blade_GameEntity_h__
#include <interface/public/geometry/IDynamicGeometry.h>
#include <interface/public/geometry/IStaticGeometry.h>
#include <interface/IEntity.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class GameEntity
	{
	public:
		inline GameEntity(IEntity* entity)
			:mEntity(entity)		{}

		/* @brief  */
		inline bool		operator<(const GameEntity& rhs) const
		{
			return mEntity < rhs.mEntity;
		}

		/* @brief  */
		inline IEntity*	getEntity() const
		{
			return mEntity;
		}

	protected:
		IEntity*	mEntity;
	};


	//////////////////////////////////////////////////////////////////////////
	class GameStaticEntity : public GameEntity
	{
	public:
		inline GameStaticEntity(IEntity* entity,IStaticGeometry* geom)
			:GameEntity(entity)
			,mGeometry(geom)		{}

		/* @brief  */
		inline IStaticGeometry*	getGeometry() const		{return mGeometry;}

	protected:
		IStaticGeometry*	mGeometry;
	};


	//////////////////////////////////////////////////////////////////////////
	class GameDynamicEntity : public GameEntity
	{
	public:
		inline GameDynamicEntity(IEntity* entity,IDynamicGeometry* geom)
			:GameEntity(entity)
			,mGeometry(geom)		{}

		/* @brief  */
		inline IDynamicGeometry*getGeometry() const		{return mGeometry;}

		/* @brief  */
		INode*					getGeometryNode() const	{return mGeometry->getGeometryNode();}

	protected:
		IDynamicGeometry*	mGeometry;
	};
	
}//namespace Blade


#endif //__Blade_GameEntity_h__