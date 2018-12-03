/********************************************************************
	created:	2010/05/08
	filename: 	GameElement.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GameElement_h__
#define __Blade_GameElement_h__
#include <interface/public/IElement.h>
#include <BladeGame.h>
#include <ElementBase.h>

namespace Blade
{
	class BLADE_GAME_API GameElement : public ElementBase
	{
	public:
		static const TString GAME_SYSTEM_TYPE;

		GameElement(const TString& type);
		virtual ~GameElement();

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual size_t			initParallelStates() { return mParallelStates.size(); }

		/*
		@describe
		@param
		@return
		*/
		virtual void				onParallelStateChange(const IParaState&)	{}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		void				initialize(IScene* scene);
	};//class GameElement

	extern template class BLADE_GAME_API Factory<GameElement>;
	typedef Factory<GameElement> GameElementFactory;
	
}//namespace Blade


#endif //__Blade_GameElement_h__