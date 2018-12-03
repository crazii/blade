/********************************************************************
	created:	2015/03/12
	filename: 	GameFSM.h
	author:		Crazii
	
	purpose:	game finite state machine
*********************************************************************/
#ifndef __Blade_GameFSM_h__
#define __Blade_GameFSM_h__
#include <Factory.h>
#include <GameLibrary.h>

namespace Blade
{
	class BLADE_GAME_LIBRARY_API IGameState
	{
	public:
		//the initial state name of IGameState factory
		//users must register at least one subclass
		//and must register one sub class use this name
		static const TString INIT_STATE;
	public:
		virtual ~IGameState() {}

		/** @brief get the shown name of the state */
		virtual const TString& getName() const = 0;

		/** @brief  */
		virtual void	initialize() = 0;

		/** @brief  */
		virtual void	shutdown() = 0;

		/** @brief  */
		virtual void	run() = 0;

		/** @brief  */
		virtual void	update() = 0;

		/** @brief  */
		virtual void	syncUpdate() = 0;
	};//class IGameState

	extern template class BLADE_GAME_LIBRARY_API Factory<IGameState>;
	typedef Factory<IGameState> GameStateFactory;

	class BLADE_GAME_LIBRARY_API GameFSM : public Allocatable
	{
	public:
		GameFSM();
		~GameFSM();

		/** @brief  */
		void initialize(const TString& state);

		/** @brief  */
		void shutdown();

		/** @brief called on framework's run phase */
		void run();

		/** @brief called on framework's update phase */
		void update();

		/** @brief  */
		void syncUpdate();

		/** @brief  */
		void changeState(const TString& state);

		/** @brief  */
		inline IGameState* getCurrentState() const
		{
			return mCurState;
		}

	protected:
		IGameState*	mCurState;
	};//class GameFSM
	
}//namespace Blade


#endif // __Blade_GameFSM_h__
