/********************************************************************
	created:	2012/09/28
	filename: 	ISerializable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ISerializable_h__
#define __Blade_ISerializable_h__
#include <Factory.h>
#include <utility/Bindable.h>
#include <BladeFramework.h>
#include "IProgressCallback.h"

namespace Blade
{
	/************************************************************************/
	/* ISerializable: base for objects with serializing notification feature                                                                     */
	/************************************************************************/
	class ISerializable : public Bindable
	{
	public:
		enum EPriority
		{
			P_LOWEST = -0x7FFF - 1,
			P_DEFALT = 0xFF,
			P_HIGHEST= 0x7FFF,
		};
	public:
		virtual ~ISerializable()				{}

		/** @brief called before saving data  */
		virtual void			prepareSave()	{}
		/** @brief called after saving data */
		virtual void			postSave()		{}

		/** @brief called after loading data in current thread \n
		(background loading thread or main sync thread),\n
		you can load your config-related resource here
		*/
		virtual void			instantProcess(const ProgressNotifier& /*callback*/)	{}
		inline void instantProcess()
		{
			ProgressNotifier callback;
			this->instantProcess(callback);
		}
		
		/** @brief priority may not important for loading resources, but it may matter for postProcess */
		virtual int16			getPriority() const { return int16(P_DEFALT); }

		/** @brief called after finish loading data in main synchronous state */
		virtual void			postProcess(const ProgressNotifier& notifier) = 0;
		inline void				postProcess()
		{
			ProgressNotifier callback;
			this->postProcess(callback);
		}
	};//class ISerializable

	extern template class BLADE_FRAMEWORK_API Factory<ISerializable>;

	struct FnSerializableSorter
	{
		static inline bool compare(const ISerializable* lhs, const ISerializable* rhs)
		{
			return lhs->getPriority() > rhs->getPriority();
		}
		inline bool operator()(const ISerializable* lhs, const ISerializable* rhs) const
		{
			return FnSerializableSorter::compare(lhs, rhs);
		}
	};

	class EmptySerializable : public ISerializable
	{
	protected:
		~EmptySerializable()	{}
	public:
		virtual void			postProcess(const ProgressNotifier& ){};
	};

	/************************************************************************/
	/* serializable map                                                                     */
	/************************************************************************/
	namespace Impl
	{
		class SerializableMapImpl;
	}//namespace Impl

	class BLADE_FRAMEWORK_API SerializableMap : public NonAssignable, public NonCopyable, public NonAllocatable
	{
	public:
		SerializableMap();
		~SerializableMap();

		/** @brief  */
		ISerializable*	getSerializable(const TString& name) const;

		/** @brief  */
		bool			addSerializable(const TString& name, ISerializable* serializable);

		/** @brief  */
		size_t			getCount() const;

		/** @brief  */
		bool			getSerializable(index_t index, TString& name, ISerializable*& serialziable) const;

	private:
		Pimpl<Impl::SerializableMapImpl> mData;
	};

}//namespace Blade

#endif //  __Blade_ISerializable_h__