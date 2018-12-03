/********************************************************************
	created:	2011/11/02
	filename: 	LogicActor.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LogicActor_h__
#define __Blade_LogicActor_h__
#include <interface/public/geometry/IGeometry.h>

namespace Blade
{
	class IEntity;
	//////////////////////////////////////////////////////////////////////////
	class LogicActor : public Allocatable
	{
	public:
		inline LogicActor(IEntity* entity, IGeometry* geometry)
			:mEntity(entity), mGeometry(geometry)		{}
		virtual ~LogicActor() {}

		/** @brief  */
		inline bool		operator<(const LogicActor& rhs) const
		{
			return mEntity < rhs.mEntity;
		}

		/** @brief  */
		inline IEntity*	getEntity() const
		{
			return mEntity;
		}

		/** @brief  */
		inline IGeometry*getGeometry() const { return mGeometry; }

		/** @brief  */
		INode*	getGeometryNode() const { return mGeometry->getGeometryNode(); }

	private:
		IEntity*	mEntity;
		IGeometry*	mGeometry;
	};

}//namespace Blade



#endif // __Blade_LogicActor_h__