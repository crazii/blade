/********************************************************************
	created:	2010/04/22
	filename: 	IVertexSource.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/IVertexSource.h>

namespace Blade
{
	class VertexSource : public IVertexSource,public Allocatable
	{
	public:
		VertexSource();
		~VertexSource();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setSource(uint16 source,const HVBUFFER& vbuffer, size_t StreamOffset = 0);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			clearSource(uint16 source);

		/*
		@describe
		@param
		@return
		*/
		virtual void			clearAllSource();

		/*
		@describe
		@param
		@return
		*/
		virtual HVERTEXSOURCE		clone() const;

		/** @brief  */
		inline void updateData()
		{
			mStreamCount = (uint16)mBuffers.size();
			mStreamInfo = mStreamCount > 0 ? &mBuffers[0] : NULL;
		}

	protected:
		typedef		Vector<StreamInfo>		BufferMap;

		BufferMap		mBuffers;
	};//class VertexSource


	//////////////////////////////////////////////////////////////////////////
	HVERTEXSOURCE	IVertexSource::create()
	{
		return HVERTEXSOURCE(BLADE_NEW VertexSource());
	}


	//////////////////////////////////////////////////////////////////////////
	VertexSource::VertexSource()
	{
		mStreamInfo = NULL;
		mStreamCount = 0;
		mBuffers.reserve(8);
	}

	//////////////////////////////////////////////////////////////////////////
	VertexSource::~VertexSource()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			VertexSource::setSource(uint16 source,const HVBUFFER& vbuffer, size_t StreamOffset/* = 0*/)
	{
		if( mBuffers.size() <= source )
		{
			if( mBuffers.capacity() <= source )
				mBuffers.reserve(source*2u+1u);
			
			mBuffers.resize(source+1u);

			this->updateData();
		}

		mBuffers[source].buffer = vbuffer;
		mBuffers[source].source = source;
		mBuffers[source].offset = StreamOffset;
	}

	//////////////////////////////////////////////////////////////////////////
	void			VertexSource::clearSource(uint16 source)
	{
		if( source < mBuffers.size() )
		{
			//directly remove last source
			if (source == mBuffers.size() - 1)
			{
				mBuffers.erase(mBuffers.end() - 1);
				this->updateData();
			}
			else
			{
				mBuffers[source].buffer = HVBUFFER::EMPTY;
				mBuffers[source].offset = 0;
				mBuffers[source].source= 0;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			VertexSource::clearAllSource()
	{
		mBuffers.clear();
		this->updateData();
	}

	//////////////////////////////////////////////////////////////////////////
	HVERTEXSOURCE	VertexSource::clone() const
	{
		HVERTEXSOURCE hVS = create();

		VertexSource* vs = static_cast<VertexSource*>(hVS);
		*vs = *this;

		vs->updateData();
		return hVS;
	}
	
}//namespace Blade