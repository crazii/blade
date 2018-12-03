/********************************************************************
	created:	2017/05/14
	filename: 	ModelBrushTool.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "ModelBrushTool.h"
#include <interface/public/graphics/SpaceQuery.h>
#include <interface/public/graphics/IGraphicsCamera.h>
#include <interface/public/graphics/IGraphicsEffectManager.h>
#include <interface/public/ui/IIconManager.h>
#include <interface/IEditorFramework.h>
#include <interface/IModelConfigManager.h>

namespace Blade
{
	const TString	ModelBrushTool::MODEL_BRUSH_TOOL_NAME = BTString("Model Brush Tool");
	static const uint MAX_MODEL_FILE_COUNT = 16;
	static const uint MAX_MODEL_COUNT = 100;
	static const fp32 SCALE_LIMIT_MIN = 0.1f;
	static const fp32 SCALE_LIMIT_MAX = 10.0f;

	//////////////////////////////////////////////////////////////////////////
	ModelBrushTool::ModelBrushTool()
		:EditorBrushTool(MODEL_BRUSH_TOOL_NAME,
			IIconManager::getSingleton().loadSingleIconImage(IEditorFramework::EDITOR_ICON_PATH + BTString("tree.png")),
			HOTKEY(), TString::EMPTY, 25)
		,mDensity(5)
		,mChanceDistribution(NULL)
		,mPainting(false)
		,mRandomRotation(true)
		,mRandomScale(false)
		,mUseScaleAsSize(false)
	{
		mMinScale = 0.1f;
		mMaxScale = 1.0f;

		DataSourceWriter<ModelBrushTool> writer(mToolConfig);
		writer.beginVersion(Version(0, 1));
		{
			writer << ModelConfigString::RANDOM_ROTATION << CUIH_CHECK << CAF_NORMAL
				<< &ModelBrushTool::onConfigChange
				<< &ModelBrushTool::mRandomRotation;

			DataSourceWriter<ModelBrushTool> scaleWriter = (writer << ModelConfigString::RANDOM_SCALE << CUIH_CHECK << CAF_NORMAL << mRandomScale
				<< &ModelBrushTool::onConfigChange
				<< DS_GROUP<ModelBrushTool>(&ModelBrushTool::mRandomScale));
			{
				//TODO:
				//scaleWriter[BTString("TRUE")] << ModelConfigString::SCALE_AS_SIZE << CUIH_CHECK << CAF_NORMAL 
				//	<< &ModelBrushTool::onConfigChange << &ModelBrushTool::mUseScaleAsSize;

				scaleWriter[BTString("TRUE")] << ModelConfigString::MIN_SCALE << ConfigAtom::Range(SCALE_LIMIT_MIN, SCALE_LIMIT_MAX, 0.1f)
					<< &ModelBrushTool::onConfigChange << &ModelBrushTool::mMinScale;

				scaleWriter[BTString("TRUE")] << ModelConfigString::MAX_SCALE << ConfigAtom::Range(SCALE_LIMIT_MIN, SCALE_LIMIT_MAX, 0.1f)
					<< &ModelBrushTool::onConfigChange << &ModelBrushTool::mMaxScale;

				scaleWriter.endVersion();
			}

			writer << ModelConfigString::DENSITY << ConfigAtom::Range(1u, MAX_MODEL_COUNT, 1u, mDensity) << CAF_NORMAL
				<< &ModelBrushTool::onConfigChange
				<< &ModelBrushTool::mDensity;

			writer << ModelConfigString::SELECTED_MODEL << CONFIG_UIHINT(CUIH_IMAGE, ModelConsts::MODEL_ANIMATION_ITEM_HINT) << &ModelBrushTool::onConfigChange
				<< CONFIG_ACCESS(CAF_NORMAL | CAF_COLLECTION_WRITE)
				<< &ModelBrushTool::mModels;

			writer << ModelConfigString::APP_FLAG << CUIH_CHECK << CAF_NORMAL
				<< &ModelBrushTool::onConfigChange
				<< ModelToolBase::getIntersectionTypesAddr<ModelBrushTool>();
		}
		writer.endVersion();

		//default value
		mBrushColor = Color(0, 1, 1);	//cyan
		//mBrushColor = Color(1, 0, 1);	//magenta
		mToolConfig->bindTarget(this);

	}

	//////////////////////////////////////////////////////////////////////////
	ModelBrushTool::~ModelBrushTool()
	{
		if (mChanceDistribution != NULL)
			BLADE_RES_FREE(mChanceDistribution);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ModelBrushTool::onMouseLeftDown(scalar x, scalar y, uint32 /*keyModifier*/, IEditorFile* file)
	{
		assert(file != NULL);

		IGraphicsView* view = file->getView();
		IGraphicsScene* scene = file->getGraphicsScene();
		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		if (view == NULL || scene == NULL || camera == NULL)
			return true;

		if (this->getBrushDecalEffect() == NULL)
			return true;

		Ray ray = camera->getSpaceRayfromViewPoint(x, y);
		RaycastQuery rcq(ray, FLT_MAX, PS_POSITIVE, mAppFlag);
		rcq.setPrecise(true);
		IElement* elem = scene->queryNearestElement(rcq);
		if (elem == NULL)
		{
			this->getBrushDecalEffect()->setVisible(false);
			return true;
		}

		mPainting = true;
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	bool				ModelBrushTool::onMouseLeftUp(scalar /*x*/, scalar /*y*/, uint32 /*keyModifier*/, IEditorFile* /*file*/)
	{
		mPainting = false;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				ModelBrushTool::mainUpdate(scalar x, scalar y, uint32 /*keyModifier*/, scalar /*deltaTime*/, IEditorFile* file)
	{
		if (x < 0 || y < 0 || x > 1 || y > 1)
			return false;

		assert(file != NULL);
		IGraphicsView* view = file->getView();
		IGraphicsScene* scene = file->getGraphicsScene();
		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		if (view == NULL || scene == NULL || camera == NULL)
			return false;

		if (this->getBrushDecalEffect() == NULL)
			return false;

		Ray ray = camera->getSpaceRayfromViewPoint(x, y);
		RaycastQuery rcq(ray, FLT_MAX, PS_POSITIVE, mAppFlag);
		rcq.setPrecise(true);
		IElement* elem = scene->queryNearestElement(rcq);
		if (elem == NULL)
		{
			this->getBrushDecalEffect()->setVisible(false);
			return false;
		}

		scalar distance = rcq[0].distance;
		POINT3 pt = ray.getRayPoint(distance);
		this->getBrushDecalEffect()->setPosition(pt);
		this->getBrushDecalEffect()->setVisible(true);

		if (mModels.empty() || mDensity == 0)
			return false;

		if (!mPainting)
			return false;

		bool modified = false;
		Vector3 size = this->getBrushDecalEffect()->getAABB().getHalfSize();
		size.y = 0;
		pt -= size;
		size *= 2;

//we can use Poisson brush to control the distribution, so don't need distance limit
#define ENABLE_DISTANCE_LIMIT 0

#if ENABLE_DISTANCE_LIMIT
		TempVector<POINT3> existingPoses;
#endif

		size_t existingModels = 0;
		{
			TPointerParam<IEntity> existingQuery;
			//use paging to get existing models since some models maybe unloaded, they are not in graphics spaces, but still in paging.
			existingModels = file->getStage()->getPagingManager()->getEntities(this->getBrushDecalEffect()->getAABB(), ModelConsts::MODEL_ELEMENT_TYPE, existingQuery, mDensity);
#if ENABLE_DISTANCE_LIMIT
			if (existingModels < mDensity)
			{
				existingPoses.reserve(existingModels + mDensity);
				for (size_t i = 0; i < existingModels; ++i)
					existingPoses.push_back(existingQuery.at(i)->getInterface(IID_GRAPHICS())->getStaticPosition());
			}
#endif
		}

#if ENABLE_DISTANCE_LIMIT
		scalar distanceThreshold = std::min<scalar>(std::min<scalar>(size.x / mDensity, size.z / mDensity), 5.0f);
		int retryCount = 0;
		const int MAX_RETRY_COUNT = 64;
#endif

		size_t count = mBrushData.getBrushSize()*mBrushData.getBrushSize();
		for (size_t i = existingModels; i < mDensity; ++i)
		{
			//need more random numbers to increase resolution, because max value in probability distribution may be larger than RAND_MAX
			int64 max = 0;
			int64 prob = 0;
			while (max < mChanceDistribution[count - 1])
			{
				prob += std::rand();
				max += (int64)RAND_MAX + 1;	//[0, RAND_MAX]
			}
			//normalize & remap
			prob = (int64)(((float)prob / (float)max) * mChanceDistribution[count - 1]);
			//find target position
			const int64* base = std::lower_bound(mChanceDistribution, mChanceDistribution + count, prob);
			index_t index = index_t((base - mChanceDistribution));
			scalar lx = (scalar)(index % mBrushData.getBrushSize()) / (scalar)mBrushData.getBrushSize();
			scalar lz = (scalar)(index / mBrushData.getBrushSize()) / (scalar)mBrushData.getBrushSize();

			Vector3 pos = pt + size*Vector3::UNIT_X*lx + size*Vector3::UNIT_Z*lz;
			pos.y = 1000000;

			Ray down(pos, Vector3::NEGATIVE_UNIT_Y);
			RaycastQuery down_rcq(down, FLT_MAX, PS_POSITIVE, mAppFlag);
			down_rcq.setPrecise(true);
			if (scene->queryNearestElement(down_rcq) == NULL)
				continue;

			pos = down.getRayPoint(down_rcq[0].distance);

#if ENABLE_DISTANCE_LIMIT
			{
				bool retry = false;
				for (size_t j = 0; !retry && j < existingPoses.size(); ++j)
				{
					if (pos.getDistance(existingPoses[j]) < distanceThreshold)
						retry = true;
				}

				if (retry)
				{
					if (++retryCount < MAX_RETRY_COUNT)
					{
						--i;
						continue;
					}
					else
						retryCount = 0;
				}
			}
			existingPoses.push_back(pos);
#endif

			//place model to pos
			index = (index_t)std::rand() % mModels.size();
			const TString& model = mModels[index];

			if (this->ModelToolBase::createModelEntity(file, model, pos, mRandomRotation, mRandomScale, mMinScale, mMaxScale))
				modified = true;
		}
		if (modified)
			file->setModified();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelBrushTool::onFileSwitch(const IEditorFile* prevFile, const IEditorFile* currentFile)
	{
		EditorBrushTool::onFileSwitch(prevFile, currentFile);

		if (currentFile == NULL)
			return;
		ILogicWorld* world = currentFile->getWorld();
		if (world == NULL)
			return;

		IStage* stage = currentFile->getStage();
		if (stage == NULL)
			return;

		this->ModelToolBase::updateAppFlag();
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelBrushTool::onEnable(bool bEnabled)
	{
		//delay init app flag on current world
		if (bEnabled)
		{
			this->ModelToolBase::initAppFlag();
		}

		EditorBrushTool::onEnable(bEnabled);
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelBrushTool::onSelection()
	{
		EditorBrushTool::onSelection();
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelBrushTool::onBrushDataChanged()
	{
		if (mChanceDistribution != NULL)
		{
			BLADE_RES_FREE(mChanceDistribution);
			mChanceDistribution = NULL;
		}

		const BrushData& brushData = mBrushData;

		size_t count = brushData.getBrushSize()*brushData.getBrushSize();
		mChanceDistribution = BLADE_RES_ALLOCT(int64, count);
		mChanceDistribution[0] = brushData.getBrushData()[0];
		for (size_t i = 1; i < count; ++i)
			mChanceDistribution[i] = brushData.getBrushData()[i] + mChanceDistribution[i - 1];
	}

	//////////////////////////////////////////////////////////////////////////
	void ModelBrushTool::onConfigChange(void * data)
	{
		if (data == &mModels)
		{
			if (mModels.size() > MAX_MODEL_FILE_COUNT)	//TODO: use input range to limit size?
				mModels.resize(MAX_MODEL_FILE_COUNT);
		}
		else if (data == &mIntersectTypes)
		{
			this->ModelToolBase::updateAppFlag();
		}
		else if (data == &mMinScale || data == &mMaxScale)
		{
			fp32& scale = *((fp32*)data);
			scale = Math::Clamp(scale, SCALE_LIMIT_MIN, SCALE_LIMIT_MAX);
		}
	}
	
}//namespace Blade