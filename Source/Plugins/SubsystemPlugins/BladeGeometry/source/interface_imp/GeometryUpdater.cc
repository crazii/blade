/********************************************************************
	created:	2010/05/07
	filename: 	GeometryUpdater.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GeometryUpdater.h"

namespace Blade
{

	GeometryUpdater::GeometryUpdater()
		:mUpdateList(BLADE_TEMPCONT_INIT)
	{

	}

	GeometryUpdater::~GeometryUpdater()
	{

	}

	/************************************************************************/
	/* INodeUpdater interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void GeometryUpdater::notifyNodeChange(Node* node)
	{
		if( node != NULL )
		{
			mUpdateList[ node->getDepthLevel() ].push_back(node);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void GeometryUpdater::updateNodes()
	{
		for( NodeUpdateList::iterator i = mUpdateList.begin(); i != mUpdateList.end(); ++i )
		{
			NodeList& list = i->second;
			for( NodeList::iterator n = list.begin(); n != list.end(); ++n )
			{
				Node* node = *n;
				node->update();
			}
		}
		mUpdateList.clear();
	}
	
}//namespace Blade