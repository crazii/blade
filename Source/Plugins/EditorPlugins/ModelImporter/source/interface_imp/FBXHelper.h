/********************************************************************
	created:	2015/09/26
	filename: 	FBXHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_FBXHelper_h__
#define __Blade_FBXHelper_h__
#include <BladePlatform.h>
#include <interface/public/file/IStream.h>
#include <utility/BladeContainer.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4365 4266)
#endif

#include <fbxsdk.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#pragma warning(disable:4505) //unreferenced local function has been removed
#endif

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BladeFbxStream : public FbxStream, public NonAssignable
	{
	public:
		BladeFbxStream(const HSTREAM& stream) :mStream(stream) {}
		/** @brief  */
		virtual EState GetState()								{return mStream->isValid() ? eOpen : eClosed;}
		/** @brief  */
		virtual bool Open(void* /*pStreamData*/)				{mStream->rewind(); return mStream->isValid();}
		/** @brief  */
		virtual bool Close()									{mStream->rewind(); return mStream->isValid();}
		/** @brief  */
		virtual bool Flush()									{mStream->flush();return true;}
		/** @brief  */
		virtual int Write(const void* /*pData*/, int /*pSize*/)  {return 0;}
		/** @brief  */
		virtual int GetReaderID() const							{return 0;}
		/** @brief  */
		virtual int GetWriterID() const							{return -1;}
		/** @brief  */
		virtual int GetError() const							{return 0;}
		/** @brief  */
		virtual void ClearError()								{}
		/** @brief  */
		virtual int Read(void* pData, int pSize) const
		{
			return (int)mStream->readData(pData, IStream::Size(pSize));
		}
		/** @brief  */
		virtual void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos)
		{
			if( pSeekPos >= FbxFile::eBegin && pSeekPos <= FbxFile::eEnd )
			{
				IStream::EStreamPos pos[] = {IStream::SP_BEGIN, IStream::SP_CURRENT, IStream::SP_END};
				mStream->seek(pos[pSeekPos], (IStream::Off)pOffset);
			}
		}
		/** @brief  */
		virtual long GetPosition() const
		{
			return (long)mStream->tell();
		}
		/** @brief  */
		virtual void SetPosition(long pPosition)
		{
			mStream->seek(IStream::SP_BEGIN, (IStream::Off)pPosition);
		}
		const HSTREAM&  mStream;
	};

	//////////////////////////////////////////////////////////////////////////
	static void*	BladeFbxMalloc(size_t bytes)
	{
		return BLADE_RES_ALIGN_ALLOC(bytes, FBXSDK_MEMORY_ALIGNMENT);
	}

	//////////////////////////////////////////////////////////////////////////
	static void*	BladeFbxRealloc(void* ptr, size_t bytes)
	{
		return BLADE_RES_ALIGN_REALLOC(ptr, bytes, FBXSDK_MEMORY_ALIGNMENT);
	}

	//////////////////////////////////////////////////////////////////////////
	static void*	BladeFbxCalloc(size_t num, size_t size)
	{
		size_t bytes = num*size;
		void* ptr = BLADE_RES_ALIGN_ALLOC(bytes, FBXSDK_MEMORY_ALIGNMENT);
		::memset(ptr, 0, bytes);
		return ptr;
	}

	//////////////////////////////////////////////////////////////////////////
	static void	BladeFbxFree(void* ptr)
	{
		return BLADE_RES_ALIGN_FREE(ptr);
	}

	//////////////////////////////////////////////////////////////////////////
	inline DualQuaternion FbxMatrix2DQ(FbxAMatrix fbxMat)
	{
		Matrix44 m44;

		FbxVector4 v = fbxMat.GetRow(0);
		m44[0] = Vector4((scalar)v[0], (scalar)v[1], (scalar)v[2], (scalar)v[3]);

		v = fbxMat.GetRow(1);
		m44[1] = Vector4((scalar)v[0], (scalar)v[1], (scalar)v[2], (scalar)v[3]);

		v = fbxMat.GetRow(2);
		m44[2] = Vector4((scalar)v[0], (scalar)v[1], (scalar)v[2], (scalar)v[3]);

		v = fbxMat.GetRow(3);
		m44[3] = Vector4((scalar)v[0], (scalar)v[1], (scalar)v[2], (scalar)v[3]);

		return DualQuaternion(m44);
	}

	//////////////////////////////////////////////////////////////////////////
	static bool BladeFbxProgressCallback(void* pArgs, float pPercentage, const char* /*pStatus*/)
	{
		ProgressNotifier* callback = (ProgressNotifier*)(pArgs);
		if( callback != NULL )
			callback->onStep((size_t)(pPercentage*1000u), 100u*1000u);
		return true;
	}
	
}//namespace Blade



#endif // __Blade_FBXHelper_h__