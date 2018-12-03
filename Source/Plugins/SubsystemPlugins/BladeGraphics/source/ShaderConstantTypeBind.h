/********************************************************************
	created:	2011/08/25
	filename: 	ShaderConstantTypeBind.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderConstantTypeBind_h__
#define __Blade_ShaderConstantTypeBind_h__
#include <AutoShaderVariable.h>
#include <math/Vector4.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>

namespace Blade
{

	namespace Impl
	{

		class ShaderVariableData
		{
			typedef Vector4		POOL_ELEMENT_TYPE;
			static const int ALIGIN_SIZE = sizeof(POOL_ELEMENT_TYPE);

		public:
			ShaderVariableData()
				:mPtr(NULL)
			{

			}

			~ShaderVariableData()
			{
				this->free();
			}

			/** @brief  */
			void*		getBuffer() const
			{
				return mPtr;
			}

			/** @brief  */
			void*	alloc(SHADER_CONSTANT_TYPE type,size_t count)
			{
				this->free();

				size_t elementBytes = IGraphicsResourceManager::getSingleton().getShaderConstantSize(type);
				size_t nAlignBytes = (count*elementBytes+ALIGIN_SIZE-1)/ALIGIN_SIZE * ALIGIN_SIZE;
#if BLADE_DEBUG
				mPtr = Allocatable::allocate<ALIGIN_SIZE>(nAlignBytes, __FILE__,__LINE__);
#else
				mPtr = Allocatable::allocate<ALIGIN_SIZE>(nAlignBytes);
#endif
				//set the initial data to empty
				std::memset(mPtr,0, nAlignBytes);
				return mPtr;
			}

			/** @brief  */
			void	free()
			{
				if( mPtr != NULL )
				{
					Allocatable::deallocate<ALIGIN_SIZE>(mPtr);
					mPtr = NULL;
				}
			}

		protected:
			void*	mPtr;
		};

	}//namespace Impl
	

}//namespace Blade



#endif // __Blade_ShaderConstantTypeBind_h__