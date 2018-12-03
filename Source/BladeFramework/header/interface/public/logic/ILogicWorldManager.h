/********************************************************************
	created:	2011/11/02
	filename: 	ILogicWorldManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ILogicWorldManager_h__
#define __Blade_ILogicWorldManager_h__

#include <interface/InterfaceSingleton.h>
#include <interface/public/logic/ILogicWorld.h>

namespace Blade
{

	class ILogicWorldManager : public InterfaceSingleton<ILogicWorldManager>
	{
	public:

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ILogicWorld*	createWorld(const TString& worldName,const TString& worldType = BTString("Default") ) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			destroyWorld(const TString& worldName) = 0;

		inline bool				destroyWorld(ILogicWorld* world)
		{
			return world == NULL?false:this->destroyWorld( world->getWorldName() );
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ILogicWorld*		getWorld(const TString& worldName) const = 0;


	};//class ILogicWorldManager



	extern template class BLADE_FRAMEWORK_API Factory<ILogicWorldManager>;
	

}//namespace Blade



#endif // __Blade_ILogicWorldManager_h__