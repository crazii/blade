/********************************************************************
	created:	2013/11/11
	filename: 	EditorConfigManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorConfigManager_h__
#define __Blade_EditorConfigManager_h__
#include <Singleton.h>
#include <utility/DataBinding.h>
#include <interface/IEditorConfigManager.h>

namespace Blade
{
	class EditorConfigManager : public IEditorConfigManager, public Bindable, public Singleton<EditorConfigManager>
	{
	public:
		using Singleton<EditorConfigManager>::getSingleton;
		using Singleton<EditorConfigManager>::getSingletonPtr;
	public:
		EditorConfigManager();
		~EditorConfigManager();
		/************************************************************************/
		/* IEditorConfigManager interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	shutdown();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadConfig(const TString& config);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveConfig(const TString& config);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	addConfig(const HCONFIG& config);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&	getConfig(const TString& name) const;

		/*
		@describe buit-in global config
		@param 
		@return 
		*/
		virtual GEOM_SPACE	getTransformSpace() const;

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		const HCONFIG&	getTransformConfig() const;

	protected:

		/** @brief  */
		void		onConfigChange(const uint32& data);

		struct EditorConfigData : public Bindable
		{
			uint32		mTransformSpace;
			//used by data binding
			bool	operator==(const EditorConfigData& rhs) const
			{
				return std::memcmp((void*)this, (void*)&rhs, sizeof(*this) ) == 0;
			}
		};
		
		EditorConfigData mData;
		HCONFIG			mEditorConfig;	//root of editor config
		HCONFIG			mGlobalConfig;	//sub config of global setting
	};
	
}//namespace Blade

#endif //  __Blade_EditorConfigManager_h__