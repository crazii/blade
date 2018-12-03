/********************************************************************
	created:	2010/04/22
	filename: 	GraphicsGeometry.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsGeometry_h__
#define __Blade_GraphicsGeometry_h__
#include <utility/Mask.h>
#include <memory/BladeMemory.h>

namespace Blade
{
	class IVertexDeclaration;
	class IVertexSource;
	class IIndexBuffer;

	class GraphicsGeometry : public Allocatable
	{
	public:
		typedef enum EGeometryPrimitiveType
		{
			GPT_UNDEFINED = 0,
			GPT_POINT_LIST  = 1,
			GPT_LINE_LIST,
			GPT_LINE_STRIP,
			GPT_TRIANGLE_LIST,
			GPT_TRIANGLE_STRIP,
			GPT_TRIANGLE_FAN,
		}PRIMITIVE_TYPE;

		typedef enum EGeomtryAttribIndex
		{
			GAI_INDEX		= 0,	//use index buffer: mIndexBuffer/mSoftwareIndex are valid 
			GAI_INSTANCING,
		}ATTRIBUTE_INDEX;

		/** @brief  */
		inline GraphicsGeometry()
		{
			this->reset();
		}

		/** @brief  */
		inline void reset()
		{
			mVertexDecl = NULL;
			mVertexSource = NULL;
			mIndexBuffer = NULL;
			mPrimitiveType = GPT_UNDEFINED;
			mVertexStart = mVertexCount = mIndexStart = mIndexCount = mInstanceCount = mInstanceSourceIndex = 0;
			mAttribMask = 0;
		}

		/** @brief  */
		inline GraphicsGeometry(const GraphicsGeometry& src)
		{
			std::memcpy(this, &src, sizeof(*this) );
		}

		/** @brief  */
		inline GraphicsGeometry& operator=(const GraphicsGeometry& rhs)
		{
			std::memcpy(this, &rhs, sizeof(*this) );
			return *this;
		}

		/** @brief  */
		inline bool hasIndexBuffer() const
		{
			return mAttribMask.checkBitAtIndex(GAI_INDEX);
		}

		/** @brief  */
		inline void	useIndexBuffer(bool useIndex)
		{
			if( useIndex )
				mAttribMask.raiseBitAtIndex(GAI_INDEX);
			else
				mAttribMask.clearBitAtIndex(GAI_INDEX);
		}

		/** @brief  */
		inline void setInstancing(bool enbaleInstancing, uint16 instanceCount = uint16(-1), uint8 instanceSourceIndex = uint8(-1))
		{
			if (enbaleInstancing)
				mAttribMask.raiseBitAtIndex(GAI_INSTANCING);
			else
				mAttribMask.clearBitAtIndex(GAI_INSTANCING);
			mInstanceCount = instanceCount;
			mInstanceSourceIndex = instanceSourceIndex;
		}

		/** @brief  */
		inline bool isInstancing() const
		{
			return mAttribMask.checkBitAtIndex(GAI_INSTANCING);
		}

	public:
		const IVertexDeclaration*	mVertexDecl;
		IVertexSource*		mVertexSource;		//vertex streams
		IIndexBuffer*		mIndexBuffer;		//index buffer

		uint32		mVertexStart;
		uint32		mVertexCount;
		uint32		mIndexStart;
		uint32		mIndexCount;

		Mask		mAttribMask;
		uint16		mInstanceCount;
		uint8		mPrimitiveType;	//PRIMITIVE_TYPE
		uint8		mInstanceSourceIndex;
	};//class GraphicsGeometry
	
}//namespace Blade


#endif //__Blade_GraphicsGeometry_h__