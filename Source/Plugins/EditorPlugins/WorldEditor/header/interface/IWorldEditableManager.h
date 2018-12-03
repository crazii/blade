/********************************************************************
	created:	2011/04/13
	filename: 	IWorldEditableManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IWorldEditableManager_h__
#define __Blade_IWorldEditableManager_h__
#include <BladeWorldEditor.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/IEditor.h>
#include <interface/public/IEditable.h>
#include <interface/public/logic/ILogicWorld.h>

namespace Blade
{
	class IWorldEditableManager : public IEditableManager, public InterfaceSingleton<IWorldEditableManager>
	{
	public:
		virtual ~IWorldEditableManager()	{}

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual ILogicWorld*	getWorld() const = 0;

	};//class IWorldEditableManager


	extern template class BLADE_WORLDEDITOR_API Factory<IWorldEditableManager>;
	

}//namespace Blade



#endif // __Blade_IWorldEditableManager_h__