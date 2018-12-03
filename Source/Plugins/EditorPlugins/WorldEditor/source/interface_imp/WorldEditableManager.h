/********************************************************************
	created:	2011/04/13
	filename: 	WorldEditableManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_WorldEditableManager_h__
#define __Blade_WorldEditableManager_h__
#include <interface/IWorldEditableManager.h>
#include <utility/BladeContainer.h>
#include "WorldEditorCommand.h"

namespace Blade
{
	typedef struct  SWorldEditableInfo : public Allocatable
	{
		ED_INFO mInfo;
		size_t	mInstanceCount;
	}WED_INFO;

	class FnEdInfoLess
	{
	public:
		bool	operator()(const WED_INFO* left,const WED_INFO* right) const
		{
			return left->mInfo.mEditableName < right->mInfo.mEditableName;
		}
	};//class FnEdInfoLess


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	class WorldEditableManager :  public IEditor, public IWorldEditableManager, public Singleton<WorldEditableManager>
	{
	public:
		using Singleton<WorldEditableManager>::getSingleton;
		using Singleton<WorldEditableManager>::getSingletonPtr;

	public:
		WorldEditableManager();
		~WorldEditableManager();

		/************************************************************************/
		/* IEditor interface                                                                     */
		/************************************************************************/

		/*
		@describe
		@param
		@return
		*/
		virtual void			initEditor();

		/*
		@describe
		@param
		@return
		*/
		virtual void			shutdownEditor();

		/************************************************************************/
		/* IEditableManager interface                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			registerEditableInfo(const ED_INFO& info);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			unregisterEditableInfo(const TString& editableName);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const ED_INFO*	getEditableInfo(const TString& editableName) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addEditable(const HEDITABLE&  pEditable);

		/**
		@describe rename the target entity's name of the editable
		@param
		@return
		*/
		virtual bool			renameEditable(IEditable* editable, const TString& newName);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IEditable*		getEditable(const TString& InstanceName) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeEditable(const TString& InstanceName);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isEditablesModified() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			saveAllEditable() const;

		/*
		@describe 
		@param 
		@return 
		*/
		IEditorFile*			getEditorFile() const;

		/************************************************************************/
		/* IWorldEditableManager                                                                    */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual ILogicWorld*	getWorld() const;


		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		WED_INFO*				getWorldEditableInfo(const TString& editableName);

		/*
		@describe 
		@param 
		@return 
		*/
		bool					setEditorFile(IEditorFile* file);

	protected:
		/*
		@describe 
		@param 
		@return 
		*/
		void					clearAll();

	protected:
		typedef Set<WED_INFO*,FnEdInfoLess>		WorldEdInfo;
		typedef Set<HEDITABLE,FnEditableLess>	WorldEditableSet;

		WorldEdInfo			mEdInfo;
		WorldEditableSet	mEditableSet;
		IEditorFile*		mEditorFile;

		WorldObjectCreateCommand		mCreateCmd;
		WorldObjectGlobalConfigCommand	mConfigCmd;
	};

	

}//namespace Blade




#endif // __Blade_WorldEditableManager_h__