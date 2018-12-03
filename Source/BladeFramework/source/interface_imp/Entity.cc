/********************************************************************
	created:	2010/04/09
	filename: 	Entity.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Entity.h"
#include <interface/IParaState.h>
#include <parallel/ParaStateSet.h>
#include <interface/IStageConfig.h>
#include <interface/public/ISerializable.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Entity::Entity(const TString& name, IStage* stage)
		:mName(name)
		,mStage(stage)
		,mMaxPages(0)
		,mCurrentPages(0)
		,mLoadingCount(0)
		,mStatus(ES_READY)
		,mSynchronized(0)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Entity::~Entity()
	{

	}

	/************************************************************************/
	/* IEntity interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		Entity::removeElement(const TString& name)
	{
		ElementMap::iterator it = mElementMap.find(name);
		if( it == mElementMap.end() )
		{
			BLADE_EXCEPT(EXC_NEXIST,BTString("No element named")+name );
		}

		HELEMENT& elem = it->second;
		assert( elem->getEntity() == this );
		IEntity::takeOwnership(elem, NULL);

		//remove parallel states
		const ParaStateSet& parastates = elem->getParallelStates();
		for( size_t i = 0; i < parastates.size(); ++i )
		{
			const NamedType<IParaState*>& namestate = parastates.at(i);
			const TString& stateName = namestate.getName();
			IParaState* state = namestate.getValue();

			//find group
			ParaStateGroup& group = mGroupMap[ stateName ];
			bool removed = group.remove( state );
			if( !removed )
			{
				assert(false && "removing state failed");
				BLADE_EXCEPT(EXC_REDUPLICATE,BTString("failed to remove states."));
			}
			else
				state->setGroup(NULL);

		}//for

		mElementMap.erase(it);
	}

	//////////////////////////////////////////////////////////////////////////
	IElement*	Entity::getElement(const TString& name) const
	{
		ElementMap::const_iterator i = mElementMap.find(name);
		if( i != mElementMap.end() )
			return i->second;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		Entity::getAllElements(TStringParam& nameList, ElementListParam& elemList) const
	{
		nameList.clear();
		elemList.clear();

		for(ElementMap::const_iterator i = mElementMap.begin(); i != mElementMap.end(); ++i)
		{
			nameList.push_back( i->first );
			elemList.push_back( static_cast<IElement*>(i->second) );
		}
		return elemList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		Entity::getElementsByType(ElementListParam& elemList, const TString& type, TStringParam* nameList/* = NULL*/) const
	{
		size_t count = elemList.size();

		for(ElementMap::const_iterator i = mElementMap.begin(); i != mElementMap.end(); ++i)
		{
			const HELEMENT& hElem = i->second;
			if (hElem->getType() == type)
			{
				elemList.push_back(static_cast<IElement*>(hElem));
				if (nameList != NULL)
					nameList->push_back(i->first);
			}
		}
		return elemList.size() - count;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		Entity::getElementsBySystemType(ElementListParam& elemList, const TString& sytemType) const
	{
		elemList.clear();
		for(ElementMap::const_iterator i = mElementMap.begin(); i != mElementMap.end(); ++i)
		{
			const HELEMENT& hElem = i->second;
			if( hElem->getSystemType() == sytemType )
				elemList.push_back(static_cast<IElement*>(hElem));
		}
		return elemList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	void		Entity::addElement(const TString& name,const HELEMENT& elem)
	{
		assert( elem != NULL && ( elem->getEntity() == NULL || elem->getEntity() == this ) );

		HELEMENT& empty = mElementMap[name];
		if( empty != NULL )
		{
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("an Element named ")+name+BTString(" alreay exist.") );
		}

		empty = elem;
		IEntity::takeOwnership(elem, this);

		if (mSynchronized)
		{
			this->initParallelStates(elem);
			this->addParallelStates(elem);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Entity::getInterfaces(InterfaceName type, TPointerParam<Interface>& interfaceList) const
	{
		for(ElementMap::const_iterator i = mElementMap.begin(); i != mElementMap.end(); ++i)
		{
			const HELEMENT& elem = i->second;
			Interface* ptr = elem->getInterface(type);
			if( ptr != NULL )
				interfaceList.push_back(ptr);
		}
		return interfaceList.size() > 0;
	}

	/***************************************;*********************************/
	/* custom methods                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const Entity::ElementMap&	Entity::getElements() const
	{
		return mElementMap;
	}
	
	//////////////////////////////////////////////////////////////////////////
	void		Entity::preSave() const
	{
		for(ElementMap::const_iterator i = mElementMap.begin(); i != mElementMap.end(); ++i)
		{
			const HELEMENT& elem = i->second;
			elem->prepareSave();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		Entity::postSave() const
	{
		for (ElementMap::const_iterator i = mElementMap.begin(); i != mElementMap.end(); ++i)
		{
			const HELEMENT& elem = i->second;
			elem->postSave();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		Entity::instantLoad(const ProgressNotifier& notifier)
	{
		if( mElementMap.size() == 0 )
		{
			notifier.onNotify(1.0f);
			return;
		}

		scalar count = (scalar)mElementMap.size();
		for(ElementMap::iterator i = mElementMap.begin(); i != mElementMap.end(); ++i)
		{
			const HELEMENT& elem = i->second;

			notifier.beginStep( 1.0f/count );
			elem->instantProcess(notifier);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		Entity::postLoad(const ProgressNotifier& notifier, bool succeeded)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		if (!mSynchronized)
		{
			for (ElementMap::iterator i = mElementMap.begin(); i != mElementMap.end(); ++i)
			{
				const HELEMENT& elem = i->second;
				this->initParallelStates(elem);
				this->addParallelStates(elem);
			}

			if (succeeded && mElementMap.size() > 0)
			{
				for (StateGroupMap::iterator i = mGroupMap.begin(); i != mGroupMap.end(); ++i)
				{
					ParaStateGroup& group = i->second;
					group.synchronize();
				}
			}

			mSynchronized = succeeded ? 1u : 0;
		}
		notifier.onNotify(1.0f);
	}

	//////////////////////////////////////////////////////////////////////////
	void		Entity::postProcess(const ProgressNotifier& notifier)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		assert(mSynchronized);

		if(mElementMap.size() == 0)
		{
			notifier.onNotify(1.0f);
			return;
		}

		TempVector<IElement*> priorityList;

		scalar count = (scalar)mElementMap.size();

		priorityList.reserve(mElementMap.size());
		for (ElementMap::iterator i = mElementMap.begin(); i != mElementMap.end(); ++i)
		{
			const HELEMENT& elem = i->second;
			priorityList.push_back(elem);
		}

		std::sort(priorityList.begin(), priorityList.end(), FnSerializableSorter());
		for (size_t i = 0; i < priorityList.size(); ++i)
		{
			notifier.beginStep(1.0f / count);
			priorityList[i]->postProcess(notifier);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		Entity::addElement(const HELEMENT& elem)
	{
		const TString& name = elem->getType() + TEXT("@Blade.Internal");
		this->addElement(name,elem);
	}

	//////////////////////////////////////////////////////////////////////////
	void		Entity::removeAllElements()
	{
		mElementMap.clear();
		mGroupMap.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void		Entity::addParallelStates(IElement* element)
	{
		//set up parallel states
		const ParaStateSet& parastates = element->getParallelStates();
		for (size_t i = 0; i < parastates.size(); ++i)
		{
			const NamedType<IParaState*>& namestate = parastates.at(i);
			const TString& stateName = namestate.getName();
			IParaState* state = namestate.getValue();

			//find group
			ParaStateGroup& group = mGroupMap[stateName];
			bool added = group.add(state, mSynchronized == 1);
			if (!added)
			{
				assert(false && "priority conflict");
				BLADE_EXCEPT(EXC_REDUPLICATE, BTString("states has same priority."));
			}

		}//for
	}
	
}//namespace Blade