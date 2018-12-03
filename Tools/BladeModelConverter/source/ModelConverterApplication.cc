/********************************************************************
	created:	2015/09/23
	filename: 	ModelConverterApplication.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <ModelConverterApplication.h>
#include <utility/StringHelper.h>
#include <utility/StringList.h>
#include <interface/IFramework.h>
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <BladeEditor.h>

#include <interface/public/ImporterParams.h>
#include <interface/IImporterManager.h>
#include <interface/IModelResource.h>
#include <interface/ISkeletonResource.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	bool	ModelConverterApplication::run(bool /*bModal*/)
	{
		const TStringList& files = ICmdParser::getSingleton().getFileList();
		const TString& output = ICmdParser::getSingleton().getOption(BTString("output"))->getValue();
		bool convertSkeleton = ICmdParser::getSingleton().getOption(BTString("skeleton"))->getValue();
		bool convertMesh = ICmdParser::getSingleton().getOption(BTString("mesh"))->getValue();

		if( !convertMesh && !convertSkeleton )
		{
			BLADE_LOGU(ToolLog::Warning, BTString("skeleton & mesh are both disabled, not ouptut, exit."));
			return true;
		}

		if( output.empty() )
		{
			BLADE_LOGU(ToolLog::Error, BTString("no output file specified."));
			return false;
		}

		if(files.size() == 0)
		{
			BLADE_LOGU(ToolLog::Warning, BTString("no input file, exit."));
			return true;
		}

		HSTREAM target;
		if( convertMesh )
		{
			target = IResourceManager::getSingleton().openStream(output, true);
			if( target == NULL )
			{
				BLADE_LOGU(ToolLog::Error, BTString("unable to open output file: ") << output);
				return false;
			}
		}

		TString targetPrefix, targetExt;
		{
			TStringHelper::getFileInfo(output, targetPrefix, targetExt);
			TStringHelper::toLowerCase(targetExt);
		}
		FormtMap::const_iterator iter = mFormatMap.find(targetExt);
		if( iter == mFormatMap.end() )
		{
			BLADE_LOGU(ToolLog::Error, BTString("unrecgnized format: ") << targetExt << BTString(" for output file: ") << output);
			return false;
		}

		ParamList params;
		params[IMPORT_MODEL_ENABLE_SKELETON] = convertSkeleton;
		params[IMPORT_MODEL_ENABLE_MESH] = convertMesh;
		params[IMPORT_MODEL_EXTERNAL_TEXTURE] = false;
		params[IMPORT_MODEL_SKELETON_FULLPATH] = false;
		params[IMPORT_MODEL_PATH] = output;

		//single conversion mode
		if( files.size() == 1 )
		{
			const TString& file = files[0];

			TString name,srcExt;
			TStringHelper::getFileInfo(file, name, srcExt);		
			TStringHelper::toLowerCase(srcExt);

			SourceImporterMap::const_iterator j = iter->second.find(srcExt);
			if( j == iter->second.end() )
			{
				BLADE_LOGU(ToolLog::Error, BTString("unrecgnized format: ") << srcExt << BTString(" for input file: ") << file);
				return false;
			}

			HSTREAM stream = IResourceManager::getSingleton().loadStream(file);
			if( stream == NULL )
			{
				BLADE_LOGU(ToolLog::Error, BTString("unable to open input file.") << file);
				return false;
			}

			Handle<IImporter> importer( BLADE_FACTORY_CREATE(IImporter, j->second) );

			TStringParam extraFiles;
			ProgressNotifier dummy;
			if( !importer->import(stream, target, params, extraFiles, dummy) )
			{
				BLADE_LOGU(ToolLog::Error, BTString("convert mesh file failed. source:") << file << BTString(" target: ") << output);
				return false;
			}
			return true;
		}

		//multiple input files, convert & merge
		TempVector<TString>	meshFiles(files.size());	//converted stream cache
		TempVector<TString> skeletonFiles(files.size());

		for(size_t i = 0; i < files.size(); ++i)
		{
			const TString& file = files[i];
			TString name,srcExt;
			TStringHelper::getFileInfo(file, name, srcExt);
			TStringHelper::toLowerCase(srcExt);

			SourceImporterMap::const_iterator j = iter->second.find(srcExt);
			if( j == iter->second.end() )
			{
				BLADE_LOGU(ToolLog::Error, BTString("unrecgnized format: ") << srcExt << BTString(" for input file: ") << file);
				return false;
			}

			Handle<IImporter> importer( BLADE_FACTORY_CREATE(IImporter, j->second) );

			TStringParam extraFiles;
			ProgressNotifier dummy;
			HSTREAM stream = IResourceManager::getSingleton().loadStream(file);
			if( stream == NULL )
			{
				BLADE_LOGU(ToolLog::Error, BTString("unable to open input file: ") << file);
				return false;
			}

			//create memory stream and merge
			HSTREAM dest = IResourceManager::getSingleton().openStream(BTString("memory:/ModelConverter/") + file, true);
			if( dest == NULL )
			{
				BLADE_LOGU(ToolLog::Error, BTString("unable to create memory stream."));
				return false;
			}

			params[IMPORT_MODEL_PATH] = dest->getName();
			if( !importer->import(stream, dest, params, extraFiles, dummy) )
			{
				BLADE_LOGU(ToolLog::Error, BTString("convert mesh file failed. source:") << file << BTString(" target: ") << output);
				return false;
			}

			meshFiles[i] = dest->getName();
			if( extraFiles.size() != 0 )
				skeletonFiles[i] = extraFiles[0];
		}

		//conversion done, merge:
		if( convertMesh )
		{
			//merge mesh
			TempVector<HRESOURCE> loadedMesh(meshFiles.size());
			IModelResource* mergeTarget = NULL;
			for(size_t i = 0; i < meshFiles.size(); ++i)
			{
				HRESOURCE hRes = IResourceManager::getSingleton().loadResourceSync(meshFiles[i]);
				loadedMesh[i] = hRes;
				IModelResource* res = static_cast<IModelResource*>(hRes);
				if( i == 0)
				{
					mergeTarget = res;
					continue;
				}
				assert(mergeTarget != NULL && i > 0);
				for(size_t j = 0; j < res->getSubMeshCount(); ++j)
				{
					const IModelResource::MESHDATA& data = res->getSubMeshData(j);
					//skip merge if sub mesh of the same name exist
					if( mergeTarget->findSubMeshData(data.mName) == INVALID_INDEX )
						mergeTarget->addSubMesh(data);
					else
						BLADE_LOGU(ILog::Warning, BTString("skip existing sub mesh:") << data.mName);
				}
			}

			if( !IResourceManager::getSingleton().saveResourceSync(loadedMesh[0], output, true) )
			{
				//leave resource not unloaded, for tools this is OK
				BLADE_LOGU(ToolLog::Error, BTString("unable to write output mesh file: ") << output);
				return false;
			}
			for(size_t i = 0; i < loadedMesh.size(); ++i)
				IResourceManager::getSingleton().unloadResource(loadedMesh[i]);
		}

		if( !convertSkeleton )
		{
			assert(convertMesh);
			return true;
		}

		//merge skeleton & animations
		if( skeletonFiles.size() <= 1 || skeletonFiles.size() != files.size() )
		{
			assert(false);
			return false;
		}

		TStringMap<uint> suffixMap;

		TempVector<HRESOURCE> loadedSkeletons(skeletonFiles.size());
		ISkeletonResource* mergeTarget = NULL;
		for(size_t i = 0; i < skeletonFiles.size(); ++i)
		{
			HRESOURCE hRes = IResourceManager::getSingleton().loadResourceSync(skeletonFiles[i]);
			loadedSkeletons[i] = hRes;
			ISkeletonResource* res = static_cast<ISkeletonResource*>(hRes);
			TString singleName;

			//use animation file name as animation name if animation file contains only one animation
			if( res->getAnimationCount() == 1 )
			{
				TString filedir, filename, filebase, fileext;
				TStringHelper::getFilePathInfo(files[i], filedir, filename, filebase, fileext);
				singleName = filebase;
			}

			if( i == 0 )
			{
				//use first resource as merge data 
				mergeTarget = hRes;
				if( res->getAnimationCount() == 1 )
				{
					TString oldName;
					float dummyDuration;
					uint32 dummyFlag;
					res->getAnimationData(0, oldName, dummyDuration, dummyFlag);
					res->setAnimationInfo(oldName, singleName);
				}
				continue;
			}

			assert( mergeTarget != NULL && i > 0 );
			if( res->getBoneCount() != mergeTarget->getBoneCount() )
				BLADE_LOGU(ToolLog::Warning, BTString("skeletons have different bone count, try merge bones."));
			//verify bone names, and reorder bones
			TStringParam boneNames;
			bool reorderOK = false;
			if( mergeTarget->getBoneCount() >= res->getBoneCount() )
				reorderOK = res->reorderBones(*mergeTarget);
			else
				reorderOK = mergeTarget->reorderBones(*res);

			if( !reorderOK)
			{
				BLADE_LOGU(ToolLog::Error, BTString("merge/reorder bone failed for skeleton file: ")
					<< skeletonFiles[0] << BTString(" & ") << skeletonFiles[i]);
			}

			size_t boneCount = mergeTarget->getBoneCount();
			size_t animCount = res->getAnimationCount();

			{
				TempVector<IK_CONSTRAINTS> constraints(boneCount);
				for(size_t j = 0; j < boneCount; ++j)
					constraints[j] = res->getBoneData()[j].mConstraints;
				mergeTarget->setIKContraints(&constraints[0], boneCount);
			}

			for(size_t j = 0; j < animCount; ++j)
			{
				bool result = false;
				TempVector<uint16> boneKeyCount(boneCount);
				
				if( res->getAnimtaionBoneKeyCount(j, &boneKeyCount[0]) )
				{
					size_t totalFrames = 0;
					for(size_t k = 0; k < boneCount; ++k)
						totalFrames += boneKeyCount[k];
					TempVector<KeyFrame> frames(totalFrames);
					TString name;
					float duration = 0;
					uint32 flags = 0;
					if( res->getAnimationData(j, name, duration, flags, &frames[0]) )
					{
						//use animation file name as animation name if animation file contains only one animation
						if( animCount == 1 )
							name = singleName;

						//test to see if the animation exists
						if( mergeTarget->setAnimationInfo(name, name) )
						{
							BLADE_LOG(Warning, BTString("animation '") << name << BTString("' already exist in ") << skeletonFiles[0]
							<< BTString(". while merge source file, add suffix.") << skeletonFiles[i] );

							TString newName = name + TStringHelper::fromUInt(suffixMap[name]++);
							//test if suffix still in conflict
							while( mergeTarget->setAnimationInfo(newName, newName) )
								newName = name + TStringHelper::fromUInt(suffixMap[name]++);

							name = newName;
						}

						mergeTarget->addAnimation(name, duration, flags, &frames[0], &boneKeyCount[0]);
						result = true;
					}
				}

				if( !result )
				{
					BLADE_LOGU(ToolLog::Error, BTString("unable to merge skeleton file: ") << skeletonFiles[i]);
					return false;
				}
			}//for each animation

		}//for each skeleton file

		//write skeleton file
		assert(loadedSkeletons[0] != NULL);
		TString skeleonOutput = targetPrefix+BTString(".bls");
		if( !IResourceManager::getSingleton().saveResourceSync(loadedSkeletons[0], skeleonOutput, true) )
		{
			BLADE_LOGU(ToolLog::Error, BTString("unable to write output skeleton file: ") << skeleonOutput);
			return false;
		}
		for(size_t i = 0; i < loadedSkeletons.size(); ++i)
			IResourceManager::getSingleton().unloadResource(loadedSkeletons[i]);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ModelConverterApplication::doInitialize()
	{
		//build conversion map
		size_t count = IImporterManager::getSingleton().getImporterCount();
		for(size_t i = 0; i < count; ++i)
		{
			const SEditorImporterInfo* impInfo = IImporterManager::getSingleton().getImporterInfo(i);
			if( impInfo->mTargetExt.compare_no_case( ModelConsts::MODEL_FILE_EXT ) )
				continue;
			for(size_t j = 0 ; j < impInfo->mExtensions.size(); ++j)
			{
				const TString& ext = impInfo->mExtensions[j];
				mFormatMap[TStringHelper::getLowerCase(impInfo->mTargetExt)][TStringHelper::getLowerCase(ext)] = impInfo->mName;
			}
		}
		//TODO: exporters: convert blm to other format

		IFramework& fm = IFramework::getSingleton();
		fm.addSubSystem(BTString("BladeGraphicsSystem"));
		fm.addSubSystem(BTString("BladeWindowSystem"));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ModelConverterApplication::initPlugins()
	{
		//need editor frame work to access IImporterManager
		//ModelImporter also depend on it
		initializeEditorFramework();

		ParamList params;
		params[BTString("BladeDevice")] = true;
		params[BTString("BladeWindow")] = true;
		params[BTString("BladeImageFI")] = true;
		params[BTString("BladeGraphics")] = true;
		params[BTString("BladeModel")] = true;
		params[BTString("ModelImporter")] = true;
		//UI system for configs if needed
		params[BTString("BladeUI")] = true;
		params[BTString("BladeUIPlugin")] = true;
		IPlatformManager::getSingleton().initialize(params, IResourceManager::getSingleton().schemePathToRealPath(BTString("plugins:")));
	}
	
}//namespace Blade
