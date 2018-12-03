/********************************************************************
	created:	2011/05/26
	filename: 	LogicWorldManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LogicWorldManager_h__
#define __Blade_LogicWorldManager_h__
#include <interface/public/logic/ILogicWorldManager.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class LogicWorldManager : public ILogicWorldManager, public Singleton<LogicWorldManager>
	{
	public:
		LogicWorldManager();
		virtual ~LogicWorldManager();

		/************************************************************************/
		/* ILogicWorldManager interface                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ILogicWorld*		createWorld(const TString& worldName,const TString& worldType = BTString("Default") );

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			destroyWorld(const TString& worldName);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ILogicWorld*		getWorld(const TString& worldName) const;

	protected:
		typedef TStringMap<ILogicWorld*>	WorldList;


		WorldList	mGameWorlds;
	};
	

}//namespace Blade



#endif // __Blade_LogicWorldManager_h__