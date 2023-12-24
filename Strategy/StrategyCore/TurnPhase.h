#pragma once

#include "RuleState.h"
#include "PlayerState.h"
#include "GameContext.h"

#include <vector>

namespace Strategy
{
	template<class Types>
	class TurnPhaseResult
	{
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		TurnPhaseResult( bool success, bool updatePlayers, std::initializer_list<Action> actions )
		{
			Success = success;
			UpdatePlayers = updatePlayers;
			Actions = { actions };
		}

		bool Success;
		bool UpdatePlayers;
		std::vector<Action> Actions;
	};

	template<class Types>
	class TurnPhase
	{
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		TurnPhaseResult HandleInput( std::vector<Input> inputs, std::vector<PlayerState<Types>> playerContexts, RuleState state, int32_t player );

		TurnPhase<Types> GetNext( const RuleState& state );

		void OnBegin( std::vector<PlayerState> playerContexts, RuleState& state );
		void OnEnd( std::vector<PlayerState> playerContexts, RuleState& state );

		GameContext Context{ get; set; }
	};
}