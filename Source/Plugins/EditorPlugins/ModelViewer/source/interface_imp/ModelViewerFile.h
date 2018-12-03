/********************************************************************
	created:	2013/04/09
	filename: 	ModelViewerFile.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelViewerFile_h__
#define __Blade_ModelViewerFile_h__
#include <interface/public/EditorFileBase.h>
#include "ModelViewTrack.h"

namespace Blade
{
	class ModelViewerFile : public EditorFileBase, public IEditorFileUpdater, public Allocatable
	{
	public:
		//limit the model size for view convenience
		static const int MODEL_VIEW_SIZE_LIMIT = 50;
	public:
		ModelViewerFile();
		~ModelViewerFile();

		/************************************************************************/
		/* IEditorFile interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void				openFile(const TString& dir,const TString& name);

		/** @brief  */
		virtual void				saveFile();

		/** @brief  */
		virtual void				newFile(const TString& dir,const TString& name);

		/** @brief  */
		virtual void				closeFile();

		/************************************************************************/
		/* IEditorFileUpdater interface                                                                     */
		/************************************************************************/

		/** @brief  */
		virtual void				update(IEditorFile* file);
		/** @brief setup window related params after file is loaded */
		virtual void				setupWindow(IUIWindow* window);
		/** @brief  */
		virtual void				onActivate(IEditorFile* file, bool active);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		const HEDITABLE&			getEditable() const	{return mModelViewEditable;}
		/** @brief  */
		inline bool					isSkeletonVisible() const {return mSkeletonVisible;}
		/** @brief  */
		inline bool					isSubmeshBoundingVisible() const {return mSubBoundingVisbile;}
		/** @brief  */
		inline void					toggleSkeletonVisible()		{mSkeletonVisible = !mSkeletonVisible;}
		/** @brief  */
		inline void					toggleSubmeshBoundingVisible() {mSubBoundingVisbile = !mSubBoundingVisbile;}

	protected:

		/** @brief change target animation on animation UI item selected */
		void			onHighLightOption(const Event& data);

		TString			mFile;
		HEDITABLE		mModelViewEditable;
		ModelViewTrack* mTrack;
		bool			mSkeletonVisible;
		bool			mSubBoundingVisbile;
	};//class ModelViewerFile
	
}//namespace Blade

#endif //  __Blade_ModelViewerFile_h__