/********************************************************************
	created:	2010/04/09
	filename: 	ParaStateDef.h
	author:		Crazii
	
	purpose:	parallel state data
				multi observers single subject mode
				so param "source" in update not used
*********************************************************************/
#ifndef __Blade_ParaStateDef_h__
#define __Blade_ParaStateDef_h__
#include <ExceptionEx.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Quaternion.h>
#include <math/AxisAlignedBox.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/graphics/Color.h>

#include <interface/public/IElement.h>
#include <interface/IParaState.h>
#include <parallel/ParaStateGroup.h>
#include <parallel/ParaStateQueue.h>
#include <parallel/ParaStateSet.h>



namespace Blade
{
	/** @brief IParaState extern template class */
	/* @remark as an observer ParaSate can only attached to the subject of the same type(ParaSate or subclass) \n
		but as an subject,any observer type could be attached
	*/
	template< typename T>
	class ParaStateT : public IParaState
	{
	public:
		typedef T*			pointer;
		typedef const T*	const_pointer;
	public:
		ParaStateT(pointer Data = NULL)
		{
			mParaData->mData = Data;

			//if binding data is NULL, we need auto-create the data
			if( Data == NULL )
			{
				//using pooled allocator instead of default, in case that allocator of T is not customized
				mParaData->mData = new (Allocatable::allocate<sizeof(T)>( sizeof(T) ) ) T();
				mParaData->mFlag.raiseBits( PARA_DATA_BLOCK::SF_MANAGED );
			}
		}

		~ParaStateT()
		{
			if( mParaData->isEnqueued() )
				mParaData->mQueue->remove(this);
			else if( mParaData->mQueue != NULL )
			{
				assert( mParaData->mQueue->remove(this) == false );
			}

			if( mParaData->mFlag.checkBits( PARA_DATA_BLOCK::SF_MANAGED) )
			{
				((pointer)mParaData->mData)->~T();
				Allocatable::deallocate<sizeof(T)>( mParaData->mData );
			}
		}

		/************************************************************************/
		/* IParaState interface                                                                     */
		/************************************************************************/

	protected:
#if BLADE_DEBUG
		//note: DO NOT use typeid(T).name() becasue typeid(T).name() is implementation-defined, 
		//C++ standard doesn't garuantee typeid(T).name() differs among different types.
		/** @brief  */
		virtual const std::type_info&	getIdentifier() const
		{
			return typeid(T);
		}
#endif

		/** @brief  */
		/** @note this function will be called during parallel running among all tasks for the same object
		so we need to care about the synchronization */
		virtual void onNotified(const void* sourceStateData, int16 sourcePriority)
		{
			//if( !Lock::compareOPAndSwap32( (int32*)&mParaData->mInputPriority, sourcePriority, sourcePriority, std::less<uint32>() ) )
			//	return;

			////TODO: input is still not thread safe!!!
			////if one priority notification cames and a higher priority notification cames after
			////both will run into this point and the higher(latter one) may be executed before the lower(former one)
			////and then overwritten by the lower priority input
			////and that will cause a input (mis-match) error
			//Lock::set( &mParaData->mInput, sourceStateData);

			//currently we use lock (simple spin + CAS) to synchronize state
			{
				ScopedLock _lock(mParaData->mSyncLock);
				if( mParaData->mInputPriority < sourcePriority )
				{
					mParaData->mInputPriority = sourcePriority;
					mParaData->mInput = sourceStateData;
				}
			}

			//thread safe queue check and push
			if( mParaData->safeEnqueue() )
				mParaData->mQueue->push(this);
		}

		/** @brief  */
		/** @note this function will NOT be called among tasks for the same object  */
		virtual void update()
		{
			mParaData->mInputPriority = PP_LOWEST;
			mParaData->resetFlags();

			if( mParaData->mInput != NULL )
			{
				*(pointer)(mParaData->mData) = *(const_pointer)(mParaData->mInput);
				mParaData->mInput = NULL;

				if( mParaData->mOwner != NULL )
					mParaData->mOwner->onParallelStateChange(*this);
			}
			else // this is highest priority source, so it has no input
				mParaData->mGroup->resetSourcePriority();
		}

		/** @brief  */
		/** @note this function will NOT be called among tasks for the same object  */
		virtual void update(const IParaState* source)
		{
			const_pointer data = static_cast<const_pointer>( source->getStateData() );
			if (*(pointer)(mParaData->mData) != *data)
			{
				*(pointer)(mParaData->mData) = *data;

				if (mParaData->mOwner != NULL)
					mParaData->mOwner->onParallelStateChange(*this);
			}

			mParaData->resetFlags();
			mParaData->mInput = NULL;
			mParaData->mInputPriority = PP_LOWEST;
		}

		/** @brief  */
		virtual PARA_DATA_BLOCK*	getInternalData() const
		{
			return mParaData;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
	public:
		/** @brief this function is used to modifying data */
		inline void	setData(const T& src)
		{
			if( this->applyChange() )
				*(pointer)(mParaData->mData) = src;
		}

		/** @brief force apply data changes */
		/* @note this function will NOT be called among tasks for the same object  */
		inline bool	applyChange()
		{
			if( mParaData->mInput != NULL )	//higher input exist
				return false;

			if( !mParaData->isChanged() )
			{
				mParaData->setChanged(true);

				if( mParaData->mGroup != NULL) //not ready (no group)?
				{
					this->notify();

					//put self in queue to reset flags later
					if( mParaData->safeEnqueue() )
						mParaData->mQueue->push(this);
				}
			}

			return true;
		}

		/** @brief  */
		inline void	synchronize()
		{
			mParaData->mGroup->synchronize();
		}

		/** @brief  */
		inline void	setUnTouched(const T& data)
		{
			*(pointer)(mParaData->mData) = data;
		}

		/** @brief  */
		inline T&	getDataRefUnTouched()
		{
			return *(pointer)mParaData->mData;
		}

		/** @brief  */
		T&			getDataRef()
		{
			this->applyChange();
			return *(pointer)(mParaData->mData);
		}

		/** @brief  */
		inline ParaStateT<T>&	operator=(const T& rhs)
		{
			if(*this != rhs)
				this->getDataRef() = rhs;
			return *this;
		}

		/** @brief this function is used to access read-only data */
		inline operator const T& () const
		{
			return *(pointer)(mParaData->mData);
		}

		/** @brief  */
		inline bool operator==(const T& rhs) const
		{
			return *(pointer)(mParaData->mData) == rhs;
		}

		/** @brief  */
		inline bool operator==(const IParaState& rhs) const
		{
			return this->equals(rhs);
		}

		/** @brief  */
		inline bool operator!=(const T& rhs) const
		{
			return *(pointer)(mParaData->mData) != rhs;
		}

		/** @brief this function is used to access read-only data */
		inline const T*	operator ->() const
		{
			return (const_pointer)mParaData->mData;
		}

	protected:
		/** @brief  */
		inline bool	equals(const IParaState& rhs) const
		{
			return static_cast<const IParaState*>(this) == &rhs;
		}

		/** @brief  */
		inline bool	notify()
		{
			return mParaData->mGroup->notify(this);
		}

		/** @brief  */
		inline void	clearState()
		{
			mParaData->resetFlags();
		}
	private:
		ParaStateT<T>(const ParaStateT<T>& src);
		const ParaStateT<T>&	operator =(const ParaStateT<T>& rhs);

		template<typename U>
		friend bool operator==(const IParaState& lhs,const ParaStateT<U>& rhs);
	};//template class ParaStateT

	template<typename T>
	inline bool operator==(const IParaState& lhs,const ParaStateT<T>& rhs)
	{
		return rhs.equals(lhs);
	}

#if 0
	extern template class BLADE_FRAMEWORK_API ParaStateT<bool>;
	extern template class BLADE_FRAMEWORK_API ParaStateT<int>;
	extern template class BLADE_FRAMEWORK_API ParaStateT<scalar>;
	extern template class BLADE_FRAMEWORK_API ParaStateT<uint>;
	extern template class BLADE_FRAMEWORK_API ParaStateT<TString>;

	extern template class BLADE_FRAMEWORK_API ParaStateT<Vector3>;
	extern template class BLADE_FRAMEWORK_API ParaStateT<Vector4>;
	extern template class BLADE_FRAMEWORK_API ParaStateT<Color>;
	extern template class BLADE_FRAMEWORK_API ParaStateT<Quaternion>;
	extern template class BLADE_FRAMEWORK_API ParaStateT<AABB>;
#endif

	typedef ParaStateT<bool>		ParaBool;
	typedef ParaStateT<int>			ParaInt;
	typedef ParaStateT<uint>		ParaUint;
	typedef ParaStateT<scalar>		ParaScalar;
	typedef ParaStateT<TString>		ParaTString;

	typedef ParaStateT<Vector3>		ParaVec3;
	typedef ParaStateT<Vector4>		ParaVec4;
	typedef ParaStateT<Color>		ParaColor;
	typedef ParaStateT<Quaternion>	ParaQuat;
	typedef ParaStateT<AABB> ParaAAB;

#define DEF_PARA_OP_RHS(_TYPE,_OP,_R_TYPE) \
	inline _TYPE operator _OP (const _R_TYPE& lhs, const ParaStateT<_TYPE>& rhs)\
	{\
		return lhs _OP static_cast<const _TYPE&>(rhs);\
	}\
	inline _TYPE operator _OP (const ParaStateT<_TYPE>& lhs, const _R_TYPE& rhs)\
	{\
		return static_cast<const _TYPE&>(lhs) _OP rhs;\
	}

#define DEF_PARA_OP(_TYPE,_OP) DEF_PARA_OP_RHS(_TYPE,_OP,_TYPE)

#define DEF_PARA_OP_ALL(_TYPE) DEF_PARA_OP(_TYPE,+) \
	DEF_PARA_OP(_TYPE,-) \
	DEF_PARA_OP(_TYPE,*) \
	DEF_PARA_OP(_TYPE,/)

DEF_PARA_OP_ALL(int)
DEF_PARA_OP_ALL(uint)
DEF_PARA_OP_ALL(scalar)
DEF_PARA_OP_ALL(Vector3)
DEF_PARA_OP_ALL(Vector4)
DEF_PARA_OP(TString,+)
DEF_PARA_OP(Quaternion,+)
DEF_PARA_OP(Quaternion,-)
DEF_PARA_OP(Quaternion,*)

}//namespace Blade


#endif //__Blade_ParaStateDef_h__