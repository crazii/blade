/********************************************************************
	created:	2013/04/09
	filename: 	IModelConfigManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IModelConfigManager_h__
#define __Blade_IModelConfigManager_h__
#include <BladeModel.h>
#include <interface/InterfaceSingleton.h>
#include <interface/IPool.h>
#include <interface/public/graphics/IGraphicsType.h>

namespace Blade
{
	class IModelConfigManager : public InterfaceSingleton<IModelConfigManager>
	{
	public:
		static const int MODEL_MEMORY_ALIGN = 64;
	public:
		virtual ~IModelConfigManager()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsType&	getStaticModelType(bool alpha = false) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsType&	getSkinnedModelType(bool alpha = false) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual IGraphicsType&	getMovingStaticType() = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual IPool*		getModelPool() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		setEditingMode(bool enable) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		isEditingModeEnabled() const = 0;

		/**
		@describe whether to update sub mesh bounding for skinned animation
		sub mesh's bounding can offer a precise culling, but animations need extra work to update it.
		@param
		@return
		*/
		virtual bool		isUpdatingSkinnedSubmeshBound() const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const TString&	getIKConfiguration() const = 0;

		inline bool		isIKEnabled() const
		{
			return this->getIKConfiguration() != ModelConsts::IK_NONE;
		}

		/**
		  @describe merge mesh buffer on load
		  @param
		  @return
		*/
		virtual bool		isMergingMeshBuffer() const = 0;

	};//class IModelConfigManager

	extern template class BLADE_MODEL_API Factory<IModelConfigManager>;


#define BLADE_MODEL_ALIGN_ALLOC(_bytes, _align)	BLADE_POOL_ALIGN_ALLOC( Blade::IModelConfigManager::getInterface().getModelPool(), _bytes, _align)
#define BLADE_MODEL_ALIGN_FREE(_ptr)			BLADE_POOL_ALIGN_FREE( Blade::IModelConfigManager::getInterface().getModelPool(), _ptr)

#define BLADE_MODEL_ALLOC(_bytes)				BLADE_MODEL_ALIGN_ALLOC( _bytes, Blade::IModelConfigManager::MODEL_MEMORY_ALIGN)
#define BLADE_MODEL_ALLOCT(_type,_count)		(_type*)BLADE_MODEL_ALIGN_ALLOC( sizeof(_type)*_count, Blade::IModelConfigManager::MODEL_MEMORY_ALIGN )
#define BLADE_MODEL_FREE(_ptr)					BLADE_MODEL_ALIGN_FREE( _ptr )
#define BLADE_MODEL_ALLOC_DEBUG(_bytes, _file, _line)	BLADE_POOL_ALIGN_ALLOC_DEBUG( Blade::IModelConfigManager::getInterface().getModelPool(), _bytes, Blade::IModelConfigManager::MODEL_MEMORY_ALIGN, _file, _line)

	//runtime data allocation by model system
	template<typename T>
	class ModelAllocatable
	{
	public:
		inline static void*	operator new(size_t size,const MEMINFO*)
		{
			return BLADE_MODEL_ALLOC(size);
		}

		inline static void*	operator new[](size_t size,const MEMINFO*)
		{
			return BLADE_MODEL_ALLOC(size);
		}

		inline static void		operator delete(void* ptr,const MEMINFO*)
		{
			return BLADE_MODEL_FREE(ptr);
		}

		inline static void		operator delete[](void* ptr,const MEMINFO*)
		{
			return BLADE_MODEL_FREE(ptr);
		}

		inline static void		operator delete(void* ptr)
		{
			return BLADE_MODEL_FREE(ptr);
		}

		inline static void		operator delete[](void* ptr)
		{
			return BLADE_MODEL_FREE(ptr);
		}

		inline static void*	operator new(size_t size,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			return BLADE_MODEL_ALLOC_DEBUG(size,sourcefile,lineNO);
		}

		inline static void		operator delete(void* ptr,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			BLADE_UNREFERENCED(sourcefile);
			BLADE_UNREFERENCED(lineNO);
			return BLADE_MODEL_FREE(ptr);
		}

		inline static void*	operator new[](size_t size,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			return BLADE_MODEL_ALLOC_DEBUG(size,sourcefile,lineNO);
		}

		inline static void		operator delete[](void* ptr,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			BLADE_UNREFERENCED(sourcefile);
			BLADE_UNREFERENCED(lineNO);
			return BLADE_MODEL_FREE(ptr);
		}

		//common placement new
		inline static void* operator new(size_t size,void* ptr)
		{
			BLADE_UNREFERENCED(size);
			return ptr;
		}

		//common placement delete
		inline static void	operator delete(void* ptr,void* place)
		{
			BLADE_UNREFERENCED(ptr);
			BLADE_UNREFERENCED(place);
		}
	};
	
}//namespace Blade

#endif //  __Blade_IModelConfigManager_h__