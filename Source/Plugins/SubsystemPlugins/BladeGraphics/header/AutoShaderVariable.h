/********************************************************************
	created:	2010/09/05
	filename: 	AutoShaderVariable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AutoShaderVariable_h__
#define __Blade_AutoShaderVariable_h__
#include <BladeGraphics.h>
#include <interface/public/graphics/IShader.h>
#include <Handle.h>

namespace Blade
{
	
	class AutoShaderVariable : public Allocatable
	{
	public:
		class BLADE_GRAPHICS_API IUpdater
		{
		public:
			virtual ~IUpdater() {}

			/** @brief  */
			virtual const void*	updateData() const = 0;

			/** @brief  */
			virtual bool		needCommit() const	{return true;}

		protected:
			/** @brief  */
			inline void		setData(AutoShaderVariable* sv,const void* data)
			{
				sv->mData = data;
				sv->mDataDirty = false;
			}
		};

	public:
		BLADE_GRAPHICS_API AutoShaderVariable(IUpdater* updater, SHADER_CONSTANT_TYPE type, size_t count, EShaderVariableUsage usage);
		BLADE_GRAPHICS_API AutoShaderVariable(IUpdater* updater, const AutoShaderVariable& src);
		virtual ~AutoShaderVariable()	{}

		/** @brief  */
		inline ShaderConstantType	getType() const			{return EShaderConstantType(mType);}

		/** @brief element size */
		inline size_t		getSize() const					{return (size_t)mSize;}

		/** @brief element count */
		inline size_t		getCount() const				{return (size_t)mCount;}

		/** @brief  */
		inline void			setRealSize(uint32 bytes)		{mBytes = bytes;}

		/** @brief  */
		inline void			markDirty()						{mDataDirty = true;}

		/** @brief  */
		inline bool			isDirty() const					{return mDataDirty;}

		/** @brief  */
		inline bool			needCommit() const				{return mUpdater->needCommit();}

		/** @brief  */
		inline EShaderVariableUsage		getUsage() const	{return EShaderVariableUsage(mUsage);}

		/** @brief  */
		inline bool			isCustom() const		{return (mUsage&SVU_CUSTOM_FLAG) != 0;}

		/** @brief  */
		inline bool			isGlobal() const		{return (mUsage&SVU_TYPE_MASK) == SVU_GLOBAL;}

		/** @brief  */
		inline bool			isPerShader() const		{return (mUsage&SVU_TYPE_MASK) == SVU_SHADER;}

		/** @brief  */
		inline bool			isPerPass() const		{return (mUsage&SVU_TYPE_MASK) == SVU_PASS;}

		/** @brief  */
		inline bool			isPerInstance() const	{return (mUsage&SVU_TYPE_MASK) == SVU_GLOBAL;}

		/** @brief  */
		inline const void*	getData()
		{
			this->update();
			return mData;
		}

		/** @brief get real size used, in bytes */
		inline size_t		getRealSize()
		{
			this->update();
			return (size_t)mBytes;
		}

		/** @brief  */
		inline void			update()
		{
			if( mDataDirty )
			{
				mData = mUpdater->updateData();
				mDataDirty = false;
			}
		}

		/** @brief  */
		inline IUpdater*	getUpdater() const
		{
			return mUpdater;
		}

		/** @brief update size info based on graphics API */
		inline void updateSizeInfo(size_t size)
		{
			mSize = (uint16)size;
			mBytes = (uint32)mSize * (uint32)mCount;
		}

	protected:
		IUpdater*		mUpdater;
		const void*		mData;
		uint32			mBytes;
		uint16			mCount;	//element count: count in entities of mType
		uint16			mSize;	//element size: size in type of mType
		EShaderVariableUsage	mUsage :8;	//EShaderVariableUsage
		EShaderConstantType		mType  :8;	//EShaderConstantType
		bool			mDataDirty;
	};//class AutoShaderVariable

	typedef Handle<AutoShaderVariable> HAUTOSHADERVARIABLE;

	
}//namespace Blade


#endif //__Blade_AutoShaderVariable_h__