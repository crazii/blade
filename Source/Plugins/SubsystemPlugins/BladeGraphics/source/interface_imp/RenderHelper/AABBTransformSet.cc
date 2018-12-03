/********************************************************************
	created:	2011/05/21
	filename: 	AABBTransformSet.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/graphics/PixelColorFormatter.h>
#include <interface/IAABBRenderer.h>
#include <Material.h>
#include "AABBTransformSet.h"

#define AABB_DYNAMIC 1

namespace Blade
{
	typedef struct SAABBVertexInstace
	{
		Vector3			mMin;
		Vector3			mMax;
		Color::COLOR	mColor;
	}AABB_INSTANCE;

#define DEFAULT_DRAW_SIZE (32)

	//////////////////////////////////////////////////////////////////////////
	AABBTransformSet::AABBTransformSet()
		:mBufferedCount(DEFAULT_DRAW_SIZE)
		,mUpdater(NULL)
	{
		mUpdateStateMask.raiseBitAtIndex( SS_PRE_RENDER );

		mUpdateFlags = CUF_DEFAULT_VISIBLE;
		mSpaceFlags = CSF_INDEPENDENT;
		mWorldAABB.setInfinite();
		mLocalAABB.setInfinite();
		mAABBCount = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	AABBTransformSet::~AABBTransformSet()
	{
	}

	/************************************************************************/
	/* IRenderable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		AABBTransformSet::update(SSTATE)
	{
		mAABBCount = this->getSize();
		if( mAABBCount != 0 )
		{
			if( mAABBCount > mBufferedCount )
			{
				while( mAABBCount > mBufferedCount )
					mBufferedCount = mBufferedCount+mBufferedCount/2;

				mVertexSource->setSource(1, HVBUFFER::EMPTY);
				mInstanceBuffer.clear();
#if AABB_DYNAMIC
				mInstanceBuffer = IGraphicsResourceManager::getSingleton().createVertexBuffer(NULL, sizeof(AABB_INSTANCE), mBufferedCount, IGraphicsBuffer::GBU_DEFAULT);
#else
				mInstanceBuffer = IGraphicsResourceManager::getSingleton().createVertexBuffer(NULL, sizeof(AABB_INSTANCE), mBufferedCount, IGraphicsBuffer::GBU_DYNAMIC_WRITE);
#endif
				mVertexSource->setSource(1, mInstanceBuffer);
			}

			size_t counting = 0;
			AABB_INSTANCE* data = (AABB_INSTANCE*)mInstanceBuffer->lock(IGraphicsBuffer::GBLF_DISCARDWRITE);
			for(TargetContentSet::const_iterator i = mContentRegistry.begin(); i != mContentRegistry.end(); ++i)
			{
				const ISpaceContent* content = i->first;
				if(content->getSpace() == NULL)
					continue;
				const Color& color = i->second;
				const AABB& aab = content->getWorldAABB();

				RGBAConverter converter = IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder;
				Color::COLOR color32 = converter.packColor(color);

				data->mMin = aab.getMinPoint();
				data->mMax = aab.getMaxPoint();
				data->mColor = color32;
				++data;
				++counting;
			}
			for(TargetSet::const_iterator i = mTargets.begin(); i != mTargets.end(); ++i)
			{
				const IAABBTarget* target = *i;
				for(size_t j = 0; j < target->getAABBCount(); ++j)
				{
					AABB aab = target->getAABB(j);
					Color color = target->getAABBColor(j);

					RGBAConverter converter = IGraphicsResourceManager::getSingleton().getGraphicsConfig().DesiredOrder;
					Color::COLOR color32 = converter.packColor(color);

					data->mMin = aab.getMinPoint();
					data->mMax = aab.getMaxPoint();
					data->mColor = color32;
					++data;
					++counting;
				}
			}

			assert( counting <= mAABBCount );

			mInstanceBuffer->unlock();
			mGeometry.setInstancing(true, (uint16)counting, 1);
			this->setVisible(true);
		}
		else
		{
			//invisible
			this->setVisible(false);
		}
	}


	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void			AABBTransformSet::initialize(IGraphicsUpdater* updater)
	{
		mUpdater = updater;
#if AABB_DYNAMIC
		mUpdater->addForUpdate(this);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AABBTransformSet::addContent(ISpaceContent* content, const Color& color)
	{
		bool result = mContentRegistry.insert( std::make_pair(content,color) ).second;
#if !AABB_DYNAMIC
		if( result )
			mUpdater->addForUpdateOnce(this);
#endif
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AABBTransformSet::removeContent(ISpaceContent* content)
	{
		bool result = mContentRegistry.erase(content) == 1;
#if !AABB_DYNAMIC
		if( result )
			mUpdater->addForUpdateOnce(this);
#endif
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AABBTransformSet::changeColor(ISpaceContent* content, const Color& newColor)
	{
		TargetContentSet::iterator i = mContentRegistry.find(content);
		if( i == mContentRegistry.end() )
			return false;
		
		if( i->second != newColor )
		{
			i->second = newColor;
#if !AABB_DYNAMIC
			mUpdater->addForUpdateOnce(this);
#endif
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AABBTransformSet::addTarget(IAABBTarget* target)
	{
		assert(target);
		bool result = mTargets.insert(target).second;
#if !AABB_DYNAMIC
		if( result )
			mUpdater->addForUpdateOnce(this);
#endif
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AABBTransformSet::removeTarget(IAABBTarget* target)
	{
		assert(target);
		bool result = mTargets.erase(target) == 1;
#if !AABB_DYNAMIC
		if( result )
			mUpdater->addForUpdateOnce(this);
#endif
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			AABBTransformSet::getSize() const
	{
		size_t size = mContentRegistry.size();

		for( TargetSet::const_iterator i = mTargets.begin(); i != mTargets.end(); ++i)
			size += (*i)->getAABBCount();
		return size;
	}

	//////////////////////////////////////////////////////////////////////////
	IVertexBuffer*	AABBTransformSet::getInstanceBuffer() const
	{
		return mInstanceBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			AABBTransformSet::getValidInstanceBufferSize() const
	{
		return this->getSize()*(sizeof(AABB_INSTANCE));
	}

	//////////////////////////////////////////////////////////////////////////
	void			AABBTransformSet::setupRenderResource()
	{
		if( mVertexDecl == NULL )
		{
			HVDECL decl = IGraphicsResourceManager::getSingleton().createVertexDeclaration();
			decl->addElement(0,0,VET_FLOAT3,VU_POSITION,0);
			//decl->addElement(0,(uint16)decl->getVertexSize(0),VET_COLOR,VU_DIFFUSE,0);

			//instance data
			decl->addElement(1,0,VET_FLOAT3,VU_TEXTURE_COORDINATES,0);								//position
			decl->addElement(1,(uint16)decl->getVertexSize(1),VET_FLOAT3,VU_TEXTURE_COORDINATES,1);	//size
			decl->addElement(1,(uint16)decl->getVertexSize(1),VET_COLOR, VU_COLOR, 0);				//color

			mVertexDecl = decl;
		}

		mVertexSource = IVertexSource::create();
		mGeometry.mIndexBuffer = NULL;
		mGeometry.mPrimitiveType = GraphicsGeometry::GPT_LINE_LIST;
		mGeometry.mVertexDecl = mVertexDecl;
		mGeometry.mVertexSource = mVertexSource;
		mGeometry.mVertexStart = 0;
		mGeometry.mVertexCount = AABB::BC_COUNT;

		struct VertexFormat
		{
			Vector3			vertex;
			//Color::COLOR	color;
		};
		
		//vertice for an AABB
		VertexFormat vertice[AABB::BC_COUNT];

		const AABB unit_aab = AABB::UNIT;
		POINT3 corners[AABB::BC_COUNT];
		unit_aab.getAllCorners(corners);
		for( size_t i = 0; i < AABB::BC_COUNT; ++i )
		{
			vertice[i].vertex = corners[i];
			//vertice[i].color = color;
		}

		mVertexBuffer = IGraphicsResourceManager::getSingleton().createVertexBuffer(vertice, sizeof(VertexFormat), sizeof(vertice)/sizeof(VertexFormat) ,IGraphicsBuffer::GBU_STATIC);
#if AABB_DYNAMIC
		mInstanceBuffer = IGraphicsResourceManager::getSingleton().createVertexBuffer(NULL, sizeof(AABB_INSTANCE),mBufferedCount,IGraphicsBuffer::GBU_DYNAMIC_WRITE);
#else
		//note that the AABB instances may not frequently changed (not at least always per-frame)
		//so use DEFAULT instead of DYNAMIC_WRITE
		mInstanceBuffer = IGraphicsResourceManager::getSingleton().createVertexBuffer( sizeof(AABB_INSTANCE),mBufferedCount,IGraphicsBuffer::GBU_DEFAULT);
#endif
		mVertexSource->setSource(0, mVertexBuffer);
		mVertexSource->setSource(1, mInstanceBuffer);

		//12 lines for an AABB
		IIndexBuffer::EIndexType type = IIndexBuffer::IT_16BIT;
		void* temp = BLADE_TMP_ALLOC(24 * IndexBufferHelper::calcIndexSize(type));
		IndexBufferHelper helper(temp, type);
		index_t index = 0;

		//0->1,3,6
		helper[index++] = 0;
		helper[index++] = 1;
		helper[index++] = 0;
		helper[index++] = 3;
		helper[index++] = 0;
		helper[index++] = 6;
		//7->1,4,6
		helper[index++] = 7;
		helper[index++] = 1;
		helper[index++] = 7;
		helper[index++] = 4;
		helper[index++] = 7;
		helper[index++] = 6;
		//5->3,4,6
		helper[index++] = 5;
		helper[index++] = 3;
		helper[index++] = 5;
		helper[index++] = 4;
		helper[index++] = 5;
		helper[index++] = 6;
		//2->1,3,4
		helper[index++] = 2;
		helper[index++] = 1;
		helper[index++] = 2;
		helper[index++] = 3;
		helper[index++] = 2;
		helper[index++] = 4;
		
		mIndexBuffer = IGraphicsResourceManager::getSingleton().createIndexBuffer(temp, IIndexBuffer::IT_16BIT, 24, IGraphicsBuffer::GBU_STATIC);
		BLADE_TMP_FREE(temp);
		mGeometry.mIndexBuffer = mIndexBuffer;
		mGeometry.mIndexStart = 0;
		mGeometry.mIndexCount = 24;
		mGeometry.useIndexBuffer(true);

		//load AABB material
		Material* material = IMaterialManager::getSingleton().getMaterial(BTString("AABB"));
		mMaterial.bind( BLADE_NEW MaterialInstance(material) );		
		mMaterial->load();
	}

}//namespace Blade
