/********************************************************************
	created:	2015/03/12
	filename: 	GameFSM.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <GameFSM.h>
#include <BladeFramework.h>

namespace Blade
{
	const TString IGameState::INIT_STATE = BTString("Initial Game State");

	template class Factory<IGameState>;

	//////////////////////////////////////////////////////////////////////////
	GameFSM::GameFSM()
		:mCurState(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GameFSM::~GameFSM()
	{
		assert(mCurState == NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	void GameFSM::initialize(const TString& state)
	{
		mCurState = BLADE_FACTORY_CREATE(IGameState, state);
		mCurState->initialize();
	}

	//////////////////////////////////////////////////////////////////////////
	void GameFSM::shutdown()
	{
		if( mCurState != NULL )
		{
			mCurState->shutdown();
			BLADE_DELETE mCurState;
			mCurState = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void GameFSM::run()
	{
		BLADE_TS_VERIFY( TS_ASYNC_RUN );

		if( mCurState != NULL )
			mCurState->run();
	}

	//////////////////////////////////////////////////////////////////////////
	void GameFSM::update()
	{
		BLADE_TS_VERIFY( TS_ASYNC_UPDATE );

		if( mCurState != NULL )
			mCurState->update();
	}

	//////////////////////////////////////////////////////////////////////////
	void GameFSM::syncUpdate()
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		//note: IGameState::syncUpdate may change state of FSM, and the object may no longer exist.
		if( mCurState != NULL )
			mCurState->syncUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	void GameFSM::changeState(const TString& state)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( mCurState != NULL && mCurState->getName() == state )
			return;

		IGameState* newState = BLADE_FACTORY_CREATE(IGameState, state);
		if( mCurState != NULL )
		{
			mCurState->shutdown();
			BLADE_DELETE mCurState;
		}
		mCurState = newState;
		mCurState->initialize();
	}
	
}//namespace Blade