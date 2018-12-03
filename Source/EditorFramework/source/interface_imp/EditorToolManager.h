/********************************************************************
	created:	2011/05/07
	filename: 	EditorToolManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorToolManager_h__
#define __Blade_EditorToolManager_h__
#include <interface/IEditorToolManager.h>
#include <Singleton.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class EditorToolManager : public IEditorToolManager , public Singleton<EditorToolManager>
	{
	public:
		EditorToolManager();
		~EditorToolManager();

		/************************************************************************/
		/* IEditorToolManager interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			initialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addTool(IEditorTool* tool);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IEditorTool*	removeTool(const TString& category,const TString& name);

		/*
		@describe for direct access tool
		@param 
		@return 
		*/
		virtual IEditorTool*	getTool(const TString& category,const TString& name) const;

		/*
		@describe for category enumeration
		@param 
		@return 
		*/
		virtual size_t			getCategoryCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getCategoryName(index_t categoryIndex) const;

		/*
		@describe for tool enumeration
		@param 
		@return 
		*/
		virtual size_t			getToolCountInCategory(index_t categoryIndex) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IEditorTool*	getEditorTool(index_t categoryIndex,index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			selectTool(IEditorTool* tool);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IEditorTool*	getSelecctedTool() const;


	protected:
		struct FnToolFinder
		{
			FnToolFinder(IEditorTool* findee)	:mTarget(findee)	{}

			bool	operator()(const IEditorTool* val) const
			{
				return val == mTarget || val->getName() == mTarget->getName();
			}
			IEditorTool* mTarget;
		};

		struct FnNameToolFinder
		{
			FnNameToolFinder(const TString& name)	:mTargetName(name)	{}

			bool	operator()(const IEditorTool* val) const
			{
				return val->getName() == mTargetName;
			}
			const TString& mTargetName;

		private:
			FnNameToolFinder& operator=(const FnNameToolFinder&);
		};

		typedef Vector<IEditorTool*>			EditorToolList;
		//
		class ToolCategory
		{
		public:
			ToolCategory()	{}
			ToolCategory(const TString& name) : mName(name)		{}
			const TString&	getName() const						{return mName;}
			bool	operator<(const ToolCategory& rhs) const	{return FnTStringFastLess().operator()(this->getName(),rhs.getName()); }
			EditorToolList& getToolList() const {return mToolList;}

		protected:
			TString	mName;
			mutable EditorToolList	mToolList;
		};//class ToolCategory

		typedef Set<ToolCategory>				EditorToolCategory;
		typedef Vector<const ToolCategory*>		CategoryList;

		EditorToolCategory	mCategoriedTools;
		CategoryList		mCategoryList;
		IEditorTool*		mSelectedTool;
		bool				mInited;
	};
	

}//namespace Blade



#endif // __Blade_EditorToolManager_h__