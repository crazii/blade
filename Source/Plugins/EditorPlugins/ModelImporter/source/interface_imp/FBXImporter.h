/********************************************************************
	created:	2015/09/24
	filename: 	FBXImporter.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_FBXImporter_h__
#define __Blade_FBXImporter_h__
#include <interface/public/IImporter.h>
#include "FBXCollector.h"
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

namespace Blade
{

	class FBXImporter : public IImporter
	{
	public:
		//class name
		static const TString FBX_IMPORTER;
	public:
		FBXImporter();
		~FBXImporter();
		
		/************************************************************************/
		/* IImporter interface                                                                     */
		/************************************************************************/
		/*
		@describe get the factory class name of the importer,
		corresponding to SEditorImporterInfo::mName
		@param
		@return
		*/
		virtual const TString&	getName() const
		{
			return FBX_IMPORTER;
		}

		/* @copydoc IImporter::import  */
		virtual bool	import(const HSTREAM& source, const HSTREAM& dest, const ParamList& params, TStringParam& extraFiles, ProgressNotifier& callback);

	protected:

		/** @brief  */
		void			collectMesh(FbxMesh* fbxMesh);

		/** @brief  */
		bool			collectSkeleton(FbxScene* scene, FbxSkeleton* fbxSkeleton);

		/** @brief  */
		void			collectLight(FbxLight* fbxLight);

		/** @brief  */
		void			collectCamera(FbxCamera* fbxCamera);

		FBXCollector	mCollector;
		uint			mUnnamedIndex;
	};
	
}//namespace Blade
#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif // __Blade_FBXImporter_h__