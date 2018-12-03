/********************************************************************
	created:	2010/09/05
	filename: 	ShaderVariableMap.h
	author:		Crazii
	purpose:	map holding custom(non-built-in) shader variables & their variable data
*********************************************************************/
#ifndef __Blade_ShaderVariableMap_h__
#define __Blade_ShaderVariableMap_h__
#include <BladeGraphics.h>
#include <Handle.h>
#include <AutoShaderVariable.h>
#include <utility/TList.h>

namespace Blade
{
	namespace Impl
	{
		class ShaderVariableMapDataImpl;
		class InstanceVariableMapDataImpl;
	}//namespace Impl

	class BLADE_GRAPHICS_API ShaderVariableMapBase : public Allocatable
	{
	public:
		class Handles : public NonAllocatable, public NonCopyable
		{
			friend class ShaderVariableMapBase;
			PointerList mVariables;
		public:
			inline Handles() {}
			inline void reset() { mVariables.clear(); }
			inline bool isValid() const {return mVariables.size() > 0;}
			bool BLADE_GRAPHICS_API setData(const void* data, size_t size);
		};
	public:
		ShaderVariableMapBase(EShaderVariableUsage usage);
		~ShaderVariableMapBase();

		ShaderVariableMapBase(const ShaderVariableMapBase&);
		ShaderVariableMapBase& operator=(const ShaderVariableMapBase&);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe
		@param
		@return the variable data
		*/
		const HAUTOSHADERVARIABLE&	createVariable(const TString& name, SHADER_CONSTANT_TYPE type, size_t count);

		/*
		@describe 
		@param 
		@return 
		*/
		void			clear();

		/*
		@describe 
		@param 
		@return 
		*/
		size_t			size() const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool			writeVariable(const TString& name,SHADER_CONSTANT_TYPE type,size_t count,const void* source);	


		/** @brief  */
		inline void*	getVariableData(const TString& name, SHADER_CONSTANT_TYPE type, size_t count)
		{
			const HAUTOSHADERVARIABLE& hAsv = this->getVariable(name);
			if (hAsv == NULL || hAsv->getType() != type || hAsv->getCount() != count)
				return NULL;
			return ShaderVariableMapBase::getVariableData(hAsv);
		}

		/** @brief  */
		inline const void*	getVariableData(const TString& name, SHADER_CONSTANT_TYPE type, size_t count) const
		{
			const HAUTOSHADERVARIABLE& hAsv = this->getVariable(name);
			if (hAsv == NULL || hAsv->getType() != type || hAsv->getCount() != count)
				return NULL;
			return ShaderVariableMapBase::getVariableData(hAsv);
		}

		/*
		@describe
		@param
		@return
		*/
		const HAUTOSHADERVARIABLE&	getVariable(const TString& name) const;

		/**
		@describe 
		@param
		@return
		*/
		bool		getVariableHandle(Handles& outHandle, const TString& name, SHADER_CONSTANT_TYPE type, size_t count) const;

		/**
		@describe
		@param
		@return
		*/
		static void*		getVariableData(const HAUTOSHADERVARIABLE& hAsv);

		/**
		@describe
		@param
		@return
		*/
		static void			writeData(void* data, const void* source, SHADER_CONSTANT_TYPE type, size_t count);

	protected:
		EShaderVariableUsage mUsage;

	private:
		LazyPimpl<Impl::ShaderVariableMapDataImpl> mData;
	};//class ShaderVariableMapBase

	//custom per-shader variable map
	class BLADE_GRAPHICS_API ShaderVariableMap : public ShaderVariableMapBase
	{
	public:
		ShaderVariableMap() :ShaderVariableMapBase(SVU_SHADER_CUSTOM)	{}
		static const ShaderVariableMap EMPTY;
	};

	//custom per pass variable map
	struct BLADE_GRAPHICS_API PassVariableMap : public ShaderVariableMapBase
	{
	public:
		PassVariableMap() :ShaderVariableMapBase(SVU_PASS_CUSTOM) {}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//custom per instance variable map
	class BLADE_GRAPHICS_API InstanceVariableMap
	{
	public:
		InstanceVariableMap();
		~InstanceVariableMap();

		/*
		@describe 
		@param 
		@return 
		*/
		void			clear();

		/*
		@describe 
		@param 
		@return 
		*/
		size_t			size() const;

		/**
		@describe 
		@param
		@return
		*/
		void*			createStorage(const TString& name, SHADER_CONSTANT_TYPE type, size_t count);

		/*
		@describe 
		@param 
		@return 
		*/
		bool			writeStorage(const TString& name, SHADER_CONSTANT_TYPE type, size_t count, const void* source);

		/**
		@describe 
		@param
		@return
		*/
		const void*		getStorage(const TString& name, SHADER_CONSTANT_TYPE type, size_t count) const;

		void*			getStorage(const TString& name, SHADER_CONSTANT_TYPE type, size_t count);

		/** @brief  */
		index_t			getStorageIndex(const TString& name);

		/** @brief  */
		const void*		getStorage(index_t index) const;

	private:	
		LazyPimpl<Impl::InstanceVariableMapDataImpl> mData;
	};
	
}//namespace Blade


#endif //__Blade_ShaderVariableMap_h__