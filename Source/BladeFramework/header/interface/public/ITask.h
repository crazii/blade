/********************************************************************
	created:	2009/03/29
	filename: 	ITask.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ITask_h__
#define __Blade_ITask_h__
#include <Handle.h>
#include <utility/String.h>
#include <utility/TList.h>
#include <BladeFramework.h>

namespace Blade
{
	class BLADE_FRAMEWORK_API ITask
	{
	public:
		enum ETaskPriority
		{
			TP_BACKGROUND	= -1,
			TP_LOW			= 0,
			TP_NORMAL		= 1,
			TP_HIGH			= 2,
		};
		//this is not usually used, but for simplify multi threading for some graphics API (i.e. GLES)
		enum EThreadAffinity
		{
			TA_NONE,	//task doesn't need to be bound to a fixed thread
			TA_ANY,		//task need to be bound to a non-main thread
			TA_MAIN,	//task need to be bound to the main thread
		};

		struct Type
		{
			TString name;
			EThreadAffinity affinity;

			inline Type(const TString& n) :name(n), affinity(TA_NONE) {}
			inline Type(const TString& n, EThreadAffinity ta) :name(n), affinity(ta) {}
			inline bool operator==(const Type& rhs) const { return name == rhs.name; }
			inline bool operator!=(const Type& rhs) const { return name != rhs.name; }
			inline bool operator<(const Type& rhs) const { return FnTStringFastLess::compare(name, rhs.name); }
		};
		static const Type ANY_TYPE;
		static const Type NO_TYPE;

		virtual ~ITask()	{};

		/** @brief  */
		virtual	const TString&	getName() const = 0;

		/**
		@describe get task main device type on which it depends,such as CPU or GPU or HDD,etc.
		@param 
		@return 
		@remark this is useful for parallel optimizing \n
		for example,a physics task maybe on CPU,or on other device like GPU (PhysX)
		@note: tasks of the same type are executed serially: 
			they are guaranteed to be executed in the same thread (Thread::getCurrentID() will return the same for the tasks)
			one exception is ANY_TYPE, tasks of this type may run on any thread.
			another exception is tasks of TP_BACKGROUND priority, they are treated as ANY_TYPE
		*/
		virtual const Type&	getType() const = 0;

		/**
		@describe get the ITask's priority level
		@param 
		@return 
		*/
		virtual ETaskPriority	getPriority() const = 0;

		/**
		@describe priority id with the same level,higher means high priority
		@param
		@return
		*/
		virtual uint32			getPriorityID() const = 0;

		/**
		@describe run the task
		@param 
		@return 
		*/
		virtual void			run() = 0;

		/**
		@describe update the task: synchronize data. not called for tasks of TP_BACKGROUND priority
		@param 
		@return 
		*/
		virtual void			update() = 0;


		/**
		@describe
		@param
		@return
		*/
		virtual void			onAccepted() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			onRemoved() = 0;

	};//class ITask

	typedef Handle<ITask> HTASK;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class IDispatchableTask : public TempAllocatable
	{
	public:
		virtual const TString& getName() const = 0;
		virtual void run() = 0;

	public:
		class IHandle
		{
		public:
			virtual ~IHandle() {}
			virtual void sync() = 0;
		};

		class Handles : public TList<IHandle*, TempAllocator<IHandle*> >, public NonAllocatable, public NonAssignable, public NonCopyable
		{
		public:
			inline void sync(size_t i)
			{
				(*this)[i]->sync();
			}
			inline void sync()
			{
				for (size_t i = 0; i < this->size(); ++i)
					(*this)[i]->sync();
			}
			inline ~Handles()
			{
				for (size_t i = 0; i < this->size(); ++i)
					BLADE_DELETE (*this)[i];
			}
		};
	};//class IDispatchableTask

	///utility class to build local task list. it should be kept local and not be copied.
	template<typename T>
	class DispatchableTaskList : public TList<T, TempAllocator<T> >, public NonAllocatable, public NonAssignable, public NonCopyable
	{
	public:
		typedef TList<T, TempAllocator<T> > Base;
		typedef TList< IDispatchableTask*, TempAllocator<IDispatchableTask*> > PointerList;

		inline DispatchableTaskList(size_t count)
		{
			this->resize(count);
		}

		/** @brief  */
		inline void resize(size_t count)
		{
			this->Base::resize(count);
			mPointerList.resize(count);
			if (count > 0)
			{
				for (size_t i = 0; i < count; ++i)
					mPointerList[i] = &(*this)[i];
			}
		}

		/** @brief  */
		IDispatchableTask**	getTasks() const
		{
			return mPointerList.size() > 0 ? &mPointerList[0] : NULL;
		}

	protected:
		using Base::resize;
		using Base::insert;
		using Base::erase;
		mutable PointerList mPointerList;
	};

}//namespace Blade

#endif // __Blade_ITask_h__