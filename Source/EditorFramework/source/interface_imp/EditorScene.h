/********************************************************************
	created:	2013/03/09
	filename: 	EditorScene.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorScene_h__
#define __Blade_EditorScene_h__
#include <interface/public/IScene.h>

namespace Blade
{
	class EditorScene : public IScene, public Allocatable
	{
	public:
		static const TString EDITOR_SCENE_TYPE;
	public:
		EditorScene();
		~EditorScene();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data */
		virtual void			postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* IScene interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getSceneType() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			getAvailableElementTypes(ElementTypeList& elemList) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IElement*		createElement(const TString& type);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onAttachToStage(IStage* pstage);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onDetachFromStage(IStage* pstage);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

	};//class EditorScene
	
}//namespace Blade

#endif//__Blade_EditorScene_h__