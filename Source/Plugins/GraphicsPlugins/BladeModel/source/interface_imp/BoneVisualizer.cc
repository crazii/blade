/********************************************************************
	created:	2015/11/16
	filename: 	BoneVisualizer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "BladePCH.h"
#include <interface/IPlatformManager.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IMaterialManager.h>
#include <interface/IRenderQueue.h>
#include "BoneVisualizer.h"
#include "Model.h"
#include "ModelConfigManager.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	BoneVisualizer::BoneVisualizer(const Model* model)
		:mModel(model)
		,mSelectedBone(INVALID_INDEX)
	{
		Material* mat = IMaterialManager::getSingleton().getMaterial(BTString("SimpleShape"));
		mMaterial.bind( BLADE_NEW MaterialInstance(mat) );
		mMaterial->enableColor(true, Color(1,0,1,0));
		mMaterial->load();

		mVertexSource = IVertexSource::create();
		mVertexSource->setSource(0, HVBUFFER::EMPTY);
		mGeometry.mIndexBuffer = NULL;
		mGeometry.mPrimitiveType = GraphicsGeometry::GPT_LINE_LIST;
		mGeometry.mVertexDecl = ModelConfigManager::getSingleton().getVisualizerDeclaration();
		mGeometry.mVertexSource = mVertexSource;
		mGeometry.mVertexStart = 0;
		mGeometry.mVertexCount = 0;
		mGeometry.useIndexBuffer(false);

		mSelectedBoneVisualizer = BLADE_NEW SingleBoneVisualizer(model, this);
		mEnabled = false;
	}

	//////////////////////////////////////////////////////////////////////////
	BoneVisualizer::~BoneVisualizer()
	{
		BLADE_DELETE mSelectedBoneVisualizer;
	}

	/************************************************************************/
	/* IRenderable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const Matrix44&			BoneVisualizer::getWorldTransform() const
	{
		return mModel->getWorldTransform();
	}

	//////////////////////////////////////////////////////////////////////////
	ISpaceContent*			BoneVisualizer::getSpaceContent() const
	{
		return const_cast<Model*>(mModel);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool BoneVisualizer::update()
	{
		if (!mEnabled && mSelectedBone == INVALID_INDEX)
			return false;
		const Skeleton* skeleton = mModel->getSkeleton();
		const DualQuaternion* bonePalette = skeleton->getBlendedTransform();
		const BONE_DATA* boneData = skeleton->getBoneData(0);
		size_t boneCount = mModel->getSkeletonResource()->getBoneCount();

		bool result = mSelectedBone < boneCount;

		if (mEnabled)
		{
			TempVector<Vector3> lines;
			lines.reserve(boneCount * 2);

			for (int i = (int)boneCount - 1; i >= 0; --i)
			{
				const BONE_DATA& bone = boneData[i];
				uint32 index = bone.mIndex;
				uint32 parent = bone.mParent;
				while (parent != uint32(-1))
				{
					IPlatformManager::prefetch<PM_READ>(boneData + parent);
					if (skeleton->isBoneActive(index) /*&& skeleton->isBoneActive(parent)*/)
					{
						DualQuaternion pdq = boneData[parent].mInitialPose * bonePalette[parent];
						DualQuaternion dq = boneData[index].mInitialPose * bonePalette[index];
						Vector3 p = pdq.getTranslation();
						Vector3 t = dq.getTranslation();
						lines.push_back(p);
						lines.push_back(t);
					}

					index = parent;
					parent = boneData[parent].mParent;
				}
			}

			HVBUFFER buffer = mVertexSource->getBuffer(0);
			if (buffer == NULL || buffer->getVertexCount() < lines.size())
			{
				mVertexSource->setSource(0, HVBUFFER::EMPTY);
				buffer.clear();
				buffer = IGraphicsResourceManager::getSingleton().createVertexBuffer(&lines[0], sizeof(Vector3), lines.size(), IGraphicsBuffer::GBU_DYNAMIC_WRITE);
				mVertexSource->setSource(0, buffer);
			}
			else if (lines.size() > 0)
			{
				assert(lines.size() % 2 == 0);
				void* data = buffer->lock(IGraphicsBuffer::GBLF_DISCARDWRITE);
				std::memcpy(data, &lines[0], lines.size() * sizeof(Vector3));
				buffer->unlock();
			}

			mGeometry.mVertexCount = (uint32)lines.size();
			result = lines.size()  > 0;
		}

		if (mSelectedBone < boneCount)
		{
			uint32 index = (uint32)mSelectedBone;
			if (skeleton->isBoneActive(boneData[index].mIndex))
			{
				DualQuaternion dq = boneData[index].mInitialPose * bonePalette[index];
				Vector3 scale = Vector3::UNIT_ALL * mModel->getLocalAABB().getHalfSize().getLength()*0.02f;
				mSelectedBoneVisualizer->setBoneTransform(dq, scale);
			}
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	void			BoneVisualizer::updateRender(IRenderQueue* queue)
	{
		if (this->isEnabled())
			queue->addRenderable(this);
		if (this->getSelectedBone() != INVALID_INDEX)
			queue->addRenderable(mSelectedBoneVisualizer);
	}
	
}//namespace Blade