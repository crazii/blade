/********************************************************************
	created:	2017/05/27
	filename: 	DataBindingHelper_Private.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_DataBindingHelper_Private_h__
#define __Blade_DataBindingHelper_Private_h__
#include <databinding/DataBindingHelper.h>
#include <utility/BladeContainer.h>
#include <Singleton.h>

namespace Blade
{

	/************************************************************************/
	/* delegate router for per-object(bindable) delegate                                                                     */
	/************************************************************************/
	///note: per-object delegate shall be stored in bindable object to gain performance,
	///but that is intrusive, so here uses a map to avoid intrusion, with a little performance & complexity cost.
	///the drawback is that user need to remove delegates manually or there will be leaks. (if delegate stored in target
	///object, usually its not needed to remove delegates, they'll be destroyed along the target object)
	///but this is OK since this usually only is used for editor, runtime uses parallel state to synchronize data.
	class DatabindingDelegateRouter : public Singleton<DatabindingDelegateRouter>
	{
	protected:
		typedef TStringMap<DelegateList> NamedDelegates;
		typedef Map<Bindable*, NamedDelegates>	RoutingData;
	public:
		~DatabindingDelegateRouter()
		{
			assert(mData.size() == 0 && "data binding notification leaks.");
		}

		/** @brief  */
		inline bool add(Bindable* target, const TString& dataName, const Delegate& d)
		{
			if (target == NULL || dataName.empty() || d.isEmpty())
				return false;
			mData[target][dataName].push_back(d);
			return true;
		}

		/** @brief  */
		inline bool remove(Bindable* target, const TString& dataName, void* ptr)
		{
			if (target == NULL || dataName.empty() || ptr == NULL)
				return false;

			RoutingData::iterator i = mData.find(target);
			if (i == mData.end())
				return false;

			NamedDelegates& delegates = i->second;
			NamedDelegates::iterator i2 = delegates.find(dataName);
			if (i2 == delegates.end())
				return false;

			DelegateList& list = i2->second;
			for (size_t j = 0; j < list.size(); ++j)
			{
				const Delegate& d = list.at(j);
				if (d.equals(ptr))
				{
					list.erase(j);
					if (list.size() == 0)
					{
						delegates.erase(i2);
						if (delegates.empty())
							mData.erase(i);
					}
					return true;
				}
			}
			return false;
		}

		/** @brief  */
		inline bool notify(Bindable* target, const TString& dataName, void* data) const
		{
			if (target == NULL || dataName.empty())
				return false;

			RoutingData::const_iterator i = mData.find(target);
			if (i == mData.end())
				return false;

			const NamedDelegates& delegates = i->second;
			NamedDelegates::const_iterator i2 = delegates.find(dataName);
			if (i2 == delegates.end())
				return false;

			i2->second.call(data);
			return true;
		}

	protected:
		RoutingData	mData;
	};

	
}//namespace Blade


#endif // __Blade_DataBindingHelper_Private_h__