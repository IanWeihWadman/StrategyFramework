#pragma once

#include "ActionModeManager.h"

namespace Strategy
{
	template<class Types>
	class PlayerState
	{
		ActionModeManager<Types> ModeManager;
		//public ITurnTimer TurnTimer{ get; set; }

		void SetMode( std::unique_ptr<ActionMode<Types>> mode )
		{
			ModeManager.SetMode( std::move( mode ) );
		}

		void StartTurn()
		{
			//TurnTimer.StartTurn();
		}

		void EndTurn()
		{
			//TurnTimer.EndTurn();
		}
	};
}