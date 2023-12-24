#pragma once

#include "ActionModeManager.h"

namespace Strategy
{
	template<class Types, class ActionMode, class RuleState>
	class PlayerState
	{
	public:
		ActionMode GetMode() const
		{
			return ModeManager.GetMode();
		}

		void SetMode( ActionMode mode )
		{
			ModeManager.SetMode( std::move( mode ) );
		}

		void ResetMementos()
		{
			ModeManager.ResetMementos();
		}

		int32_t RevertToMemento()
		{
			return ModeManager.RevertToMemento();
		}

		auto ReceiveInput( const RuleState& state, const typename Types::Input& input )
		{
			return ModeManager.ReceiveInput( state, input );
		}

		void StartTurn()
		{
			//TurnTimer.StartTurn();
		}

		void EndTurn()
		{
			//TurnTimer.EndTurn();
		}

	private:
		ActionModeManager<Types, ActionMode, RuleState> ModeManager;
		//public ITurnTimer TurnTimer{ get; set; }

	};
}