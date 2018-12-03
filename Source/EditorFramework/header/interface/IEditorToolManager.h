/********************************************************************
	created:	2011/05/07
	filename: 	IEditorToolManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IEditorToolManager_h__
#define __Blade_IEditorToolManager_h__
#include <interface/InterfaceSingleton.h>
#include <interface/public/IEditorTool.h>
#include <BladeEditor.h>

namespace Blade
{

	class IEditorToolManager : public InterfaceSingleton<IEditorToolManager>
	{
	public:
		virtual ~IEditorToolManager()		{}

		/**
		@describe called by framework on initialization
		@param 
		@return 
		*/
		virtual bool			initialize() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			addTool(IEditorTool* tool) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IEditorTool*	removeTool(const TString& category,const TString& name) = 0;
		inline IEditorTool*		removeTool(IEditorTool* tool)
		{
			IEditorTool* ret = NULL;
			if(tool != NULL)
				ret = this->removeTool(tool->getMannaulCategory(),tool->getName());
			assert(ret == tool);
			return ret;
		}

		/**
		@describe for direct access tool
		@param 
		@return 
		*/
		virtual IEditorTool*	getTool(const TString& category,const TString& name) const = 0;

		/**
		@describe for category enumeration
		@param 
		@return 
		*/
		virtual size_t			getCategoryCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getCategoryName(index_t categoryIndex) const = 0;

		/**
		@describe for tool enumeration
		@param 
		@return 
		*/
		virtual size_t			getToolCountInCategory(index_t categoryIndex) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IEditorTool*	getEditorTool(index_t categoryIndex,index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			selectTool(IEditorTool* tool) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IEditorTool*	getSelecctedTool() const = 0;
	};

	extern template class BLADE_EDITOR_API Factory<IEditorToolManager>;
		

}//namespace Blade



#endif // __Blade_IEditorToolManager_h__