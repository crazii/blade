/********************************************************************
	created:	2010/04/30
	filename: 	IGraphicsUpdater.h
	author:		Crazii
	purpose:	
	log:		2012/11/20 rename from IGraphicsStateQueue to IGraphicsUpdater
*********************************************************************/
#ifndef __Blade_IGraphicsUpdater_h__
#define __Blade_IGraphicsUpdater_h__
#include <interface/IParaState.h>
#include <interface/InterfaceSingleton.h>
#include <BladeGraphics.h>
#include <Handle.h>
#include <utility/Mask.h>

namespace Blade
{
	typedef enum ESynchronizingState
	{
		//main loop (main sync update)
		SS_MAIN_UPDATE = 0,

		//on run async state
		SS_PRE_CULLING,
		SS_POST_CULLING,
		SS_PRE_RENDER,
		SS_POST_RENDER,	//for none-render related updates, it's better to update after render

		//on update async state
		SS_ASYNC_UPDATE,
		//add new state here

		//
		SS_FIRST_STATE = SS_PRE_CULLING,
		SS_FINAL_STATE = SS_ASYNC_UPDATE,//this may need change

		SS_TOTAL_COUNT,
	}SSTATE;

	class BLADE_GRAPHICS_API IGraphicsUpdatable
	{
	public:
		virtual ~IGraphicsUpdatable()	{}

		/** @brief  */
		Mask getStateMask() const	{return mUpdateStateMask;}
		/** @brief  */
		void stateUpdate(SSTATE state)
		{
#if BLADE_DEBUG
			if( !mUpdateStateMask.checkBitAtIndex(state) )
			{
				assert(false);
				return;
			}
#endif
			this->update(state);
		}
	protected:
		virtual void update(SSTATE state) = 0;

		Mask mUpdateStateMask;
	};//class IGraphicsUpdatable

	class BLADE_GRAPHICS_API IGraphicsUpdater
	{
	public:
		virtual ~IGraphicsUpdater()		{}

		/** @brief  */
		virtual bool addForUpdateOnce(IGraphicsUpdatable* updatable) = 0;

		/** @brief  */
		virtual bool addForUpdate(IGraphicsUpdatable* updatable) = 0;

		/** @brief  */
		virtual bool removeFromUpdate(IGraphicsUpdatable* updatable) = 0;

		/** @brief  */
		virtual bool addForIntervalUpate(IGraphicsUpdatable* updatable, scalar interval) = 0;

		/** @brief  */
		virtual bool removeFromIntervalUpdate(IGraphicsUpdatable* updatable, scalar intervalSec) = 0;

		/** @brief  */
		virtual void update(SSTATE state) = 0;
	};

	typedef Handle<IGraphicsUpdater> HGRAPHICSUPDATER;
	
}//namespace Blade


#endif //__Blade_IGraphicsUpdater_h__