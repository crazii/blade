/********************************************************************
	created:	2013/04/09
	filename: 	ModelElement.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelElement_h__
#define __Blade_ModelElement_h__
#include <Element/GraphicsElement.h>
#include <interface/ModelState.h>
#include <utility/BladeContainer.h>
#include <BladeModel.h>
#include "Model.h"

namespace Blade
{
#define MODEL_STATIC_BATCHING 0	//TODO: test

	class ModelElement : public GraphicsElement, public IModel, public Allocatable
	{
	public:
		ModelElement();
		~ModelElement();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called before saving data  */
		virtual void			prepareSave();
		/** @brief  */
		virtual void			postSave();
		/** @brief called after loading data */
		virtual void			postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual Interface*		getInterface(InterfaceName type);

		/** @brief  */
		virtual size_t			initParallelStates();

		/** @brief  */
		virtual void			onParallelStateChange(const IParaState& data);

		/** @brief  */
		virtual void			getResourceParams(ParamList& outParams, TString& outResType, TString& outSerializer);

		/** @brief  */
		virtual void			setResourcePath(const TString& resourcePath)
		{
			if (mOriginalResource != resourcePath)
			{
				if (mDesc->mResourcePath == resourcePath)
					return;
				mOriginalResource = resourcePath;
			}
			if (MODEL_STATIC_BATCHING && !this->isDynamic())
				mDesc->mResourcePath = mOriginalResource + BTString(".static_") + TStringHelper::fromHex((uintptr_t)this);
			else
				mDesc->mResourcePath = mOriginalResource;
		}

		/************************************************************************/
		/* ElementBase interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void onResourceUnload();

		/************************************************************************/
		/* GraphicsElement interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			onInitialize();

		/** @brief  */
		virtual bool setDynamic(index_t, const bool& _dynamic);

		/************************************************************************/
		/* IModel interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void			getSubMeshNameList(TStringParam& names) const
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			ScopedLock lock(mSyncLock);
			for (size_t i = 0; i < mModel->getSubMeshCount(); ++i)
			{
				const TString& name = mModel->getSubMeshName(i);
				names.push_back(name);
			}
		}
		/** @brief  */
		virtual void			setSubMeshVisible(const TString& name, bool visible)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			ScopedLock lock(mSyncLock);
			mModel->setSubMeshVisible(name, visible);
		}

		/** @brief  */
		virtual void			getAnimationList(TStringParam& animations) const
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			ScopedLock lock(mSyncLock);

			size_t animCount = mModel->getAnimtaionCount();
			if (animCount == 0)
				return;

			typedef TempVector<const tchar*> StringPointerList;
			StringPointerList stringList;
			for (size_t i = 0; i < animCount; ++i)
			{
				const tchar* name = mModel->getAnimtaionName(i);
				stringList.push_back(name);
			}

			if (stringList.size() != 0)
			{
				//sort by alphabet order
				std::sort(stringList.begin(), stringList.end(), FnTCharNCLess());
				animations.reserve(stringList.size());

				for (size_t i = 0; i < stringList.size(); ++i)
					animations.push_back(TString(stringList[i]));
			}
			//mPSInAnimation = animations[0];
		}
		/** @brief  */
		virtual void			getBoneList(TStringParam& bones) const
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			ScopedLock lock(mSyncLock);

			const ISkeletonResource* res = mModel->getSkeletonResource();
			if (res != NULL)
			{
				bones.reserve(bones.size() + res->getBoneCount());
				for (size_t i = 0; i < res->getBoneCount(); ++i)
				{
					const tchar* name = res->getBoneName(res->getBoneData()[i].mIndex);
					bones.push_back(TString(name));
				}
			}
		}
		/** @brief  */
		virtual bool			setAnimation(const TString& animName)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			ScopedLock lock(mSyncLock);
			return mModel->setSkinnedAnimation(animName);
		}
		/** @brief  */
		virtual void			setSelectedBone(const TString& boneName)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			ScopedLock lock(mSyncLock);
			mModel->setSelectedBone(boneName);
		}

	protected:
		/** @brief  */
		void			applyMeshConfig();
		/** @brief  */
		void			initMeshConfig(bool force, bool reset = false);

		Model*			mModel;
		TString			mOriginalResource;
		mutable Lock	mSyncLock;

		//note: use standalone config object for model's sub mesh
		//this is because on serialization, sub mesh doesn't exist yet. sub mesh only exist after loading mesh
		//(ModelElement dynamically load its own resource after stage/page/entity/element serialization)
		struct SubMeshConfig : public Bindable
		{
			TString		mName;
			bool		mVisible;

			/** @brief full comparison helper. and used for data binding compare */
			inline bool operator==(const SubMeshConfig& rhs) const
			{
				return mName == rhs.mName && mVisible == rhs.mVisible;
			}

			//data binding. TODO add material instance data. i.e. per instance shader constant config
			/** @brief  */
			inline const bool& isVisible(index_t) const { return mVisible; }
			/** @brief  */
			inline bool		setVisible(index_t, const bool& visible) { mVisible = visible; return true; }

			inline const TString& getName(index_t) const { return mName; }

			friend class ModelPlugin;
		};

		//finder helper
		struct FnSubMeshConfigFinder : public NonAssignable
		{
			const TString& nameRef;

			FnSubMeshConfigFinder(const TString& name) :nameRef(name) {}

			inline bool operator()(const SubMeshConfig& target) const
			{
				return target.mName == nameRef;
			}
		};

		typedef List<SubMeshConfig> SubMeshConfigList;
		SubMeshConfigList	mMeshConfig;

		//UI notify
		const TString&	getSkeleton(index_t) const { return mModel != NULL ? mModel->getSkeletonFile() : TString::EMPTY; }
		//config & serialization for sub meshes
		const SubMeshConfigList& getSubMeshes(index_t) const { return mMeshConfig; }
		void onSubMeshConfigChanged(void* data);

		ParaTString			mPSInAnimation;

		friend class ModelPlugin;
	};//class ModelElement
	
}//namespace Blade

#endif //  __Blade_ModelElement_h__