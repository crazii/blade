/********************************************************************
	created:	2017/05/14
	filename: 	ModelToolBase.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelToolBase_h__
#define __Blade_ModelToolBase_h__
#include <utility/BladeContainer.h>
#include <utility/StringHelper.h>
#include <math/Quaternion.h>
#include <interface/IPage.h>
#include <interface/public/graphics/GraphicsDefs.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/logic/ILogicWorld.h>
#include <interface/IWorldEditableManager.h>
#include <databinding/DataSourceWriter.h>
#include <BladeModelEditor.h>

namespace Blade
{
	namespace ModelConfigString
	{
		static const TString SELECTED_MODEL = BXLang("Target Model");
		static const TString APP_FLAG = BXLang("Intersection Flags");
		static const TString RANDOM_ROTATION = BXLang("Random Rotation");
		static const TString RANDOM_SCALE = BXLang("Random Scale");
		static const TString SCALE_AS_SIZE = BXLang("Scale as World Space Size");
		static const TString MIN_SCALE = BXLang("Min Scale");
		static const TString MAX_SCALE = BXLang("Max Scale");
		static const TString DENSITY = BXLang("Model Density");

	}//namespace ConfigString

	class ModelToolBase
	{
	protected:
		ModelToolBase()
			:mAppFlag(0)
		{
			//TODO: use IWorldEditableManager's editable info to enumerate app flags?
			//note: use IWorldEditableManager need to delay after file created.
			for (size_t i = 0; i < IGraphicsService::getSingleton().getGraphicsTypeCount(); ++i)
			{
				IGraphicsType* type = IGraphicsService::getSingleton().getGraphicsType(i);
				//hack!!! TODO: better ways
				if (type->getName() == BTString("Light") || type->getName() == BTString("RenderHelper"))
					continue;
				mIntersectTypes[type->getName()] = true;
				mAppFlag |= type->getAppFlag();
			}
		}
		virtual ~ModelToolBase() {}

		/** @brief init default app flag. this method should be called after app flags set by world */
		void		initAppFlag()
		{
			size_t count = 0;
			for (size_t i = 0; i < IGraphicsService::getSingleton().getGraphicsTypeCount(); ++i)
			{
				IGraphicsType* type = IGraphicsService::getSingleton().getGraphicsType(i);
				if (type->getName() == BTString("Light") || type->getName() == BTString("RenderHelper"))
					continue;
				++count;
			}
			if (mIntersectTypes.size() == count)
				return;
			
			bool updateFlag = mAppFlag == 0;
			for (size_t i = 0; i < IGraphicsService::getSingleton().getGraphicsTypeCount(); ++i)
			{
				IGraphicsType* type = IGraphicsService::getSingleton().getGraphicsType(i);
				if (type->getName() == BTString("Light") || type->getName() == BTString("RenderHelper"))
					continue;

				if (type->getAppFlag() != INVALID_APPFLAG)
				{
					mIntersectTypes[type->getName()] = true;
					if(updateFlag)
						mAppFlag |= type->getAppFlag();
				}
				else
					mIntersectTypes[type->getName()] = false;
			}
		}

		/** @brief  */
		void		updateAppFlag()
		{
			mAppFlag = 0;
			for (AppFlagMap::const_iterator i = mIntersectTypes.begin(); i != mIntersectTypes.end(); ++i)
			{
				if (i->second)
				{
					IGraphicsType* type = IGraphicsService::getSingleton().getGraphicsType(i->first);
					mAppFlag |= type->getAppFlag();
				}
			}
		}


		/** @brief  */
		bool		createModelEntity(IEditorFile* file, const TString& modelFile, const POINT3& pos, bool randomRotation,
			bool randomScale = false, scalar minscale = 0.1f, scalar maxscale = 1.0f)
		{

			//create entity
			const ED_INFO* pInfo = IWorldEditableManager::getInterface().getEditableInfo(ModelEditorConsts::MODEL_EDITABLE_NAME);

			HEDITABLE hEditable = HEDITABLE(BLADE_FACTORY_CREATE(IEditable, pInfo->mClassName));
			Quaternion rotation = Quaternion::IDENTITY;
			Vector3 scale = Vector3::UNIT_ALL;

			//init with rotation.
			//set rotation after init will need to notify paging system, with additional overhead.
			if (randomRotation)
				rotation = Quaternion(Vector3::UNIT_Y, Math::Random(0.0f, Math::TWICE_PI));

			if (randomScale)
				scale *= Math::Random(minscale, maxscale);

			ParamList params;
			params[ModelEditorConsts::ENTITY_FILE_PARAM] = file;
			params[ModelEditorConsts::MODEL_RESOURCE_PARAM] = modelFile;
			params[ModelEditorConsts::ENTITY_POS_PARAM] = pos;
			params[ModelEditorConsts::ENTITY_ROTATION_PARAM] = rotation;
			params[ModelEditorConsts::ENTITY_SCALE_PARAM] = scale;
			
			if (!hEditable->initialize(pInfo->mConfigOnCreation, &params))
				return false;

			return IWorldEditableManager::getInterface().addEditable(hEditable);
		}

		typedef Map<TString, bool>	AppFlagMap;

		/** @brief  */
		template<typename T>
		inline static AppFlagMap T::* getIntersectionTypesAddr()
		{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
			//work around MSVC 14 bugs:
			//https://social.msdn.microsoft.com/Forums/sqlserver/en-US/15529acd-259b-488d-8961-13e834cdc1fe/c-pointer-to-member-nullptr-check-fails-with-multiple-inheritance?forum=vclanguage
			AppFlagMap ModelToolBase::* ptr = &T::mIntersectTypes;
			return ptr;
#else
			return static_cast<AppFlagMap T::*>(&ModelToolBase::mIntersectTypes);
#endif
		}

		AppFlagMap	mIntersectTypes;
		AppFlag		mAppFlag;
	};//class ModelToolBase
	
}//namespace Blade

#endif // __Blade_ModelToolBase_h__