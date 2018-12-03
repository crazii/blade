/********************************************************************
	created:	2010/04/13
	filename: 	IGraphicsBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IGraphicsBuffer_h__
#define __Blade_IGraphicsBuffer_h__
#include <math/Box3i.h>

namespace Blade
{

	class IGraphicsBuffer
	{
	public:
		typedef enum EGraphicsBufferAccessFLag
		{
			GBAF_NONE		= 0,
			GBAF_READ		= 0x01,	///read by graphics device or CPU
			GBAF_WRITE		= 0x02,	///write by graphics device or CPU
			GBAF_READ_WRITE	= GBAF_READ | GBAF_WRITE,
		}ACCESS;

		enum EGraphicsBufferUsage
		{
			//flags

			/** @brief indicates the buffer will be modified frequently */
			GBUF_DYNAMIC		= 0x04,
			GBUF_CPU_READ		= 0x08,

			///note: the difference between GBUF_DYNMIAC & GBU_DYNAMIC_WRITE is:\n
			///there's internal extra data copy for GBUF_DYNMIAC, so you can lock it as ready-only buffer like GBU_DEFAULT,
			///also with the extra data copy in D3D9 mode, we will auto recreate & fill the dynamic buffers.
			///while GBU_DYNAMIC_WRITE doesn't have this data copy because it is assumed to be written in every frame.

			//available usages
			GBU_NONE			= GBAF_NONE,
			GBU_STATIC			= 0x10,									//D3D11_USAGE_IMMUTABLE
			GBU_DYNAMIC			= GBUF_DYNAMIC | GBAF_READ_WRITE,		//D3D11_USAGE_DYNAMIC; D3DUSAGE_DYNAMIC + manual cache management
			GBU_DEFAULT			= GBU_STATIC | GBAF_READ_WRITE,			//D3D11_USAGE_DEFAULT; D3DUSAGE(0)
			GBU_DYNAMIC_WRITE	= GBUF_DYNAMIC | GBAF_WRITE,			//D3D11_USAGE_DYNAMIC; D3DUSAGE_DYNAMIC (no cache management, cannot be read)
			GBU_CPU_READ		= GBUF_CPU_READ | GBAF_READ,			//D3D11_USAGE_IMMUTABLE; D3DUSAGE(0) + D3DPOOL_DEFAULT + manual cache management

			//extra flags
			
			//for pixel buffer only
			/** @brief as depth stencil buffer */
			GBUF_DEPTHSTENCIL	= 0x20,

			/** @brief render target usage */
			GBUF_RENDERTARGET	= 0x40,

			/** @brief auto generate mip-map */
			GBUF_AUTOMIPMAP		= 0x80,
		};

		typedef class Usage
		{
		public:
			Usage(){}
			Usage(EGraphicsBufferUsage usage) :mUsage(usage)		{}
			Usage(int usage) :mUsage( (EGraphicsBufferUsage)usage)	{}
			operator EGraphicsBufferUsage() const				{return mUsage;}
			bool	operator==(EGraphicsBufferUsage rhs) const	{return mUsage == rhs;}
			bool	operator!=(EGraphicsBufferUsage rhs) const { return !(mUsage == rhs); }
			Usage&	operator=(EGraphicsBufferUsage rhs)			{mUsage = rhs;return *this;}
			inline bool		isDirectRead() const				{return (mUsage&GBUF_CPU_READ) != 0;}
			inline bool		isDynamic() const					{return (mUsage&GBUF_DYNAMIC) != 0;}
			inline bool		isReadable() const					{return (mUsage&GBAF_READ) != 0;}
			inline bool		isWriteable() const					{return (mUsage&GBAF_WRITE) != 0;}
			inline bool		isAutoMipmap() const				{return (mUsage&GBUF_AUTOMIPMAP) != 0;}
			inline bool		isRenderTarget() const				{return (mUsage&GBUF_RENDERTARGET) != 0;}
			inline bool		isDepthStencil() const				{return (mUsage&GBUF_DEPTHSTENCIL) != 0;}
			inline ACCESS	getCPUAccess() const				{return ACCESS(mUsage&GBAF_READ_WRITE);}
		protected:
			EGraphicsBufferUsage	mUsage;
		}USAGE;

		enum EGraphicsBufferLockFlag
		{
			GBLF_UNDEFINED = 0,

			/** @brief lock for read only */
			GBLF_READONLY = 0x01,

			/** @brief  */
			GBLF_WRITEONLY = 0x02,

			/** @brief  */
			GBLF_DISCARD = 0x04,

			/** @brief indicates not overwrite buffer that's being used rendering current frame */
			GBLF_NO_OVERWRITE = 0x08,

			/** @brief */
			GBLF_NO_DIRTY_UPDATE = 0x10,

			//
			GBLF_FORCEWORD = 0xFFFFFFFF,

			/** @brief lock for read/write */
			GBLF_NORMAL = GBLF_READONLY|GBLF_WRITEONLY,
			/** @brief discard buffer after modifying,need recreated */
			GBLF_DISCARDWRITE = GBLF_WRITEONLY | GBLF_DISCARD,
		};

		class LOCKFLAGS
		{
		public:
			LOCKFLAGS()	:mFlags(GBLF_UNDEFINED)						{}
			LOCKFLAGS(const LOCKFLAGS& src) :mFlags(src.mFlags)		{}
			LOCKFLAGS(EGraphicsBufferLockFlag flag) :mFlags(flag)	{}
			LOCKFLAGS(int flags) :mFlags(flags)						{}
			int operator&(EGraphicsBufferLockFlag flag)	const		{return mFlags&flag;}
			int operator&(int flags) const							{return mFlags&flags;}
			int operator|(EGraphicsBufferLockFlag flag)	const		{return mFlags|flag;}
			int operator|(int flags)								{return mFlags|flags;}
			LOCKFLAGS& operator|=(EGraphicsBufferLockFlag flag)		{mFlags |= flag; return *this;}
			LOCKFLAGS& operator|=(int flags)						{mFlags |= flags; return *this;}
			LOCKFLAGS& operator&=(EGraphicsBufferLockFlag flag)		{mFlags &= flag; return *this;}
			LOCKFLAGS& operator&=(int flags)						{mFlags &= flags; return *this;}
			bool operator == (int flags) const { return mFlags == flags; }
			bool operator != (int flags) const { return mFlags != flags; }

		protected:
			union
			{
				EGraphicsBufferLockFlag mEnum;	//debug watch
				int mFlags;
			};
		};

		virtual ~IGraphicsBuffer()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void*		lock(size_t offset, size_t length, LOCKFLAGS lockflags) = 0;


		/**
		@describe 
		@param 
		@return 
		*/
		virtual void*		lock(const Box3i& box, size_t& outPitch, LOCKFLAGS lockflags) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		unlock(void) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual USAGE		getUsage() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		isLocked() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		addDirtyRegion(const Box3i& dirtyBox) = 0;

		/** @brief  */
		inline  void*		lock(LOCKFLAGS lockflags)
		{
			return this->lock(0,(size_t)-1,lockflags);
		}

		inline void*		lock(size_t& outPitch,LOCKFLAGS lockflags)
		{
			return this->lock(Box3i::EMPTY, outPitch, lockflags);
		}

	};//class IGraphicsBuffer
	
}//namespace Blade



#endif //__Blade_IGraphicsBuffer_h__