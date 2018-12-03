/********************************************************************
	created:	2010/04/22
	filename: 	IVertexSource.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IVertexSource_h__
#define __Blade_IVertexSource_h__
#include "IVertexBuffer.h"

namespace Blade
{
	struct StreamInfo
	{
		HVBUFFER	buffer;
		size_t		offset;
		uint16		source;
	};

	class BLADE_BASE_API IVertexSource
	{
	public:
		

		virtual ~IVertexSource()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setSource(uint16 source, const HVBUFFER& vbuffer, size_t StreamOffset = 0) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			clearSource(uint16 source) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void			clearAllSource() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual Handle<IVertexSource>	clone() const = 0;

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		inline uint16				getBufferSourceCount() const { return mStreamCount; }

		/**
		@describe
		@param
		@return
		*/
		inline const StreamInfo&	getStreamInfo(uint16 index) const { assert(index < mStreamCount); return mStreamInfo[index]; }

		/**
		@describe
		@param
		@return
		*/
		inline const HVBUFFER&		getBuffer(uint16 source) const { return this->getStreamInfo(source).buffer; }

		/*
		@describe
		@param
		@return
		*/
		inline void					setOffset(uint16 source, size_t StreamOffset)
		{
			assert(source < mStreamCount);
			mStreamInfo[source].offset = StreamOffset;
		}

		/** @brief  */
		static Handle<IVertexSource>	create();

	protected:
		StreamInfo* mStreamInfo;
		uint16		mStreamCount;
	};//class IVertexSource

	typedef Handle<IVertexSource> HVERTEXSOURCE;
	
}//namespace Blade


#endif //__Blade_IVertexSource_h__