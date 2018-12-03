/********************************************************************
	created:	2011/05/13
	filename: 	EditorLogic.h
	author:		Crazii
	purpose:	default editor logic bound to IViewportWindow
*********************************************************************/
#ifndef __Blade_EditorLogic_h__
#define __Blade_EditorLogic_h__
#include <interface/public/IEditorLogic.h>
#include <interface/public/ui/IUIService.h>
#include <interface/public/IEditorFile.h>

namespace Blade
{
	class EditorLogic : public IEditorLogic, public IMouse::IListener, public Allocatable
	{
	public:
		static const TString DEFAULT_TYPE;
	public:
		EditorLogic();
		~EditorLogic();

		/************************************************************************/
		/* IEditorLogic interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getName()	{return mName;}

		/*
		@describe
		@param
		@return
		*/
		virtual void	onAddToScene(IStage* stage,ILogicScene* scene);

		/*
		@describe
		@param
		@return
		*/
		virtual void	onRemove();

		/*
		@describe task running (asynchronous)
		@param
		@return
		*/
		virtual void	onRun();

		/*
		@describe task update (asynchronous)
		@param
		@return
		*/
		virtual void	onUpdate();

		/*
		@describe  main loop update (synchronous)
		@param 
		@return 
		*/
		virtual void	onMainLoopUpdate();

		/************************************************************************/
		/* IEditorLogic interface                                                                     */
		/************************************************************************/
		/** @brief set up window for a new file. i.e. add tool bars */
		virtual bool	initalize(IEditorFile* file, IUIWindow* window);

		/** @brief  */
		virtual IEditorFile*	getFile() const		{return mTargetFile;}


		/************************************************************************/
		/* IMouse::Listener interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	onMouseButtonDown(MSBUTTON button);

		/** @brief  */
		virtual bool	onMouseButtonUp(MSBUTTON button);

		/** @brief  */
		virtual void	onMouseButtonClick(MSBUTTON button);

		/** @brief  */
		virtual void	onMouseButtonDBClick(MSBUTTON button);


	protected:

		/** @brief  */
		void	onCameraStateChange(const Event& data);

		TString			mName;
		IEditorFile*	mTargetFile;
		POINT2I			mMousePos;
		bool			mToolNeedUpdate;
	};

}//namespace Blade



#endif // __Blade_EditorLogic_h__