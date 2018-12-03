/********************************************************************
	created:	2011/06/02
	filename: 	CustomEffect.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "CustomEffect.h"
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IMaterialManager.h>
#include <interface/ISpace.h>
#include <RenderUtility.h>
#include <Element/GraphicsElement.h>
#include <FrameEvents.h>
#include "EffectManager.h"

namespace Blade
{
	HVDECL	CustomEffect::msVertexDeclaration;
	const TString CustomEffect::CUSTOM_EFFECT_TYPE = BTString("CustomEffect");

	//////////////////////////////////////////////////////////////////////////
	CustomEffect::CustomEffect(Material* material, bool _dynamic, bool independent)
		:IGraphicsEffect(CUSTOM_EFFECT_TYPE)
		,mNeedListenMaterial(true)
		,mListeningMaterialLoading(false)
		,mInSpace(false)
	{
		if( msVertexDeclaration == NULL )
		{
			//msVertexDeclaration = IGraphicsResourceManager::getSingleton().createVertexDeclaration();
			//msVertexDeclaration->addElement(0,0,VET_FLOAT3,VU_POSITION,0);
			msVertexDeclaration = RenderUtility::getVertexDeclaration(RenderUtility::BE_POSITION);
		}
		//msVertexDeclaration.getRefCount()->safeIncrement();
		mSpaceFlags = CSF_VIRTUAL;
		mUpdateFlags = CUF_DEFAULT_VISIBLE | CUF_HIDDEN;

		mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
		mGeometry.mVertexDecl = msVertexDeclaration;
		mGeometry.mIndexStart = 0;
		mGeometry.mVertexStart = 0;
		mMaterial.bind( BLADE_NEW MaterialInstance( material ) );

		if (_dynamic)
		{
			mSpaceFlags |= CSF_DYNAMIC;
			IEventManager::getSingleton().addEventHandler(FrameEndEvent::NAME, EventDelegate(this, &CustomEffect::clearImmediateBuffer));
		}
		if(independent)
			mSpaceFlags |= CSF_INDEPENDENT;
	}

	//////////////////////////////////////////////////////////////////////////
	CustomEffect::~CustomEffect()
	{
		if( msVertexDeclaration != NULL )
			msVertexDeclaration.clear();

		if(mListeningMaterialLoading)
			mMaterial->removeListener(this);

		if(this->isDynamic())
			IEventManager::getSingleton().removeEventHandlers(this);
	}

	/************************************************************************/
	/* IGraphicsEffect interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	CustomEffect::onAttach()
	{
		mInSpace = this->getSpace() != NULL;
		GraphicsElement* element = static_cast<GraphicsElement*>( this->getHost() );
		if( !mInSpace )
			return element->getSpace()->addContent(this);
		else
			return this->getSpace() == element->getSpace();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	CustomEffect::onDetach()
	{
		GraphicsElement* element = static_cast<GraphicsElement*>( this->getHost() );
		if( mInSpace )
			return true;
		else
			return element->getSpace()->removeContent(this);
	}

	/************************************************************************/
	/* SpaceContent interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			CustomEffect::updateRender(IRenderQueue* queue)
	{
		if( mMaterial->isLoaded() )
			queue->addRenderable(this);
	}

	/************************************************************************/
	/* IRenderable interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	RenderType&				CustomEffect::getRenderType() const
	{
		return EffectManager::getEffectRenderType();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void					CustomEffect::setImmediateBuffer(const Vector3* vb, size_t vbCount, const uint32* ib, size_t ibCount, bool buildBounding)
	{
		if (buildBounding)
			mWorldAABB.setNull();

		if(mWorldAABB.isNull())
		{
			if( ib != NULL )
			{
				const IndexBufferHelper ibuffer((void*)ib, IIndexBuffer::IT_32BIT);
				for( size_t i = 0; i < ibCount; ++i )
				{
					uint32 index = ibuffer[i];
					mWorldAABB.merge(vb[index]);
				}
			}
			else
			{
				for(size_t i = 0; i < vbCount; ++i)
					mWorldAABB.merge(vb[i]);
			}
		}


		mMaterial->load();

		if (mMaterial->isLoaded())
			this->setVisible(true);
		else if (mNeedListenMaterial)
		{
			mListeningMaterialLoading = true;
			mMaterial->addListener(this);
		}

		if (mVertexBuffer != NULL || mIndexBuffer != NULL)
		{
			if (!this->isDynamic())
			{
				assert(false && "only dynamic effect can setImmediateBuffer for multiple times");
				return;
			}
		}

		IGraphicsResourceManager& manager = IGraphicsResourceManager::getSingleton();
		IGraphicsBuffer::USAGE usage = this->isDynamic() ? IGraphicsBuffer::GBU_DYNAMIC_WRITE : IGraphicsBuffer::GBU_STATIC;

		//TODO: vertex buffer pooling for large quantity of custom effects
		if (mVertexBuffer == NULL || mVertexBuffer->getVertexCount() < vbCount)
			mVertexBuffer = manager.createVertexBuffer(vb, sizeof(Vector3), vbCount, usage);
		else
		{
			void* dest = mVertexBuffer->lock(IGraphicsBuffer::GBLF_DISCARDWRITE);
			std::memcpy(dest, vb, sizeof(Vector3)*vbCount);
			mVertexBuffer->unlock();
		}

		mGeometry.mVertexCount = (uint32)mVertexBuffer->getVertexCount();
		if (mVertexSource == NULL)
		{
			mVertexSource = IVertexSource::create();
			mGeometry.mVertexSource = mVertexSource;
		}
		mVertexSource->setSource(0, mVertexBuffer);
		mGeometry.mVertexCount = (uint32)vbCount;

		if (ib != NULL && ibCount != 0)
		{
			if(mIndexBuffer == NULL || mIndexBuffer->getIndexCount() < ibCount)
				mIndexBuffer = manager.createIndexBuffer(ib, IIndexBuffer::IT_32BIT, ibCount, usage);
			else
			{
				void* dest = mIndexBuffer->lock(IGraphicsBuffer::GBLF_DISCARDWRITE);
				std::memcpy(dest, ib, sizeof(uint32)*ibCount);
				mIndexBuffer->unlock();
			}

			mGeometry.useIndexBuffer(true);
			mGeometry.mIndexCount = (uint32)ibCount;
			mGeometry.mIndexBuffer = mIndexBuffer;
		}
		else
		{
			mGeometry.useIndexBuffer(false);
			mGeometry.mIndexCount = 0;
			mGeometry.mIndexBuffer = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					CustomEffect::clearImmediateBuffer(const Event&)
	{
		if( this->isDynamic() )
			mGeometry.mIndexCount = mGeometry.mVertexCount = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	const HMATERIALINSTANCE&	CustomEffect::setCustomMaterial(Material* material)
	{
		mMaterial.bind( BLADE_NEW MaterialInstance( material ) );
		return mMaterial;
	}

}//namespace Blade
