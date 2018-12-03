/********************************************************************
	created:	2010/05/07
	filename: 	GeometryUpdater.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GeometryUpdater_h__
#define __Blade_GeometryUpdater_h__
#include <Node.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class GeometryNode;

	class GeometryUpdater : public INodeUpdater
	{
	public:
		GeometryUpdater();
		~GeometryUpdater();

		/************************************************************************/
		/* INodeUpdater interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void notifyNodeChange(Node* node);

		/** @brief  */
		virtual void updateNodes();

	protected:
		typedef TempVector<Node*>				NodeList;
		typedef TempMap<index_t,NodeList>		NodeUpdateList;

		NodeUpdateList	mUpdateList;
	};
	
}//namespace Blade


#endif //__Blade_GeometryUpdater_h__